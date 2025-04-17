ccflags-y += -I$(src) -Wframe-larger-than=1280
obj-y += compat.o

# Kernel backport compatibility code
compat-$(CONFIG_BACKPORT_KERNEL_5_5) += backport-5.5.o
compat-$(CONFIG_BACKPORT_KERNEL_5_10) += backport-5.10.o
compat-$(CONFIG_BACKPORT_KERNEL_5_11) += backport-5.11.o
compat-$(CONFIG_BACKPORT_KERNEL_5_13) += backport-5.13.o
compat-$(CONFIG_BACKPORT_KERNEL_5_15) += backport-5.15.o
