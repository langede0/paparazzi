/*
 * $Id$
 *  
 * Copyright (C) 2010 The Paparazzi Team
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#include "booz_radio_control.h"

#include <stm32/rcc.h>
#include <stm32/gpio.h>
#include <stm32/tim.h>
#include <stm32/misc.h>

#include "sys_time.h"

#include "my_debug_servo.h"


/*
 *
 * This a radio control ppm driver for stm32
 * signal on PA1 TIM2/CH2 (uart1 trig on lisa/L)
 *
 */
uint8_t  booz_radio_control_ppm_cur_pulse;
uint32_t booz_radio_control_ppm_last_pulse_time;

uint32_t debug_len;

void tim2_irq_handler(void);

void booz_radio_control_ppm_arch_init ( void ) {

  /* TIM2 channel 2 pin (PA.01) configuration */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* GPIOA clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

 /* TIM2 configuration: Input Capture mode ---------------------
     The external signal is connected to TIM2 CH2 pin (PA.01)  
     The Rising edge is used as active edge,
  ------------------------------------------------------------ */
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);


  /* Enable the TIM2 global Interrupt */
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
 
  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);

  booz_radio_control_ppm_last_pulse_time = 0;
  booz_radio_control_ppm_cur_pulse = RADIO_CONTROL_NB_CHANNEL;

  DEBUG_SERVO2_INIT();

}


void tim2_irq_handler(void) {

  DEBUG_S4_ON();

  if(TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
    
    uint32_t now = TIM_GetCapture2(TIM2);
    uint32_t length = now - booz_radio_control_ppm_last_pulse_time;
    debug_len = length;
    booz_radio_control_ppm_last_pulse_time = now;			
    									
    if (booz_radio_control_ppm_cur_pulse == RADIO_CONTROL_NB_CHANNEL) {	
      if (length > SYS_TICS_OF_USEC(PPM_SYNC_MIN_LEN) &&		
	  length < SYS_TICS_OF_USEC(PPM_SYNC_MAX_LEN)) {		
	booz_radio_control_ppm_cur_pulse = 0;				
      }									
    }									
    else {								
      if (length > SYS_TICS_OF_USEC(PPM_DATA_MIN_LEN) &&		
	  length < SYS_TICS_OF_USEC(PPM_DATA_MAX_LEN)) {		
	booz_radio_control_ppm_pulses[booz_radio_control_ppm_cur_pulse] = length;
	booz_radio_control_ppm_cur_pulse++;				
	if (booz_radio_control_ppm_cur_pulse == RADIO_CONTROL_NB_CHANNEL) { 
	  booz_radio_control_ppm_frame_available = TRUE;		
	}								
      }									
      else								
	booz_radio_control_ppm_cur_pulse = RADIO_CONTROL_NB_CHANNEL;	
    }									
  }

  DEBUG_S4_OFF();

}