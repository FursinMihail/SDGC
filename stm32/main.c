#include "stm32f10x.h"                  // Device header
#include "speed_tan.h"
#include "stdio.h"

//____Макросы для работы с I2C

#define MPU6050_ADDRES 0xD0

#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define INT_STATUS 0x3A
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
#define WHO_AM_I 0x75

#define d_StartEnd            (I2C2->SR1 & I2C_SR1_SB)      //__Старт сформирован?
#define d_AdrSendEnd         (I2C2->SR1 & I2C_SR1_ADDR)   //__Адрес отправлен?
#define d_ByteSendEnd         (I2C2->SR1 & I2C_SR1_TXE)      //__Динные отправлены?
#define d_I2C_WoytBusy      (I2C2->SR2 & I2C_SR2_BUSY)   //__Флаг занятости установлен?

#define d_I2C_Start()         I2C2->CR1 |= (I2C_CR1_START | I2C_CR1_PE)      //__Сформировать старт I2C2.
#define d_I2C_Stop()         I2C2->CR1 |= I2C_CR1_STOP      //__Сформировать стоп I2C2.

#define d_I2C_Byte(Byte)   I2C2->DR = Byte                     //__Байт для отправки.

#define d_I2C_SR1_Clear()   (void)I2C2->SR1                     //__Очищает регистр SR2 путем его чтения.
#define d_I2C_SR2_Clear()   (void)I2C2->SR2                     //__Очищает регистр SR2 путем его чтения.

///////////////////////___Инициализация I2C___/////////////////////////
//
//   Настраивает модуль I2C в необходимый режим.
//
///////////////////////////////////////////////////////////////////////
void I2C_HeaderInit (void)
{
		//RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;      //__Включить тактирование I2C1.
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;      //__Включить тактирование I2C2.
		I2C2 -> CR1 &= ~I2C_CR1_PE;
		////___Настройка пинов___////

		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;   //__Включить тактироване порта B.

		GPIOB->CRH |= GPIO_CRH_CNF10;      //__B10 открытый сток, альтернативная функция.
		GPIOB->CRH |= GPIO_CRH_CNF11;      //__B11 открытый сток, альтернативная функция.

		GPIOB->CRH |= GPIO_CRH_MODE10;      //__B10 режим HighSpeed.
		GPIOB->CRH |= GPIO_CRH_MODE11;      //__B11 режим HighSpeed.

		////___Настройка I2C модуля___////

		I2C2->CR2 = ((~I2C_CR2_FREQ & I2C2->CR2) | 36);      //__Частота шины APB1 - 36МГц.
		//I2C2->CCR = 0x8000;   //   ~I2C_CCR_FS;      //__SM мод.
		//I2C2->CCR |= I2C_CCR_DUTY;                     //__Режим 16:9.
		I2C2->CCR = ((I2C2->CCR & ~I2C_CCR_CCR) | 100);      //__Настройка частоты.

		I2C2->TRISE = 0x04;
		
		I2C2 -> CR1 |= I2C_CR1_PE;
}

uint16_t ReadReg(uint8_t adr){
		uint16_t tmout = 250;
	
		d_I2C_Start();                  //__Старт.
		tmout = 250;
		while(!d_StartEnd)
				tmout--;

		d_I2C_Byte(MPU6050_ADDRES);     //__Адрес запись.
		tmout = 250;
		while(!d_AdrSendEnd)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов!

		d_I2C_Byte(adr);                //__Адрес регистра
		tmout = 250;
		while(!d_ByteSendEnd)
				tmout--;

		d_I2C_Stop();                   //__Стоп.
		tmout = 250;
		while(d_I2C_WoytBusy)
				tmout--;
	
		d_I2C_Start();                  //__Старт.
		tmout = 250;
		while(!d_StartEnd)
				tmout--;
	
		d_I2C_Byte(MPU6050_ADDRES + 1); //__Адрес чтение.
		tmout = 250;
		while(!d_AdrSendEnd)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов! 
		
		d_I2C_Stop();                   //__Стоп.
		
		tmout = 250;
		while (!(I2C2->SR1 & I2C_SR1_RXNE))
				tmout--; 
		uint16_t data = I2C2->DR;								//__Чтение регистра.
		d_I2C_SR2_Clear();							//__Чтение регистра для очистки в нем флагов! 
		
		tmout = 250;
		while(d_I2C_WoytBusy)
				tmout--;
		
		return data;
}

