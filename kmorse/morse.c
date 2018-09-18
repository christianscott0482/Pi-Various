// A. Sheaff 3/14/2017
// Morse code kernel driver
// GPIO4 is active low enable
// GPIO17 is active high BPSK encoded morse data
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/types.h>
#include "morse.h"

struct morse_moddat_t *morse_dat=NULL;		// Data to be passed around the calls
static DEFINE_MUTEX(fp_mutex);			//locking init


// Data to be "passed" around to various functions
struct morse_moddat_t {
	int major;			// Device major number
	struct class *morse_class;	// Class for auto /dev population
	struct device *morse_dev;	// Device for auto /dev population
	struct gpio_desc *enable;	// gpiod Enable pin
	struct gpio_desc *bm;		// gpiod BPSK Morse
	struct gpio_desc *shdn;		// Shutdown pin

	int irq;					// Shutdown IRQ
};
int morse_open(struct inode *inode, struct file *filp);
ssize_t morse_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *o);
int morse_release(struct inode *inode, struct file *filp);
static irqreturn_t morse_irq(int irq, void *data);
void checksumfunc(int sum, char *sumarray);
const char * convert(char pass);
int blink_string(int time, char *morse, int phase);
void blink_init(int time);
void blink_fin(int time);

// File operations for the morse device
static const struct file_operations morse_fops = {

	.open = morse_open,
	.write = morse_write,
	.release = morse_release,
};

// Place file_operations functions here


int morse_open(struct inode *inode, struct file *filp)
{
	//checks if the file opened is writable
	if(filp->f_mode==O_RDONLY){
	     	printk(KERN_DEBUG "file was not writable\n");
		return -EBADF;
	}
	return 0;
}

ssize_t morse_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *o)
{
	//copy user space data - copy_from_user()
	//morse with the pins
	char *kbuf = NULL;
	int check = 0;		//error checking
	int i = 0;
	int k = 0;
	char preamble[4] = "__*_";//preamble morse string
	int morse_length = 0;   //length of the morse string
	char constring[17]; 	//hold morse string from loaded char
	char sumstring[8];	//hold check sum morse string
	int checksum = 1;	//start checksum at 1 from preamble
	int time_unit = 500000; //time unit = 500,000usec or .5sec
	int htime = (time_unit / 2);	//half a time unit
	int phase = 0;		//keeps track of bpsk phase
	char space[3] = "___";


	//lock write function
	mutex_lock(&fp_mutex);

	//allocates memory for userspace data
	kbuf = kmalloc(cnt, 0);
	if(!kbuf){
		printk("buffer allocation failed\n");
		mutex_unlock(&fp_mutex);
		return -ECANCELED;
	}

	//gets data from userspace
	check = copy_from_user(kbuf, buf, cnt);
	if(check){
		kfree(kbuf);
		mutex_unlock(&fp_mutex);
		printk(KERN_DEBUG "copy from user failed\n");
		return -EBADMSG;
	}
	
	//prints test string
	for (i = 0; i < cnt; i++)
	{
		printk("%c", kbuf[i]);
	}
	
	//initialize pins to proper values
	gpiod_set_value(morse_dat->enable, 1);
	gpiod_set_value(morse_dat->bm, 0);


	//BPSK Logic
	blink_init(time_unit);
	phase = blink_string(htime, preamble, phase);
	for (i = 0; i < cnt - 1; i++){
		strcpy(constring, convert(kbuf[i]));
		morse_length = strlen(constring);
		for (k = 0; k < morse_length; k++){
			if (constring[k] == '*') checksum += 1;
		}
		checksumfunc(checksum, sumstring);
		phase = blink_string(htime, constring, phase);
		phase = blink_string(htime, space, phase);
	}
	phase = blink_string(htime, sumstring, phase);
	blink_fin(time_unit);





	//frees the allocated memory
	if(kbuf){
		kfree(kbuf);
	}
	
	//unlock function once again
	mutex_unlock(&fp_mutex);	
	return 0;
}

int morse_release(struct inode *inode, struct file *filp)
{
	//release function, no contents needed
	return 0;
}

//takes a number calculated in BPSK logic, and outputs
//that number in a morse string
void checksumfunc(int sum, char *sumarray)
{
	unsigned int c = 0;	//used to check each bit
	unsigned int count = 7; //array iterator, counts down from 8(8bits)
	for (c = 1 << 7; c > 0; c = c / 2){
		if (sum &c){
			sumarray[count] = '_';
		}
		else {
			sumarray[count] = '*';
		}
		count -= 1;
	}
	return;
}

