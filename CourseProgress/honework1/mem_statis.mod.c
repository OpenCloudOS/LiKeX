#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9f593a9b, "module_layout" },
	{ 0x37a0cba, "kfree" },
	{ 0xe346a5d9, "filp_close" },
	{ 0x46a4b118, "hrtimer_cancel" },
	{ 0xe1a9a80, "filp_open" },
	{ 0xed5f6e, "kmem_cache_alloc_trace" },
	{ 0x25931311, "kmalloc_caches" },
	{ 0x3c5d543a, "hrtimer_start_range_ns" },
	{ 0x2d0684a9, "hrtimer_init" },
	{ 0x828e22f4, "hrtimer_forward" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xc5850110, "printk" },
	{ 0x4297aa96, "kernel_read" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DFFEB3C0C535823680BCC0E");
