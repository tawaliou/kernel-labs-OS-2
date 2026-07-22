// SPDX-License-Identifier: GPL-2.0+

/*
 * list.c - Linux kernel list API
 *
 * TODO 1/0: Fill in name / email
 * Author: Tawaliou ALAO <alaotawaliou@gmail.com>
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#define PROCFS_MAX_SIZE 512
#define PROCFS_CMD_MAX_SIZE 5
#define PROCFS_NAME_MAX_SIZE (PROCFS_MAX_SIZE - PROCFS_CMD_MAX_SIZE - 1)

#define CDM_ADDF "addf"
#define CDM_ADDE "adde"
#define CDM_DELF "delf"
#define CDM_DELA "dela"

#define procfs_dir_name "list"
#define procfs_file_read "preview"
#define procfs_file_write "management"

struct proc_dir_entry *proc_list;
struct proc_dir_entry *proc_list_read;
struct proc_dir_entry *proc_list_write;

/* TODO 2: define your list! */
struct list_proc_data {
	char name[PROCFS_NAME_MAX_SIZE];
	struct list_head list;
};

struct list_head head;

static struct list_proc_data *list_proc_alloc(char *name)
{
	struct list_proc_data *lp;

	lp = kmalloc(sizeof(*lp), GFP_KERNEL);
	if (lp == NULL)
		return NULL;

	memcpy(lp->name, name, PROCFS_NAME_MAX_SIZE);

	return lp;
}

static void list_proc_addf(char *name)
{
	struct list_proc_data *lp;

	lp = list_proc_alloc(name);
	list_add(&lp->list, &head);
}

static void list_proc_adde(char *name)
{
	struct list_proc_data *lp;

	lp = list_proc_alloc(name);
	list_add_tail(&lp->list, &head);
}

static void list_proc_delf(char *name)
{
	struct list_head *pos;
	struct list_head *n;
	struct list_proc_data *lp;

	list_for_each_safe (pos, n, &head) {
		lp = list_entry(pos, struct list_proc_data, list);
		if (!strcmp(lp->name, name)) {
			list_del(pos);
			kfree(lp);
			return;
		}
	}
}

static void list_proc_dela(char *name)
{
	struct list_head *pos;
	struct list_head *n;
	struct list_proc_data *lp;

	list_for_each_safe (pos, n, &head) {
		lp = list_entry(pos, struct list_proc_data, list);
		if (!strcmp(lp->name, name)) {
			list_del(pos);
			kfree(lp);
		}
	}
}

static int list_proc_show(struct seq_file *m, void *v)
{
	struct list_head *pos;
	struct list_proc_data *tmp;
	/* TODO 3: print your list. One element / line. */
	// seq_puts(m, "Remove this line\n");
	list_for_each (pos, &head) {
		tmp = list_entry(pos, struct list_proc_data, list);
		seq_printf(m, "%s", tmp->name);
	}

	return 0;
}

static void list_proc_purge(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct list_proc_data *tmp;

	list_for_each_safe (pos, n, &head) {
		tmp = list_entry(pos, struct list_proc_data, list);
		list_del(pos);
		kfree(tmp);
	}
}

static int list_read_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_write_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static ssize_t list_write(struct file *file, const char __user *buffer,
			  size_t count, loff_t *offs)
{
	char local_buffer[PROCFS_MAX_SIZE];
	unsigned long local_buffer_size = 0;
	char cmd[PROCFS_CMD_MAX_SIZE];
	char *name = local_buffer + PROCFS_CMD_MAX_SIZE;

	local_buffer_size = count;
	if (local_buffer_size > PROCFS_MAX_SIZE)
		local_buffer_size = PROCFS_MAX_SIZE;

	memset(local_buffer, 0, PROCFS_MAX_SIZE);
	if (copy_from_user(local_buffer, buffer, local_buffer_size))
		return -EFAULT;

	/* local_buffer contains your command written in /proc/list/management
	 * TODO 4/0: parse the command and add/delete elements.
	 */
	memcpy(cmd, local_buffer, PROCFS_CMD_MAX_SIZE - 1);
	cmd[PROCFS_CMD_MAX_SIZE - 1] = '\0';

	// memcpy(name, local_buffer + PROCFS_CMD_MAX_SIZE,
	//        PROCFS_NAME_MAX_SIZE - 1);
	name[PROCFS_NAME_MAX_SIZE - 1] = '\0';

	if (!strcmp(cmd, CDM_ADDF)) {
		list_proc_addf(name);
	} else if (!strcmp(cmd, CDM_ADDE)) {
		list_proc_adde(name);
	} else if (!strcmp(cmd, CDM_DELF)) {
		list_proc_delf(name);
	} else if (!strcmp(cmd, CDM_DELA)) {
		list_proc_dela(name);
	}

	return local_buffer_size;
}

static const struct proc_ops r_pops = {
	.proc_open = list_read_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

static const struct proc_ops w_pops = {
	.proc_open = list_write_open,
	.proc_write = list_write,
	.proc_release = single_release,
};

static int list_init(void)
{
	proc_list = proc_mkdir(procfs_dir_name, NULL);
	if (!proc_list)
		return -ENOMEM;

	proc_list_read =
		proc_create(procfs_file_read, 0000, proc_list, &r_pops);
	if (!proc_list_read)
		goto proc_list_cleanup;

	proc_list_write =
		proc_create(procfs_file_write, 0000, proc_list, &w_pops);
	if (!proc_list_write)
		goto proc_list_read_cleanup;

	INIT_LIST_HEAD(&head);

	return 0;

proc_list_read_cleanup:
	proc_remove(proc_list_read);
proc_list_cleanup:
	proc_remove(proc_list);
	return -ENOMEM;
}

static void list_exit(void)
{
	proc_remove(proc_list);
	list_proc_purge();
}

module_init(list_init);
module_exit(list_exit);

MODULE_DESCRIPTION("Linux kernel list API");
/* TODO 5: Fill in your name / email address */
MODULE_AUTHOR("Tawaliou ALAO <alaotawaliou@gmail.com>");
MODULE_LICENSE("GPL v2");
