#include "main.h"
#include "stm8l15x.h"
#include "stm8l15x_flash.h"

uint32_t tmp1 = 0;
uint8_t tmp2 = 0;
uint16_t crc_recieve = 0;
uint16_t crc_culc = 0;
uint16_t d=0xFF;
int main( void )
{      
  uint8_t timcnt=0;
  disableInterrupts();
  CLK->CKDIVR = 0x00;         /* Set High speed internal clock prescaler */
  if (ReadKey() == BOOTLOADER_KEY_VALUE) //Проверяем ключ, если ключ записан верный то прыгаем в основную программу, если нет то включаем режим бутлоадера
  {
    UART_INIT(); 
    TIM2_INIT();
    TIM2->CR1 |= 0x01;    // TIM2 старт
    while(timcnt < 6)
    {
      Recieve_message_withou_while();
      if ((TIM2->CNTRH == 0xF4) && (TIM2->CNTRL >= 0x21))
      {
        TIM2->CNTRH == 0x00;
        TIM2->CNTRL == 0x00;
        timcnt++;
      }
    }
    if (ReadKey() == BOOTLOADER_KEY_VALUE) //Проверяем снова ключ, если ключ записан верный то прыгаем в основную программу, если нет то включаем режим бутлоадера
    {
      UART_DEINIT();
      TIM2_DEINIT();
      asm("LDW X,  SP ");
      asm("LD  A,  $FF");
      asm("LD  XL, A  ");
      asm("LDW SP, X  ");
      asm("JPF " MAIN_PROGRAM_JUMP_ADDRESS_ASM);// это стартовый адрес основной программы
    }
  }
  else
  {
    UART_INIT();                //настраиваем UART
    //UART_Transmit(0x51);
    Received_count=0; 
  }
  while(1)
  {
    crc_recieve=0;
    tmp2=0;
    crc_culc=0;
    UART_Receive(&DataBuffer[N_COMMAND]);
    UART_Receive(&DataBuffer[NEG_N_COMMAND]);
    tmp2 = ~DataBuffer[NEG_N_COMMAND];
    if (DataBuffer[N_COMMAND] == tmp2)
    {
      switch (DataBuffer[N_COMMAND])
      {
        case WM_Command:                WM_command_handle(); break;
        case RESET_Command:             RESET_command_handle(); break;
        case RESET_KEY_Command:         RESET_KEY_command_handle(); break;
        case SET_KEY_Command:           SET_KEY_command_handle(); break;
        case READ_CPU_Command:          READ_CPU_command_handle(); break;
        default:                        UART_Transmit(NACK);
      }
    }
    else UART_Transmit(NACK);
  }
  return 0;
}

