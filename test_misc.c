#include <stdio.h>
#include <stdlib.h>
#include <linux/kvm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>


typedef unsigned long uint64_t;
const unsigned char code[] = {
        0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
        0x00, 0xd8,       /* add %bl, %al */
        0x04, '0',        /* add $'0', %al */
        0xee,             /* out %al, (%dx) */
        0xb0, '\n',       /* mov $'\n', %al */
        0xee,             /* out %al, (%dx) */
        0xb8, 0x00, 0x00, 0x00, 0x00,   /* mov $0x0,%eax*/
        0x0f, 0xa2,                     /* cpuid */ 
        0xf4,             /* hlt */
 };


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

void print_run(struct kvm_run *run) 
{

  printf(" exit_reason 0x%x,", run->exit_reason );
  printf(" 0x%x,", run->request_interrupt_window );
  printf(" 0x%x \n", run->if_flag );
}

void print_regs(struct kvm_regs * regs)
{
  printf("rax = 0x%lx,", regs->rax);
  printf("rbx = 0x%lx,", regs->rbx);
  printf("rcx = 0x%lx,", regs->rcx);
  printf("rdx = 0x%lx,", regs->rdx);
  printf("rsi = 0x%lx,", regs->rsi);
  printf("rdi = 0x%lx,", regs->rdi);
  printf("rbp = 0x%lx,", regs->rbp);
  printf("rsp = 0x%lx,", regs->rsp);
  printf("rip = 0x%lx,", regs->rip);
  printf("r8 = 0x%lx,", regs->r8);
  printf("r9 = 0x%lx,", regs->r9);
  printf("r10 = 0x%lx,", regs->r10);
  printf("r11 = 0x%lx,", regs->r11);
  printf("r12 = 0x%lx,", regs->r12);
  printf("r13 = 0x%lx,", regs->r13);
  printf("r14 = 0x%lx,", regs->r14);
  printf("r15 = 0x%lx,", regs->r15);
  printf("rflags = 0x%lx \n,", regs->rflags);
} 
void main(void)
{
  int fd, ret;
  struct kvm_sregs sregs;
  struct kvm_run * run;
  unsigned long mem = 0;
  char buf[4096];
  fd = open("/dev/rust_kvm",O_RDWR);
  if (fd < 0) {
	printf(" open fd failed \n");
        exit(1);
  }
  printf(" open success \n");

  ret = ioctl(fd, KVM_CREATE_VM, 0);
  if (ret < 0) {
    printf(" ioctl failed \n");
    exit(1);
  }
  mem = (unsigned long)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  memcpy((char *)mem, code, sizeof(code));
  region.userspace_addr = (uint64_t)mem;
  ret = ioctl(fd, KVM_SET_USER_MEMORY_REGION, &region);
  if (ret < 0) {
     printf(" ioctl set memory failed, ret=%d, errno=%d \n",ret, errno);
     exit(1);
  }
  printf(" set mmeory success \n");
  ret = ioctl(fd, KVM_CREATE_VCPU, 0);
  if (ret < 0) {
    printf(" ioctl vcpu failed \n");
    exit(1);
  }
  
  run = (struct kvm_run *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  memset((char *)run, 0, 4096); 
  printf(" mmap success \n");
  memset((char*)&sregs, 0, sizeof(sregs));  
  ioctl(fd, KVM_GET_SREGS, &sregs);
  printf(" get_sregs success \n");
  sregs.cs.base = 0;
  sregs.cs.selector = 0;
  ioctl(fd, KVM_SET_SREGS, &sregs);
  ioctl(fd, KVM_SET_REGS, &regs);
  ioctl(fd, KVM_GET_REGS, &regs);
  print_regs(&regs);
  printf(" set regs success \n");
         while (1) {
                ioctl(fd, KVM_RUN, NULL);
                printf("exit_reason : %x\n", run->exit_reason);
                printf("checkpoint---1\n");
                switch (run->exit_reason) {
                        /* Handle exit */
                        case KVM_EXIT_HLT:
                                printf("KVM_EXIT_HLT \n");
                                return;
                        case KVM_EXIT_INTR:
                                printf("KVM_EXIT_CPUID\n");
                                break;                                
                        case KVM_EXIT_IO:
                                // printf("in-exit info: %x, %x, %x, %x\n", run->io.direction, run->io.size, run->io.port, run->io.count);
                                if (//run->io.direction == KVM_EXIT_IO_OUT &&
                                        run->io.size == 1 &&
                                        run->io.port == 0x3f8 &&
                                        run->io.count == 1) {
                                        printf("exit io:%c \n", (*(((char *)run) + run->io.data_offset)));
                                
                                }else{
                                        // printf("checkpoint-----2\n");
                                        exit(1);
                                }
                                // 0
                                break;
                        case KVM_EXIT_FAIL_ENTRY:
                                printf(" fail entry\n");
                                return;
                        case KVM_EXIT_INTERNAL_ERROR:
                                printf(" internal error\n");
                                return;
                        default:
                                printf(" default:0x%x\n", run->exit_reason);
                                memset((char*)&regs, 0, sizeof(regs));
                                ioctl(fd, KVM_GET_REGS, &regs);
                                print_regs(&regs);
                                print_run(run);
                                return;

                }
                printf("checkpoint-------------3\n");
        }
        printf(" end \n");
}