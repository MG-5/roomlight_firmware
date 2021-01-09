#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

extern "C" void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr;  /* Link register. */
    volatile uint32_t pc;  /* Program counter. */
    volatile uint32_t psr; /* Program status register. */

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;

    (void)r12;
    (void)lr;
    (void)pc;
    (void)psr;

    /* When the following line is hit, the variables contain the register values. */

    while (1)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);

        for (int i = 0; i < 700'000; ++i)
            asm volatile("nop");
    }
}

extern "C" void hard_fault_handler(void)
{
    /*
     * Get the appropriate stack pointer, depending on our mode,
     * and use it as the parameter to the C handler. This function
     * will never return
     */

    __asm(".syntax unified\n"
          "MOVS   R0, #4  \n"
          "MOV    R1, LR  \n"
          "TST    R0, R1  \n"
          "BEQ    _MSP    \n"
          "MRS    R0, PSP \n"
          "B      prvGetRegistersFromStack      \n"
          "_MSP:  \n"
          "MRS    R0, MSP \n"
          "B      prvGetRegistersFromStack      \n"
          ".syntax divided\n");
}

extern "C" void vApplicationMallocFailedHook(void)
{
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    while (1)
    {
    }
}

extern "C" void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
    (void)pxTask;
    (void)pcTaskName;

    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    while (1)
    {
    }
}