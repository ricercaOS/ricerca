//! @file local.h
//! @brief File containing declarations of local (per-core) scheduler functions

#pragma once

#include <lib/callback.h>
#include <lib/target.h>
#include <misc/attributes.h>
#include <misc/types.h>
#include <thread/locking/spinlock.h>
#include <thread/tasking/task.h>

//! @brief Local scheduler data
struct thread_localsched_data {
	//! @brief Tasks heap
	struct pairing_heap heap;
	//! @brief Task lock
	struct thread_spinlock lock;
	//! @brief APIC id of the owner
	uint32_t apic_id;
	//! @brief True if CPU is idle (waiting for new tasks to run)
	bool idle;
	//! @brief Number of tasks in the queue
	size_t tasks_count;
	//! @brief Idle unfairness
	uint64_t idle_unfairness;
	//! @brief Current task
	struct thread_task *current_task;
};

//! @brief Initialize local scheduler on this AP
void thread_localsched_init(void);

//! @brief Bootstrap local scheduler on this AP
attribute_noreturn void thread_localsched_bootstrap(void);

//! @brief Associate task with the local scheduler on the given CPU
//! @param logical_id ID of the core
//! @param task Pointer to the task
void thread_localsched_associate(uint32_t logical_id, struct thread_task *task);

//! @brief Yield current task
void thread_localsched_yield(void);

//! @brief Suspend current task
//! @param callback Callback to run in suspend scope
void thread_localsched_suspend_current(struct callback_void callback);

//! @brief Wake up task
//! @param task Pointer to the task to wake up
void thread_localsched_wake_up(struct thread_task *task);

//! @brief Terminate current task
attribute_noreturn void thread_localsched_terminate(void);

//! @brief Get pointer to the current task
//! @return Pointer to the current task
struct thread_task *thread_localsched_get_current_task();

//! @brief Export target for local scheduler initialization
EXPORT_TARGET(thread_localsched_available)
