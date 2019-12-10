#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "threads.h"
#include "mem.h"

#include "monitor.h" // user interface

extern struct command_d const commands[];
extern int commands_size;

/*
    USART
*/

extern cyg_io_handle_t serial_h;
extern cyg_serial_info_t serial_i; // struct with configs for usart

extern Cyg_ErrNo err;

int commands_in_argc[TRGI];


void rx_F(cyg_addrword_t data){
    char char_recieved;
    unsigned int num_bytes = 1;
    char cmd_name;
    int i = 0;
    char cmd_name_flag = 1;
    char num_args = 0;
    int recieved_args = 0;
    char expected_args = 0;
    char buff[10];
    char buff_size = 0;
    int buff_index = 0;

    /*
    char buffer[30] = {(char)  0, (char) 30, (char) 27, (char) 30, (char)  1,
                       (char)  0, (char) 32, (char) 10, (char) 20, (char)  2,
                       (char)  1, (char) 21, (char)  9, (char) 60, (char)  0,
                       (char)  5, (char) 19, (char) 59, (char) 50, (char)  0,
                       (char) 13, (char) 59, (char) 59, (char) 10, (char)  1,
                       (char) 23, (char) 59, (char) 59, (char) 20, (char)  1}; // for testing
    */

    //pushMem(buffer, 30);

    while(1){
        char_recieved = 0;
        cmd_name_flag = 0;
        num_args = 0;
        recieved_args = 0;
        buff_size = 0;
        buff_index = 0;
        expected_args = 0;

        cyg_io_read(serial_h, &char_recieved, &num_bytes);        // 7 = nº bytes a serem lidos

        if(char_recieved != SOM){
            continue;
        }

        //Receber o nome do comando
        cyg_io_read(serial_h, &cmd_name, &num_bytes);
        
        //Verificar se o comando é válido
        for(i = 0; i < commands_size; i++){
            if(cmd_name == commands[i].cmd_code){
                cmd_name_flag = 1;
                break;
            }
        }
        if(!cmd_name_flag){         //Se o comando não for válido
            continue;
        }
        //Escrever o command name no buffer para enviar
        buff[buff_index] = cmd_name;
        buff_index++;

        //Ver o numero de argumentos q é suposto receber
        expected_args = commands_in_argc[cmd_name];
         buff_index++;       

        //Receber os argumentos
        for(cyg_io_read(serial_h, &char_recieved, &num_bytes); char_recieved != EOF; cyg_io_read(serial_h, &char_recieved, &num_bytes)){
            if(buff_index < buff_size){
                recieved_args++;
                buff[buff_index] = char_recieved;
                buff_index++;
            }else{
                break;
            }
        }

        if(recieved_args == 1){
            if(recieved_args == CMD_OK){
                continue;
            }else if(recieved_args == CMD_ERROR){
                //FALTA eviar msg ao TX para enviar de novo a cena
            }
        }

        //Se a mensagem recebida não fizer sentido
        if(recieved_args != expected_args){
            continue;
        }

        

        //Enviar a mensagem recebida
        //writeCommand(cmd_name, expected_args);

        printf("%d ", (int)cmd_name);
        for(i = 2; i < recieved_args + 2; i++){
            printf("%d ", (int)buff[i]);
        }
        printf("\n");

        /*switch (getName(cmd_in)){
            case RX_TRANSFERENCE:
                cmd_out = writeCommand(RX_TRANSFERENCE, 0);
                cyg_mbox_tryput(proc.mbox_h, (void*) cmd_out);
            break;
        }*/
        

        //printf("%s\n", buffer_rx);
        //sprintf(stdout_buff, "%s\n", buffer_rx);
        //queueStdout(stdout_buff);

        __DELAY();
    }
}


void tx_F(cyg_addrword_t data){
    char buffer_tx[50];
    char *cmd_out, *cmd_in;
    int i;
    int num_args = 0;
    int num_bytes = 5;

    while(1) {
        //cmd_in = (char*)cyg_mbox_get(tx.mbox_h);            //Bloquear enquanto não houver cenas para enviar para o PIC

        buffer_tx[0] = ':';
        /*
        AskRead(&rs_rwf);           //Pedir para ler o ring buffer com inter threads

        buffer_tx[1] = getName(cmd_in);
        num_args = getArgc(cmd_in);

        for(i = 2; i < num_args + 2; i++){
            buffer_tx[i] = getArg(cmd_in, i-1);
        }
        FreeRead(&rs_rwf);          //Largar as keys para ler o ring buffer com inter threads
        */
        buffer_tx[1] = '10';
        buffer_tx[2] = '69';
        buffer_tx[3] = 'l';
        buffer_tx[4] = '?';
        err = cyg_io_write(serial_h, buffer_tx, &num_bytes);

        //sprintf(stdout_buff, "Sent, err =%x\n", err);
        //queueStdout(stdout_buff);

        cyg_thread_delay(40);

        /*buffer_tx[num_args + 2] = '?';
        num_bytes = num_args + 3;
        cyg_io_write(serial_h, buffer_tx, &num_bytes)*/
    }
}
