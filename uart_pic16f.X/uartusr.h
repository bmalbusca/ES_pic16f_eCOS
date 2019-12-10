#ifndef UARTUSR_H
#define UARTUSR_H



void write_UART( uint8_t rxData);
void write_str_UART(char * string , uint8_t size);
uint8_t read_str_UART(char * buff, uint8_t max_len);
void reply_UART_ERROR(char cmd);
void reply_UART_OK(char cmd);
uint8_t echo(char * string, uint8_t string_size);
uint8_t * char_to_hex(char * message, uint8_t message_size, uint8_t * hex , uint8_t hex_size);
void dump_UART_FIFO();
uint8_t read_UART();
void parse_message(char * message, uint8_t max_size);

#endif
