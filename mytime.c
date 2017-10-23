
#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#define  DEVICE_NAME "char"    ///< The device will appear at /dev/char using this value
#define  CLASS_NAME  ""        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Yang Liu");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  charClass  = NULL; ///< The device-driver class struct pointer
static struct device* charDevice = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     open(struct inode *, struct file *);
static int     release(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
  .open = open,
  .read = read,
  .write = write,
  .release = release,
};

static int __init char_init(void){
  printk(KERN_INFO "Char: Initializing the Char LKM\n");

  // Try to dynamically allocate a major number for the device -- more difficult but worth it
  majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majorNumber<0){
     printk(KERN_ALERT "Char failed to register a major number\n");
     return majorNumber;
  }
  printk(KERN_INFO "Char: registered correctly with major number %d\n", majorNumber);

  // Register the device class
  charClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(charClass)){                // Check for error and clean up if there is
     unregister_chrdev(majorNumber, DEVICE_NAME);
     printk(KERN_ALERT "Failed to register device class\n");
     return PTR_ERR(charClass);          // Correct way to return an error on a pointer
  }
  printk(KERN_INFO "Char: device class registered correctly\n");

  // Register the device driver
  charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(charDevice)){               // Clean up if there is an error
     class_destroy(charClass);           // Repeated code but the alternative is goto statements
     unregister_chrdev(majorNumber, DEVICE_NAME);
     printk(KERN_ALERT "Failed to create the device\n");
     return PTR_ERR(charDevice);
  }
  printk(KERN_INFO "Char: device class created correctly\n"); // Made it! device was initialized
  return 0;
}

static void __exit char_exit(void){
  device_destroy(charClass, MKDEV(majorNumber, 0));     // remove the device
  class_unregister(charClass);                          // unregister the device class
  class_destroy(charClass);                             // remove the device class
  unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
  printk(KERN_INFO "Char: Goodbye from the LKM!\n");
}

static int open(struct inode *inodep, struct file *filep){
  numberOpens++;
  printk(KERN_INFO "Char: Device has been opened %d time(s)\n", numberOpens);
  return 0;
}

static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset){
  int error_count = 0;
  // copy_to_user has the format ( * to, *from, size) and returns 0 on success
  error_count = copy_to_user(buffer, message, size_of_message);

  if (error_count==0){            // if true then have success
     printk(KERN_INFO "Char: Sent %d characters to the user\n", size_of_message);
     return (size_of_message=0);  // clear the position to the start and return 0
  }
  else {
     printk(KERN_INFO "Char: Failed to send %d characters to the user\n", error_count);
     return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
  }
}

static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
  sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
  size_of_message = strlen(message);                 // store the length of the stored message
  printk(KERN_INFO "Char: Received %zu characters from the user\n", len);
  return len;
}

static int release(struct inode *inodep, struct file *filep){
  printk(KERN_INFO "Char: Device successfully closed\n");
  return 0;
}

module_init(char_init);
module_exit(char_exit);
