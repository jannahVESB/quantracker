#ifndef QUANTRACKER_AIR_OSD_PROCESSORS_BOARD_TYPE4_HPP_INCLUDED
#define QUANTRACKER_AIR_OSD_PROCESSORS_BOARD_TYPE4_HPP_INCLUDED

/*
 Copyright (c) 2013 -2016 Andy Little 

 With Grateful Acknowledgements to the prior work of:
   Sami Korhonen(Openpilot.org)
   taulabs ( taulabs.com) 
   brainFPV ( brainfpv.com)
   Thomas Oldbury (super-osd.com)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#include <quan/stm32/freertos/freertos_usart_task.hpp>
#include <quan/stm32/freertos/freertos_usart_task_tx_only.hpp>
#include <quan/stm32/serial_port_lite.hpp>
#include <quan/voltage.hpp>
#include <quan/stm32/gpio.hpp>
#include <quan/stm32/spi.hpp>
#include <quan/stm32/tim.hpp>
#include <quan/stm32/usart.hpp>


#define SWDIO_DEBUG_DRAW_TASK 1
#define SWDIO_DEBUG_MAVLINK_TASK 2
#define SWDIO_DEBUG SWDIO_DEBUG_MAVLINK_TASK

//timers used
typedef quan::stm32::tim2                       video_columns_gate_timer;
// tim3 is also used as the internal video mode timer
typedef quan::stm32::tim3                       video_rows_line_counter;
// 
typedef quan::stm32::tim9                       spi_clock_timer;
//Think this could be changed if necessary
// maybe change this to 32 bit timer and leave running for microsecs
typedef quan::stm32::tim12                      sync_sep_timer;

#if !defined(QUAN_AERFLITE_BOARD)
// only used when writing video dac
typedef quan::stm32::tim10                      video_level_dac_irq_timer;
#endif

// not really part of system
typedef quan::stm32::tim6                       fsk_dac_timer;
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::stm32::tim7                       dac2_timer;
#endif

/*Timers potentially avail for expansion some multiplexed with other functions ( with i/o)
 N.B. Timer1 CH2 could go out on PA9 (currently Mavlink TX via digitally isolated port
 which might make a nice digitally isolated ESC pwm rc output
 PA10 (Mavlink Rx)  could be used for PPMSUM capture on TIM1 CH3 
TIM1_CH1 
TIM1_CH2-3( on 100 pin part)
TIM1_CH4
TIM4_CH1
TIM4_CH2
TIM4_CH3-4 ( on 100 pin part)
TIM4_CH2
TIM5_CH1
TIM5_CH2
TIM8_CH4
TIM10_CH1 
TIM11_CH1 

/expansion 100 pin io
 10 for timers
 6 for usarts
 3 for spi
 6 for i2c
 6 for ADC
 8 gp port
 2 can
-------------------
 total 43 + power + gnd
 abs min 42 -> 48/4 -> 4 * 13 ( 2 rows of 13 * 2) 52
*/
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::stm32::usart1                     mavlink_usart;
#endif
// usart2 avail for expansion on 100 pin part
// usart4 avail for expansion on all parts
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::stm32::usart3                     frsky_usart; // maybe inverted but not on f4
typedef quan::stm32::usart6                     av_telem_usart;
#endif
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::stm32::spi1                       imu_spi;
#else
// SPI1 is available for expansion
#endif

typedef quan::stm32::spi2                       video_mux_out_black_spi;
typedef quan::stm32::spi3                       video_mux_out_white_spi;
// I2C1 I2C2 and I2C3 avail for expansion
//----PORTA---------------------------------------
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,0>    usart4_txo_pin ;// USART4_TX
typedef quan::mcu::pin<quan::stm32::gpioa,1>    usart4_rxi_pin; // USART4_RX
#endif
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,2>    video_spi_clock;// TIM9_CH1
typedef quan::mcu::pin<quan::stm32::gpioa,3>    telem_cmp_enable_pin; // TIM2_CH4
#else
typedef quan::mcu::pin<quan::stm32::gpioa,2>    usart2_txo_rxi_pin;// USART2_TX
typedef quan::mcu::pin<quan::stm32::gpioa,3>    video_spi_clock;// TIM9_CH2
#endif

typedef quan::mcu::pin<quan::stm32::gpioa,4>    fsk_dac_out_pin; // NOT MOVABLE

