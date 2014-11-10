#include "ttimer.h"
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>
#include "SysProcessor.h"
#include "util.h"

#ifdef PART_TM4C123GH6PGE
    #define SYSCTL_PERIPH_USR_LED   SYSCTL_PERIPH_GPIOG
    #define GPIO_BASE_USR_LED       GPIO_PORTG_BASE
    #define GPIO_PIN_USR_LED        GPIO_PIN_2
#endif
#ifdef PART_TM4C1294NCPDT
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

DWORD testTask( void* lpParam );

struct STGPIOOutputConfig
{
    DWORD dwSYSCTL;
    DWORD dwPortBase;
    DWORD dwPin;
};

struct STUserLedControl
{
    DWORD dwCount;
    DWORD dwID;
    DWORD dwHandle;
};


/*----------------------------------------------------------------------------*/

const struct STGPIOOutputConfig stUserLedCfg[] =
{
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
};

static struct STUserLedControl userLed[4] =
{
    [0]=
    {
        .dwID       = 0,
        .dwCount    = 0,
        .dwID       = 0,
        .dwHandle   = 0,
    },
    [1]=
    {
        .dwID       = 0,
        .dwCount    = 0,
        .dwID       = 0,
        .dwHandle   = 0,
    },
    [2]=
    {
        .dwID       = 0,
        .dwCount    = 0,
        .dwID       = 0,
        .dwHandle   = 0,
    },
    [3]=
    {
        .dwID       = 0,
        .dwCount    = 0,
        .dwID       = 0,
        .dwHandle   = 0,
    },
};

/*----------------------------------------------------------------------------*/

void main()
{
    DWORD dwCounter;
    struct STUserLedControl*            pUserLedControl;
    const struct STGPIOOutputConfig*    pUserLedCfg;
    
    SetSystemClock();

#ifdef PART_TM4C123GH6PGE
    SysCtlPeripheralEnable( SYSCTL_PERIPH_USR_LED );
    GPIOPinTypeGPIOOutput( GPIO_BASE_USR_LED, GPIO_PIN_USR_LED );
#endif
#ifdef PART_TM4C1294NCPDT
    for (  dwCounter = 0, pUserLedCfg = stUserLedCfg; dwCounter < GET_ARRAY_LEN( stUserLedCfg ); dwCounter++, pUserLedCfg++ )
    {
        SysCtlPeripheralEnable( pUserLedCfg->dwSYSCTL );
        GPIOPinTypeGPIOOutput( pUserLedCfg->dwPortBase, pUserLedCfg->dwPin );
    }
#endif
    
    TTimerCfgTimeOut( 500 );
    
#ifdef PART_TM4C123GH6PGE
    TTimerRegisterCallBack( 600*TTIMER_1MS_INTERVAL, TimerPeriodic, testTask, &dwTimerParam, &dwTimerHandle );
    TTimerStart( dwTimerHandle );
#endif
#ifdef PART_TM4C1294NCPDT

    for( dwCounter = 0, pUserLedControl = userLed; dwCounter < GET_ARRAY_LEN( userLed ); dwCounter++, pUserLedControl++ )
    {

        pUserLedControl->dwID = (DWORD)dwCounter;
        TTimerRegisterCallBack( (100*(dwCounter+1))*TTIMER_1MS_INTERVAL,
                                TimerPeriodic,
                                testTask,
                                (void*)dwCounter,
                                &pUserLedControl->dwHandle );
        TTimerStart( pUserLedControl->dwHandle );
    }
    
#endif

    for( ;; );
}

/******************************************************************************/

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
