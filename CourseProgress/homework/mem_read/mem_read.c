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
MODULE_AUTHOR("SUN");
MODULE_DESCRIPTION("A little test for hrtimer and memory read");
MODULE_VERSION("0.01");


static struct hrtimer timer;
ktime_t kt;
struct timespec64 past_time, current_time;

char *buf, *meminfo, *timeinfo;
struct file *filp = NULL, *outfilp = NULL;
int read_cnt = 0;
loff_t wr_pos;

static void finish_meminfo(char* buf, char* info, int bufSize, int infoSize) {
    int i = 0, j = 0;
    while(i < bufSize) {
        if(buf[i] == ' ' || buf[i] == '\r' || buf[i] == '\n') {
            i++;
            continue;
        }
        else{
            info[j] = buf[i];
            i++;
            j++;
            if(j >= infoSize) {
                break;
            }
        }
    }
    info[infoSize - 1] = '\n';
}


static enum hrtimer_restart hrtimer_hander(struct hrtimer *timer)               //the call back function
{
	ktime_get_real_ts64(&current_time); 
    past_time = current_time;
    
    snprintf(timeinfo, 30, "Timestamp:%lld", current_time.tv_sec);   
    read_cnt = kernel_read(filp, buf, 150, 0);
    finish_meminfo(buf, meminfo, 150, 100);
        
    meminfo[99] = 0;
    printk("hrtimer_handler: %s%s\n", timeinfo, meminfo);
    

    hrtimer_forward(timer,timer->base->get_time(),kt);
    return HRTIMER_RESTART;
}
 
static int __init test_init(void)
{
    printk("Welcome to my hrtimer and memory test \r\n");
    
	ktime_get_real_ts64(&past_time);  //To get the system time
    kt = ktime_set(3, 0);  //3s
    hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);  //To initialize hrtimer
    hrtimer_start(&timer,kt,HRTIMER_MODE_REL);              //To start hrtimer
    timer.function = hrtimer_hander;                        //The call back function

    buf = (char*)kmalloc(150, GFP_KERNEL);                  //To alloc kernel memory for process
    meminfo = (char*)kmalloc(100, GFP_KERNEL);
    timeinfo = (char*)kmalloc(30, GFP_KERNEL);
    wr_pos = 0;

    filp = filp_open("/proc/meminfo", O_RDONLY, 0);

    if(IS_ERR(filp)) 
    {
        printk("Load file /proc/meminfo failed,filp:%p \n",filp);
        return FALSE;
    } 
    
    outfilp = filp_open("/var/log/time_mem_info", O_WRONLY | O_APPEND | O_CREAT, 0664);

    if(IS_ERR(outfilp)) 
    {
        printk("Load file /var/log/time_mem_info failed,outfilp:%p \n",outfilp);
        return FALSE;
    } 

    return 0;
}
 
static void __exit test_exit(void)
{
    hrtimer_cancel(&timer);                 //To cancel the hrtimer

    if(filp != NULL) {
        filp_close(filp, NULL);
    }
    if(outfilp != NULL) {
        filp_close(outfilp, NULL);
    }


    kfree(buf);
    kfree(meminfo);
    kfree(timeinfo);
    printk("The test finished\r\n");
}
 


module_init(test_init);
module_exit(test_exit);