#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,5>    dac2_out_pin; // NOT MOVABLE
#else
typedef quan::mcu::pin<quan::stm32::gpioa,5>    video_in_tim2_hsync_pin; // CSYNC TIM2_CH1
#endif

#if !(defined QUAN_DISCOVERY)
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,6>    av_dac_nsync; // software no af
typedef quan::mcu::pin<quan::stm32::gpioa,7>    av_dac_data; // softawre no af
#else
typedef quan::mcu::pin<quan::stm32::gpioa,6>    video_in_tim3_hsync_pin ;  // TIM3_CH1 csync and internal out 
typedef quan::mcu::pin<quan::stm32::gpioa,7>    ppmsum_in_pin; // TIM14_CH1
#endif
#endif

#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,8>    i2c3_scl ;
#endif
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,9>    usart1_txo_pin; // USART1_TX
typedef quan::mcu::pin<quan::stm32::gpioa,10>   usart1_rxi_pin; // USART1_RX
#else
typedef quan::mcu::pin<quan::stm32::gpioa,9>    mavlink_txo_pin; // USART1_TX
typedef quan::mcu::pin<quan::stm32::gpioa,10>   mavlink_rxi_pin; // USART1_RX
#endif
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,11>   can1_rx;
typedef quan::mcu::pin<quan::stm32::gpioa,12>   can1_tx;
#endif

typedef quan::mcu::pin<quan::stm32::gpioa,13>   swdio;
typedef quan::mcu::pin<quan::stm32::gpioa,14>   swdclk;
#if defined(QUAN_AERFLITE_BOARD)
// The JTAG debug pins are also LED outputs, but connected to mosfet gates so
// can be overridden by JTAG functionality
typedef quan::mcu::pin<quan::stm32::gpioa,13>   notify_led1;
typedef quan::mcu::pin<quan::stm32::gpioa,14>   notify_led2;
#endif

#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioa,15>   video_in_tim2_hsync_pin ; // TIM2_CH1 ( also TIM2_ETR)
#else
typedef quan::mcu::pin<quan::stm32::gpioa,15>   imu_nss_pin; // SPI1_NSS hardware nss pin
#endif
//----PORTB---------------------------------------------
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpiob,0>    video_adc_pin ; // ADC12_IN8
typedef quan::mcu::pin<quan::stm32::gpiob,1>    vin_voltage_pin ;//adc1 ADC12_IN9 
typedef quan::mcu::pin<quan::stm32::gpiob,2>    boot1_pin;
#else
typedef quan::mcu::pin<quan::stm32::gpiob,0>    rcout5_pin; // TIM1_CH2N
typedef quan::mcu::pin<quan::stm32::gpiob,1>    rcout6_pin;  // TIM1_CH3N
typedef quan::mcu::pin<quan::stm32::gpiob,2>    video_isync_pin;// BOOT0 high for internal video
#endif

#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpiob,3>    spi1_sck; // imu
typedef quan::mcu::pin<quan::stm32::gpiob,4>    spi1_miso; // imu
typedef quan::mcu::pin<quan::stm32::gpiob,5>    spi1_mosi; // imu

typedef quan::mcu::pin<quan::stm32::gpiob,6>    rcout1_pin; 
typedef quan::mcu::pin<quan::stm32::gpiob,7>    rcout2_pin; 
typedef quan::mcu::pin<quan::stm32::gpiob,8>    rcout3_pin; 
typedef quan::mcu::pin<quan::stm32::gpiob,9>    rcout4_pin; 
#endif

#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpiob,10>   usart3_txo_pin; // USART3_TX
typedef quan::mcu::pin<quan::stm32::gpiob,11>   usart3_rxi_pin; // USART3_RX
#else
typedef quan::mcu::pin<quan::stm32::gpiob,10>   frsky_txo_pin; // USART3_TX
typedef quan::mcu::pin<quan::stm32::gpiob,11>   frsky_rxi_pin; // USART3_RX
#endif
#if !(defined QUAN_DISCOVERY)
typedef quan::mcu::pin<quan::stm32::gpiob,12>   heartbeat_led_pin;
#endif

