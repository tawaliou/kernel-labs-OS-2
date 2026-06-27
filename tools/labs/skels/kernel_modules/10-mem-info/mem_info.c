#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("Virtual Memory Areas of the Current process");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

// task_struct -> mm/active_mm (mm_struct) -> mmap (vm_area_struct) -> vm_next/vm_prev (vm_area_struct)

// vm_area_struct -> vm_next/vm_prev (vm_area_struct)

static int mem_info_init(void)
{
	pr_info("mem_info init\n");
	pr_info("pid: %d\n", current->pid);
	if (!current->mm)
		return 0;

	struct vm_area_struct *mem = current->mm->mmap;
	while (mem != NULL) {
		pr_info("[%lx; %lx]\n", mem->vm_start, mem->vm_end);
		mem = mem->vm_next;
	}

	return 0;
}

static void mem_info_exit(void)
{
	pr_info("mem_info exit\n");
}

module_init(mem_info_init);
module_exit(mem_info_exit);
