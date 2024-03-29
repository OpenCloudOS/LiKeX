# LiKeX
## 背景
虚拟化技术是云计算的基础，目前大量的公有云以及私有云环境都选择 KVM 作为Hypervisor。KVM 作为 Type 2 型的系统级硬件辅助虚拟化技术的典型方案，相对于 Xen 的优势有：

KVM 作为一个 Linux 内核模块方式实现，因此其硬件生态兼容性、扩展性较好；KVM 中没有实现调度等功能，而是依赖 Linux 内核的实现，如 Linux 的调度器，实现充分利用了 Linux 内核的成熟模块，实现相对精简。

但是随着 FaaS、Serverless 等新场景的出现，KVM 现有的实现就显得复杂了。新的场景趋向于以容器为中心，虚拟化技术在新的场景中主要的作用是弥补传统容器技术的隔离性不足的问题。因此对于 KVM 技术有进一步轻量化的需求。

Rust 作为内存安全语言已经被业界广泛认可。但 Rust 语言不只是追求安全，而且在并发和性能方面也有很好的表现，因此 Rust 不仅被很多大公司如 AWS、微软、Google 等用于应用软件的开发，而且其已经用于构建操作系统，比如 Redox OS。

Rust 也逐渐成为未来底层软件开发的首选语言。目前 Rust 已经被 Linux 内核开源社区接受作为开发语言，主要用于 Linux 内核驱动模块的开发。

## Why LiKeX ？

LiKeX 是一个对 KVM 的 Rust 重构项目，但又不仅仅是简单的重构。

LiKeX 中的「Li」代表轻量，「K」代表 KVM，「e」代表 extension，「X」是一个统一的后缀对于下一代云原生 OS 项目。

LiKeX 项目开发的目标是针对 FaaS 场景，提供轻量化的 Hypervisor 解决方案。在架构设计上，考虑到 FaaS 应用的特点：单实例短生命周期，单实例资源占用低，高实例密度，因此 LiKeX 在不少设计上与 KVM 有较大差异，部分对比项如下：

