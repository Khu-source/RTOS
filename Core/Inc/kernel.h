#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>
#include "stm32f4xx_hal.h"

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV



#define THREAD_STACK_SIZE 0x400
#define STACK_MAX_SIZE 0x4000

extern uint32_t* MSP_INIT_VAL;

extern uint32_t* endptr;



uint32_t stack_allocater();


extern int osCreateThread (void (*thread_function)(void*), void* args);

extern void osKernelInitialize();

extern void osKernelStart();

void SVC_Handler_Main( unsigned int *svc_args );


typedef struct k_thread{
	uint32_t* sp; //stack pointer
	int timeslice;
	int runtime;
	void (*thread_function)(void*); //function pointer

}thread;


extern thread globalthreadstruct[16];

void osSched();

void osYield(void);

int osCreateThreadWithDeadline(void (*thread_function)(void*), void* args, int timeslice, int runtime);


#endif

