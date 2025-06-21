#ifndef _SOFT_TASKS_AVR
#define _SOFT_TASKS_AVR

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "prj_com.h"

typedef volatile struct task_descriptor_s task_descriptor_st;
typedef void (*func_p_t) (void);

typedef volatile struct task_descriptor_s
{
	func_p_t _func;
	union {
		struct {
			bool cyclic   : 1;
			bool canc	  : 1;
			bool listed   : 1;
		} fl;
		uint8_t all;
	}_flags;
	bool _locked;
	uint32_t _delay;
	uint32_t _tick_start;
	task_descriptor_st* prew;
	void* cntx;
} task_descriptor_st;

void SoftTaskRun (void);
void SoftTimerTickIrq (void);
bool SoftTaskAdd (uint32_t period, func_p_t func, bool cyclic, volatile task_descriptor_st* descr);

bool SoftTaskCancel (volatile task_descriptor_st* descr);
bool SoftTaskForce (volatile task_descriptor_st* descr);
bool SoftTaskReset (volatile task_descriptor_st* descr);

uint32_t SoftTaskGetLeftTime (volatile task_descriptor_st* descr);
uint32_t SoftTaskGetRemTime (volatile task_descriptor_st* descr);
bool SoftTaskIsActive (volatile task_descriptor_st* descr);
task_descriptor_st* SoftTaskGetSelf (void);

#endif