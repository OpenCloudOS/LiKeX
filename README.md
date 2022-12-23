# LiKeX

LiKeX(old name RustKVM) is a KVM rust refactoring project. With the development of container, cloud native and serverless technologies, virtualization technology is mainly used for container isolation, so there is a need for lightweighthypervisor such as kvm.

Now LiKeX is used as a trademark to replace "RustKVM". "Li" stands for lightweight, 'K' stands for KVM, 'e' stands for extension, and 'X' is a unified suffix for cloud native projects.

## Advantage

Rust language is memory safe language.
RustKVM is implemented according to the requirements of container isolation, reducing the code attack surface.

## route map

The overall architecture of kvm will not change. First, the preliminary kvm interface (vm, vcpu) is implemented, so that a specified Guest App can be run in non-root mode under intel x86. Next, the mmu part of kvm will be gradually improved; finally, the effect of running with RustVmm will be achieved.

Keep the kvm api unchanged (but some kvm apis may not support it), but the specific implementation may not be completely consistent.

At present, the goal of the first stage "build a basic framework and load a piece of binary code to run in guest state" has been achieved.
The next stage goals are:
Code sorting: 
1. Since the high version of the rust compiler supports inline asm, part of the public code encapsulated in the kernel can be directly implemented in rustkvm.
2. Normalize calls to rust/kernel
3. Encapsulate some kernel macros to help

Function realization: 
1. Improve vpid processing
2. Improve the processing of mmu
## code path

https://github.com/OpenCloudOS/linux
This is from 'Rust-for-linux' which provide basic rust library for supporting linux kernel.

## test

test_misc.c
