	.syntax unified
	.thumb

	.text

    .extern   _HardFault_Handler
	.extern   SysTick_Handler

	.extern   WWDG_IRQHandler
	.extern   PVD_IRQHandler
	.extern   TAMPER_IRQHandler
	.extern   RTC_IRQHandler
	.extern   FLASH_IRQHandler
	.extern   RCC_IRQHandler
	.extern   EXTI0_IRQHandler
	.extern   EXTI1_IRQHandler
	.extern   EXTI2_IRQHandler
	.extern   EXTI3_IRQHandler
	.extern   EXTI4_IRQHandler
	.extern   DMA1_Channel1_IRQHandler
	.extern   DMA1_Channel2_IRQHandler
	.extern   DMA1_Channel3_IRQHandler
	.extern   DMA1_Channel4_IRQHandler
	.extern   DMA1_Channel5_IRQHandler
	.extern   DMA1_Channel6_IRQHandler
	.extern   DMA1_Channel7_IRQHandler
	.extern   ADC1_2_IRQHandler
	.extern   USB_HP_CAN1_TX_IRQHandler
	.extern   USB_LP_CAN1_RX0_IRQHandler
	.extern   CAN1_RX1_IRQHandler
	.extern   CAN1_SCE_IRQHandler
	.extern   EXTI9_5_IRQHandler
	.extern   TIM1_BRK_IRQHandler
	.extern   TIM1_UP_IRQHandler
	.extern   TIM1_TRG_COM_IRQHandler
	.extern   TIM1_CC_IRQHandler
	.extern   TIM2_IRQHandler
	.extern   TIM3_IRQHandler
	.extern   TIM4_IRQHandler
	.extern   I2C1_EV_IRQHandler
	.extern   I2C1_ER_IRQHandler
	.extern   I2C2_EV_IRQHandler
	.extern   I2C2_ER_IRQHandler
	.extern   SPI1_IRQHandler
	.extern   SPI2_IRQHandler
	.extern   USART1_IRQHandler
	.extern   USART2_IRQHandler
	.extern   USART3_IRQHandler
	.extern   EXTI15_10_IRQHandler
	.extern   RTCAlarm_IRQHandler
	.extern   USBWakeUp_IRQHandler

	.word	  0x20005000
	.word	  __start

	.word     0               			@ NMI Handler
	.word     _HardFault_Handler        @ Hard Fault Handler
	.word     0        					@ MPU Fault Handler
	.word     0         				@ Bus Fault Handler
	.word     0       					@ Usage Fault Handler
	.word     0                         @ Reserved
	.word     0                         @ Reserved
	.word     0                         @ Reserved
	.word     0                         @ Reserved
	.word     0				            @ SVCall Handler
	.word     0          				@ Debug Monitor Handler
	.word     0                         @ Reserved
	.word     0            				@ PendSV Handler
	.word     SysTick_Handler           @ SysTick Handler

	.word     WWDG_IRQHandler           @ Window Watchdog
	.word     PVD_IRQHandler            @ PVD through EXTI Line detect
	.word     TAMPER_IRQHandler         @ Tamper
	.word     RTC_IRQHandler            @ RTC
	.word     FLASH_IRQHandler          @ Flash
	.word     RCC_IRQHandler            @ RCC
	.word     EXTI0_IRQHandler          @ EXTI Line 0
	.word     EXTI1_IRQHandler          @ EXTI Line 1
	.word     EXTI2_IRQHandler          @ EXTI Line 2
	.word     EXTI3_IRQHandler          @ EXTI Line 3
	.word     EXTI4_IRQHandler          @ EXTI Line 4
	.word     DMA1_Channel1_IRQHandler  @ DMA1 Channel 1
	.word     DMA1_Channel2_IRQHandler  @ DMA1 Channel 2
	.word     DMA1_Channel3_IRQHandler  @ DMA1 Channel 3
	.word     DMA1_Channel4_IRQHandler  @ DMA1 Channel 4
	.word     DMA1_Channel5_IRQHandler  @ DMA1 Channel 5
	.word     DMA1_Channel6_IRQHandler  @ DMA1 Channel 6
	.word     DMA1_Channel7_IRQHandler  @ DMA1 Channel 7
	.word     ADC1_2_IRQHandler         @ ADC1 & ADC2
	.word     0  						@ USB High Priority or CAN1 TX
	.word     0 						@ USB Low  Priority or CAN1 RX0
	.word     0       					@ CAN1 RX1
	.word     0       					@ CAN1 SCE
	.word     EXTI9_5_IRQHandler        @ EXTI Line 9..5
	.word     TIM1_BRK_IRQHandler       @ TIM1 Break
	.word     TIM1_UP_IRQHandler        @ TIM1 Update
	.word     TIM1_TRG_COM_IRQHandler   @ TIM1 Trigger and Commutation
	.word     TIM1_CC_IRQHandler        @ TIM1 Capture Compare
	.word     TIM2_IRQHandler           @ TIM2
	.word     TIM3_IRQHandler           @ TIM3
	.word     TIM4_IRQHandler           @ TIM4
	.word     I2C1_EV_IRQHandler        @ I2C1 Event
	.word     I2C1_ER_IRQHandler        @ I2C1 Error
	.word     I2C2_EV_IRQHandler        @ I2C2 Event
	.word     I2C2_ER_IRQHandler        @ I2C2 Error
	.word     SPI1_IRQHandler           @ SPI1
	.word     SPI2_IRQHandler           @ SPI2
	.word     USART1_IRQHandler         @ USART1
	.word     USART2_IRQHandler         @ USART2
	.word     USART3_IRQHandler         @ USART3
	.word     EXTI15_10_IRQHandler      @ EXTI Line 15..10
	.word     RTCAlarm_IRQHandler       @ RTC Alarm through EXTI Line
	.word     USBWakeUp_IRQHandler      @ USB Wakeup from suspend

	.global	__start
  	.type 	__start, %function
