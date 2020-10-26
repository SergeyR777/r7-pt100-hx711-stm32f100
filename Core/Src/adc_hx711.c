/*
 * adc_hx711.c
 *
 *  Created on: 23 окт. 2020 г.
 *      Author: SergeyR7
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "main.h"
#include "adc_hx711.h"

#define CLK_Port	HX711_CLK_GPIO_Port
#define CLK_Pin		HX711_CLK_Pin
#define DATA1_Port	HX711_DATA1_GPIO_Port
#define DATA1_Pin	HX711_DATA1_Pin
#define DATA2_Port	HX711_DATA2_GPIO_Port
#define DATA2_Pin	HX711_DATA2_Pin

typedef struct {
	float rt;
	float t;
	int value_chA;		// Канал A
	int value_chB;		// Канал B
	int value_chA_prev;	// Канал A - предыдущие значение
	int errno;			// Ошибка преобразования.
} HX711_Channel_t;

static HX711_Channel_t ch1;
static HX711_Channel_t ch2;

static int value_ch1;
static int value_ch2;

static int ADC_HX711_CheckAndCalculate(HX711_Channel_t *ch);
static void ADC_HX711_PrintInfo(HX711_Channel_t *ch);

/*
 * Инициализация АЦП
 */
void ADC_HX711_Init() {
	// запускаем АЦП
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &value_ch1, &value_ch2);

	// Запускаем ШИМ
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_OC_SetCompareCH1(TIM3, 0);
	LL_TIM_OC_SetCompareCH2(TIM3, 0);
	LL_TIM_EnableAllOutputs(TIM3);
	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2);

	// ЦАП
	LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
	LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_2);
	LL_DAC_ConvertDualData12RightAligned(DAC1, 0, 0);
}

/*
 * Процесс получения значений с АЦП и вычисление температуры
 * Пока простые циклы ожиданя без таймаутов
 */
void ADC_HX711_Process() {
	// получаем значение с канала B обоих АЦП
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &ch1.value_chB, &ch2.value_chB);
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_A_64x, &value_ch1, &value_ch2);
	ch1.value_chB += value_ch1;
	ch2.value_chB += value_ch2;

	// получаем значение с канала A обоих АЦП
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_A_64x, &ch1.value_chA, &ch2.value_chA);
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &value_ch1, &value_ch2);
	ch1.value_chA += value_ch1;
	ch2.value_chA += value_ch2;

	ADC_HX711_CheckAndCalculate(&ch1);
	ADC_HX711_CheckAndCalculate(&ch2);

	// выводим информацию по каналам
	printf("CH1  ");
	ADC_HX711_PrintInfo(&ch1);
	printf("      CH2  ");
	ADC_HX711_PrintInfo(&ch2);
	printf("\r\n");

	// Значение выходов в соответствии с таблицей.
	// Пока таблицы нет - просто линейный график от 0 до 500 градусов Цельсия

	float k = 8191.0f / 500.0f;

	int v1 = ch1.errno == HX711_OK ? (int)(ch1.t * k) : 0;
	if (v1 < 0) {
		v1 = 0;
	} else if (v1 > 8191) {
		v1 = 8191;
	}

	int v2 = ch2.errno == HX711_OK ? (int)(ch2.t * k) : 0;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 8191) {
		v2 = 8191;
	}

	// ШИМ (13 бит)
	LL_TIM_OC_SetCompareCH1(TIM3, (uint32_t)v1);
	LL_TIM_OC_SetCompareCH2(TIM3, (uint32_t)v2);

	// ЦАП (12 бит)
	LL_DAC_ConvertDualData12RightAligned(DAC1, ((uint32_t)v1) >> 1, ((uint32_t)v2) >> 1);
}

/*
 * Вычисление температуры
 */
