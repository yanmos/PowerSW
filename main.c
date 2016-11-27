/*
    proj.name:      Power Switch
    cir.diagram:    Rev 1
    MCU:            AT90S2323, 4MHz
*/
#include <avr/io.h>
#include <avr/interrupt.h>

#define TRUE                    (1)
#define FALSE                   (0)
#define TIMEOUT                 (5000)              /* [ms] */
#define SW_PORT                 (PORTB)
#define SW_BIT                  (2)
#define RELOAD_CNT              (256 - 63 + 1)      /* 63*64/4MHz=1.008ms */
#define enable_timerint()       (TIMSK = 0x02)
#define disable_timerint()      (TIMSK = 0x00)
#define turn_off_sw()           (SW_PORT |= _BV(SW_BIT))    /* Hi */
#define turn_on_sw()            (SW_PORT &= ~_BV(SW_BIT))   /* Lo */

typedef unsigned char   uint8;
typedef unsigned int    uint16;

int main(void);
void exec_cmd(void);
uint16 get_count1ms(void);
void init_MCU(void);

volatile uint16 Count1ms;
volatile uint8  Flag1ms;

int main(void)
{
    init_MCU();
    Count1ms = 0;
    Flag1ms = FALSE;
    exec_cmd();             /* Init */
    enable_timerint();
    sei();
    while(TRUE) {
        if(Flag1ms) {
            Flag1ms = FALSE;
            exec_cmd();
        }
    }
    return(1);
}

void exec_cmd(void)
{
    enum state_list { CMD_INIT, CMD_WAIT, CMD_ON, CMD_OFF };

    static enum state_list  state = CMD_INIT;
    static uint16   timer;

    switch(state) {
        case CMD_INIT:
            timer = get_count1ms();
            state = CMD_WAIT;
            break;
        case CMD_WAIT:
            if((uint16)(get_count1ms() - timer) > TIMEOUT) {
                turn_on_sw();
                state = CMD_ON;
            }
            break;
        case CMD_ON:
            break;
        default:
            break;
    }
}

uint16 get_count1ms(void)
{
	uint16  count;

	cli();
	count = Count1ms;
	sei();
	return(count);
}

void init_MCU(void)
{
    /* MCU controll */
    MCUCR = 0x00;               /* 00000000 */
    
    /* port */
    PORTB = 0x04;               /* 00000100 B2=~FETgate */
    DDRB = 0x07;                /* 00000111 B2=~FETgate */

    /* 8bit timer 0 */
    TIMSK = 0x02;               /* enable TOIE0 */
    TIFR = 0x00;                /* clear TOV0 */
    TCNT0 = RELOAD_CNT;
    TCCR0 = 0x03;               /* 00000011 f=CK/64 */
}

SIGNAL(SIG_OVERFLOW0)           /* 1ms interval */
{
    TCNT0 = RELOAD_CNT;
    Flag1ms = TRUE;
    Count1ms++;
}
