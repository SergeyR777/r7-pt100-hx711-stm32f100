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

static int value_ch1;	// для прлучения значения с АЦП
static int value_ch1a;	// Канал A
static int value_ch1b;	// Канал B
static int value_ch1a_old;	// Канал A - предыдущие значение

static int value_ch2;
static int value_ch2a;
static int value_ch2b;
static int value_ch2a_old;

static int ADC_HX711_CheckAndCalculate(int va, int vb, int va_old);

/*
 * Инициализация АЦП
 */
void ADC_HX711_Init() {
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &value_ch1, &value_ch2);
}

/*
 * Процесс получения значений с АЦП и вычисление температуры
 */
void ADC_HX711_Process() {
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &value_ch1b, &value_ch2b);
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_A_64x, &value_ch1, &value_ch2);
	value_ch1b += value_ch1;
	value_ch2b += value_ch2;

	//LL_mDelay(2);

	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_A_64x, &value_ch1a, &value_ch2a);
	while (!ADC_HX711_IsReady()) {
	};
	ADC_HX711_ReadValueDouble(HX711_NEXT_B_32x, &value_ch1, &value_ch2);
	value_ch1a += value_ch1;
	value_ch2a += value_ch2;

	printf("CH1: ");
	ADC_HX711_CheckAndCalculate(value_ch1a, value_ch1b, value_ch1a_old);

	printf("CH2: ");
	ADC_HX711_CheckAndCalculate(value_ch2a, value_ch2b, value_ch2a_old);

	value_ch1a_old = value_ch1a;
	value_ch2a_old = value_ch2a;
}

/*
 * Вычисление температуры
 */
static int ADC_HX711_CheckAndCalculate(int va, int vb, int va_old) {
	// теперь B / A = сопротивление PT100 относительно резистора 100 Ом
	// Сейчас число 25 бит
	// B сдвигаем влево на 4 бита - лучше умножить на 100, чтобы точку выравнять до единиц Ома
	// A сдвигаем вправо на 10 бит
	// и делаем целочисленное деление

	// проверяем правильность подключения.
	// Если обрыв в цепи, значение опорного сопротивления будет близко к нулю.
	if (abs(va) < 100000) {
		// R0 - маленькое - обрыв датчика
		printf("Error - Sensor circuit open\r\n");
		return HX711_ERROR_OPEN;
	} else if (abs(vb) < 100000) {
		// Rt - маленькое - замыкание датчика
		printf("Error - Sensor circuit short\r\n");
		return HX711_ERROR_SHORT;
	} else if (vb > 0xE80000) // 24 бита со знаком * 2
			{
		printf("Error - Exceeding the value limit\r\n");
		return HX711_ERROR_LIMIT;
	} else if ((va < 0) || (vb < 0)) {
		// Полярность поменялась - замыкание на + или отказ АЦП
		printf("Error - Sensor value cannot be obtained\r\n");
		return HX711_ERROR_UNKNOWN;
	};

	int intk = ((vb >> 2) * 100) / (va >> 11); // число с фиксированной точкой 24.8 - в Омах

	float kr = (vb >> 2) / (((va + va_old) >> 12) * 256.0); // измеренное сопротивление термосопротивления (отношение Rt/R0)

	float r0 = 100.0; // опорное сопротивление
	float rt = r0 * kr;

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
	//float t2 = (-xb - sqrtD) / (2.0 * xa); Это не наше решение

	printf("ADC Value A: %d    B: %d    k: %d     Rt: %f    t: %f\r\n", va, vb, intk, rt, t1);

	return HX711_OK;
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

