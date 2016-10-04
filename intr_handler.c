#include <stdio.h>
#include "xtmrctr.h"
#include "register.h"
#include "xil_io.h"
#include "xtmrctr_l.h"

extern void Acknowledge_PClk_Lost_Intr();
extern void Acknowledge_Timer_Intr();

extern void Tmr0_Cntr_Stop(u8 TmrCtrNumber);


void PClk_Lock_Intr_Handler(void * CallbackRef) {
	//printf("Pclk Intr Triggered\n");

	Acknowledge_PClk_Lost_Intr();
}

void Tmr0_Intr_Handler (void * CallbackRef) {
	
	unsigned int diff = 0x0;
	unsigned int TLR_Timer0_Cntr0 = 0x0;
	unsigned int TLR_Timer0_Cntr1 = 0x0;

	u32 ControlStatusReg;

	//printf("Timer Intr Triggered\n");

	XTmrCtr *InstancePtr = (XTmrCtr *) CallbackRef;

//	if (XTmrCtr_IsExpired(InstancePtr, 1)) //basavam Optimisation
	{
		//printf("Timer - Cntr 1 Intr Triggered\n");

		Xil_Out32((APB2GPIO_BASE + PHYSTATUS_SUCCESS), 0x01);

		TLR_Timer0_Cntr0 = XTmrCtr_GetCaptureValue (InstancePtr, 0);
		TLR_Timer0_Cntr1 = XTmrCtr_GetCaptureValue (InstancePtr, 1);

		diff = TLR_Timer0_Cntr1 - TLR_Timer0_Cntr0;
		//printf("Diff Captured: %04X \n", diff);

		Xil_Out32((APB2GPIO_BASE + TIMER0_CNT), diff);
	}

/*	else
   	{
    	Xil_Out32((APB2GPIO_BASE + PHYSTATUS_SUCCESS), 0x00);
    	printf("PHYStatus Trigger not detected\n");
    	return;
   	}
*/
	// Acknowledge Timer Interrupt
	ControlStatusReg = XTmrCtr_ReadReg(InstancePtr->BaseAddress,
					      1, XTC_TCSR_OFFSET);
	/*
	 * Disable the timer counter such that it's not running
	 */
	ControlStatusReg &= ~(XTC_CSR_ENABLE_TMR_MASK | XTC_CSR_ENABLE_ALL_MASK);

	XTmrCtr_WriteReg(InstancePtr->BaseAddress, 1,XTC_TCSR_OFFSET,
			  ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);

	Tmr0_Cntr_Stop(0);
	//printf("Timer Intr Ack\n");
}
