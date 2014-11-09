#include "ttimer.h"
#include "util.h"
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <inc/hw_memmap.h>
#include "errno.h"

/*----------------------------------------------------------------------------*/

#define TIMER_ENTRY_CB      16
#define TIMER_ENTRY_NULL    0

#define TIMER_TIMEBASE_us (SysCtlClockGet() / 1000000.)
#define TIMER_TIMEBASE_ms (SysCtlClockGet() / 1000.)

#define TIMER_SYSCTL    SYSCTL_PERIPH_TIMER0
#define TIMER_BASE      TIMER0_BASE

#define TIMER_HANDLE_INITIAL_VALUE  (0xEEDF)

/*----------------------------------------------------------------------------*/

struct STTimer{
   DWORD    dwHandle;
   long long    iCount;
   long long    iReloadValue;
   TimerType    type;
   void*    lpParam;
   BYTE     bStarted;
   callbacktimer_func   callback_func;
};

/*----------------------------------------------------------------------------*/
//! local declarations
static  DWORD dwTimerTimeBase = 0;
static  BYTE  bInit = 0;
static DWORD  dwHandleCount = 0;
static struct STTimer stCBTimer[TIMER_ENTRY_CB];
static BYTE   bInstanceCounter = 0;
/*----------------------------------------------------------------------------*/
//! timer ISR
void Timer_ISR_Handler( void );
/*----------------------------------------------------------------------------*/

void TTimerCfgTimeOut( DWORD dwTimeMicro )
{
    DWORD dwCountValue;
    
    if ( dwTimeMicro < 10 )
    {
        dwTimeMicro = 10;
    }
    
    dwCountValue = (DWORD)((dwTimeMicro * TIMER_TIMEBASE_us)+0.5);

    if ( !bInit )
    {
        DWORD dwCounter = 0;
        struct STTimer * pTTimer = &stCBTimer[0];
        
        for( ; dwCounter < GET_ARRAY_LEN( stCBTimer ); dwCounter++, pTTimer++ )
        {
            pTTimer->dwHandle       = TIMER_ENTRY_NULL;
            pTTimer->iCount         = 0;
            pTTimer->iReloadValue   = 0;
            pTTimer->type           = TimerOneShot;
            pTTimer->lpParam        = NULL;
            pTTimer->callback_func  = NULL;
            pTTimer->bStarted       = 0;
        }    

        SysCtlPeripheralEnable( TIMER_SYSCTL );
        TimerConfigure(TIMER_BASE, TIMER_CFG_PERIODIC);
        TimerLoadSet( TIMER_BASE, TIMER_A, dwCountValue );
        TimerIntRegister( TIMER_BASE, TIMER_A, Timer_ISR_Handler );
        TimerIntClear( TIMER_BASE, TIMER_TIMA_TIMEOUT );
        TimerIntEnable( TIMER_BASE, TIMER_TIMA_TIMEOUT );
        TimerEnable(TIMER_BASE, TIMER_A);
        
        bInit = 1;
    }
   
    dwTimerTimeBase = dwTimeMicro;
    
    return;
}

/*----------------------------------------------------------------------------*/

DWORD TTimerGetTimeBase( void )
{
    return dwTimerTimeBase;
}

/*----------------------------------------------------------------------------*/