void WriteReg(uint8_t adr, uint8_t data){
		uint16_t tmout = 250;
	
		d_I2C_Start();                  //__Старт.
		I2C2->CR1 |= I2C_CR1_ACK;
		tmout = 250;
		while(!d_StartEnd && tmout)
				tmout--;

		d_I2C_Byte(MPU6050_ADDRES);     //__Адрес запись.
		tmout = 250;
		while(!d_AdrSendEnd && tmout)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов!

		d_I2C_Byte(adr);                //__Адрес регистра
		tmout = 250;
		while(!d_ByteSendEnd && tmout)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов!
	
		d_I2C_Byte(data);                //__data
		tmout = 250;
		while(!d_ByteSendEnd && tmout)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов!
		
		d_I2C_Stop();                   //__Стоп.
		tmout = 250;
		while(d_I2C_WoytBusy && tmout)
				tmout--;
		I2C2->CR1 &= ~I2C_CR1_ACK;
}

void ReadRegs(uint8_t adr, uint8_t len, uint8_t* data){
		uint16_t tmout = 250;
	
		uint8_t pos = 0; 
		d_I2C_Start();                  //__Старт.
		I2C2->CR1 |= I2C_CR1_ACK;
		tmout = 250;
		while(!d_StartEnd && tmout)
				tmout--;

		d_I2C_Byte(MPU6050_ADDRES);     //__Адрес запись.
		tmout = 250;
		while(!d_AdrSendEnd && tmout)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов!

		d_I2C_Byte(adr);                //__Адрес регистра
		tmout = 250;
		while(!d_ByteSendEnd && tmout)
				tmout--;

		d_I2C_Stop();                   //__Стоп.
		tmout = 250;
		while(d_I2C_WoytBusy && tmout)
				tmout--;
	
		d_I2C_Start();                  //__Старт.
		tmout = 250;
		while(!d_StartEnd && tmout)
				tmout--;
	
		d_I2C_Byte(MPU6050_ADDRES + 1); //__Адрес чтение.
		tmout = 250;
		while(!d_AdrSendEnd && tmout)
				tmout--;
		d_I2C_SR2_Clear();              //__Чтение регистра для очистки в нем флагов! 
		
		while(--len){
				tmout = 250;
				while (!(I2C2->SR1 & I2C_SR1_RXNE) && tmout)
						tmout--; 																//__Чтение регистра.
				data[pos++] = I2C2->DR;
				d_I2C_SR2_Clear();							//__Чтение регистра для очистки в нем флагов! 
		}
		I2C2->CR1 &= ~I2C_CR1_ACK;
		d_I2C_Stop();                   //__Стоп.
		
		tmout = 250;
		while (!(I2C2->SR1 & I2C_SR1_RXNE) && tmout)
				tmout--;															 //__Чтение регистра.
		data[pos++] = I2C2->DR;
		d_I2C_SR2_Clear();							//__Чтение регистра для очистки в нем флагов! 
		
		tmout = 250;
		while(d_I2C_WoytBusy && tmout)
				tmout--;
}

int16_t gyro_x, gyro_y, gyro_z;
int16_t accel_x, accel_y, accel_z;

void read_gyro(){
		uint8_t data[6];
		ReadRegs(GYRO_XOUT_H, 6, data);
		gyro_x = data[0] << 8 | data[1];
		gyro_y = data[2] << 8 | data[3];
		gyro_z = data[4] << 8 | data[5];
	
		gyro_x += 900;
		gyro_y -= 70;
		gyro_z += 420;
}

void read_ax(){
		uint8_t data[6];
		ReadRegs(ACCEL_XOUT_H, 6, data);
		accel_x = data[0] << 8 | data[1];
		accel_y = data[2] << 8 | data[3];
		accel_z = data[4] << 8 | data[5];
		
}

