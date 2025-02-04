//! @file e9.c
//! @brief File containing e9 driver

#include <drivers/output/e9/e9.h>
#include <lib/log.h>
#include <lib/target.h>
#include <misc/attributes.h>
#include <misc/types.h>
#include <sys/ports.h>

MODULE("drivers/output/e9")

//! @brief Print string to e9
//! @param self Pointer to the logging subsystem object
//! @param data Pointer to the string to be printed
//! @param size Size of the string
attribute_maybe_unused static void e9_puts(struct log_subsystem *self, const char *data,
                                           size_t size) {
	(void)self;
	for (size_t i = 0; i < size; ++i) {
		outb(0xe9, data[i]);
	}
}

//! @brief Detect e9 log presence
//! @return True if e9 log was detected
attribute_maybe_unused static bool e9_detect(void) {
	return inb(0xe9) == 0xe9;
}

//! @brief Register e9 log if present
//! @return True if e9 log is present
bool e9_register(void) {
// If profiling is turned on, e9 is used for communication profiling information from QEMU to the
// host. Hence, we can't use e9 for kernel logs
#ifndef PROFILE
	if (e9_detect()) {
		static struct log_subsystem instance;
		instance.callback = e9_puts;
		log_register_subsystem(&instance);
		LOG_SUCCESS("e9 log subsystem regsitered!");
		return true;
	}
#endif
	return false;
}
