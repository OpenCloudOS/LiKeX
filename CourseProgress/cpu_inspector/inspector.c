#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/processor.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/slab.h>
#include <linux/kernel_read_file.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CyberBard");
MODULE_DESCRIPTION("An inspector for CPU's usage statistics.");
MODULE_VERSION("1.0");
/**
 * The statistics information should be read directly from the cpu_info in header `<linux/processer.h>`. 
 * But it trigger SEGMENT FAULT and KERNEL READ ACCESS error, which cannot be handle currently.
 * So this module make a workaround, we read information from `/proc/stat`. Just like `top` and `htop` do.
 */

/**
 * @brief type to record a line in file `/proc/state`
 * 
 */
typedef struct
{
	char name[20];
	unsigned long long user;
	unsigned long long nice;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long irq;
	unsigned long long softirq;
	unsigned long long steal;
	unsigned long long guest;
	unsigned long long allbasytime;
	unsigned long long allruntime;
} CPUStat;

#define BUFFER_SIZE 1024 * 1024
char buf[BUFFER_SIZE] = {0};
int cpu_num = 0;
CPUStat *formers = NULL;

static void read_stat_file(void)
{
	// mount the procfs filesystem
	struct file_system_type *proc_fs_type = get_fs_type("proc");
	struct vfsmount *proc_mnt = kern_mount(proc_fs_type);

	// get the `/proc/stat` file
	struct file *stat_file_ptr = file_open_root_mnt(proc_mnt, "stat", O_RDONLY, 0);

	loff_t loff = 0;

	kernel_read(stat_file_ptr, buf, BUFFER_SIZE, &loff);
	printk(KERN_INFO "Read %lld char.", loff);
	// printk(KERN_INFO "The data loaded is:\n%s", buf);
}

char *get_next_line(char *start)
{
	char *t = strchr(start, '\n');
	if (t)
	{
		return t + 1;
	}
	return t;
}

int count_CPU(void)
{
	int ret = 0;
	char *line = buf;
	while (line = get_next_line(line))
	{ // remove the first line of `/proc/stat` file : cpu...
		char tmp = line[3];
		line[3] = '\0';
		if (strcmp(line, "cpu") != 0)
			break;
		line[3] = tmp;
		ret += 1;
	}
	return ret;
}

CPUStat *initCPUs(int cpu_num)
{
	CPUStat *ret = (CPUStat *)kmalloc(sizeof(CPUStat) * (cpu_num + 5), GFP_KERNEL);
	char *line = buf;
	line = get_next_line(line);
	int i = 0;
	for (; i < cpu_num; i++, line = get_next_line(line))
	{
		sscanf(line, "%s%llu%llu%llu%llu%llu%llu%llu%llu%llu", ret[i].name, &ret[i].user, &ret[i].nice, &ret[i].system, &ret[i].idle, &ret[i].iowait, &ret[i].irq, &ret[i].softirq, &ret[i].steal, &ret[i].guest);
		ret[i].allbasytime = ret[i].user + ret[i].nice + ret[i].system + ret[i].irq + ret[i].softirq;
		ret[i].allruntime = ret[i].allbasytime + ret[i].idle + ret[i].iowait + ret[i].steal + ret[i].guest;
	}
	return ret;
}

void calculate_user_rate(CPUStat former, CPUStat new)
{
	unsigned long long delta_allbasytime = new.allbasytime - former.allbasytime;
	unsigned long long delta_allruntime = new.allruntime - former.allruntime;
	// printk("%lld %lld",delta_allbasytime,delta_allruntime);
	if (delta_allruntime != 0)
	{
		unsigned long long userrate = (delta_allbasytime) * 100 /
									  (delta_allruntime) ;
		if (userrate > 1) // 
		{
			printk(KERN_WARNING "%s rate:%lld%%\n", new.name, userrate);
		}
	}
}



static struct hrtimer timer;
ktime_t kt;
struct timespec64 oldtc;
// callback function
static enum hrtimer_restart hrtimer_hander(struct hrtimer *timer)
{
	// stuff for timer
	{
		struct timespec64 tc;
		// printk("(debug)I am in hrtimer hander : %lu... \r\n", jiffies);
		ktime_get_real_ts64(&tc); // new time
		// printk("(debug)interval: new : %lld  old : %lld \r\n", tc.tv_sec, oldtc.tv_sec);
		oldtc = tc; // refresh old tc
		hrtimer_forward(timer, timer->base->get_time(), kt);
	}

	{
		// read new info
		read_stat_file();
		CPUStat *curs = initCPUs(cpu_num);
		int i = 0;
		for (; i < cpu_num; i++)
		{
			calculate_user_rate(formers[i], curs[i]);
		}
		// swap
		kfree(formers);
		formers = curs;
	}

	return HRTIMER_RESTART;
}

static int __init inspector_init(void)
{
	// initiate original value
	read_stat_file();
	cpu_num = count_CPU();
	printk(KERN_INFO "cpu_num:%d\n", cpu_num);
	formers = initCPUs(cpu_num);


	// set timer and its callback function
	ktime_get_real_ts64(&oldtc); 
	kt = ktime_set(1, 0);		 //1s
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL);
	timer.function = hrtimer_hander;
	return 0;
}

static void __exit inspector_exit(void)
{
	hrtimer_cancel(&timer);
	printk(KERN_INFO "Goodbye, World!!!\n");
}

module_init(inspector_init);
module_exit(inspector_exit);


// stuffs about reading

// {
// 	// mount the procfs filesystem
// 	struct file_system_type *proc_fs_type = get_fs_type("proc");
// 	struct vfsmount *proc_mnt = kern_mount(proc_fs_type);

// 	// get the `/proc/stat` file
// 	struct file *stat_file_ptr = file_open_root_mnt(proc_mnt, "stat", O_RDONLY, 0);

// 	void *buf = (void *)kmalloc(BUFFER_SIZE, GFP_KERNEL);
// 	size_t file_size;

// 	kernel_read_file(stat_file_ptr, 0, &buf, BUFFER_SIZE, &file_size, READING_UNKNOWN);

// 	printk(KERN_INFO "Hello, World!%d\n", file_size);
// }
// {
// 	void *buf = (void *)kmalloc(BUFFER_SIZE, GFP_KERNEL);
// 	size_t file_size;
// 	kernel_read_file_from_path("/proc/stat", 0, &buf, BUFFER_SIZE, &file_size, READING_UNKNOWN);

// 	printk(KERN_INFO "Hello, World!%d\n", file_size);
// }
// {
// 	// mount the procfs filesystem
// 	struct file_system_type *proc_fs_type = get_fs_type("proc");
// 	struct vfsmount *proc_mnt = kern_mount(proc_fs_type);

// 	// get the `/proc/stat` file
// 	struct file *stat_file_ptr = file_open_root_mnt(proc_mnt, "stat", O_RDONLY, 0);

// 	void *buf = (void *)kmalloc(BUFFER_SIZE, GFP_KERNEL);
// 	size_t file_size;
// 	loff_t loff = 0;

// 	kernel_read(stat_file_ptr,buf,BUFFER_SIZE,&loff);
// 	printk(KERN_INFO "Read %lld char.\n", loff);
// 	// printk(KERN_INFO "%s",(char*)buf);
// }
// {
// 	struct file *f;
// 	char buf[BUFFER_SIZE];
// 	mm_segment_t fs;

// 	int i;
// 	for (i = 0; i < BUFFER_SIZE; i++)
// 		buf[i] = 0;
// 	// f = file_open() // no longer support
// }
