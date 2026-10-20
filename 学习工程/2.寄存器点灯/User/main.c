#include "led.h"
#include "key.h"

int main(void)
{
	LED_Init ();
	KEY_Init ();
	
    while (1)
    {
		if(KEY_Scan ()==1)
		{
			LED_Toggle ();
		}
    }
}