/***************************************************************************************/
// Function                SetKey()
// Description             Sets bootloader key
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void SetKey()
{
  FLASH_Unlock(FLASH_MemType_Data);
  FLASH_ProgramWord(BOOTLOADER_KEY_START_ADDRESS, BOOTLOADER_KEY_VALUE);
  FLASH_Lock(FLASH_MemType_Data);
} // End of SetKey()
/***************************************************************************************/
// Function                ResetKey()
// Description             Resets bootloader key
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void ResetKey()
{
  FLASH_Unlock(FLASH_MemType_Data);
  *(uint32_t*)BOOTLOADER_KEY_START_ADDRESS = 0x00000000;
  FLASH_EraseBlock(BOOTLOADER_KEY_START_ADDRESS, FLASH_MemType_Data);
  FLASH_Lock(FLASH_MemType_Data);
} // End of ResetKey()
/***************************************************************************************/
// Function                ReadKey()
// Description             Reads bootloader key value
// Parameters              None
// RetVal                  None
/***************************************************************************************/
uint32_t ReadKey()
{
    return (*(__IO uint32_t*) BOOTLOADER_KEY_START_ADDRESS);
} // End of ReadKey()
/***************************************************************************************/
// Function                Write_page(uint16_t page, uint8_t bytes)
// Description             записывает задданное количество байт из буфера в заданную странциу flash
// Parameters              page - страница, начинается с 0 (----), bytes - количество байт
// RetVal                  None
/***************************************************************************************/
void Write_page(uint16_t page, uint8_t bytes)
{
  uint16_t addr=0;
  FLASH_Unlock(FLASH_MemType_Program);
  
  if(page==0)
  {
    addr = MAIN_PROGRAM_START_ADDRESS;
    FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+0, DataBuffer[0+FIRST_BYTE_IN_MES]);
    addr++;
    FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+1, DataBuffer[1+FIRST_BYTE_IN_MES]);
    addr++;
    FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+2, DataBuffer[2+FIRST_BYTE_IN_MES]);
    addr++;
    FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+3, DataBuffer[3+FIRST_BYTE_IN_MES]);
    addr++;
    for (uint8_t i=4; i<bytes; i++)
    {
      FLASH_ProgramByte( addr, DataBuffer[i+FIRST_BYTE_IN_MES]);
      addr++;
    }
  }
  else
  {
    addr = MAIN_PROGRAM_START_ADDRESS+(page*BLOCK_BYTES);
    for(uint8_t t=0; t<bytes; t++)
    {
      //FLASH_ProgramByte( addr, DataBuffer[t+FIRST_BYTE_IN_MES]);
      *(uint8_t*)addr++ = DataBuffer[t+FIRST_BYTE_IN_MES];
    }
   }
  
  
 /* for(uint8_t t=0; t<bytes; t++)
  {
    addr = MAIN_PROGRAM_START_ADDRESS+t+(page*BLOCK_BYTES);
    switch (addr)
    {
    case MAIN_PROGRAM_START_ADDRESS+0: FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+0, DataBuffer[t+FIRST_BYTE_IN_MES]); break;
    case MAIN_PROGRAM_START_ADDRESS+1: FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+1, DataBuffer[t+FIRST_BYTE_IN_MES]); break;
    case MAIN_PROGRAM_START_ADDRESS+2: FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+2, DataBuffer[t+FIRST_BYTE_IN_MES]); break;
    case MAIN_PROGRAM_START_ADDRESS+3: FLASH_ProgramByte( MAIN_PROGRAM_JUMP_ADDRESS+3, DataBuffer[t+FIRST_BYTE_IN_MES]); break;
    default: FLASH_ProgramByte( addr, DataBuffer[t+FIRST_BYTE_IN_MES]);
    }
  }*/
  /*while(d) d--;
  d=0xFF;*/
  FLASH_Lock(FLASH_MemType_Program);
} // End of Write_page()
/***************************************************************************************/
// Function                UART_INIT()
// Description             Настраивает юарт
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void UART_INIT(void)
{
  /* Init DE pin for RS485 */
  GPIOD->DDR |= 0x80;   // PD7 as Output 
  GPIOD->CR1 |= 0x80;   // PD7 as Push-Pull 
  GPIOD->ODR |= 0x00;   // PD7 reset bit 
  /* Init Rx(PC2)/Tx(PC3) pin for RS485 */
  GPIOC->DDR &= ~0x20;   // PC2 as Input 
  GPIOC->DDR |= 0x40;   // PC3 as Output 
  GPIOC->CR1 |= 0x80;   // PC2 as Push-up 
  /* Enable USART peripheral clock */
  CLK->PCKENR1 |= 0x20 ;
  /* Set 9bit data + Parity Control bit with Even parity */
  USART1->CR1 |= (u8)  0x14;
  /* UART1 Enable */
  USART1->CR1 &= (u8)(~0x20); 
  /* UART RXTX enable */
  USART1->CR2 |= (u8)  0x0C;
  //Set BR to 9600 
  USART1->BRR2 = 0x01;
  USART1->BRR1 = 0x34;
  //9600-3,68; 19200 - 1,34; 115200 - b,8
} // End of UART_INIT()
/***************************************************************************************/
// Function                UART_DEINIT()
// Description             Настраивает юарт
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void UART_DEINIT(void)
{
  /* De-Init DE pin for RS485 */
  GPIOD->DDR &= ~0x80;   // PD7 as input 
  GPIOD->CR1 &= !0x80;   // PD7 as floating
  GPIOD->ODR |= 0x00;   // PD7 reset bit 
  /* Init Rx(PC2)/Tx(PC3) pin for RS485 */
  GPIOC->DDR &= ~0x02;   // PC2 as Input 
  GPIOC->DDR &= ~0x03;   // PC3 as Input 
  GPIOC->CR1 &= ~0x00;   // PC2 and PC3 as floating
  /* DIsable USART peripheral clock */
  CLK->PCKENR1 &= ~0x20 ;
  /* Set 9bit data + Parity Control bit with Even parity */
  USART1->CR1 |= (u8)  0x14;
  /* UART1 Disable */
  USART1->CR1 |= (u8)(0x20); 
  /* UART RXTX Disable */
  USART1->CR2 &= (u8)  ~0x0C;
  //Set BR to 9600 
  USART1->BRR2 = 0x01;
  USART1->BRR1 = 0x34;
  //9600-3,68; 19200 - 1,34; 115200 - b,8
} // End of UART_DEINIT()
/***************************************************************************************/
// Function                UART_Transmit()
// Description             Отправляет байт
// Parameters              data
// RetVal                  none
/***************************************************************************************/
void UART_Transmit(uint8_t data)
{
  uint8_t sr;   // working copy of uart_SR register
  sr = USART1->SR;      //wait for Tx empty
  while(!(sr & 0x80/*TxNE*/)) sr = USART1->SR;
  GPIOD->ODR |= 0x80;   // PD7 set bit //driver enable
  USART1->DR = data;    //send data
  sr = USART1->SR;      //wait for transmission complete
  while(!(sr & 0x40/*TxNE*/)) sr = USART1->SR;
  GPIOD->ODR &= ~0x80;   // PD7 reset bit 
} // End of UART_Transmit()
/***************************************************************************************/
// Function                UART_Receive()
// Description             Принимает байт и отправляет его по указателю ReceivedData 
// Parameters              ReceivedData
// RetVal                  return 1 - no error, return 0 -error
/***************************************************************************************/
uint8_t UART_Receive(uint8_t* ReceivedData)
{
  uint8_t sr;   // working copy of uart_SR register
  sr = USART1->SR;      //wait for Rx full
  while(!(sr & 0x20 /*RXNE*/)) sr = USART1->SR; //check if overrun or parity error
  if((sr & 0x08/*OR*/)||(sr & 0x01/*PE*/))
  {
    *ReceivedData = USART1->DR ;        //receive data to clear error
    UART_Transmit(NACK);     //send NACK to host
    return 0;           //and return error
  }  
  *ReceivedData = USART1->DR;     //receive data
  
  return 1;       //and return no error
} // End of UART_Receive()
/***************************************************************************************/
// Function                WM_command_handle()
// Description             Обработчик команды WM_command
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void WM_command_handle(void)
{
  UART_Receive(&DataBuffer[N_PAGE]);
  UART_Receive(&DataBuffer[SIZE_MES]);
  if(DataBuffer[SIZE_MES] > BLOCK_BYTES) return;
  for(uint8_t m=0; m<DataBuffer[SIZE_MES]; m++)     //цикл в котором принимаем количество байт равное SIZE_MES
  {
    UART_Receive(&DataBuffer[FIRST_BYTE_IN_MES+m]);
    crc_culc=crc_culc+DataBuffer[FIRST_BYTE_IN_MES+m];
    }
  UART_Receive(&tmp2);
  crc_recieve = tmp2;
  crc_recieve = crc_recieve<<8;
  UART_Receive(&tmp2);
  crc_recieve+=tmp2;
  if(crc_culc == crc_recieve)
  {
    Write_page(DataBuffer[N_PAGE], DataBuffer[SIZE_MES]);
    UART_Transmit(ACK);
  }
  else UART_Transmit(NACK);
} // End of WM_command_handle()
/***************************************************************************************/
// Function                RESET_command_handle()
// Description             Обработчик команды RESET_command
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void RESET_command_handle(void)
{
  uint32_t temp = 0;
  uint8_t dummy = 2;
  for (uint8_t k=0; k<4; k++)
  {
    temp<<=8;
    UART_Receive(&DataBuffer[dummy]);
    temp+=DataBuffer[dummy];
    dummy++;
  }
  if (BOOTLOADER_KEY_VALUE != temp)
  {
    UART_Transmit(NACK);
    return;
  }
  UART_Transmit(ACK);
  IWDG->KR = 0xCC;      //запускаем ватчдог чтобы он перезагрузил процессор
  while(1){}
} // End of RESET_command_handle()
/***************************************************************************************/
// Function                SET_KEY_command_handle()
// Description             Обработчик команды SET_KEY_command
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void SET_KEY_command_handle(void)
{
  uint32_t temp = 0;
  uint8_t dummy = 2;
  for (uint8_t k=0; k<4; k++)
  {
    temp<<=8;
    UART_Receive(&DataBuffer[dummy]);
    temp+=DataBuffer[dummy];
    dummy++;
  }
  if (BOOTLOADER_KEY_VALUE != temp)
  {
    UART_Transmit(NACK);
    return;
  }
  UART_Transmit(ACK);
  SetKey();
}// End of SET_KEY_command_handle()
/***************************************************************************************/
// Function                RESET_KEY_command_handle()
// Description             Обработчик команды RESET_KEY_command
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void RESET_KEY_command_handle(void)
{
  uint32_t temp = 0;
  uint8_t dummy = 2;
  for (uint8_t k=0; k<4; k++)
  {
    temp<<=8;
    UART_Receive(&DataBuffer[dummy]);
    temp+=DataBuffer[dummy];
    dummy++;
  }
  if (BOOTLOADER_KEY_VALUE != temp)
  {
    UART_Transmit(NACK);
    return;
  }
  UART_Transmit(ACK);
  ResetKey(); 
}// End of RESET_KEY_command_handle()

