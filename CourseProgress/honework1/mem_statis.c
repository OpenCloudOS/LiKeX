/*
sudo insmod 
sudo rmmod
hrtimer 已调通

export C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/src/linux-headers-5.13.0-30/include/uapi


 /proc/meminfo
MemUsed = MemTotal - Buffers - Cache - MemFree

*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHOU");
MODULE_DESCRIPTION("Test for hrtimer callback");
MODULE_VERSION("0.01");


static struct hrtimer timer;
ktime_t kt;
struct timespec64 oldtc, tc;

// char buf[140];
// char meminfo[93];
// char timeinfo[21];

char *buf, *meminfo, *timeinfo;
struct file *file = NULL, *outfile = NULL;
int read_cnt = 0;
loff_t wr_pos;

static void parse_meminfo(char* buf, char* info, int bufSize, int infoSize) {
    int i = 0, j = 0;
    while(i < bufSize) {
        if(buf[i] == ' ' || buf[i] == '\r' || buf[i] == '\n') {
            i++;
            continue;
        }
        else{
            info[j] = buf[i];
            i++;j++;
            if(j >= infoSize) {
                break;
            }
        }
    }
    info[infoSize - 1] = '\n';
}


static enum hrtimer_restart hrtimer_hander(struct hrtimer *timer)
{
	ktime_get_real_ts64(&tc); 
    oldtc = tc;
    
    snprintf(timeinfo, 21, "Timestamp:%lld", tc.tv_sec);   
    read_cnt = kernel_read(file, buf, 140, 0);
    parse_meminfo(buf, meminfo, 140, 93);
        
    meminfo[92] = 0;
    printk("(debug)%s%s\n", timeinfo, meminfo);
        
//    kernel_write(outfile, (const void *)meminfo, 93, &wr_pos);

    hrtimer_forward(timer,timer->base->get_time(),kt);
    return HRTIMER_RESTART;
}
 
static int __init test_init(void)
{
    printk("(debug)---------test start-----------\r\n");
    
	ktime_get_real_ts64(&oldtc);  //获取当前系统时间
    kt = ktime_set(2, 0);  //2s
    hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
    hrtimer_start(&timer,kt,HRTIMER_MODE_REL);
    timer.function = hrtimer_hander;

    buf = (char*)kmalloc(140, GFP_KERNEL);
    meminfo = (char*)kmalloc(93, GFP_KERNEL);
    timeinfo = (char*)kmalloc(21, GFP_KERNEL);
    wr_pos = 0;

    if(file == NULL) {
        file = filp_open("/proc/meminfo", O_RDONLY, 0);
    }
    if(IS_ERR(file)) {
        printk("(!!debug)error occured while opening /proc/meminfo \n");
        return 0;
    } 
    else {
        printk("(debug)succefffully open /proc/meminfo");
    }
    
    if(outfile == NULL) {
        outfile = filp_open("/var/log/time_mem_info", O_WRONLY | O_APPEND | O_CREAT, 0664);
    }
    if(IS_ERR(outfile) || outfile == NULL) {
        printk("(!!debug)error occured while opening /var/log/time_mem_info \n");
        return 0;
    } 
    else {
        printk("(debug)succefffully open /var/log/time_mem_info");
    }

    return 0;
}
 
static void __exit test_exit(void)
{
    hrtimer_cancel(&timer);

    if(file != NULL) {
        filp_close(file, NULL);
    }
    if(outfile != NULL) {
        filp_close(outfile, NULL);
    }


    kfree(buf);
    kfree(meminfo);
    kfree(timeinfo);
    printk("(debug)------------test over---------------\r\n");
}
 


module_init(test_init);
module_exit(test_exit);
