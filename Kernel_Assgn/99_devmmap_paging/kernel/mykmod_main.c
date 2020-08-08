#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <mydev.h>

MODULE_DESCRIPTION("My kernel module - mykmod");
MODULE_AUTHOR("maruthisi.inukonda [at] gmail.com");
MODULE_LICENSE("GPL");

// Dynamically allocate major no
#define MYKMOD_MAX_DEVS 256
#define MYKMOD_DEV_MAJOR 0

static int mykmod_init_module(void);
static void mykmod_cleanup_module(void);

static int mykmod_open(struct inode *inode, struct file *filp);
static int mykmod_close(struct inode *inode, struct file *filp);
static int mykmod_mmap(struct file *filp, struct vm_area_struct *vma);

module_init(mykmod_init_module);
module_exit(mykmod_cleanup_module);

static struct file_operations mykmod_fops = {
	.owner = THIS_MODULE,	/* owner (struct module *) */
	.open = mykmod_open,	/* open */
	.release = mykmod_close,	/* release */
	.mmap = mykmod_mmap,	/* mmap */
};

static void mykmod_vm_open(struct vm_area_struct *vma);
static void mykmod_vm_close(struct vm_area_struct *vma);
//static int mykmod_vm_fault(struct vm_fault *vmf);
static int mykmod_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf);

// TODO Data-structure to keep per device info 
struct dev_info {
	char *data;
	size_t size;
};
struct dev_info *info;		//declaring global pointer which points to device info. 

// TODO Device table data-structure to keep all devices
struct dev_info *dev_table[MYKMOD_MAX_DEVS];	//array of pointers.

// TODO Data-structure to keep per VMA info 
struct vma_info {
	struct dev_info *devinfo;
	int npagefaults;
};
struct vma_info *vma_inf;	//declaring global pointer which points to vma info. 

static const struct vm_operations_struct mykmod_vm_ops = {
	.open = mykmod_vm_open,
	.close = mykmod_vm_close,
	.fault = mykmod_vm_fault
};

int mykmod_major;
int i = 0;			//global variable to keep track of no.of devices.

static int mykmod_init_module(void)
{
	printk("mykmod loaded\n");
	printk("mykmod initialized at=%p\n", init_module);

	if ((mykmod_major =
	     register_chrdev(MYKMOD_DEV_MAJOR, "mykmod", &mykmod_fops)) < 0) {
		printk(KERN_WARNING "Failed to register character device\n");
		return 1;
	} else {
		printk("register character device %d\n", mykmod_major);
	}
	// TODO initialize device table

	return 0;
}

static void mykmod_cleanup_module(void)
{
	int j;
	printk("mykmod unloaded\n");
	unregister_chrdev(mykmod_major, "mykmod");
	// TODO free device info structures from device table
	//freeing all the devices through device table.
	for (j = 0; j < i; j++) {
		kfree(dev_table[j]->data);
		kfree(dev_table[j]);
	}

	return;
}

static int mykmod_open(struct inode *inodep, struct file *filep)
{
	printk("mykmod_open: filep=%p f->private_data=%p "
	       "inodep=%p i_private=%p i_rdev=%x maj:%d min:%d\n",
	       filep, filep->private_data,
	       inodep, inodep->i_private, inodep->i_rdev, MAJOR(inodep->i_rdev),
	       MINOR(inodep->i_rdev));

	// TODO: Allocate memory for devinfo and store in device table and i_private.
	if (inodep->i_private == NULL) {
		info = kmalloc(sizeof(struct dev_info), GFP_KERNEL);
		info->data = (char *)kzalloc(MYDEV_LEN, GFP_KERNEL);
		info->size = MYDEV_LEN;

		dev_table[i] = info;	//storing the device info in device table
		i++;

		inodep->i_private = info;	//also storing device info in i_private.
	}
	// Store device info in file's private_data as well
	filep->private_data = inodep->i_private;

	return 0;
}

static int mykmod_close(struct inode *inodep, struct file *filep)
{
	// TODO: Release memory allocated for data-structures.
	printk("mykmod_close: inodep=%p filep=%p\n", inodep, filep);

	return 0;
}

static int mykmod_mmap(struct file *filp, struct vm_area_struct *vma)
{
	printk("mykmod_mmap: filp=%p vma=%p flags=%lx\n", filp, vma,
	       vma->vm_flags);

	//TODO setup vma's flags, save private data (devinfo, npagefaults) in vm_private_data
	vma->vm_ops = &mykmod_vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

	vma_inf = kmalloc(sizeof(struct vma_info), GFP_KERNEL);
	vma_inf->devinfo = filp->private_data;	//pointing devinfo to device info (info) using filp->private_data.
	vma->vm_private_data = vma_inf;	//saving the private data of device in vm_private_data by pointing it to vma_inf.

	mykmod_vm_open(vma);

	//return -ENOSYS; // Remove this once mmap is implemented.
	return 0;
}

static void mykmod_vm_open(struct vm_area_struct *vma)
{
	vma_inf->npagefaults = 0;	//initializing the no.of page faults.
	printk("mykmod_vm_open: vma=%p npagefaults:%d\n", vma,
	       vma_inf->npagefaults);
}

static void mykmod_vm_close(struct vm_area_struct *vma)
{
	printk("mykmod_vm_close: vma=%p npagefaults:%d\n", vma,
	       vma_inf->npagefaults);
	vma_inf->npagefaults = 0;	//re-initializing no.of page faults to '0'.
	kfree(vma->vm_private_data);	//freeing the memory allocated for vma_inf struct in mykmod_mmap.
}

static int mykmod_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	unsigned long offset;
	struct page *pageptr = NULL;

	printk("mykmod_vm_fault: vma=%p vmf=%p pgoff=%lu page=%p\n", vma, vmf,
	       vmf->pgoff, vmf->page);
	// TODO: build virt->phys mappings

	vma_inf->npagefaults++;	//incrementing the no.of page faults, as this method is called if page fault occurs.

	offset = ((vmf->pgoff << PAGE_SHIFT) + (vma->vm_pgoff << PAGE_SHIFT));	//calculating the offset (adding both vma and vmf offsets).

	pageptr = virt_to_page(info->data + offset);	//getting the page by giving virtual address as argument.

	get_page(pageptr);	//to increment reference count for this page.
	vmf->page = pageptr;

	return 0;
}
