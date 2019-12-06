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
#include "defines.h"

/*
                         Main application
 */
#define UART_BUFF_SIZE 14
#define UART_STR_RLY 6


void write_UART( uint8_t rxData, uint8_t led);
void write_str_UART(char * string , uint8_t size);
char * read_str_UART(char * buff, uint8_t max_len);
void reply_UART_ERROR(unsigned char cmd);
void reply_UART_OK(unsigned char cmd);
void parse_message(unsigned char cmd);
uint8_t echo(char * string, uint8_t string_size);





void main(void)
{

    SYSTEM_Initialize();
    char str_send[UART_BUFF_SIZE] = "hello mike\n\r";
    char str_rc[UART_BUFF_SIZE]="\0";

 
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    
    
    IO_RA7_SetLow();
    write_str_UART(str_send, UART_BUFF_SIZE);

    while (1)
    {
            

        if(PIR3bits.RCIF || EUSART_is_rx_ready()){
                
             read_str_UART(str_rc, UART_BUFF_SIZE);
                
            
           
        }
        else{
            
            __delay_ms(1000);

        }
        

    }
}


void reply_UART_OK(unsigned char cmd){
    printf("%02x%02x%02x%02x\n",SOM,cmd,CMD_OK,EOM);
}

void reply_UART_ERROR(unsigned char cmd){
   printf("%02x%02x%02x%02x\n",SOM,cmd,CMD_ERROR,EOM);
}



void parse_message(unsigned char cmd){


    switch(cmd){
            case SOM:
                printf("Begin ");
                break;
            case  RCLK:
                break;
            case  SCLK:
                break;
            case  RTL:
                break;
            case  RPAR:
                break;
            case  MMP:
                break; 
            case MTA: 
                break;
            case RALA:
                break;
            case DATL:
                break;
            case AALA:
                break;
            case IREG:
                break;
            case TRGC:
                break; 
            case TRGI:
                break;
            case NMFL:
                break;

    }
    
    printf("%d\n",cmd);
   


    

}


char * read_str_UART(char * buff, uint8_t max_len){
     
    volatile uint8_t rxData = '0';
    uint8_t i=0;

    for(i=0; i < max_len && rxData !='\n'; i++){
            
            while(!EUSART_is_rx_ready()){
                __delay_ms(1);
            };
                
            rxData = EUSART_Read();
            parse_message(rxData);
            buff[i] = rxData;
            buff[i+1] = '\0';
                
            }
    write_str_UART(buff, max_len);
return  buff; 
    
}


uint8_t echo(char * string, uint8_t string_size)
{
	volatile uint8_t rxData = '0';
    uint8_t i ;
    for(i=0; i < string_size && rxData !='\n'; i++){
        
        while(!EUSART_is_rx_ready()){
            __delay_ms(1);
        };
            
        rxData = EUSART_Read();
        if(EUSART_is_tx_ready())

        {   
            if (rxData == '\0' ){       // Only for debug
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
            //EUSART_Write(rxData);
        }
                
                
    }
    write_str_UART(string, string_size);
    return i;            
    
}




void write_str_UART(char * string , uint8_t size){
    uint8_t  id;
   
    for(id=0; id <= size && string[id]!= '\0'; ++id){
        
        write_UART(string[id], 0);
    }
    
   return;  
  
}

void write_UART( uint8_t rxData, uint8_t led){
    

   if(EUSART_is_tx_ready())
    {
		if (led){
            IO_RA7_SetHigh();
        }
        EUSART_Write(rxData);
    }
	
    if(led){
        while(!EUSART_is_tx_done());
        IO_RA7_SetLow();     
    }
}
/**
 End of File
*/
