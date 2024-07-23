#include "stm8l15x.h"

// Bootloader key configuration
#define BOOTLOADER_KEY_START_ADDRESS                            (uint16_t)0x11A0
#define BOOTLOADER_KEY_PAGE_NUMBER                              0
#define BOOTLOADER_KEY_VALUE                                    0xAAAA5555
// Flash configuration
#define MAIN_PROGRAM_START_ADDRESS                              (uint16_t)0x8000
#define MAIN_PROGRAM_JUMP_ADDRESS                               (uint16_t)0xFA00
#define MAIN_PROGRAM_JUMP_ADDRESS_ASM                           "$FA00"
#define MAIN_PROGRAM_PAGE_NUMBER                                20
#define NUM_OF_PAGES                                            256   //pages
#define BLOCK_BYTES                                             128 //bytes per page
#define CPU_NAME                                                "STM8L151C8"

//ACK, NACK, SYNCHR bytes
#define ACK             0x79				/* 	hex Value send for acknowledge */
#define NACK            0x1F				/*  hex Value send for non-acknowledge */
#define IDENT           0x7F				/*  hex Value rfequired for master identification */

//indexes used for received data buffer
#define N_COMMAND               0
#define NEG_N_COMMAND           1
#define N_PAGE                  2 //номер страницы для записи
#define SIZE_MES                3 //колчисетво байт далее
#define FIRST_BYTE_IN_MES       4 //Первый байт данных в посылке

//commands numbers
#define GT_Command              0x00    /* Get command */
#define RM_Command              0x11    /* Read Memory command */
#define GO_Command              0x21    /* Go command */
#define WM_Command              0x31    /* Write Memory command */
#define EM_Command              0x43    /* Erase Memory command */
#define RESET_Command           0x3A    /* RESET mcu command */
#define RESET_KEY_Command       0xAB    /* Reset key word*/
#define SET_KEY_Command         0xBB    /* Set key word*/
#define READ_CPU_Command        0x4C    /* Set key word*/

//typedefs
typedef  void (*pFunction)(void);


//functions
void SetKey();
void ResetKey();
void Write_page(uint16_t page, uint8_t bytes);
void WM_command_handle(void);
void RESET_command_handle(void);
void RESET_KEY_command_handle(void);
void SET_KEY_command_handle(void);
void READ_CPU_command_handle(void);
void TIM2_INIT(void);
void TIM2_DEINIT(void);


//uart_functions
//================================================
void UART_INIT(void);
void UART_DEINIT(void);
uint8_t UART_Receive(uint8_t* ReceivedData);
void UART_Transmit(uint8_t data);
void Recieve_message_withou_while(void);

//================================================
uint32_t ReadKey();
pFunction Jump_To_Application;

//variables
uint16_t jumpAddress;
uint8_t* ReceivedData;
uint8_t DataBuffer[140];
uint8_t Received_count;


