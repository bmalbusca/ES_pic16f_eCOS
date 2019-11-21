/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.77
        Device            :  PIC16F18875
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */

#include <stdio.h>
#include "mcc_generated_files/mcc.h"
#include <xc.h>

/*
                         Main application
 */
#define UART_BUFF_SIZE 14

uint8_t read_UART ();
void write_UART( uint8_t rxData);
void write_str_UART(char * string , unsigned int size);
uint8_t read_str_UART(char * buff, unsigned int max_len);
uint8_t  simple_rx_uart();
unsigned char rx(char * string);


void main(void)
{
    int read_num = 0;
    // initialize the device
    SYSTEM_Initialize();
    //char str_send[UART_BUFF_SIZE] = "hello mike\n\r";
    char str_rc[UART_BUFF_SIZE]="\0";
    
 
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    volatile uint8_t rxData;
    
    //write_str_UART("\n", 2);
    IO_RA7_SetLow();
    //write_str_UART(str_send, UART_BUFF_SIZE);

    while (1)
    {
       
        
        rx(str_rc);
     /*   
        
       if(EUSART_is_rx_ready())
            {
                rxData = EUSART_Read();
                if(EUSART_is_tx_ready())
                {   
                    if (rxData == '\0' || rxData == '\n'|| rxData == '\r'){
                          EUSART_Write('|');
                          EUSART_Write(read_num +'0');
                          read_num =0;
                    }
                    else{
                          EUSART_Write('-');
                          read_num++;
                     }
                    
                    
                    EUSART_Write(rxData);
                }
            }
        
    */  
        

    }
}


uint8_t read_str_UART(char * buff, unsigned int max_len){
    unsigned int size=0;
    char c;  
    
    for (size = 0; size < max_len; ++size){  
//        c = rx();
        
        if (c == '\0' || c == '\n'|| c == '\r'){
            break;
        }
        
    }
    
    buff[size+1] = '\0';
    return size+1;
    
}

uint8_t read_UART (){
    
    volatile uint8_t rxData;
    volatile eusart_status_t rxStatus;
     
    if(EUSART_is_rx_ready())
            {
        if(eusartRxCount!=0){
                rxData = EUSART_Read();
                rxStatus = EUSART_get_last_status();
                if(rxStatus.ferr){
                  IO_RA7_SetHigh();
                }

        }
     }
  
    
    
     __delay_ms(2);
    IO_RA7_SetLow();
    return rxData;
    
}
unsigned char rx(char * string)
{
	volatile uint8_t rxData = '0';
    int i ;
    for(i=0; i < 10 && rxData !='\n'; i++){
        
        while(!EUSART_is_rx_ready()){
            __delay_ms(1);
        };
            
        rxData = EUSART_Read();
        if(EUSART_is_tx_ready())

        {   
            if (rxData == '\0' ){
                  EUSART_Write('|');
            }else if(rxData == '\n'){
                    EUSART_Write(':');
            }
            else if(rxData == '\r'){
                    EUSART_Write('*');
            }
            else{
                  EUSART_Write('-');

             }

            string[i] = rxData;
            string[i+1] = '\0';
            EUSART_Write(rxData);
        }
                
                
    }
                write_str_UART(string, 11);
    return i;            
    
}


uint8_t  simple_rx_uart(){
    
    uint8_t  rxData; 
     IO_RA7_SetHigh();
    while(!(EUSART_get_last_status().ferr));
    
    rxData = EUSART_Read();   
    
    
    IO_RA7_SetLow();
    return rxData;
    
}

void write_str_UART(char * string , unsigned int size){
    unsigned int  id;
   
    for(id=0; id <= size && string[id]!= '\0'; ++id){
        
        write_UART(string[id]);
    }
    
   return;  
  
}

void write_UART( uint8_t rxData){
    
   if(EUSART_is_tx_ready())
            {
				IO_RA7_SetHigh();
                EUSART_Write(rxData);
            }
			if(EUSART_is_tx_done())
            {
                IO_RA7_SetLow();
            }  
    
}
/**
 End of File
*/