uint8_t i_am = 0;
uint8_t smprt = 0;
uint8_t pwr = 0;
uint8_t config_old = 0;
uint8_t config = 0;

double x_abs = 0;
double y_abs = 0;
double z_abs = 0;

uint16_t tm = 0;

uint8_t tx_buf[1024];
uint16_t len = 0;
uint16_t pos = 0;

uint16_t s_x = 0;
uint16_t s_y = 0;


uint8_t bj = 0;
uint8_t btn_state = 0;
uint8_t btn_state_now = 0;
uint8_t btn_tm = 0;

uint8_t mode = 2;
uint16_t m_tm = 0;
uint8_t b1 = 0;
uint8_t btn1_state = 0;
uint8_t btn1_state_now = 0;
uint8_t btn1_tm = 0;

uint8_t b2 = 0;
uint8_t btn2_state = 0;
uint8_t btn2_state_now = 0;
uint8_t btn2_tm = 0;

uint8_t b3 = 0;
uint8_t btn3_state = 0;
uint8_t btn3_state_now = 0;
uint8_t btn3_tm = 0;


double l1 = 0, l2 = 0, l3 = 0;

void AdcInit(void);

void SysTick_Handler(void){
		if(mode != 2)
				tm++;
	
		if(GPIOA->IDR & GPIO_IDR_IDR6){
				btn_state_now = 0;
		}
		else{
				btn_state_now = 1;
		}
		
		if(GPIOB->IDR & GPIO_IDR_IDR5){
				btn1_state_now = 0;
		}
		else{
				btn1_state_now = 1;
		}
		
		if(GPIOB->IDR & GPIO_IDR_IDR4){
				btn2_state_now = 0;
		}
		else{
				btn2_state_now = 1;
		}
		
		if(GPIOB->IDR & GPIO_IDR_IDR8){
				btn3_state_now = 0;
		}
		else{
				btn3_state_now = 1;
		}
		
		if(btn_state_now != btn_state){
				btn_tm++;
				if(btn_tm > 50){
						btn_state = btn_state_now;
						if(btn_state)
								bj = !bj;
				}
		}
		
		if(btn1_state_now != btn1_state){
				btn1_tm++;
				if(btn1_tm > 50){
						btn1_state = btn1_state_now;
						if(btn1_state){
								b1 = !b1;
								//if(b1)
									mode++;
								if(mode >= 3)
									mode = 0;
						}
				}
		}
		
		if(btn2_state_now != btn2_state){
				btn2_tm++;
				if(btn2_tm > 50){
						btn2_state = btn2_state_now;
						if(btn2_state)
								b2 = !b2;
				}
		}
		
		if(btn3_state_now != btn3_state){
				btn3_tm++;
				if(btn3_tm > 50){
						btn3_state = btn3_state_now;
						if(btn3_state)
								b3 = !b3;
				}
		}
		
		if(b3){
				GPIOB -> ODR |= GPIO_ODR_ODR3;
		}
		else{
				GPIOB -> ODR &= ~GPIO_ODR_ODR3;
		}
		
		if(b2){
				GPIOB -> ODR |= GPIO_ODR_ODR6;
		}
		else{
				GPIOB -> ODR &= ~GPIO_ODR_ODR6;
		}
		
		if(mode == 1){
				GPIOB -> ODR |= GPIO_ODR_ODR7;
		}
		else if(mode == 0){
				if(m_tm < 200)
						GPIOB -> ODR &= ~GPIO_ODR_ODR7;
				else
						GPIOB -> ODR |= GPIO_ODR_ODR7;
				
				m_tm++;
				
				if(m_tm >= 400)
						m_tm = 0;
		}
		else{
				GPIOB -> ODR &= ~GPIO_ODR_ODR7;
		}
}

void init_USART2(){
		RCC->APB2ENR|= RCC_APB2ENR_IOPAEN;
		RCC->APB1ENR|= RCC_APB1ENR_USART2EN;
	
		USART2->BRR = SystemCoreClock/115200/2;
		USART2->CR1  |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
		//USART2->CR1 |= USART_CR1_TCIE;
	
		NVIC_EnableIRQ(USART2_IRQn);
	
		GPIOA->CRL	&= ~GPIO_CRL_CNF2;
		GPIOA->CRL	|= GPIO_CRL_CNF2_1;
		GPIOA->CRL	|= GPIO_CRL_MODE2_0;
		GPIOA->CRL	&= ~GPIO_CRL_CNF3;	
		GPIOA->CRL	|= GPIO_CRL_CNF3_0;	
		GPIOA->CRL	&= ~GPIO_CRL_MODE3;	
}