typedef quan::mcu::pin<quan::stm32::gpiob,13>   video_mux_out_black_sck; // SPI2_SCK AF5 input from pixelclk
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpiob,14>   video_in_hsync_first_edge_pin; // TIM12_CH1
#else
typedef quan::mcu::pin<quan::stm32::gpiob,14>   video_mux_out_black_miso; // actually MASK 
#endif
typedef quan::mcu::pin<quan::stm32::gpiob,15>   video_in_hsync_second_edge_pin; // TIM12_CH2
//-----PORTC---------------------------------------
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,0>    video_adc_pin; //ADC123_IN10
typedef quan::mcu::pin<quan::stm32::gpioc,1>    analog_in; //ADC123_IN11 undedicated anlog input
#endif
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,2>    video_mux_out_black_miso; // SPI2_MISO AF5
#else
typedef quan::mcu::pin<quan::stm32::gpioc,2>    analog_rssi_in; 
typedef quan::mcu::pin<quan::stm32::gpioc,3>    analog_batt_current_in; //ADC123_IN13
typedef quan::mcu::pin<quan::stm32::gpioc,4>    analog_batt_voltage_in; //ADC123_IN14
typedef quan::mcu::pin<quan::stm32::gpioc,5>    analog_airspeed_in; //ADC123_IN15
#endif

#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,6>    usart6_txo_pin; // USART6_TX
typedef quan::mcu::pin<quan::stm32::gpioc,7>    usart6_rxi_pin; // USART6_RX
#else
typedef quan::mcu::pin<quan::stm32::gpioc,6>    av_telem_tx; // USART6_TX
typedef quan::mcu::pin<quan::stm32::gpioc,7>    av_telem_rx; // USART6_RX
#endif
#if !(defined QUAN_DISCOVERY)
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,8>    av_dac_clk; // software no af
#else
typedef quan::mcu::pin<quan::stm32::gpioc,8>    sdio_d0_pin; //SDIO_D0
#endif
#endif
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,9>    i2c3_sda ;
#endif
typedef quan::mcu::pin<quan::stm32::gpioc,10>   video_mux_out_white_sck; // SPI3_SCK input
typedef quan::mcu::pin<quan::stm32::gpioc,11>   video_mux_out_white_miso; // SPI3_MISO
#if defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpioc,12>   sdio_sck_pin ; //SDIO_CK
#endif
#if !defined(QUAN_AERFLITE_BOARD)
//############Limited output current ###########################
typedef quan::mcu::pin<quan::stm32::gpioc,13>   frsky_txo_sign_pin;
#else
typedef quan::mcu::pin<quan::stm32::gpioc,13>   mpu_drdy_pin;
typedef quan::mcu::pin<quan::stm32::gpioc,14>   usart2_direction_pin;
typedef quan::mcu::pin<quan::stm32::gpioc,15>   sd_card_detect_pin;
#endif

//############Limited output current ############################
// NA on 64 pin part except PD2 PH0 PH1
//---------PORTD---------------------------------------------------------------------
/*
PD0 GP no useful AF no ADC

*/
#if defined QUAN_DISCOVERY
typedef quan::mcu::pin<quan::stm32::gpiod,1>     av_dac_nsync; // software no af
#endif
#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::mcu::pin<quan::stm32::gpiod,2>     video_in_tim3_hsync_pin; // TIM3_ETR
#else
typedef quan::mcu::pin<quan::stm32::gpiod,2>     sdio_cmd_pin;
#endif
#if defined QUAN_DISCOVERY
typedef quan::mcu::pin<quan::stm32::gpiod,3>     av_dac_data; // software no af
#endif
/*
PD4 GP No useful AF no ADC (NC)
*/
//typedef quan::mcu::pin<quan::stm32::gpiod,5>     usart2_tx;
//typedef quan::mcu::pin<quan::stm32::gpiod,6>     usart2_rx;
/*
PD7 GP no AF no ADC (NC)
*/
#if defined QUAN_DISCOVERY
typedef quan::mcu::pin<quan::stm32::gpiod,7>     av_dac_clk; // software no af
#endif
//typedef quan::mcu::pin<quan::stm32::gpiod,8>    usart3_tx;
//typedef quan::mcu::pin<quan::stm32::gpiod,9>    usart3_rx;
/*
PD10 GP no AF no ADC (NC)
PD11 GP no AF no ADC (NC)
*/
//typedef quan::mcu::pin<quan::stm32::gpiod,12>   tim4_ch1;   // No other AF
//typedef quan::mcu::pin<quan::stm32::gpiod,13>   tim4_ch2;   // No other AF   
//typedef quan::mcu::pin<quan::stm32::gpiod,14>   tim4_ch3;  // No other AF
#if defined QUAN_DISCOVERY
typedef quan::mcu::pin<quan::stm32::gpiod,12>   green_led_pin;   // green led on Discovery
typedef quan::mcu::pin<quan::stm32::gpiod,13>   orange_led_pin;       // orange led on Discovery
typedef quan::mcu::pin<quan::stm32::gpiod,14>   heartbeat_led_pin;
typedef quan::mcu::pin<quan::stm32::gpiod,15>   blue_led_pin;       // blue led on Discovery
#endif