//converts a character to it's corersponding morse string
const char * convert(char pass)
{
	if ((pass == 'A') || (pass == 'a')) return "*_***";
	if ((pass == 'B') || (pass == 'b')) return "***_*_*_*";
        if ((pass == 'C') || (pass == 'c')) return "***_*_***_*";
        if ((pass == 'D') || (pass == 'd')) return "***_*_*";
        if ((pass == 'E') || (pass == 'e')) return "*";
        if ((pass == 'F') || (pass == 'f')) return "*_*_***_*";
        if ((pass == 'G') || (pass == 'g')) return "***_***_*";
        if ((pass == 'H') || (pass == 'h')) return "*_*_*_*"; 
 	if ((pass == 'I') || (pass == 'i')) return "*_*";
        if ((pass == 'J') || (pass == 'j')) return "*_***_***_***";
        if ((pass == 'K') || (pass == 'k')) return "***_*_***";
        if ((pass == 'L') || (pass == 'l')) return "*_***_*_*";
        if ((pass == 'M') || (pass == 'm')) return "***_***";
        if ((pass == 'N') || (pass == 'n')) return "***_*";
        if ((pass == 'O') || (pass == 'o')) return "***_***_***";
        if ((pass == 'P') || (pass == 'p')) return "*_***_***_*";
        if ((pass == 'Q') || (pass == 'q')) return "***_***_*_***";
        if ((pass == 'R') || (pass == 'r')) return "*_***_*";
        if ((pass == 'S') || (pass == 's')) return "*_*_*";
        if ((pass == 'T') || (pass == 't')) return "***";
        if ((pass == 'U') || (pass == 'u')) return "*_*_***";
        if ((pass == 'V') || (pass == 'v')) return "*_*_*_***";
        if ((pass == 'W') || (pass == 'w')) return "*_***_***";
        if ((pass == 'X') || (pass == 'x')) return "***_*_*_***";
        if ((pass == 'Y') || (pass == 'y')) return "***_*_***_***";
        if ((pass == 'Z') || (pass == 'z')) return "***_***_*_*";
        if (pass == '1') return "*_***_***_***_***";
        if (pass == '2') return "*_*_***_***_***";
        if (pass == '3') return "*_*_*_***_***";
        if (pass == '4') return "*_*_*_*_***";
	if (pass == '5') return "*_*_*_*_*";
        if (pass == '6') return "***_*_*_*_*";
        if (pass == '7') return "***_***_*_*_*";		
	if (pass == '8') return "***_***_***_*_*";
        if (pass == '9') return "***_***_***_***_*";
	if (pass == '0') return "***_***_***_***_***";
	if (pass == ' ') return "____";				
	return 0;
}

//blinks an LED according to a morse string passed in
int blink_string(int time, char *morse, int phase)
{
	//0 has a rising edge
	//1 has a falling edge
	int size = 0;
	int iter = 0;
	int bphase = phase;

	size = strlen(morse);
	for (iter = 0; iter < size - 1; iter++){
		//if the character is a one, change phase
		if (morse[iter] == '*'){
			bphase = 1 - bphase;
		}
		if (bphase == 1){
			gpiod_set_value(morse_dat->bm, 5);
			usleep_range(time, time);
			gpiod_set_value(morse_dat->bm, 0);
			usleep_range(time, time);
		}
		else if(bphase == 0){
			gpiod_set_value(morse_dat->bm, 0);
			usleep_range(time, time);
			gpiod_set_value(morse_dat->bm, 5);
			usleep_range(time, time);
		}
	}
	return bphase;
}

//clears enable LED, then delays one time unit
void blink_init(int time)
{
	gpiod_set_value(morse_dat->enable, 0);	
	usleep_range(time, time);
	return;
}

//delays one time unit, then sets enable LED
void blink_fin(int time)
{
	usleep_range(time, time);
	gpiod_set_value(morse_dat->enable, 1);
	return;
}

// End functions

// Sets device node permission on the /dev device special file
static char *morse_devnode(struct device *dev, umode_t *mode)
{
	if (mode) *mode = 0666;
	return NULL;
}