void USART_SendChar(const char data) {
    while (!(USART2->SR & USART_SR_TXE)); 
    USART2->DR = data;
}

void USART2_IRQHandler(void){
		if(USART2->SR & USART_SR_RXNE){
				USART2->SR &= ~USART_SR_RXNE;
				uint8_t tmp = USART2->DR;					
		}
		if(USART2->SR & USART_SR_TC){
				USART2->SR&=~USART_SR_TC;
				if(len){
						USART2->DR = tx_buf[pos++];
						if(pos >= len){
								pos = 0;
								len = 0;
						}
				}
				else{
						USART2->CR1 &= ~USART_CR1_TCIE;
				}
		}
		if(USART2->SR & USART_SR_ORE){
				USART2->SR &=~ USART_SR_ORE;
		}
}

int main(){
		SystemCoreClockUpdate();
		
		SysTick_Config(SystemCoreClock / 1000);
		
		RCC->APB2ENR|= RCC_APB2ENR_IOPAEN;
		RCC->APB2ENR|= RCC_APB2ENR_IOPBEN;
	
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
		AFIO->MAPR|=AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
		
		init_USART2();
		AdcInit();
	
		GPIOA->CRL &= ~GPIO_CRL_CNF6; GPIOA->CRL &= ~GPIO_CRL_MODE6;
		GPIOA->CRL |= GPIO_CRL_CNF6_0;
	
		GPIOB->CRL &= ~GPIO_CRL_CNF5; GPIOB->CRL &= ~GPIO_CRL_MODE5;
		GPIOB->CRL |= GPIO_CRL_CNF5_1;
		GPIOB->ODR |= GPIO_ODR_ODR5;
	
		GPIOB->CRL &= ~GPIO_CRL_CNF4; GPIOB->CRL &= ~GPIO_CRL_MODE4;
		GPIOB->CRL |= GPIO_CRL_CNF4_1;
		GPIOB->ODR |= GPIO_ODR_ODR4;
	
		GPIOB->CRH &= ~GPIO_CRH_CNF8; GPIOB->CRH &= ~GPIO_CRH_MODE8;
		GPIOB->CRH |= GPIO_CRH_CNF8_1;
		GPIOB->ODR |= GPIO_ODR_ODR8;
		
		GPIOB->CRL &= ~GPIO_CRL_CNF3; GPIOB->CRL |= GPIO_CRL_MODE3;
		GPIOB->CRL &= ~GPIO_CRL_CNF6; GPIOB->CRL |= GPIO_CRL_MODE6;
		GPIOB->CRL &= ~GPIO_CRL_CNF7; GPIOB->CRL |= GPIO_CRL_MODE7;
		
		for(uint32_t i = 0; i < 10000000; i++);
	
		I2C_HeaderInit();
		
		i_am = ReadReg(WHO_AM_I);
		smprt = ReadReg(SMPLRT_DIV);
		pwr = ReadReg(PWR_MGMT_1);
	
		WriteReg(PWR_MGMT_1, 0);
	
		pwr = ReadReg(PWR_MGMT_1);	
		
		
		while(1){
				
				//l1 = (double)accel_y / accel_x;
				if(accel_x == 0){
						if(accel_y > 0)
								l2 = s_tan(10000.0);
						else
								l2 = -s_tan(10000.0);
				}
				else{
						l2 = s_tan((double)accel_y / accel_x);
				}
				
				if(accel_z == 0){
						if(accel_x > 0)
								l2 = s_tan(10000.0);
						else
								l2 = -s_tan(10000.0);
				}
				else{
						l3 = s_tan((double)accel_x / accel_z);
				}
				
				if(l3 > 0)
					l3 = 90 - l3;
				else
					l3 = -90 - l3;
			
				read_gyro();
				read_ax();
				z_abs = 0.985 * (z_abs + gyro_z / 131.0 / 1000.0) + 0.015 * -l2; //+ 0.0385 * (accel_x / 16384.0) * 90;//57.2957795; 0.9615
				y_abs = 0.985 * (y_abs + gyro_y / 131.0 / 1000.0) + 0.015 * l3; //+ 0.0385 * (accel_y / 16384.0) * 90;//57.2957795;
				if(gyro_x > 350 || gyro_x < -350)
						x_abs += gyro_x / 131.0 / 1000.0;
				
				if((tm >= 20 && mode == 1) || (tm >= 60 && mode == 0)){
						
						if(x_abs > 0)	
								x_abs -= 0.06;
						
						if(x_abs < 0)	
								x_abs += 0.06;
						
						tm = 0;
					
						s_x = ADC1->JDR2;
						s_y = ADC1->JDR1; 
						
						s_x /= 20;
						s_y /= 20;
					
						int16_t s_x_i = s_x;
						int16_t s_y_i = s_y;
					
						s_x_i -= 100;
						s_y_i -= 100;
					
						len = sprintf(tx_buf, "P:\"%i\" R:\"%i\" Y:\"%i\" V:\"%i\" H:\"%i\" bj:\"%i\" b1:\"%i\" b2:\"%i\" b3:\"%i\"\n", (int16_t)(y_abs*10.0), (int16_t)(z_abs*10.0), (int16_t)(x_abs*10.0), s_x_i, s_y_i, bj, b1, b2, b3);
						USART2->CR1 |= USART_CR1_TCIE;
						USART2->DR = tx_buf[pos++];
				}
				//accel_x / 16384.0;
				//accel_y / 16384.0;
				//accel_z / 16384.0;
				/*if(z > 300 || z < -300){
						z_abs += z / 131 / 2000.0;
				}*/
				//for(uint32_t i = 0; i < 100000; i++);
		}
		
		return 0;
}