//typedef quan::mcu::pin<quan::stm32::gpiod,15>   tim4_ch4;    // No other AF 

//----------PORTE--------------------------
/* 8 bit software port 
PE0 TIM4_ETR NO ADC
PE1 GP NO AF NO ADC
PE2 GP NO AF NO ADC
PE3 GP NO AF NO ADC
PE4 GP NO AF NO ADC
//#####################################
// could multiplex sync_sep timer with spi clock via these 2 pins?
/ but only on 100 pin part
PE5 TIM9_CH1 NO ADC
PE6 TIM9_CH2 NO ADC
//################################
PE7 TIM1_ETR
PE8 GP NO ADC (NC)
*/
//typedef quan::mcu::pin<quan::stm32::gpioe,9> tim1_ch1;
/*
PE10 NO ADC (NC)
*/
//typedef quan::mcu::pin<quan::stm32::gpioe,11> tim1_ch2;
/*
PE12 NO ADC (NC)
*/
//typedef quan::mcu::pin<quan::stm32::gpioe,13> tim1_ch3;
//typedef quan::mcu::pin<quan::stm32::gpioe,14> tim1_ch4;
/*
PE15 NO ADC (NC)
*/
//----------------PORTH---------------
/*
PH0 OSC_IN
PH1 OSC_OUT
*/

#if !defined(QUAN_AERFLITE_BOARD)
typedef quan::stm32::freertos::usart_tx_rx_task<
   mavlink_usart,
   100,100, 
   mavlink_txo_pin,mavlink_rxi_pin,
   uint8_t
> mavlink_tx_rx_task;

typedef quan::stm32::serial_port_lite<
      mavlink_usart,
      mavlink_txo_pin,
      mavlink_rxi_pin
> flash_menu_sp;

typedef quan::stm32::freertos::usart_tx_rx_task<
   frsky_usart,
   10,2, 
   frsky_txo_pin,frsky_rxi_pin,
   char
> frsky_tx_rx_task;
#endif
// 0- 15 lower numerical is higher logical priority
 // NB anything above certain level is not masked
  // what is that level?
struct interrupt_priority {
     static constexpr uint32_t video_level = 15;
     static constexpr uint32_t frsky_serial_port= 14;
     static constexpr uint32_t telemetry_input_port = 13;
     static constexpr uint32_t fsk_dac_timer = 12;
     static constexpr uint32_t video = 11;
     static constexpr uint32_t low_priority_video = 15;
};

struct task_priority{
#if defined QUAN_OSD_TELEM_TRANSMITTER

   static constexpr uint32_t vsync_telem_tx =( tskIDLE_PRIORITY + 3UL ) ;
#endif
#if defined QUAN_OSD_TELEM_RECEIVER
   static constexpr uint32_t vsync_telem_rx =( tskIDLE_PRIORITY + 3UL ) ;
#endif
    static constexpr uint32_t draw =( tskIDLE_PRIORITY + 2UL ) ;

// these prob dont belong here should prob be in user namespace
 // somehow to get priority
   static constexpr uint32_t fsk = ( tskIDLE_PRIORITY + 1UL );
   static constexpr uint32_t mavlink = ( tskIDLE_PRIORITY + 4UL );
   static constexpr uint32_t frsky = ( tskIDLE_PRIORITY + 3UL );

   static constexpr uint32_t heartbeat = ( tskIDLE_PRIORITY + 1UL );

};

/*
 DMA used 
   DMA1.Stream4.Channel0  == SPI2 TX
   DMA1.Stream5.Channel0  == SPI3 TX
   
   DMA2.Stream1.Ch5  ==   only if TelemeReceiver
*/

#endif // QUANTRACKER_AIR_OSD_PROCESSORS_BOARD_TYPE4_HPP_INCLUDED