static struct gpio_desc *morse_dt_obtain_pin(struct device *dev, struct device_node *parent, char *name, int init_val)
{
	struct device_node *child=NULL;
	struct gpio_desc *gpiod_pin=NULL;
	char *label=NULL;
	int pin=-1;
	int ret=-1;

	// Find the child node - release with of_node_put()
	child=of_get_child_by_name(parent,name);
	if (child==NULL) {
		printk(KERN_INFO "No device child\n");
		return NULL;
	}
	// Get the child pin number - Does not appear to need to be released
	pin=of_get_named_gpio(child,"gpios",0);
	if (pin<0) {
		printk(KERN_INFO "no GPIO pin\n");
		of_node_put(child);
		return NULL;
	}
	printk(KERN_INFO "Found %s pin %d\n",name,pin);
	// Verify the pin is OK
	if (!gpio_is_valid(pin)) {
		of_node_put(child);
		return NULL;
	}
	// Get the of string tied to pin - Does not appear to need to be released
	ret=of_property_read_string(child,"label",(const char **)&label);
	if (ret<0) {
		printk(KERN_INFO "Cannot find label\n");
		of_node_put(child);
		return NULL;
	}
	// Request the pin - release with devm_gpio_free() by pin number
	if (init_val>=0) {
		ret=devm_gpio_request_one(dev,pin,GPIOF_OUT_INIT_HIGH,label);
		if (ret<0) {
			dev_err(dev,"Cannot allocate gpio pin\n");
			of_node_put(child);
			return NULL;
		}
	} else {
		ret=devm_gpio_request_one(dev,pin,GPIOF_IN,label);
		if (ret<0) {
			dev_err(dev,"Cannot allocate gpio pin\n");
			of_node_put(child);
			return NULL;
		}
	}

	// Release the device node
	of_node_put(child);

	// Get the gpiod pin struct
	gpiod_pin=gpio_to_desc(pin);
	if (gpiod_pin==NULL) {
		of_node_put(child);
		devm_gpio_free(dev,pin);
		printk(KERN_INFO "Failed to acquire enable gpio\n");
		return NULL;
	}

	// Make sure the pin is set correctly
	if (init_val>=0) gpiod_set_value(gpiod_pin,init_val);

	return gpiod_pin;
}

// My data is going to go in either platform_data or driver_data
//  within &pdev->dev. (dev_set/get_drvdata)
// Called when the device is "found" - for us
// This is called on module load based on ".of_match_table" member
static int morse_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;	// Device associcated with platform
	struct device_node *dn=NULL;			// Start of my device tree
	int ret;	// Return value


	// Allocate device driver data and save
	morse_dat=kmalloc(sizeof(struct morse_moddat_t),GFP_ATOMIC);
	if (morse_dat==NULL) {
		printk(KERN_INFO "Memory allocation failed\n");
		return -ENOMEM;
	}
	memset(morse_dat,0,sizeof(struct morse_moddat_t));
	
	// Tag in device data to the device
	dev_set_drvdata(dev,morse_dat);

	// Create the device - automagically assign a major number
	morse_dat->major=register_chrdev(0,"morse",&morse_fops);
	if (morse_dat->major<0) {
		printk(KERN_INFO "Failed to register character device\n");
		ret=morse_dat->major;
		goto fail;
	}

	// Create a class instance
	morse_dat->morse_class=class_create(THIS_MODULE, "morse_class");
	if (IS_ERR(morse_dat->morse_class)) {
		printk(KERN_INFO "Failed to create class\n");
		ret=PTR_ERR(morse_dat->morse_class);
		goto fail;
	}

	// Setup the device so the device special file is created with 0666 perms
	morse_dat->morse_class->devnode=morse_devnode;
	morse_dat->morse_dev=device_create(morse_dat->morse_class,NULL,MKDEV(morse_dat->major,0),(void *)morse_dat,"morse");
	if (IS_ERR(morse_dat->morse_dev)) {
		printk(KERN_INFO "Failed to create device file\n");
		ret=PTR_ERR(morse_dat->morse_dev);
		goto fail;
	}

	// Find my device node
	dn=of_find_node_by_name(NULL,"morse");
	if (dn==NULL) {
		printk(KERN_INFO "Cannot find device\n");
		ret=-ENODEV;
		goto fail;
	}
	morse_dat->enable=morse_dt_obtain_pin(dev,dn,"Enable",1);
	if (morse_dat->enable==NULL) {
		goto fail;
	}
	morse_dat->bm=morse_dt_obtain_pin(dev,dn,"BPSK_Morse",0);
	if (morse_dat->bm==NULL) {
		goto fail;
	}
	morse_dat->shdn=morse_dt_obtain_pin(dev,dn,"Shutdown",-1);
	if (morse_dat->shdn==NULL) {
		goto fail;
	}

	// Release the device node
	if (dn) of_node_put(dn);

	// Initialize the output pins - should already be done above....
	gpiod_set_value(morse_dat->enable,1);
	gpiod_set_value(morse_dat->bm,0);

	// Get the IRQ # tagged with the input shutdown pin
	morse_dat->irq=gpiod_to_irq(morse_dat->shdn);
	if (morse_dat->irq<0) {
		printk(KERN_INFO "Failed to get shutdown IRQ #\n");
		ret=-ENODEV;
		goto fail;
	}
	printk(KERN_INFO "IRQ: %d\n",morse_dat->irq);
	// Actually request and register a handler
	ret=request_irq(morse_dat->irq,morse_irq,IRQF_TRIGGER_RISING,"morse#shutdown",(void *)morse_dat);
	if (ret<0) {
		printk(KERN_INFO "Failed to register shutdown IRQ\n");
		ret=-ENODEV;
		goto fail;
	}
	printk(KERN_INFO "IRQ Registered\n");

	printk(KERN_INFO "Registered\n");
	dev_info(dev, "Initialized");
	return 0;

