#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL");
int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos);
void memory_exit(void);
int memory_init(void);
struct file_operations memory_fops = {
.read = memory_read,
.write = memory_write,
.open = memory_open,
.release = memory_release
};
module_init(memory_init);
module_exit(memory_exit);
int memory_major = 60;
char *memory_buffer;
int memory_init(void)
{
	int result;
	result = register_chrdev(memory_major, "3", &memory_fops);
	if(result < 0) {
		printk("<1>memory: cannot obtain major number %d\n", memory_major);
	}
	memory_buffer = kmalloc(1, GFP_KERNEL);
	if(!memory_buffer) {
		result = -ENOMEM;
		memory_exit();
		return result;
	}
	memset(memory_buffer, 0, 1);
	printk("<1>Inserting memory-module\n");
	return 0;
}

void memory_exit(void)
{
	unregister_chrdev(memory_major, "3");
	if(memory_buffer) {
		kfree(memory_buffer);
	}
	printk("<1>Removing memory-module\n");
}

int memory_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int memory_release(struct inode *inode,struct file *filp)
{
	return 0;
}

ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	copy_to_user(buf, memory_buffer, 1);
	if(*f_pos == 0) {
		*f_pos += 1;
		return 1;
	} else {
		return 0;
	}
}
ssize_t memory_write(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	char *tmp;
	tmp = buf + count - 1;
	copy_from_user(memory_buffer, tmp, 1);
	return 1;
}

