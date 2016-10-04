#include "xparameters.h"
#include "xil_cache.h"
#include <xintc.h>
#include <xil_exception.h>
#include <stdio.h>
#include "xil_io.h"
#include "xtmrctr.h"
#include "xtmrctr_i.h"
#include "xtmrctr_l.h"
#include <stdint.h>
#include "register.h"

void config_test_regs(void)
{
	Xil_Out32((APB2GPIO_BASE + LTSSM_U1_EXIT), 0x00);
	Xil_Out32((APB2GPIO_BASE + LTSSM_U0_U1), 0x00);	
}


#define PCLK_LOCK_INTR  XPAR_DESIGN_3_MICROBLAZE_0_AXI_INTC_SYSTEM_IN1_0_INTR
#define TIMER0_INTR    XPAR_DESIGN_3_MICROBLAZE_0_AXI_INTC_DESIGN_3_AXI_TIMER_0_INTERRUPT_INTR

#define RESET_VALUE	0x00000000

static XIntc intc;
static XTmrCtr design_3_axi_timer_0_Timer;
static uint8_t usbIrqStatus;

u32 TmrConfigure()
{
	int Status;

	/*
	 * Initialize the TmrCtr driver so that it iss ready to use
	 */
	Status = XTmrCtr_Initialize(&design_3_axi_timer_0_Timer, XPAR_DESIGN_3_AXI_TIMER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 1st timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(&design_3_axi_timer_0_Timer, 0x0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

extern void PClk_Lock_Intr_Handler (void * CallbackRef);
extern void Tmr0_Intr_Handler (void * CallbackRef);

/*** fn void local_irq_disable() - Disable Interrupts */

void localIrqDisable() {
#ifdef PCLK_LOCK_INTR
    XIntc_Disable(&intc, PCLK_LOCK_INTR);
#endif
#ifdef TIMER0_INTR
    XIntc_Disable(&intc, TIMER0_INTR);
#endif
	usbIrqStatus = 0;
}

/*** fn void local_irq_enable()-Enable Interrupts */

void localIrqEnable() {
	usbIrqStatus = 1;
#ifdef PCLK_LOCK_INTR
    XIntc_Enable(&intc, PCLK_LOCK_INTR);
#endif
#ifdef TIMER0_INTR
	XIntc_Enable(&intc, TIMER0_INTR);
#endif
}

/**
 * Configure Interrupt Controller.
 */
u32 gicConfigure() {
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is
	 * ready to use.
	 */
	Status = XIntc_Initialize(&intc, XPAR_INTC_0_DEVICE_ID);
	//printf("Init-%d\n",Status);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//printf("Init-%d\n",Status);

	/*
	 * Perform a self-test to ensure that the hardware was built  correctly.
	 */
	/*Status = XIntc_SelfTest(&intc);
	//printf("ST-%x\n",Status);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}*/

	/*
	 * Initialize the exception table
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_DeviceInterruptHandler,
			(void*) 0);

	/*
	 * Enable exceptions. Might not be needed - TBC_yash
	 */
	Xil_ExceptionEnable();

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	Status = XIntc_Start(&intc, XIN_REAL_MODE);
	//printf("START-%d\n",Status);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void gicStop() {

	localIrqDisable();

	XIntc_Disconnect(&intc, TIMER0_INTR);
	XIntc_Disconnect(&intc, PCLK_LOCK_INTR);

	XIntc_Stop(&intc);
}

u32 InterruptConfig() {
	int Status;

#ifdef PCLK_LOCK_INTR
	Status = XIntc_Connect(&intc, PCLK_LOCK_INTR,
			(XInterruptHandler)PClk_Lock_Intr_Handler,
			(void *)0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

#ifdef TIMER0_INTR
	Status = XIntc_Connect(&intc, TIMER0_INTR,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *) &design_3_axi_timer_0_Timer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires, specify a pointer to the
	 * timer counter driver instance as the callback reference so the handler
	 * is able to access the instance data
	 */
	XTmrCtr_SetHandler(&design_3_axi_timer_0_Timer, (XTmrCtr_Handler ) Tmr0_Intr_Handler,
					   &design_3_axi_timer_0_Timer);
#endif

	return 0;
}

void Timer0_Cntr_Settings() {

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(&design_3_axi_timer_0_Timer, 0, XTC_CAPTURE_MODE_OPTION);

	XTmrCtr_SetOptions(&design_3_axi_timer_0_Timer, 1,
				XTC_INT_MODE_OPTION | XTC_CAPTURE_MODE_OPTION);

	/*
	 * Set a reset value for the timer counter such that it will expire
	 * eariler than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(&design_3_axi_timer_0_Timer, 0, RESET_VALUE);
	XTmrCtr_SetResetValue(&design_3_axi_timer_0_Timer, 1, RESET_VALUE);

	/**
	* Resets the specified timer counter of the device. A reset causes the timer
	* counter to set it's value to the reset value.
	*/
	XTmrCtr_Reset (&design_3_axi_timer_0_Timer, 0);
	XTmrCtr_Reset (&design_3_axi_timer_0_Timer, 1);
}

void Tmr0_Cntr_Start() {

	u32 ControlStatusReg;

	/*
	 * Read the current register contents such that only the necessary bits
	 * of the register are modified in the following operations
	 */
	ControlStatusReg = XTmrCtr_ReadReg(design_3_axi_timer_0_Timer.BaseAddress,
					      1, XTC_TCSR_OFFSET);

	/*
	 * Enable All Timers
	 */
	XTmrCtr_WriteReg(design_3_axi_timer_0_Timer.BaseAddress, 1,XTC_TCSR_OFFSET,
			  ControlStatusReg | XTC_CSR_ENABLE_ALL_MASK);
}

void Tmr0_Cntr_Stop(u8 TmrCtrNumber) {

	u32 ControlStatusReg;

	/*
	 * Read the current register contents
	 */
	ControlStatusReg = XTmrCtr_ReadReg(design_3_axi_timer_0_Timer.BaseAddress,
					      TmrCtrNumber, XTC_TCSR_OFFSET);
	/*
	 * Disable the timer counter such that it's not running
	 */
	ControlStatusReg &= ~(XTC_CSR_ENABLE_TMR_MASK);

	/*
	 * Write out the updated value to the actual register.
	 */
	XTmrCtr_WriteReg(design_3_axi_timer_0_Timer.BaseAddress, TmrCtrNumber,
			  XTC_TCSR_OFFSET, ControlStatusReg);
}

void initPlatform() {

        Xil_DCacheDisable();
        Xil_ICacheDisable();

        config_test_regs();
}

void Acknowledge_PClk_Lost_Intr() {
	XIntc_Acknowledge(&intc, PCLK_LOCK_INTR);
}

void Acknowledge_Timer_Intr() {
	XIntc_Acknowledge(&intc, TIMER0_INTR);
}