void AdcInit(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;    // Разрешить тактирование порта PORTA
  //Конфигурирование PORTA.4 - аналоговый вход
  GPIOA->CRL   &= ~GPIO_CRL_MODE4;       //Очистить биты MODE
  GPIOA->CRL   &= ~GPIO_CRL_CNF4;        //Очистить биты CNF
  //Конфигурирование PORTA.5 - аналоговый вход
  GPIOA->CRL   &= ~GPIO_CRL_MODE5;       //Очистить биты MODE
  GPIOA->CRL   &= ~GPIO_CRL_CNF5;        //Очистить биты CNF
 
  RCC->APB2ENR |=  RCC_APB2ENR_ADC1EN;   //подаем тактирование АЦП 
  RCC->CFGR    &= ~RCC_CFGR_ADCPRE;      //входной делитель
  ADC1->CR1     =  0;                    //предочистка регистра 
  ADC1->CR2    |=  ADC_CR2_CAL;          //запуск калибровки 
  while (!(ADC1->CR2 & ADC_CR2_CAL)){};  //ждем окончания калибровки
  ADC1->CR2     =  ADC_CR2_JEXTSEL;      //выбрать источником запуска разряд  JSWSTART
  ADC1->CR2    |=  ADC_CR2_JEXTTRIG;     //разр. внешний запуск инжектированной группы
  ADC1->CR2    |=  ADC_CR2_CONT;         //режим непрерывного преобразования 
  ADC1->CR1    |=  ADC_CR1_SCAN;         //режим сканирования (т.е. несколько каналов)
  ADC1->CR1    |=  ADC_CR1_JAUTO;	 //автомат. запуск инжектированной группы
  ADC1->JSQR    =  (uint32_t)(2-1)<<20;  //задаем количество каналов в инжектированной группе
  ADC1->JSQR   |=  (uint32_t)4<<(5*2);   //номер канала для третьего преобразования
  ADC1->JSQR   |=  (uint32_t)5<<(5*3);   //номер канала для четвертого преобразования
  ADC1->CR2    |=  ADC_CR2_ADON;         //включить АЦП
  ADC1->CR2    |=  ADC_CR2_JSWSTART;     //запустить процес преобразования
}