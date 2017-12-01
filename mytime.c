#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Yang Liu");    ///< The author -- visible when you use modinfo

static ssize_t my_read(struct file *file, char __user * out, size_t size, loff_t * off) {
  return 0;
}

static int my_open (struct inode * id, struct file * filep){
  printk(KERN_ALERT "Char Device has been opened.\n");
  return 0;
}

static int my_close(struct inode * id, struct file * filep) {
  printk(KERN_ALERT "Char Device successfully closed.\n");
  return 0;
}

static struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .read = my_read,
};

static struct miscdevice my_misc_device = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "mytime",
  .fops = &my_fops
};


static int __init my_init(void) {
  printk(KERN_ALERT "Init mytime sucessfully.\n");
  misc_register(&my_misc_device);
  return 0;
}

static void __exit my_exit(void) {
  misc_deregister(&my_misc_device);
  printk(KERN_ALERT "Exit mytime.\n");
}



module_init(my_init);
module_exit(my_exit);
