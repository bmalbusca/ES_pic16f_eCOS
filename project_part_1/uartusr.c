

#include <xc.h>
#include "mcc_generated_files/mcc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "uartusr.h"

/*
void write_UART( uint8_t rxData, uint8_t led);
void write_str_UART(char * string , uint8_t size);
char * read_str_UART(char * buff, uint8_t max_len);
void reply_UART_ERROR(unsigned char cmd);
void reply_UART_OK(unsigned char cmd);
uint8_t echo(char * string, uint8_t string_size);
uint8_t * char_to_hex(char * message, uint8_t message_size, uint8_t * hex , uint8_t hex_size);
*/

void reply_UART_OK(char cmd){
    //printf("%02x%02x%02x%02x\n", SOM,cmd,CMD_OK,EOM);
    //printf("%c%c%c%c\n",(char)SOM,(char)cmd,(char)CMD_OK,(char)EOM);
    
    write_UART((char)SOM);
    write_UART((char)cmd);
    write_UART((char)CMD_OK);
    write_UART((char)EOM);
}


void reply_UART_ERROR(char cmd){
    //printf("%02x%02x%02x%02x\n",SOM,cmd,CMD_ERROR,EOM);
     printf("%c%c%c%c",(char)SOM,cmd,(char)CMD_ERROR,(char)EOM);
    
    write_UART((char)SOM);
    write_UART((char)cmd);
    write_UART((char)CMD_ERROR);
    write_UART((char)EOM);

    
}


    


uint8_t read_str_UART(char * buff, uint8_t max_len){
     
    volatile uint8_t rxData = '0';
    uint8_t i=0;
    memset(buff, 0, max_len); 
    
    rxData = read_UART();
    if( rxData == (char)SOM ){
        for(i=0; i < max_len && (rxData !='\n'&& rxData != (char)EOM ); i++){
                buff[i] = rxData;
                rxData = read_UART();
                
           }
           
        buff[i+1] = '\0'; 
        
    }
    else{
     reply_UART_ERROR((char)CMD_ERROR);   
    }
    dump_UART_FIFO();
    

    return  i; 
    
}

uint8_t read_UART(){
    
    while(!EUSART_is_rx_ready()){
               __delay_ms(1);
            };
    return EUSART_Read();
    
    
}





void dump_UART_FIFO(){
    uint8_t dump;
    if(EUSART_is_rx_ready() && (dump != SOM || dump != EOM )) dump = EUSART_Read();
    
}


void write_str_UART(char * string , uint8_t size){
    uint8_t  id;
   
    for(id=0; id <= size && (string[id]!= '\0' || string[id]!=(char)EOM); ++id){
        
        write_UART(string[id]);
    }
    
   return;   
}


void write_UART( uint8_t rxData){
   
   while(!EUSART_is_tx_done());

   if(EUSART_is_tx_ready())
    {
        
        EUSART_Write(rxData);
    }
    
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
            EUSART_Write(rxData);
            
        }
         
    }
    
    string[i+1] = '\0'; 
    write_str_UART(string, string_size);
    return i;            
    
}