static int ADC_HX711_CheckAndCalculate(HX711_Channel_t *ch) {
	// теперь B / A = сопротивление PT100 относительно резистора 100 Ом
	// Сейчас число 25 бит
	// B сдвигаем влево на 4 бита - лучше умножить на 100, чтобы точку выравнять до единиц Ома
	// A сдвигаем вправо на 10 бит
	// и делаем целочисленное деление

	// проверяем правильность подключения.
	// Если обрыв в цепи, значение опорного сопротивления будет близко к нулю.
	if (abs(ch->value_chA) < 100000) {
		// R0 - маленькое - обрыв датчика
		ch->errno = HX711_ERROR_OPEN;
	} else if (abs(ch->value_chB) < 100000) {
		// Rt - маленькое - замыкание датчика
		ch->errno = HX711_ERROR_SHORT;
	} else if (ch->value_chB > 0xE80000) // 24 бита со знаком * 2
			{
		ch->errno = HX711_ERROR_LIMIT;
	} else if ((ch->value_chA < 0) || (ch->value_chB < 0)) {
		// Полярность поменялась - замыкание на + или отказ АЦП
		ch->errno = HX711_ERROR_UNKNOWN;
	} else {
		ch->errno = HX711_OK;
	}

	if (ch->errno != HX711_OK)
		return ch->errno;

	//int intk = ((ch->value_chB >> 2) * 100) / (ch->value_chA >> 11); // число с фиксированной точкой 24.8 - в Омах

	float kr = (ch->value_chB >> 2) / (((ch->value_chA + ch->value_chA_prev) >> 12) * 256.0); // измеренное сопротивление термосопротивления (отношение Rt/R0)

	float r0 = 100.0; // опорное сопротивление
	ch->rt = r0 * kr;

	float k = 1.0 / kr;	// отношение R0/Rt

	float A = 3.9083e-3;
	float B = -5.775e-7;

	// коэф. квадратного уравнения
	float xa = B * k;
	float xb = A * k;
	float xc = k - 1.0;

	float D = xb * xb - 4.0 * xa * xc;
	float sqrtD = sqrtf(D);

	float t1 = (-xb + sqrtD) / (2.0 * xa);
	//float t2 = (-xb - sqrtD) / (2.0 * xa); Это не наше решение уравнения

	//printf("ADC Value A: %8d    B: %8d    k: %8d     Rt: %4.2f    t: %4.2f\r\n", ch->value_chA, ch->value_chB, intk, rt, t1);

	ch->t = t1;
	ch->value_chA_prev = ch->value_chA;

	return ch->errno;
}

/*
 * Проверяет готовность АЦП
 */
int ADC_HX711_IsReady() {
	return !(LL_GPIO_IsInputPinSet(DATA1_Port, DATA1_Pin)
			| LL_GPIO_IsInputPinSet(DATA2_Port, DATA2_Pin));
}

/*
 * Чтение значения из 2х АЦП. Запускает следующее измерение для указанного канала.
 * АЦП HX711 должны быть с общим CLK.
 * Возвращает два числа со знаком с обоих АЦП
 */
void ADC_HX711_ReadValueDouble(int next_ch, int *adc1, int *adc2) {
	uint32_t value1 = 0;
	uint32_t value2 = 0;

	// получаем знак
	LL_GPIO_SetOutputPin(CLK_Port, CLK_Pin);
	LL_GPIO_ResetOutputPin(CLK_Port, CLK_Pin);
	if (LL_GPIO_IsInputPinSet(DATA1_Port, DATA1_Pin))
		value1--; // получеам 0xFFFFFFFF
	if (LL_GPIO_IsInputPinSet(DATA2_Port, DATA2_Pin))
		value2--; // получеам 0xFFFFFFFF

	// получаем значение
	for (int i = 0; i < 23; ++i) {
		LL_GPIO_SetOutputPin(CLK_Port, CLK_Pin);
		value1 <<= 1;
		value2 <<= 1;
		LL_GPIO_ResetOutputPin(CLK_Port, CLK_Pin);
		if (LL_GPIO_IsInputPinSet(DATA1_Port, DATA1_Pin))
			value1++;
		if (LL_GPIO_IsInputPinSet(DATA2_Port, DATA2_Pin))
			value2++;
	}

	// выбор канала и коэф. усиления для следующего значения
	for (int i = 0; i < next_ch; ++i) {
		LL_GPIO_SetOutputPin(CLK_Port, CLK_Pin);
		LL_GPIO_ResetOutputPin(CLK_Port, CLK_Pin);
	}

	*adc1 = (int) value1;
	*adc2 = (int) value2;
}

static void ADC_HX711_PrintInfo(HX711_Channel_t *ch)
{
	switch (ch->errno) {
		case HX711_OK:
			printf("rt: %7.2f  t: %7.2f", ch->rt, ch->t);
			break;

		case HX711_ERROR_OPEN:
			printf("-------- OPEN ---------");
			break;

		case HX711_ERROR_SHORT:
			printf("-------- SHORT --------");
			break;

		case HX711_ERROR_LIMIT:
			printf("------ OVERHEAT -------");
			break;

		case HX711_ERROR_UNKNOWN:
			printf("------ ADC ERROR ------");
			break;

		default:
			printf("-------- ERROR --------");
			break;
	}

}