![Table](https://github.com/OpenCloudOS/LiKeX/blob/main/table.png)

LiKeX优势主要体现在两个方面，一方面是由RUST语言提供的安全性。具体体现在rust kernel的封装已经确保LiKeX调用Linux kernel的kabi是安全的，不会出现空指针等内存安全问题。另一方面是轻量化，其体现在架构设计上：部分不常用的组件模拟在用户态，指令模拟在用户态，kvm设备/vm/vcpu 使用同一个文件句柄。体现在功能实现上：不支持设备透传等对于FAAS基本不会使用的功能。

LiKeX针对kvm的优势（以及用户态采用轻量化的设备模拟）主要是轻量化带来的计算单元的启动/关闭耗时的减少，资源占用的减少，从而整体提升部署密度。同时 LiKeX 在设计上尽量保持用户态接口与 KVM 兼容或者最小修改，这样对于基于 KVM 的设备模拟组件 VMM 可以较容易的迁移到 LiKeX 上。

在支持FaaS方面,LiKeX有两种模式,一种模式是没有GuestOS,直接将一段可执行程序放到Guest态运行,这样达到了隔离的效果.另一种模式是存在一个GuestOS,可执行程序运行在GuestOS内.


## LiKeX 现状

目前，LiKeX 项目已经开源到 OpenCloudOS 社区并在社区内进行持续演进.LiKeX 项目已经实现阶段性目标，可以加载一段用户态程序到 Guest 态运行(第一种模式)，并支持多个 memslot。

测试程序(精简)：

```
//Guest态运行的程序
const unsigned char code[] = {
            0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
            0x00, 0xd8,       /* add %bl, %al */
            0x04, '0',        /* add $'0', %al */
            0xee,             /* out %al, (%dx) */
            0xb0, '\n',       /* mov $'\n', %al */
            0xee,             /* out %al, (%dx) */
            0xf4,            /* hlt */
 };
//Guest内存区域
struct kvm_userspace_memory_region region = {
                .slot = 0,
                .flags = 0,
                .guest_phys_addr = 0, 
                .memory_size = 0x1000,
};

struct kvm_regs regs = {
                .rip = 0, 
                .rax = 2,
                .rbx = 2,
                .rflags = 0x2,
};

void main(void)
{
  fd = open("/dev/rust_kvm",O_RDWR); //打开rust_kvm设备
  ret = ioctl(fd, KVM_CREATE_VM, 0);//创建虚拟机
  mem = (unsigned long)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  memset((char*)mem, 0, 4096);
  memcpy((char *)mem, code, sizeof(code));//拷贝Guest态代码到当前进程某段地址空间
  region.userspace_addr = (uint64_t)mem;
  ret = ioctl(fd, KVM_SET_USER_MEMORY_REGION, &region);//传递Guest地址空间到内核部分到rust_kvm
  ret = ioctl(fd, KVM_CREATE_VCPU, 0); //创建VCPU
  run = (struct kvm_run *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  memset((char*)&sregs, 0, sizeof(sregs));
  ioctl(fd, KVM_GET_SREGS, &sregs);
  sregs.cs.base = 0;
  sregs.cs.selector = 0;
  ioctl(fd, KVM_SET_SREGS, &sregs);
  ioctl(fd, KVM_SET_REGS, &regs);
  ioctl(fd, KVM_GET_REGS, &regs);
         while (loop0 < 8) {
                ioctl(fd, KVM_RUN, NULL); //进入Guest态运行
                switch (run->exit_reason) {//从Guest态退出，根据退出原因在用户态调用对应处理函数
                        /* Handle exit */
                        case KVM_EXIT_HLT:
                                return;
                        case KVM_EXIT_IO:
                                if (run->io.direction == KVM_EXIT_IO_OUT &&
                                        run->io.size == 1 &&
                                        run->io.port == 0x3f8 &&
                                        run->io.count == 1)
                                        printf("exit io:%c \n", (*(((char *)run) + run->io.data_offset)));
                                break;
                        case KVM_EXIT_FAIL_ENTRY:
                                printf(" fail entry\n");
                                memset((char*)&regs, 0, sizeof(regs));
                                ioctl(fd, KVM_GET_REGS, &regs);
                                return;
                        case KVM_EXIT_INTERNAL_ERROR:
                                printf(" internal error\n");
                                return;
                        default:
                                printf(" default:0x%x\n", run->exit_reason);
                                return;
                	}
              loop0++;
        }
}
```
内核部分运行结果：
```
rust_kvm: Rust KVM Open
rust_kvm: Rust kvm: vmcs=ffff888110f02000,revision=4
rust_kvm: written new cr4 value
rust_kvm: Rust kvm: IOCTL_KVM_CREATE_VM
rust_kvm: Rust kvm: IOCTL_KVM_SET_USER_MEMORY_REGION
rust_kvm:  add_memory_region slot= 0,uaddr=7f531f3e5000, gpa = 0, npages=1
rust_kvm: Rust kvm: IOCTL_KVM_CREATE_VCPU
rust_kvm: RkvmMmu hpa(va) = ffff88811d6ed000
rust_kvm: RkvmMmu hpa(phy) = 11d6ed000
rust_kvm: ad_disabled = false, ecex_only = true
rust_kvm:  setup pin=17,cpu=a581e5f2, cpu2=82,exit=3fefff, entry=d1ff
rust_kvm:   pin_based = 17
rust_kvm:   cpu_based = a581e5f2
rust_kvm:   cpu_2nd_based = 82
rust_kvm:  vcpu_vmcs_init
rust_kvm:  init_mmu_root
rust_kvm: hpa= 11d6ed000, eptp = 11d6ed01e
rust_kvm: Rust kvm: IOCTL_KVM_CREATE_VCPU finish
rust_kvm: Rust KVM mmap
rust_kvm: Rust kvm: IOCTL_KVM_GET_SREGS
rust_kvm: Rust kvm: IOCTL_KVM_VCPU_RUN
rust_kvm:  vcpu_run state guest rip = 0, read guest rip = 0
rust_kvm:  vmentry: launched = false
rust_kvm: Enter handle EPT violation
rust_kvm: pagefault: pfn=1999950
rust_kvm: rkvm_tdp_map level=4, gfn=0, spte=14a55c907
rust_kvm: rkvm_tdp_map level=3, gfn=0, spte=1567bf907
rust_kvm: rkvm_tdp_map level=2, gfn=0, spte=11bcc5907
rust_kvm: rkvm_tdp_map level=1, gfn=0, spte=6000001e844eb07
rust_kvm: ret=Ok(1),after vcpu_exit_handler
rust_kvm:  vmentry: launched = true
rust_kvm:  vmexit: guest_rip=7
rust_kvm:  handle_io port =3f8
rust_kvm: ret=Ok(0), after vcpu_exit_handler
rust_kvm:  vmentry: launched = true
rust_kvm:  vmexit: guest_rip=9
rust_kvm:  handle hlt
rust_kvm: ret=Ok(0), after vcpu_exit_handler
```
LiKeX 项目作为 OpenCloudOS 下一代云原生 OS 的一部分，其在整个系统中的位置如下图：

![OSArch](https://github.com/OpenCloudOS/LiKeX/blob/main/osarch.png)

从架构上看，分为 Guest 和 Host 两部分，作为 Host 部分的重要组成部分，LiKeX 为设备模拟组件 VMM 和容器编排引擎提供了更轻量、更安全的 Rust 基础，保证兼容原有协议，并根据用户需求扩展新协议，从而实现自主可控的下一代云原生操作操作系统。

## 规划
下一步计划:
1.为了支持第二种模式(即可运行一个GuestOS),需要实现一个用户态的轻量化设备模拟程序.在设备模拟程序中实现ioapic/pic的模拟.
2.为了简化LiKeX的复杂度,指令模拟部分也在设备模拟程序中实现.
3.LiKeX完整支持lapic.

## 总结

从整个 OpenCloudOS 云原生操作系统层面看，LiKeX 作为整个系统虚拟化的底座，提供了轻量级安全的 Hypervisor，能够更好的支持上层 FaaS 等新场景。

总之，LiKeX 项目希望从架构和功能两个方面以 Rust 语言重新开发出适合 FaaS 等新场景的轻量、安全的 Hypervisor，以达到提升容器部署密度、启停速率、隔离安全性的目标。

## code path

https://github.com/OpenCloudOS/linux
This is from 'Rust-for-linux' which provide basic rust library for supporting linux kernel.

https://github.com/OpenCloudOS/kvmtool
基于kvmtool开发LiKeX项目的轻量化VMM。
## test

test_misc.c
