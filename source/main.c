#include "ttimer.h"
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>

DWORD testTask( void* lpParam );

volatile unsigned long g_ulSystemClock;

void main()
{
    //! handler para o timer
    DWORD dwTimerHandle;
    
    //! parâmetro a ser passada para a função de callback
    DWORD dwTimerParam = 1000;

    //! configura o clock do processador
    SysCtlClockSet( SYSCTL_SYSDIV_3     | 
                    SYSCTL_USE_PLL      | 
                    SYSCTL_OSC_MAIN     | 
                    SYSCTL_XTAL_16MHZ   );
    g_ulSystemClock = SysCtlClockGet();
    
    //! específico para os dispositivos TIVA: habilita  e configura o periférico
    SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOG );
    GPIOPinTypeGPIOOutput( GPIO_PORTG_BASE, GPIO_PIN_2 );

    //! inicializa o timer para gerar interrupção a cada 500us
    TTimerCfgTimeOut( 500 );
    
    //! registra a função de callback testTask para ser executada a cada 250 ms
    TTimerRegisterCallBack( 250*TTIMER_1MS_INTERVAL , 
                            TimerPeriodic           , 
                            testTask                , 
                            &dwTimerParam           , 
                            &dwTimerHandle          );
    
    //! inicia o TTimer dado por dwTimerHandle
    TTimerStart( dwTimerHandle );

    for( ;; );
}

/******************************************************************************/

DWORD testTask( void* lpParam )
{
    DWORD* pVal = (DWORD*)lpParam;
    static DWORD dwCount = 0;

    if( dwCount & 1 )
    {
        GPIOPinWrite( GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2 );
    }
    else
    {
        GPIOPinWrite( GPIO_PORTG_BASE, GPIO_PIN_2, ~GPIO_PIN_2 );
    }
    
    dwCount++;
    *pVal = dwCount;

    return 0;
}

/******************************************************************************/