fail:
	if (morse_dat->shdn) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->shdn));
		gpiod_put(morse_dat->shdn);
	}
	if (morse_dat->bm) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->bm));
		gpiod_put(morse_dat->bm);
	}
	if (morse_dat->enable) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->enable));
		gpiod_put(morse_dat->enable);
	}
	if (morse_dat->morse_class) class_destroy(morse_dat->morse_class);
	if (morse_dat->major) unregister_chrdev(morse_dat->major,"morse");
	dev_set_drvdata(dev,NULL);
	kfree(morse_dat);
	printk(KERN_INFO "Morse Failed\n");
	return ret;
}

// Called when the device is removed or the module is removed
static int morse_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct morse_moddat_t *morse_dat;	// Data to be passed around the calls

	// Obtain the device driver data
	morse_dat=dev_get_drvdata(dev);

	if (morse_dat->irq>0) free_irq(morse_dat->irq,(void *)morse_dat);

	if (morse_dat->shdn) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->shdn));
		gpiod_put(morse_dat->shdn);
	}
	if (morse_dat->bm) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->bm));
		gpiod_put(morse_dat->bm);
	}
	if (morse_dat->enable) {
		devm_gpio_free(dev,desc_to_gpio(morse_dat->enable));
		gpiod_put(morse_dat->enable);
	}
	
	// Release the device
	device_destroy(morse_dat->morse_class,MKDEV(morse_dat->major,0));

	// Release the class
	class_destroy(morse_dat->morse_class);

	// Release the character device
	unregister_chrdev(morse_dat->major,"morse");

	// Free the device driver data
	dev_set_drvdata(dev,NULL);
	kfree(morse_dat);

	printk(KERN_INFO "Removed\n");
	dev_info(dev, "GPIO mem driver removed - OK");

	return 0;
}


//From Caf on StackOverflow - "Shutdown (embedded) linux from kernel-space"
//static char *poweroff_argv[]={
//	"/sbin/poweroff",NULL,
//};

static irqreturn_t morse_irq(int irq, void *data)
{//	struct afsk_data_t *afsk_dat=(struct afsk_data_t *)data;
//	int p;

//	call_usermodehelper(poweroff_argv[0],poweroff_argv,NULL,UMH_NO_WAIT);
	printk(KERN_INFO "In IRQ\n");
//	p=gpiod_get_value(afsk_dat->shdn);
//	printk(KERN_INFO "GPIO: %d\n",p);
	return 0;

}

static const struct of_device_id morse_of_match[] = {
    {.compatible = "brcm,bcm2835-morse",},
    { /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, morse_of_match);

static struct platform_driver morse_driver = {
    .probe = morse_probe,
    .remove = morse_remove,
    .driver = {
           .name = "bcm2835-morse",
           .owner = THIS_MODULE,
           .of_match_table = morse_of_match,
           },
};

module_platform_driver(morse_driver);

MODULE_DESCRIPTION("Morse pin modulator");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Morse");
//MODULE_ALIAS("platform:morse-bcm2835");
