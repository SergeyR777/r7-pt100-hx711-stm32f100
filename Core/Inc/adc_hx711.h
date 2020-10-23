/*
 * adc_hx711.h
 *
 *  Created on: 23 окт. 2020 г.
 *      Author: SergeyR7
 */

#ifndef INC_ADC_HX711_H_
#define INC_ADC_HX711_H_


#define HX711_NEXT_A_128x	(25 - 24)
#define HX711_NEXT_A_64x	(27 - 24)
#define HX711_NEXT_B_32x	(26 - 24)

#define HX711_OK			0
#define HX711_ERROR_OPEN	1
#define HX711_ERROR_SHORT	2
#define HX711_ERROR_LIMIT	3
#define HX711_ERROR_UNKNOWN	4

void ADC_HX711_Init();
void ADC_HX711_Process();
int ADC_HX711_IsReady();
void ADC_HX711_ReadValueDouble(int next_ch, int *adc1, int *adc2);


#endif /* INC_ADC_HX711_H_ */
