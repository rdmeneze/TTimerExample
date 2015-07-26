/**
 *  @file   main.c
 *  @brief  main file of TTimerExample project
 *  @author Rafael Dias <rdmeneze@gmail.com>
 *  @date   jul/2015 
 */

#include "ttimer.h"
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>
#include "SysProcessor.h"
#include "util.h"

/* definitions of board leds configurations  */
#ifdef PART_TM4C123GH6PGE

    #define SYSCTL_PERIPH_USR_LED   SYSCTL_PERIPH_GPIOG
    #define GPIO_BASE_USR_LED       GPIO_PORTG_BASE
    #define GPIO_PIN_USR_LED        GPIO_PIN_2

#elif PART_TM4C1294NCPDT

    #define SYSCTL_PERIPH_USR_LED1  SYSCTL_PERIPH_GPION
    #define GPIO_BASE_USR_LED1      GPIO_PORTN_BASE
    #define GPIO_PIN_USR_LED1       GPIO_PIN_1

    #define SYSCTL_PERIPH_USR_LED2  SYSCTL_PERIPH_GPION
    #define GPIO_BASE_USR_LED2      GPIO_PORTN_BASE
    #define GPIO_PIN_USR_LED2       GPIO_PIN_0

    #define SYSCTL_PERIPH_USR_LED3  SYSCTL_PERIPH_GPIOF
    #define GPIO_BASE_USR_LED3      GPIO_PORTF_BASE
    #define GPIO_PIN_USR_LED3       GPIO_PIN_4

    #define SYSCTL_PERIPH_USR_LED4  SYSCTL_PERIPH_GPIOF
    #define GPIO_BASE_USR_LED4      GPIO_PORTF_BASE
    #define GPIO_PIN_USR_LED4       GPIO_PIN_0

#endif

/* local methods declarations                          */

DWORD testTask( void* lpParam );

/*******************************************************/

/* local data type definitions                         */

/**
 *  @struct STGPIOOutputConfig
 *  @brief  struct to represent the GPIO configurations
 */
struct STGPIOOutputConfig
{
    DWORD dwSYSCTL;     /**< the SYSCTL value   */
    DWORD dwPortBase;   /**< the PortBase value */
    DWORD dwPin;        /**< the Pin value      */
};

/**
 *  @struct STUserLedControl
 *  @brief  struct to represent the Led control structure
 */
struct STUserLedControl
{
    DWORD dwCount;      /**< the count variable. Used to control the state of ON/OFF    */
    DWORD dwID;         /**< ID of LED                                                  */
    DWORD dwHandle;     /**< HANDLE of LED                                              */
};

/**********************************************************************************/

/* GPIOs struct configuration                                                     */
const struct STGPIOOutputConfig stUserLedCfg[] = 
{
#ifdef PART_TM4C123GH6PGE
    [0] = 
    {
        .dwSYSCTL   = SYSCTL_PERIPH_USR_LED,
        .dwPortBase = GPIO_BASE_USR_LED,
        .dwPin      = GPIO_PIN_USR_LED,
    }
#elif PART_TM4C1294NCPDT
    [0] = 
    {
        .dwSYSCTL   = SYSCTL_PERIPH_USR_LED1,
        .dwPortBase = GPIO_BASE_USR_LED1,
        .dwPin      = GPIO_PIN_USR_LED1,
    },
    [1]=
    {
        .dwSYSCTL   = SYSCTL_PERIPH_USR_LED2,
        .dwPortBase = GPIO_BASE_USR_LED2,
        .dwPin      = GPIO_PIN_USR_LED2,
    },
    [2]=
    {
        .dwSYSCTL   = SYSCTL_PERIPH_USR_LED3,
        .dwPortBase = GPIO_BASE_USR_LED3,
        .dwPin      = GPIO_PIN_USR_LED3,
    },
    [3]=
    {
        .dwSYSCTL   = SYSCTL_PERIPH_USR_LED4,
        .dwPortBase = GPIO_BASE_USR_LED4,
        .dwPin      = GPIO_PIN_USR_LED4,
    },
#endif    
};

static struct STUserLedControl userLed[] = 
{
#ifdef PART_TM4C123GH6PGE
    [0]=
    {
        .dwID       = 0, 
        .dwCount    = 0,
        .dwHandle   = 0, 
    }
#elif PART_TM4C1294NCPDT
    [0]=
    {
        .dwID       = 0, 
        .dwCount    = 0,
        .dwHandle   = 0, 
    },
    [1]=
    {
        .dwID       = 0, 
        .dwCount    = 0,
        .dwHandle   = 0, 
    },
    [2]=
    {
        .dwID       = 0, 
        .dwCount    = 0,
        .dwHandle   = 0, 
    },
    [3]=
    {
        .dwID       = 0, 
        .dwCount    = 0,
        .dwHandle   = 0, 
    },
#endif
};

/*----------------------------------------------------------------------------*/

/**
 * @brief main function
 * @return none
 */
int main()
{
    BYTE bCounter;
    struct STUserLedControl*            pUserLedControl;
    const struct STGPIOOutputConfig*    pUserLedCfg;
    
    SetSystemClock();  /* configure the system clock  */    

    for (  bCounter = 0, pUserLedCfg = stUserLedCfg; bCounter < GET_ARRAY_LEN( stUserLedCfg ); bCounter++, pUserLedCfg++ )
    {
        SysCtlPeripheralEnable( pUserLedCfg->dwSYSCTL );
        GPIOPinTypeGPIOOutput( pUserLedCfg->dwPortBase, pUserLedCfg->dwPin );
    }
    
    TTimerCfgTimeOut( 500 );  /* initialize the ttimer system  */
    
    for( bCounter = 0, pUserLedControl = userLed; bCounter < GET_ARRAY_LEN( userLed ); bCounter++, pUserLedControl++ )
    {
        /* create a TTimer entry point */
        pUserLedControl->dwID = (DWORD)bCounter;
        TTimerRegisterCallBack( (200*(bCounter+1))*TTIMER_1MS_INTERVAL, 
                                TimerPeriodic, 
                                testTask, 
                                (void*)bCounter, 
                                &pUserLedControl->dwHandle );
        TTimerStart( pUserLedControl->dwHandle );
    }

    for( ;; );
}

/******************************************************************************/

/**
 * @brief testTask worker task
 */
DWORD testTask( void* lpParam )
{
    const DWORD dwIndex                                 = (DWORD)lpParam;
    struct STUserLedControl*            pUserLedControl = &userLed[dwIndex];
    const struct STGPIOOutputConfig*    pUserLedCfg     = &stUserLedCfg[dwIndex];
    
    if( pUserLedControl->dwCount & 1 )
    {
        GPIOPinWrite( pUserLedCfg->dwPortBase, pUserLedCfg->dwPin, pUserLedCfg->dwPin );
    }
    else
    {
        GPIOPinWrite( pUserLedCfg->dwPortBase, pUserLedCfg->dwPin, ~pUserLedCfg->dwPin );
    }
    
    pUserLedControl->dwCount++;

    return 0;
}

/******************************************************************************/
