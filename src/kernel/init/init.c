//! @file init.c
//! @brief File containing kernel entrypoint and kernel init functions

#include <drivers/output/e9/e9.h>
#include <drivers/output/stivale2/stivale2.h>
#include <init/init.h>
#include <init/stivale2.h>
#include <lib/log.h>
#include <lib/panic.h>
#include <lib/profiling.h>
#include <lib/target.h>
#include <mem/heap/heap.h>
#include <mem/mem.h>
#include <misc/misc.h>
#include <sys/arch/interrupts.h>
#include <sys/ic.h>
#include <test/tests.h>
#include <thread/smp/core.h>
#include <thread/tasking/balancer.h>
#include <thread/tasking/localsched.h>
#include <thread/tasking/task.h>
#include <thread/tasking/tasking.h>
#include <user/target.h>

MODULE("init")

//! @brief True if stivale2 terminal was registered
static bool kernel_stivale2_term_loaded = false;

//! @brief Stack used by kernel on bootstrap
static uint8_t kernel_stack[65536];

//! @brief RSDP tag or NULL if not found
struct stivale2_struct_tag_rsdp *init_rsdp_tag;

//! @brief Memory map tag or NULL if not found
struct stivale2_struct_tag_memmap *init_memmap_tag;

//! @brief Modules tag or NULL if not found
struct stivale2_struct_tag_modules *init_modules_tag;

//! @brief Stivale2 5-level paging tag
static struct stivale2_tag stivale2_5lvl_paging_tag = {
    .identifier = STIVALE2_HEADER_TAG_5LV_PAGING_ID,
    .next = 0,
};

//! @brief Stivale2 framebuffer tag
static struct stivale2_header_tag_framebuffer stivale2_fb_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
            .next = (uint64_t)&stivale2_5lvl_paging_tag},
    .framebuffer_height = 0,
    .framebuffer_width = 0,
    .framebuffer_bpp = 0,
};

//! @brief Stivale2 terminal tag
static struct stivale2_header_tag_terminal stivale2_term_tag = {
    .tag =
        {
            .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
            .next = (uint64_t)&stivale2_fb_tag,
        },
    .flags = 0};

//! @brief Stivale2 header
//! @note This part is taken from limine-bootloader/limine-barebones
__attribute__((section(".stivale2hdr"), used)) struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)kernel_stack + sizeof(kernel_stack),
    .flags = 0b10,
    .tags = (uint64_t)&stivale2_term_tag,
};

//! @brief Query stivale for the tag
//! @param info Virtual pointer to stivale2 info structure
//! @param id Tag ID
struct stivale2_tag *stivale2_query(struct stivale2_struct *info, uint64_t id) {
	struct stivale2_tag *tag = (struct stivale2_tag *)info->tags;
	while (tag != NULL) {
		if (tag->identifier == id) {
			return tag;
		}
		tag = (struct stivale2_tag *)tag->next;
	}
	return tag;
}

//! @brief Try to initialize stivale2 terminal
//! @param info Info passed from bootloader
void kernel_load_stivale2_term(struct stivale2_struct *info) {
	struct stivale2_struct_tag_terminal *term_tag =
	    (struct stivale2_struct_tag_terminal *)stivale2_query(info,
	                                                          STIVALE2_STRUCT_TAG_TERMINAL_ID);
	if (term_tag != NULL) {
		kernel_stivale2_term_loaded = true;
		stivale2_term_register(term_tag);
		LOG_SUCCESS("Stivale2 terminal registered!");
	} else {
		LOG_WARN("Stivale 2 terminal was not found!");
	}
}

//! @brief Unregister stivale2 terminal if it was loaded
void kernel_unload_stivale2_term(void) {
	if (kernel_stivale2_term_loaded) {
		stivale2_term_unregister();
		LOG_SUCCESS("Stivale2 terminal unregistered!");
	}
}

//! @brief Kernel init stage2
void kernel_init_stage2() {
	LOG_SUCCESS("Running in stage 2");
	tests_run();
	hang();
}

//! @brief Kernel entrypoint
//! @param info Info passed from bootloader
void kernel_init(struct stivale2_struct *info) {
	// Load e9 logging subsystem
	e9_register();
	// Attempt to initialize stivale2 terminal
	kernel_load_stivale2_term(info);

	// Attempt to load some stivale2 tags
	init_rsdp_tag =
	    (struct stivale2_struct_tag_rsdp *)stivale2_query(info, STIVALE2_STRUCT_TAG_RSDP_ID);

	init_memmap_tag =
	    (struct stivale2_struct_tag_memmap *)stivale2_query(info, STIVALE2_STRUCT_TAG_MEMMAP_ID);

	init_modules_tag =
	    (struct stivale2_struct_tag_modules *)stivale2_query(info, STIVALE2_STRUCT_TAG_MODULES_ID);

	// If we are running PROFILE kernel, initialize performance counters subsystems first
#ifdef PROFILE
	// Compute init plan to initialize profiling
	struct target *profile_plan = target_compute_plan(profiling_available);

	// Execute plan
	target_plan_dump(profile_plan);
	target_execute_plan(profile_plan);

#endif
	// Compute init plan to initialize userspace
	struct target *plan = target_compute_plan(userspace_available);

	// Execute plan
	target_plan_dump(plan);
	target_execute_plan(plan);

	// Initialize local scheduler on BSP
	thread_localsched_init();

	// Create stage 2 task
	struct thread_task *stage2_task =
	    thread_task_create_call(CALLBACK_VOID(kernel_init_stage2, NULL));
	if (stage2_task == NULL) {
		PANIC("Failed to allocate stage2 task");
	}
	thread_balancer_allocate_to_any(stage2_task);

	// Bootstrap local scheduler
	thread_localsched_bootstrap();
	// Scheduler should have picked this new task
	UNREACHABLE;
}
