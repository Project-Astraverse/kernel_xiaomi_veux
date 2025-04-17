ifeq ($(CONFIG_BACKPORT_INTEGRATE),)
# Since 2.6.21, try-run is available, but cc-disable-warning
# was only added later, so we add it here ourselves:
backport-cc-disable-warning = $(call try-run,\
	$(CC) $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) -W$(strip $(1)) -c -x c /dev/null -o "$$TMP",-Wno-$(strip $(1)))

NOSTDINC_FLAGS := \
	-I$(M)/backport-include/ \
	-I$(M)/backport-include/uapi \
	-I$(M)/include/ \
	-I$(M)/include/uapi \
	-include $(M)/backport-include/backport/backport.h \
	$(call backport-cc-disable-warning, unused-but-set-variable) \
	-DCONFIG_BACKPORT_VERSION=\"$(BACKPORTS_VERSION)\" \
	-DCONFIG_BACKPORT_KERNEL_VERSION=\"$(BACKPORTED_KERNEL_VERSION)\" \
	-DCONFIG_BACKPORT_KERNEL_NAME=\"$(BACKPORTED_KERNEL_NAME)\" \
	$(BACKPORTS_GIT_TRACKER_DEF) \
	$(CFLAGS)

export backport_srctree = $(M)
else
export BACKPORT_DIR = backports/
export backport_srctree = $(srctree)/$(BACKPORT_DIR)
NOSTDINC_FLAGS := \
	-I$(backport_srctree)/backport-include/ \
	-I$(backport_srctree)/backport-include/uapi \
	-I$(backport_srctree)/include/ \
	-I$(backport_srctree)/include/uapi \
	-include $(backport_srctree)/backport-include/backport/backport.h \
	$(CFLAGS)
endif


obj-y += compat/

obj-$(CONFIG_BACKPORT_BT) += net/bluetooth/
obj-$(CONFIG_BACKPORT_BT) += drivers/bluetooth/
