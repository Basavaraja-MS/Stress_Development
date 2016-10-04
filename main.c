#include <stdio.h>
#include <stdint.h>
#include "xstatus.h"
#include "register.h"
#include "xil_io.h"

extern void initPlatform();
extern void cleanupPlatform();

extern uint32_t gicConfigure();
extern uint32_t gicStop();
extern uint32_t InterruptConfig();

extern uint32_t TmrConfigure();

extern void Timer0_Cntr_Settings();
extern void Tmr0_Cntr_Start();

extern void initPlatform();
extern void localIrqDisable();
extern void localIrqEnable();

#if 0
int main()
{
	unsigned int ltssm = 0;
	u32 TEST_ITER = 0;
	u32 ITER_NO = 1;
	unsigned int ltssm_status, pclk_lock_status, phystatus_status;

	initPlatform();
	/*Platform initialization*/
	//printf("Start Platform initialization\n");
start:

	if (gicConfigure() == XST_FAILURE) {
		//printf("GIC Error\n");
		while (1);
	}

	if (TmrConfigure() == XST_FAILURE) {
		//printf("Timer Error\n");
		while (1);
	}

	localIrqDisable();
    
	if(InterruptConfig(NULL)== XST_FAILURE) {
		//printf("GIC Controller Initialization Error\n");
		goto start;
	}
	
	localIrqEnable();
	
	//printf("Platform Initialization Done\n");

	printf("\nRun the Test Script from Lecroy\n");

	//printf("\n\nTest_Iter : Pass/Fail \n\n");

	TEST_ITER = Xil_In32(APB2GPIO_BASE + ITER_CNT);
	printf("%d\n", TEST_ITER);

	while (TEST_ITER >= ITER_NO)
	{
		//Set the Timer0 - Counters to their Respective Settings
		Timer0_Cntr_Settings();

		//Enable Timer0 - Counters
		Tmr0_Cntr_Start();

		while(Xil_In32(APB2GPIO_BASE + LTSSM_U0_U1) != 0x01) {}

	    printf("EnterU1\n");
	    Xil_Out32((APB2GPIO_BASE + LTSSM_SUCCESS), 0x01);
	    //Xil_Out32((APB2GPIO_BASE + LTSSM_U0_U1), 0x00);

	    while(Xil_In32(APB2GPIO_BASE + LTSSM_U1_EXIT) != 0x01) {}

	    printf("ExitU1\n");
	    //Xil_Out32((APB2GPIO_BASE + LTSSM_U1_EXIT), 0x00);

	    ltssm_status = Xil_In32(APB2GPIO_BASE + LTSSM_SUCCESS);
	    phystatus_status = Xil_In32(APB2GPIO_BASE + PHYSTATUS_SUCCESS);
	    pclk_lock_status = Xil_In32(APB2GPIO_BASE + PCLK_LOCK_LOST);
   		//printf("a: %d\n",phystatus_status);

   		if ((ltssm_status & phystatus_status & !pclk_lock_status) == 0x1)
	    	{
	    		//printf("%d:P\n\n",ITER_NO);
	    		Xil_Out32((APB2GPIO_BASE + LED), 0x0F);
	    	}
	    else
	    	{
	    		printf("%d:F - %d,%d,%d\n\n",ITER_NO,ltssm_status,phystatus_status,!pclk_lock_status);
	    		Xil_Out32((APB2GPIO_BASE + LED), 0xF0);
	    		break;
	    	}

	   	//printf("RESET ITER\n");

   		ITER_NO = ITER_NO + 1;
	    Xil_Out32((APB2GPIO_BASE + LTSSM_SUCCESS), 0x00);
	    Xil_Out32((APB2GPIO_BASE + PHYSTATUS_SUCCESS), 0x00);
	    Xil_Out32((APB2GPIO_BASE + PCLK_LOCK_LOST), 0x00);
	    Xil_Out32((APB2GPIO_BASE + LTSSM_U0_U1), 0x00);
	    Xil_Out32((APB2GPIO_BASE + LTSSM_U1_EXIT), 0x00);

	}
	printf("%d:P\n\n",ITER_NO);
	printf("%d Iterations Done\n", TEST_ITER);

	gicStop();

	return 0;
}
#else
//#include <stdio.h>
//#include <stdint.h>
#include <stdlib.h>

#define TRUE    1
#define FALSE   0

struct usb_data {
        uint8_t LTSM_Status:1;
        uint8_t PHY_Status:1;
        uint8_t PCLOCK_Status:1;
};

struct TestResult{
        uint8_t dataValid :1;
        uint32_t iterationCount:24;
        uint8_t testLayer:2;
        uint8_t testNumberMain:6;
        uint8_t testNumberSubTest:4;
        uint8_t testStatus:1;
        struct usb_data data;
        uint16_t  timerValue;
}iTestResult;


