// SPDX-License-Identifier: GPL-2.0-only
/*
 * Xiaomi thermal_message sysfs bridge for mi_thermald / framework.
 * Ported for Qualcomm holi (veux) from stock MIUI kernel behaviour.
 */

#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/thermal.h>

#define MI_THERMAL_BUF_SZ		128
#define MI_DEFAULT_BOARD_SENSOR		"sdm-skin-therm-usr"

static struct device thermal_message_dev;

static atomic_t switch_mode = ATOMIC_INIT(0);
static atomic_t temp_state = ATOMIC_INIT(0);
static atomic_t screen_state = ATOMIC_INIT(1);
static atomic_t usb_online = ATOMIC_INIT(0);

static char board_sensor_name[MI_THERMAL_BUF_SZ];
static char board_sensor_temp[MI_THERMAL_BUF_SZ];
static char board_sensor_second_temp[MI_THERMAL_BUF_SZ];
static char board_sensor_temp_comp[MI_THERMAL_BUF_SZ];
static char board_sensor_other_temp[MI_THERMAL_BUF_SZ];
static char board_sensor_charge_temp[MI_THERMAL_BUF_SZ];
static char cpu_limits[MI_THERMAL_BUF_SZ];
static char cpu_nolimit_temp[MI_THERMAL_BUF_SZ];
static char charger_only_index[MI_THERMAL_BUF_SZ];
static char atc_enable[MI_THERMAL_BUF_SZ];
static char charger_temp[MI_THERMAL_BUF_SZ];
static char boost_buf[MI_THERMAL_BUF_SZ];
static char ambient_sensor[MI_THERMAL_BUF_SZ];
static char ambient_sensor_temp[MI_THERMAL_BUF_SZ];

static int mi_thermal_message_use_default_sensor(void)
{
	struct thermal_zone_device *tz;

	tz = thermal_zone_get_zone_by_name(MI_DEFAULT_BOARD_SENSOR);
	if (IS_ERR(tz))
		return -ENODEV;

	strlcpy(board_sensor_name, MI_DEFAULT_BOARD_SENSOR,
		sizeof(board_sensor_name));
	pr_info("Thermal: board sensor (fallback): %s\n", board_sensor_name);
	return 0;
}

static int mi_thermal_message_parse_dt(void)
{
	struct device_node *np;
	const char *sensor;
	int ret;

	if (board_sensor_name[0])
		return 0;

	np = of_find_node_by_path("/thermal-message");
	if (!np)
		np = of_find_compatible_node(NULL, NULL, "xiaomi,thermal-message");
	if (!np)
		np = of_find_node_by_name(NULL, "thermal-message");
	if (!np) {
		pr_warn("Thermal: thermal-message node not found in DT\n");
		return mi_thermal_message_use_default_sensor();
	}

	ret = of_property_read_string(np, "board-sensor", &sensor);
	of_node_put(np);
	if (ret) {
		pr_warn("Thermal: board-sensor missing in thermal-message node\n");
		return mi_thermal_message_use_default_sensor();
	}

	strlcpy(board_sensor_name, sensor, sizeof(board_sensor_name));
	pr_info("Thermal: board sensor: %s\n", board_sensor_name);
	return 0;
}

static ssize_t thermal_sconfig_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&switch_mode));
}

static ssize_t thermal_sconfig_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t len)
{
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	atomic_set(&switch_mode, val);
	return len;
}

static ssize_t thermal_temp_state_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&temp_state));
}

static ssize_t thermal_temp_state_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t len)
{
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	atomic_set(&temp_state, val);
	return len;
}

static ssize_t thermal_board_sensor_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	mi_thermal_message_parse_dt();

	if (!board_sensor_name[0])
		return snprintf(buf, PAGE_SIZE, "invalid\n");

	return snprintf(buf, PAGE_SIZE, "%s\n", board_sensor_name);
}

static int mi_thermal_read_zone_temp(const char *name, char *out, size_t out_sz)
{
	struct thermal_zone_device *tz;
	int temp, ret;

	if (!name || !name[0])
		return snprintf(out, out_sz, "0\n");

	tz = thermal_zone_get_zone_by_name(name);
	if (IS_ERR(tz))
		return snprintf(out, out_sz, "0\n");

	ret = thermal_zone_get_temp(tz, &temp);
	if (ret)
		return snprintf(out, out_sz, "0\n");

	return snprintf(out, out_sz, "%d\n", temp);
}

static ssize_t thermal_board_sensor_temp_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	mi_thermal_message_parse_dt();

	if (board_sensor_temp[0])
		return snprintf(buf, PAGE_SIZE, "%s", board_sensor_temp);

	return mi_thermal_read_zone_temp(board_sensor_name, buf, PAGE_SIZE);
}

static ssize_t thermal_board_sensor_temp_store(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t len)
{
	size_t n = min(len, (size_t)(MI_THERMAL_BUF_SZ - 1));

	memcpy(board_sensor_temp, buf, n);
	board_sensor_temp[n] = '\0';
	return len;
}