__start:

	.extern __RO_LIMIT__
	.extern __RW_BASE__
	.extern __ZI_BASE__
	.extern __ZI_LIMIT__

	ldr		r0, =__RO_LIMIT__
	ldr		r1, =__RW_BASE__
	ldr		r3, =__ZI_BASE__

	cmp		r0, r1
	beq		2f

1:
	cmp		r1, r3
	ittt	lo
	ldrlo	r2, [r0], #4
	strlo	r2, [r1], #4
	blo		1b

2:
	ldr		r1, =__ZI_LIMIT__
	mov		r2, #0x0
3:
	cmp		r3, r1
	itt		lo
	strlo	r2, [r3], #4
	blo		3b

	.extern Main

	.equ RAM_END,	(0x20004FFF)
	.equ PSP_BASE,	(MSP_LIMIT)
	.equ MSP_BASE,	(RAM_END + 1)
	.equ MSP_SIZE,	(1*1024)
	.equ MSP_LIMIT,	(MSP_BASE - MSP_SIZE)

	ldr		r0, =PSP_BASE
	msr		psp, r0
	ldr		r0, =(0x1<<1)|(0x0<<0)
	msr		control, r0

	bl		Main

	b		.

    .extern	Uart1_Printf

    .global _Print_Reg
  	.type 	_Print_Reg, %function
_Print_Reg:
    PUSH	{r0-r12,lr}
    MRS     r1, PSR
    PUSH    {r0, r1}

    MOV     r6, #14
    MOV     r4, #4
    LDR     r5, =_PSR
_loop:
    LDR     r0, =fmt
    MOV     r1, r5
    LDR     r2, [sp, r4]
    BL      Uart1_Printf

    ADD     r4, r4, #4
    ADD     r5, r5, #4
    SUBS    r6, r6, #1
    BGT     _loop

    LDR     r0, =fmt
    MOV     r1, r5
    MRS     r2, psp
    BL      Uart1_Printf

    POP     {r0, r1}
	MSR     APSR_nzcvq, r1
	MSR     IEPSR, r1
    POP		{r0-r12, pc}

fmt:	.string "%s : 0x%.8X\n"
_PSR:   .string "PSR"
_r0:    .string "R00"
_r1:    .string "R01"
_r2:    .string "R02"
_r3:    .string "R03"
_r4:    .string "R04"
_r5:    .string "R05"
_r6:    .string "R06"
_r7:    .string "R07"
_r8:    .string "R08"
_r9:    .string "R09"
_r10:   .string "R10"
_r11:   .string "R11"
_r12:   .string "R12"
_PSP:   .string "PSP"

	.align 2

    .extern  HardFault_Handler

    .global _HardFault_Handler
  	.type 	_HardFault_Handler, %function
_HardFault_Handler:

	PUSH	{r0, lr}
	BL    	_Print_Reg
	ADD   	r0, sp, #8
	LDR   	r1, [sp, #4]
	MRS   	r2, psp
	BL    	HardFault_Handler
	POP	 	{r0, pc}

	.end
