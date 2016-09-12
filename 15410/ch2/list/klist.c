#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

struct birthday {
	int day;
	int month;
	int year;
	struct list_head list;
};

static LIST_HEAD(birthday_list);

int klist_init(void) {
	int i = 0;
	struct birthday *ptr;
	printk(KERN_INFO "loading modules\n");
	for (; i < 5; i++) {
		struct birthday *person;
		person = kmalloc(sizeof(*person), GFP_KERNEL);
		person->day = i;
		person->month = i + 5;
		person->year = 1990 + i;
		INIT_LIST_HEAD(&person->list);
		list_add_tail(&person->list, &birthday_list);
	}
	list_for_each_entry(ptr, &birthday_list, list)	{
		printk(KERN_INFO "person day = %d\n", ptr->day);
		printk(KERN_INFO "person month = %d\n", ptr->month);
		printk(KERN_INFO "person year = %d\n", ptr->year);
	}
	return 0;
}

void klist_exit(void) {
	struct birthday *ptr, *next;
	list_for_each_entry_safe(ptr,next, &birthday_list, list)	{
		printk(KERN_INFO "del and free person day = %d\n", ptr->day);
		list_del(&ptr->list);
		kfree(ptr);
	}
}

module_init(klist_init);
module_exit(klist_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("list traverse");
MODULE_AUTHOR("Han");