#define MI_THERMAL_STRING_ATTR(_name, _storage)					\
	static ssize_t _name##_show(struct device *dev,				\
				    struct device_attribute *attr, char *buf)	\
	{									\
		return snprintf(buf, PAGE_SIZE, "%s", _storage);		\
	}									\
	static ssize_t _name##_store(struct device *dev,			\
				     struct device_attribute *attr,		\
				     const char *buf, size_t len)		\
	{									\
		size_t n = min(len, (size_t)(MI_THERMAL_BUF_SZ - 1));		\
										\
		memcpy(_storage, buf, n);					\
		_storage[n] = '\0';						\
		return len;							\
	}									\
	static DEVICE_ATTR(_name, 0664, _name##_show, _name##_store)

#define MI_THERMAL_ATOMIC_ATTR(_name, _atomic)					\
	static ssize_t _name##_show(struct device *dev,				\
				    struct device_attribute *attr, char *buf)	\
	{									\
		return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&_atomic)); \
	}									\
	static ssize_t _name##_store(struct device *dev,			\
				     struct device_attribute *attr,		\
				     const char *buf, size_t len)		\
	{									\
		int val;							\
										\
		if (kstrtoint(buf, 10, &val))					\
			return -EINVAL;						\
		atomic_set(&_atomic, val);					\
		return len;							\
	}									\
	static DEVICE_ATTR(_name, 0664, _name##_show, _name##_store)

MI_THERMAL_STRING_ATTR(board_sensor_second_temp, board_sensor_second_temp);
MI_THERMAL_STRING_ATTR(board_sensor_temp_comp, board_sensor_temp_comp);
MI_THERMAL_STRING_ATTR(board_sensor_other_temp, board_sensor_other_temp);
MI_THERMAL_STRING_ATTR(board_sensor_charge_temp, board_sensor_charge_temp);
MI_THERMAL_STRING_ATTR(cpu_limits, cpu_limits);
MI_THERMAL_STRING_ATTR(cpu_nolimit_temp, cpu_nolimit_temp);
MI_THERMAL_STRING_ATTR(charger_only_index, charger_only_index);
MI_THERMAL_STRING_ATTR(atc_enable, atc_enable);
MI_THERMAL_STRING_ATTR(charger_temp, charger_temp);
MI_THERMAL_STRING_ATTR(boost, boost_buf);
MI_THERMAL_STRING_ATTR(ambient_sensor, ambient_sensor);
MI_THERMAL_STRING_ATTR(ambient_sensor_temp, ambient_sensor_temp);
MI_THERMAL_ATOMIC_ATTR(screen_state, screen_state);
MI_THERMAL_ATOMIC_ATTR(usb_online, usb_online);

static DEVICE_ATTR(sconfig, 0664, thermal_sconfig_show, thermal_sconfig_store);
static DEVICE_ATTR(temp_state, 0664, thermal_temp_state_show,
		   thermal_temp_state_store);
static DEVICE_ATTR(board_sensor, 0664, thermal_board_sensor_show, NULL);
static DEVICE_ATTR(board_sensor_temp, 0664, thermal_board_sensor_temp_show,
		   thermal_board_sensor_temp_store);

static struct attribute *mi_thermal_message_attrs[] = {
	&dev_attr_sconfig.attr,
	&dev_attr_temp_state.attr,
	&dev_attr_board_sensor.attr,
	&dev_attr_board_sensor_temp.attr,
	&dev_attr_board_sensor_second_temp.attr,
	&dev_attr_board_sensor_temp_comp.attr,
	&dev_attr_board_sensor_other_temp.attr,
	&dev_attr_board_sensor_charge_temp.attr,
	&dev_attr_cpu_limits.attr,
	&dev_attr_cpu_nolimit_temp.attr,
	&dev_attr_charger_only_index.attr,
	&dev_attr_atc_enable.attr,
	&dev_attr_charger_temp.attr,
	&dev_attr_screen_state.attr,
	&dev_attr_usb_online.attr,
	&dev_attr_boost.attr,
	&dev_attr_ambient_sensor.attr,
	&dev_attr_ambient_sensor_temp.attr,
	NULL,
};

ATTRIBUTE_GROUPS(mi_thermal_message);

int mi_thermal_message_register(struct class *thermal_class)
{
	int ret;

	if (!thermal_class)
		return -EINVAL;

	ret = mi_thermal_message_parse_dt();
	if (ret)
		pr_warn("Thermal: Can not parse thermal message node, return %d\n",
			ret);

	device_initialize(&thermal_message_dev);
	thermal_message_dev.class = thermal_class;
	thermal_message_dev.groups = mi_thermal_message_groups;
	dev_set_name(&thermal_message_dev, "thermal_message");

	ret = device_add(&thermal_message_dev);
	if (ret) {
		put_device(&thermal_message_dev);
		pr_warn("Thermal: create thermal message node failed, return %d\n",
			ret);
		return ret;
	}

	return 0;
}
