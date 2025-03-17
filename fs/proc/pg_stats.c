#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>

// Add one entry per process
static int pg_stats_show(struct seq_file *m, void *v)
{
	struct task_struct *task;

	rcu_read_lock();
	for_each_process(task) {
		if (task->state == TASK_RUNNING ||
		    task->state == TASK_INTERRUPTIBLE ||
		    task->state == TASK_UNINTERRUPTIBLE) {
			seq_printf(
				m,
				"%d: [%llu,%llu,%llu], [%llu,%llu,%llu], [%llu,%llu,%llu], [%llu,%llu,%llu]\n",
				task->pid, task->pgd_alloc_count,
				task->pgd_free_count, task->pgd_set_count,
				task->pud_alloc_count, task->pud_free_count,
				task->pud_set_count, task->pmd_alloc_count,
				task->pmd_free_count, task->pmd_set_count,
				task->pte_alloc_count, task->pte_free_count,
				task->pte_set_count);
		}
	}
	rcu_read_unlock();
	return 0;
}

// Write the stats to the file
static int pg_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, pg_stats_show, NULL);
}

#ifdef CONFIG_PROC_FS

static const struct proc_ops pg_stats_proc_ops = {
	.proc_open = pg_stats_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#endif

// Create the file when the module is loaded
static int __init pg_stats_init(void)
{
	struct proc_dir_entry *entry;

#ifdef CONFIG_PROC_FS
	entry = proc_create("pg_stats", 0, NULL, &pg_stats_proc_ops);
	if (!entry) {
		pr_err("Failed to create /proc/pg_stats\n");
		return -ENOMEM;
	}
#endif
	pr_info("Created /proc/pg_stats\n");
	return 0;
}

// Delete the file when the module is unloaded
static void __exit pg_stats_exit(void)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry("pg_stats", NULL);
#endif
	pr_info("Deleted /proc/pg_stats\n");
}

module_init(pg_stats_init);
module_exit(pg_stats_exit);

MODULE_AUTHOR("Robin Jehn");
MODULE_DESCRIPTION("Shows page table stats");
