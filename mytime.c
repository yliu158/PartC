#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Yang Liu");    ///< The author -- visible when you use modinfo

static struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  // .read = my_read,
};

static struct miscdevice my_misc_device = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "my device",
  .fops = &my_fops
};


static int __init my_module_init() {
  misc_register(&my_misc_device);
}

static void __exit my_exit(void) {
  misc_deregister(&my_misc_device);
}

static ssize_t my_read(
  struct file *file,
  char __user * out,
  size_t size,
  loff_t * off) {

  if (access_ok(VERIFY_READ, my_misc_device, size)){
    struct timespec current_time = current_kernel_time();
    char* buf = (char*)kmalloc(size, GFP_KERNEL);
    snprintf(buf, size, “Hello World\n”);

    int cp = copy_to_user(out, buf, strlen(buf)+1);
    if (cp > 0) {
      return cp;
    }
    printk(KERN_ALERT "current_kernel_time:%ld\n", current_time->tv_sec);
    free(buf);
    return 0;
  } else {
    return EFAULT;
  }
}

static int my_open(struct miscdevice *, struct file *) {
  printk(KERN_ALERT "Char Device has been opened.\n");
}

static int my_close(struct inode *inodep, struct file *filep) {
  printk(KERN_ALERT "Char Device successfully closed.\n");
}

module_init(__init);
module_exit(__exit);
