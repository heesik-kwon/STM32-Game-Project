#include "device_driver.h"

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->APB2ENR, 3);
	Macro_Write_Block(GPIOB->CRL, 0xff, 0x44, 24);
}

int Key_Get_Pressed(void)
{
	return Macro_Extract_Area(~GPIOB->IDR,0x3,6);
}

void Key_Wait_Key_Released(void)
{
	while(Key_Get_Pressed());
}

int Key_Wait_Key_Pressed(void)
{
	int k;

	do
	{
		k = Key_Get_Pressed();
	}while(!k);

	return k;
}

void Key_ISR_Enable(int en)
{
	if(en)
	{
		Macro_Set_Bit(RCC->APB2ENR,3);
		Macro_Set_Bit(RCC->APB2ENR,0);

		Macro_Write_Block(GPIOB->CRL,0xff,0x44,24);
		Macro_Write_Block(AFIO->EXTICR[1],0xff,0x11,8);
		Macro_Write_Block(EXTI->FTSR,0x3,0x3,6);
		Macro_Write_Block(EXTI->IMR,0x3,0x3,6);

		EXTI->PR = (0x3<<6);
		NVIC_ClearPendingIRQ((IRQn_Type)23);
		NVIC_EnableIRQ((IRQn_Type)23);
	}

	else
	{
		NVIC_DisableIRQ((IRQn_Type)23);
	}
}
