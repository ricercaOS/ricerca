//! @file task.h
//! @brief File containing definition of a task structure

#pragma once

#include <misc/types.h>
#include <sys/arch/interrupts.h>

struct thread_task {
	//! @brief General registers
	struct interrupt_frame frame;
	//! @brief Next task to run
	struct thread_task *next;
	//! @brief Task stack
	uintptr_t stack;
	//! @brief ID of the core task was allocated to
	uint32_t core_id;
};