DWORD TTimerRegisterCallBack( DWORD dwDelay, TimerType type, callbacktimer_func callback_func, void* lpParam, DWORD* cbHandle )
{
    int i;
    int count;
    struct STTimer * pTTimer;
    
    if ( !bInit )
        return EPERM;
    
    if ( callback_func == NULL )
        return EPERM;
    
    if ( bInstanceCounter++ > TIMER_ENTRY_CB )
        return ENOMEM;
    
    if ( dwDelay < dwTimerTimeBase )
        dwDelay = dwTimerTimeBase;
    
    count = (DWORD)((dwDelay / (float)dwTimerTimeBase) + 0.5);
    
    for ( pTTimer = stCBTimer, i = 0; i < GET_ARRAY_LEN( stCBTimer ); i++, pTTimer++ )
    {
        if ( pTTimer->dwHandle == TIMER_ENTRY_NULL )
        {
            pTTimer->dwHandle       = (TIMER_HANDLE_INITIAL_VALUE) + (++dwHandleCount);
            pTTimer->iReloadValue   = count;            
            pTTimer->iCount         = pTTimer->iReloadValue;
            pTTimer->type           = (TimerType)type;
            pTTimer->lpParam        = lpParam;
            pTTimer->callback_func  = callback_func;
            pTTimer->bStarted       = 0;
            
            break;
        }
    }
    
    if ( i == GET_ARRAY_LEN( stCBTimer ) )
    {
        return EFAULT;
    }
    
    if ( cbHandle != NULL )
    {
        *cbHandle = pTTimer->dwHandle;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------*/

DWORD TTimerUnregisterCallBack( DWORD dwHandle )
{
    int i;
    struct STTimer * pTTimer;
    
    for ( pTTimer = stCBTimer, i = 0; i < GET_ARRAY_LEN( stCBTimer ); i++, pTTimer++ )
    {
        if ( pTTimer->dwHandle == dwHandle )
        {
            pTTimer->dwHandle       = TIMER_ENTRY_NULL;
            pTTimer->iCount        = 0;
            pTTimer->iReloadValue  = 0;
            pTTimer->type           = TimerOneShot;
            pTTimer->lpParam        = NULL;
            pTTimer->callback_func  = NULL;
            pTTimer->bStarted       = 0;
            
            break;
        }
    }
    
    if ( i == GET_ARRAY_LEN( stCBTimer ) )
    {
        return EFAULT;
    }
    
    bInstanceCounter--;
    
    return 0;
}

/*----------------------------------------------------------------------------*/

void Timer_ISR_Handler( void )
{
    int i;
    struct STTimer * pTTimer;
    DWORD dwExecuteCBFlag = 0;
    DWORD dwCBRet = 0;
    
	for ( i = 0, pTTimer = stCBTimer; i < GET_ARRAY_LEN( stCBTimer ); i++, pTTimer++ )
    {
        if ( pTTimer->dwHandle != TIMER_ENTRY_NULL )
        {
            if ( pTTimer->bStarted )
            {
                pTTimer->iCount--;
                if ( !dwExecuteCBFlag )
                {
                    if ( pTTimer->iCount <= 0 )
                    {
                        if ( pTTimer->callback_func != NULL )
                        {
                            dwExecuteCBFlag = 1;
                            
                            dwCBRet = pTTimer->callback_func( pTTimer->lpParam );	// execute the callback code
                            
                            if ( dwCBRet )
                            {
                                // an error ocurred in the call back execution
                                TTimerUnregisterCallBack( pTTimer->dwHandle );
                                continue;
                            }
                            
                            if ( pTTimer->type == TimerPeriodic )
                            {
                                pTTimer->iCount = pTTimer->iReloadValue;
                            }
                            else
                            {
                                pTTimer->bStarted = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    
    TimerIntClear( TIMER_BASE, TIMER_TIMA_TIMEOUT );
    
    return;
}

/*----------------------------------------------------------------------------*/

DWORD TTimerStart( DWORD dwHandle )
{
    int i;
    struct STTimer* pTTimer;
    
    for ( i = 0, pTTimer = stCBTimer; i < GET_ARRAY_LEN( stCBTimer ); i++, pTTimer++ )
    {
        if ( pTTimer->dwHandle == dwHandle )
        {
            pTTimer->iCount    = pTTimer->iReloadValue;
            pTTimer->bStarted  = 1;
            break;
        }
    }
    
    if ( i == GET_ARRAY_LEN( stCBTimer ) )
    {
        return -1;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------*/        

DWORD TTimerStop( DWORD dwHandle )
{
    int i;
    struct STTimer* pTTimer;
    
    for ( i = 0, pTTimer = stCBTimer; i < GET_ARRAY_LEN( stCBTimer ); i++, pTTimer++ )
    {
        if ( pTTimer->dwHandle == dwHandle )
        {
            pTTimer->bStarted  = 0;
            break;
        }
    }
    
    if ( i == GET_ARRAY_LEN( stCBTimer ) )
    {
        return -1;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------*/        

DWORD TTimerRestart( DWORD dwHandle )
{
    DWORD dwRet = TTimerStop( dwHandle );
    
    if ( !dwRet )
    {
        dwRet = TTimerStart( dwHandle );
    }
    return dwRet;
}

/*----------------------------------------------------------------------------*/        
