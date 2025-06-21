#include "soft_timers.h"

task_descriptor_st blink_descr = { 0 };

void blink_handler (void)
{
	static int tog_counter = 0;
	toggle_led(&led1);
	if(--tog_counter == 6){
		task_descriptor_st* this_task = SoftTaskGetSelf();
		SoftTaskCancel(this_task);
	}
}

ISR (TIMER0_COMPA_vect)
{ // 1ms Period
	SoftTimerTickIrq();
}

int main(void)
{
	SoftTaskAdd(500, blink_handler, true, &blink_descr);
	while(1){
		SoftTaskRun();
	}
}