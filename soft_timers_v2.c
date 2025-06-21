#include "soft_timers.h"

#include <stdio.h>

static task_descriptor_st *_last, *_next, *_now;
static uint32_t _system_tick;

static inline void UnlistTask (task_descriptor_st* unlisted)
{
	bool irq;
	_GINT_DIS(irq);
	if(_next){ // This is last task
		_next->prew = unlisted->prew;
	} else {
		_last = unlisted->prew;
	}
	_GINT_EN(irq);
	unlisted->_flags.all = 0;
}

void SoftTimerTickIrq (void)
{
	_system_tick++;
}

bool SoftTaskCancel (volatile task_descriptor_st* descr)
{
	if(descr){
		descr->_flags.fl.canc = true;
		return true;
	}
	return false;
}

bool SoftTaskAdd (uint32_t period, func_p_t func, bool cyclic, volatile task_descriptor_st* descr)
{
	if(period && func && descr && !descr->_locked){
		descr->_locked = true;
		descr->_flags.fl.canc = false;
		descr->_flags.fl.cyclic = cyclic;
		descr->_func = func;
		descr->_delay = period;
		bool irq;
		_GINT_DIS(irq);
		descr->_tick_start = _system_tick;
		if(!descr->_flags.fl.listed){
			descr->_flags.fl.listed = true;
			descr->prew = _last;
			_last = descr;
			if(!_next){
				_next = descr;
			}
		}
		_GINT_EN(irq);
		descr->_locked = false;
		return true;
	}
	return false;
}

void SoftTaskRun (void)
{
	bool irq;
	_GINT_DIS(irq);
	_next = NULL;
	_now = _last;
	uint32_t system_tick_local = _system_tick;
	_GINT_EN(irq);
	while(_now){
		_now->_locked = true; // Must be atomic
		bool callhandler = false;
		do{
			if(_now->_flags.fl.canc){
				UnlistTask(_now);
				break;
			} 
			if((system_tick_local - _now->_tick_start) >= _now->_delay){
				callhandler = true;
				if(_now->_flags.fl.cyclic){
					_now->_tick_start = system_tick_local;
				} else {
					UnlistTask(_now);
					break;
				}
			}	
			_GINT_DIS(irq);
			_next = _now;
			_GINT_EN(irq);
		} while (0);
		task_descriptor_st* prew = _now->prew;
		_now->_locked = false;
		if(callhandler){
			_now->_func();
		}
		_now = prew;
	}
}

task_descriptor_st* SoftTaskGetSelf (void)
{
	return _now;
}

uint32_t SoftTaskGetRemTime (volatile task_descriptor_st* descr)
{
	if(descr && !descr->_locked){
		bool irq;
		_GINT_DIS(irq);
		uint32_t time;
		if((descr->_flags.fl.listed) && (!descr->_flags.fl.canc)){
			time = _system_tick - descr->_tick_start;
		} else {
			time = descr->_delay;
		}
		_GINT_EN(irq);
		return time;
	}
	return -1;
}

uint32_t SoftTaskGetLeftTime (volatile task_descriptor_st* descr)
{
	if(descr && !descr->_locked){
		bool irq;
		_GINT_DIS(irq);
		uint32_t time;
		if((descr->_flags.fl.listed) && (!descr->_flags.fl.canc)){
			time = descr->_delay - (_system_tick - descr->_tick_start);
		} else {
			time = 0;
		}
		_GINT_EN(irq);
		return time;
	}
	return -1;
}

bool SoftTaskIsActive (volatile task_descriptor_st* descr)
{
	if(descr && !descr->_locked){
		bool irq;
		_GINT_DIS(irq);
		bool act = (bool)descr->_flags.fl.listed && (!(bool)descr->_flags.fl.canc);
		_GINT_EN(irq);
		return act;
	}
	return true;
}

bool SoftTaskForce (volatile task_descriptor_st* descr)
{
	if(descr && !descr->_locked){
		bool irq;
		_GINT_DIS(irq);
		descr->_tick_start -= descr->_delay;
		_GINT_EN(irq);
		return true;
	}
	return false;
}

bool SoftTaskReset (volatile task_descriptor_st* descr)
{
	if(descr && !descr->_locked){
		bool irq;
		_GINT_DIS(irq);
		descr->_tick_start = _system_tick;
		_GINT_EN(irq);
		return true;
	}
	return false;
}
