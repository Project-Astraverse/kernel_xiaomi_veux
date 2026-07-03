/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _MI_THERMAL_MESSAGE_H
#define _MI_THERMAL_MESSAGE_H

struct class;

#if IS_ENABLED(CONFIG_MI_THERMAL_MESSAGE)
int mi_thermal_message_register(struct class *thermal_class);
#else
static inline int mi_thermal_message_register(struct class *thermal_class)
{
	return 0;
}
#endif

#endif