void duble_pointer_memory(int rowCount){
        struct TestResult **TestResults;

        //DDR part
        struct TestResult **VerifyResults;

        int  i;

        TestResults = (struct TestResult **)malloc(sizeof(struct TestResult *) * rowCount);
        if (TestResults == NULL)
        	printf ("Error in the malloc in 1d\n");
        for (i =0; i < rowCount; i++){
                TestResults[i] = (struct TestResult *)malloc(sizeof(struct TestResult));
                if (TestResults[i] == NULL)
                	printf("Error in malloc of %d in 2d\n", i);
        }


        //Test
        for(i = 0; i < rowCount; i++){
                TestResults[i]->dataValid= TRUE;
                TestResults[i]->iterationCount=6789;
                TestResults[i]->testLayer= 1;
                TestResults[i]->testNumberMain=3;
                TestResults[i]->testNumberSubTest=2;
                TestResults[i]->testStatus=TRUE;
                TestResults[i]->data.LTSM_Status=TRUE;
                TestResults[i]->data.PHY_Status=FALSE;
                TestResults[i]->data.PCLOCK_Status=TRUE;
                TestResults[i]->timerValue=1234;
        }




        //Retrive
        VerifyResults = TestResults;

        for(i = 0; i < rowCount; i++){
                printf ("%d\n", VerifyResults[i]->dataValid);
                printf ("%d\n", VerifyResults[i]->iterationCount);
                printf ("%d\n", VerifyResults[i]->testLayer);
                printf ("%d\n", VerifyResults[i]->testNumberMain);
                printf ("%d\n", VerifyResults[i]->testNumberSubTest);
                printf ("%d\n", VerifyResults[i]->testStatus);
                printf ("%d\n", VerifyResults[i]->data.LTSM_Status);
                printf ("%d\n", VerifyResults[i]->data.PHY_Status);
                printf ("%d\n", VerifyResults[i]->data.PCLOCK_Status);
                printf ("%d\n", VerifyResults[i]->timerValue);
        }



        for (i =0; i < rowCount; i++)
                free(TestResults[i]);

        free(TestResults);



}


void single_pointer_memory(int rowCount){
        int i;
        struct TestResult *TestResults;
        TestResults = (struct TestResult *)malloc( rowCount * sizeof(struct TestResult));

         for (i = 0; i <  rowCount; i++){
                TestResults[i].dataValid= TRUE;
                TestResults[i].iterationCount=6789;
                TestResults[i].testLayer= 1;
                TestResults[i].testNumberMain=3;
                TestResults[i].testNumberSubTest=2;
                TestResults[i].testStatus=TRUE;
                TestResults[i].data.LTSM_Status=TRUE;
                TestResults[i].data.PHY_Status=FALSE;
                TestResults[i].data.PCLOCK_Status=TRUE;
                TestResults[i].timerValue=1234;
        }
        for (i =0; i < rowCount; i++) {
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].dataValid);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].iterationCount);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].testLayer);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].testNumberMain);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].testNumberSubTest);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].testStatus);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].data.LTSM_Status);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].data.PHY_Status);
			printf ("Addr %u - Value %d\n", &TestResults[i], TestResults[i].data.PCLOCK_Status);
			printf ("Addr %u - Value %d\n", &TestResults[i].timerValue, TestResults[i].timerValue);
        }
}


int main ( ) {
        int buffer_count = 1;

#if 0
        struct TestResult *Heap_TestResult;
        //Heap_TestResult = (struct TestResult *) malloc(sizeof (Heap_TestResult) * buffer_count);

        printf ("Size of structure %lu\n", sizeof (*Heap_TestResult));
        printf ("Size of structure %lu\n", sizeof (struct usb_data));
        Heap_TestResult->dataValid= TRUE;
        Heap_TestResult->iterationCount=6789;
        Heap_TestResult->testLayer= 1;
        Heap_TestResult->testNumberMain=3;
        Heap_TestResult->testNumberSubTest=2;
        Heap_TestResult->testStatus=TRUE;
        Heap_TestResult->data.LTSM_Status=TRUE;
        Heap_TestResult->data.PHY_Status=FALSE;
        Heap_TestResult->data.PCLOCK_Status=TRUE;
        Heap_TestResult->timerValue=1234;


        printf ("%d\n", Heap_TestResult->dataValid);
        printf ("%d\n", Heap_TestResult->iterationCount);
        printf ("%d\n", Heap_TestResult->testLayer);
        printf ("%d\n", Heap_TestResult->testNumberMain);
        printf ("%d\n", Heap_TestResult->testNumberSubTest);
        printf ("%d\n", Heap_TestResult->testStatus);
        printf ("%d\n", Heap_TestResult->data.LTSM_Status);
        printf ("%d\n", Heap_TestResult->data.PHY_Status);
        printf ("%d\n", Heap_TestResult->data.PCLOCK_Status);
        printf ("%d\n", Heap_TestResult->timerValue);

        free(Heap_TestResult);


        duble_pointer_memory(0x3);
#endif
}


#endif
