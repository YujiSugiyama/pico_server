#ifndef __CORE1_H__
#define __CORE1_H__

extern int core1_thrd;
extern mutex_t mtx_core1_thrd;

void core1_entry(void);
void led_process(const char *, int, int);

#endif		// __CORE1_H__
