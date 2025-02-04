//! @file mem.c
//! @brief Implementation of memory management initialization

#include <init/init.h>
#include <lib/log.h>
#include <lib/panic.h>
#include <mem/bootstrap.h>
#include <mem/heap/heap.h>
#include <mem/mem.h>
#include <mem/misc.h>
#include <mem/phys/phys.h>
#include <sys/acpi/numa.h>
#include <sys/numa/numa.h>

MODULE("mem");
TARGET(mem_add_numa_ranges_available, mem_add_numa_ranges,
       {numa_available, acpi_numa_available, mem_bootstrap_alloc_available})
TARGET(mem_all_available, META_DUMMY, {mem_phys_available, mem_heap_available})
META_DEFINE_DUMMY()

//! @brief Estimate maximum number of memory region entries
//! @note This function does a pivot parsing pass that effectively duplicates that in
//! mem_add_numa_ranges
static size_t mem_estimate_boot_ranges_upper_bound(void) {
	size_t result = 0;
	struct stivale2_struct_tag_memmap *memmap = init_memmap_tag;
	// Enumerate all usable memory map entries
	for (size_t i = 0; i < memmap->entries; ++i) {
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) {
			continue;
		}
		uintptr_t start = memmap->memmap[i].base;
		const uintptr_t end = start + memmap->memmap[i].length;
		struct acpi_numa_phys_range_iter iter = ACPI_NUMA_PHYS_RANGE_ITER_INIT(start, end);
		struct acpi_numa_memory_range buf;
		// Iterate all subregions of this memory range
		while (acpi_numa_get_memory_range(&iter, &buf)) {
			result++;
		}
	}
	return result;
}

//! @brief Initialize NUMA nodes with memory ranges
//! @param memmap Stivale2 memory map tag
static void mem_add_numa_ranges(void) {
	if (init_memmap_tag == NULL) {
		LOG_PANIC("No memory map");
	}
	// Allocate memory pool for boot memory ranges
	size_t num_entries = mem_estimate_boot_ranges_upper_bound();
	size_t size = sizeof(struct mem_range) * num_entries;
	struct mem_range *entries = mem_bootstrap_alloc(size);
	size_t current_entry = 0;
	// Terminate bootstrap allocator and get border of bootstrap allocated memory
	uintptr_t border = mem_bootstrap_terminate_allocator();
	// Enumerate all usable memory map entries
	struct stivale2_struct_tag_memmap *memmap = init_memmap_tag;
	LOG_INFO("Number of memory map entries: %U", memmap->entries);
	for (size_t i = 0; i < memmap->entries; ++i) {
		if (memmap->memmap[i].type != STIVALE2_MMAP_USABLE) {
			continue;
		}
		uintptr_t start = memmap->memmap[i].base;
		const uintptr_t end = start + memmap->memmap[i].length;
		if (end <= border) {
			continue;
		}
		if (start < border) {
			start = border;
		}
		struct acpi_numa_phys_range_iter iter = ACPI_NUMA_PHYS_RANGE_ITER_INIT(start, end);
		struct acpi_numa_memory_range buf;
		// Iterate all subregions of this memory range
		while (acpi_numa_get_memory_range(&iter, &buf)) {
			LOG_INFO("Usable memory range %p - %p belongs to domain %u", buf.start, buf.end,
			         (uint32_t)buf.node_id);
			// Allocate memory range object for this physical region
			ASSERT(current_entry < num_entries,
			       "Failed to statically allocate memory range object");
			struct mem_range *range = entries + (current_entry++);
			// Get corresponding NUMA node object
			struct numa_node *node = numa_nodes + buf.node_id;
			if (node == NULL) {
				LOG_PANIC("Unknown NUMA node");
			}
			// Initialize range info
			mem_phys_slab_init(&range->slab, buf.start, buf.end - buf.start);
			// Add range to node's ranges list
			range->next_range = node->ranges;
			node->ranges = range;
		}
	}
}
