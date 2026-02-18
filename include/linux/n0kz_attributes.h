#ifndef _LINUX_N0KZ_ATTRIBUTES_H
#define _LINUX_N0KZ_ATTRIBUTES_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#include <linux/sched.h>

#define N0KZ_BLOCKLIST_STRLEN 256
#define N0KZ_MAX_BLOCKED 16

bool n0kz_comm_blocked(const char *comm);

struct n0kz_attributes {
    int kgsl_skip_zeroing;
    int avoid_dirty_pte;
    char bg_blocklist[N0KZ_BLOCKLIST_STRLEN];
};

extern struct n0kz_attributes n0kz_data;

#endif /* _LINUX_N0KZ_ATTRIBUTES_H */
