#include "kernel.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include <cmsis_gcc.h>
#include "core_cm4.h"

#define YIELD 4

uint32_t* MSP_INIT_VAL;

uint32_t* endptr;


thread globalthreadstruct[16];

int thread_counter = 0;
int thread_num = 0;


uint32_t stack_allocater() {

	if (STACK_MAX_SIZE - ((uint32_t)MSP_INIT_VAL - (uint32_t)endptr) < ((uint32_t)THREAD_STACK_SIZE)){
			return NULL;
	}

	if(endptr == 0){
		return NULL;
	}

	endptr = (uint32_t*)((uint32_t)endptr - THREAD_STACK_SIZE);


//	endptr = endptr - THREAD_STACK_SIZE;

	return (uint32_t*)endptr;
}

int osCreateThreadWithDeadline(void (*thread_function)(void*), void* args, int timeslice, int runtime){
	globalthreadstruct[thread_counter].timeslice = timeslice;
	globalthreadstruct[thread_counter].runtime = runtime;

	globalthreadstruct[thread_counter].sp = stack_allocater();

	if (globalthreadstruct[thread_counter].sp == NULL || 16 < thread_counter){
		return 0;
	}

	globalthreadstruct[thread_counter].thread_function = thread_function;

	*(--globalthreadstruct[thread_counter].sp) = 1<<24; //A magic number, this is xPSR
	*(--globalthreadstruct[thread_counter].sp) = (uint32_t)thread_function; //the function name

	*(--globalthreadstruct[thread_counter].sp)=0xA; //lr
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r12
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r3
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r2
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r1

	*(--globalthreadstruct[thread_counter].sp) = (uint32_t)args; //r0

	*(--globalthreadstruct[thread_counter].sp)=0xA; //r11
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r10
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r9
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r8
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r7
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r6
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r5
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r4


	thread_counter++;


	return 1;
}

int osCreateThread (void (*thread_function)(void*), void* args){

	globalthreadstruct[thread_counter].timeslice = 5;
	globalthreadstruct[thread_counter].runtime = 5;

	globalthreadstruct[thread_counter].sp = stack_allocater();

	if (globalthreadstruct[thread_counter].sp == NULL || 16 < thread_counter){
		return 0;
	}

	globalthreadstruct[thread_counter].thread_function = thread_function;

	*(--globalthreadstruct[thread_counter].sp) = 1<<24; //A magic number, this is xPSR
	*(--globalthreadstruct[thread_counter].sp) = (uint32_t)thread_function; //the function name

	*(--globalthreadstruct[thread_counter].sp)=0xA; //lr
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r12
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r3
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r2
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r1

	*(--globalthreadstruct[thread_counter].sp) = (uint32_t)args; //r0

	*(--globalthreadstruct[thread_counter].sp)=0xA; //r11
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r10
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r9
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r8
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r7
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r6
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r5
	*(--globalthreadstruct[thread_counter].sp)=0xA; //r4


	thread_counter++;


	return 1;

}

void osKernelInitialize(){

	thread_counter = 0;
	thread_num = 0;

	MSP_INIT_VAL = *(uint32_t**)0x0;
	endptr = MSP_INIT_VAL;

	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV

}

void osKernelStart(){

//	__set_PSP((uint32_t)(globalthreadstruct[thread_num].sp));
	__asm("SVC #3");
}

void osSched(){

	globalthreadstruct[thread_num].sp = (uint32_t*)(__get_PSP() - 8*4);

	thread_num = (thread_num + 1)%thread_counter;

	globalthreadstruct[thread_num].runtime = globalthreadstruct[thread_num].timeslice;

	__set_PSP(globalthreadstruct[thread_num].sp);

	return;
}

void osYield(void){
	__asm("SVC #4");
}


void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case 0: //17 is sort of arbitrarily chosen
			printf("Success!\r\n");
		break;

		case 1:
			printf("Working.\r\n");
		break;

		case 2:
			printf("Exiting now.\r\n");
		break;

		case 0x3:
			__set_PSP(globalthreadstruct[thread_num].sp);
			runFirstThread();
			break;

		case YIELD:
		//Pend an interrupt to do the context switch
		_ICSR |= 1<<28;
		__asm("isb");
		break;

		default: /* unknown SVC */
		break;
	}
}