/***************************************************************************************/
// Function                READ_CPU_command_handle()
// Description             Обработчик команды READ_CPU
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void READ_CPU_command_handle(void)
{
  uint32_t temp = 0;
  uint8_t dummy = 2;
  for (uint8_t k=0; k<4; k++)
  {
    temp<<=8;
    UART_Receive(&DataBuffer[dummy]);
    temp+=DataBuffer[dummy];
    dummy++;
  }
  if (BOOTLOADER_KEY_VALUE != temp)
  {
    UART_Transmit(NACK);
    return;
  }
  else
  {
    for(uint8_t j=0; j<sizeof(CPU_NAME)-1; j++)
    {
      UART_Transmit(CPU_NAME[j]);
    }
  }
}

/***************************************************************************************/
// Function                TIM2_INIT()
// Description             Настраивает таймер2
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void TIM2_INIT(void)
{
  CLK->PCKENR1 |= 0x01;        // включение тактирования периферии
  TIM2->PSCR |= 0x03;    // деление основной частоты на 128 (2^8)
  TIM2->ARRH = 0xF4;    // 16МГц/128=125000 - тактов за одну секунду
  TIM2->ARRL = 0x24;    // считаем до 62500(0xF424) - тогда за 1 цикл счетчика будет равен 0.5 сек
  TIM2->CR1 |= 0x04;
}
//End of TIM2_INIT()
/***************************************************************************************/
// Function                TIM2_DEINIT()
// Description             Сбрасывает настройки таймер2
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void TIM2_DEINIT(void)
{
  TIM2->CR1 = 0;
  TIM2->IER = 0;
  TIM2->SR1 = 0;
  TIM2->SR2 = 0;
  TIM2->EGR = 0;
  TIM2->CCMR1 = 0;
  TIM2->CCMR2 = 0;
  TIM2->CCER1 = 0;
  TIM2->CNTRH = 0;
  TIM2->CNTRL = 0;
  TIM2->PSCR = 0;
  TIM2->ARRH = 0xFF;
  TIM2->ARRL = 0xFF;
  TIM2->CCR1H = 0;
  TIM2->CCR1L = 0;
  TIM2->CCR2H = 0;
  TIM2->CCR2L = 0;
  CLK->PCKENR1 &= ~0x01;
}
//End of TIM2_DEINIT()
/***************************************************************************************/
// Function                Recieve_message_withou_while()
// Description             Проверяет наличие сообзщения в юарте
// Parameters              None
// RetVal                  None
/***************************************************************************************/
void Recieve_message_withou_while(void)
{
  static uint8_t rcnt=0;
  uint8_t RESET_KEY_MESSAGE[] = {RESET_KEY_Command, ~RESET_KEY_Command, 0xAA, 0xAA, 0x55, 0x55};
  uint8_t sr;   // working copy of uart_SR register
  sr = USART1->SR;      //wait for Rx full
  if((sr & 0x20 /*RXNE*/)) //check if overrun or parity error
  {
    if((sr & 0x08/*OR*/)||(sr & 0x01/*PE*/))
    {
      sr = USART1->DR ;        //receive data to clear error
    }
    else
    {
      DataBuffer[rcnt] = USART1->DR;     //receive data
      if(DataBuffer[rcnt] == RESET_KEY_MESSAGE[rcnt]) 
      {
        rcnt++;
      }
      if (rcnt==6)      //если приняли 6 байт то проверить их
      {
        UART_Transmit(ACK);
        ResetKey(); 
      }
    }
  }
}
//End of Recieve_message_withou_while()
/***************************************************************************************/

/***************************************************************************************/

/***************************************************************************************/

/***************************************************************************************/

/***************************************************************************************/
