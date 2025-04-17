#ifndef __BACKPORT_DEVICE_H_
#define __BACKPORT_DEVICE_H_
#include_next <linux/device.h>
#include <linux/version.h>

#ifndef DEVICE_ATTR_ADMIN_RW
#define DEVICE_ATTR_ADMIN_RW(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RW_MODE(_name, 0600)
#endif

#if LINUX_VERSION_IS_LESS(5, 5, 0)
extern struct device *device_find_any_child(struct device *parent);
#endif

#endif /* __BACKPORT_DEVICE_H_ */
