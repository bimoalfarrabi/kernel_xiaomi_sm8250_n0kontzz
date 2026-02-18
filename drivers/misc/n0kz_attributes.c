#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/n0kz_attributes.h>

static char n0kz_blocked[N0KZ_MAX_BLOCKED][TASK_COMM_LEN];
static u8 n0kz_blocked_len[N0KZ_MAX_BLOCKED];
static int n0kz_blocked_cnt;

struct n0kz_attributes n0kz_data = {
	.kgsl_skip_zeroing = 0,
	.avoid_dirty_pte = 0,
	.bg_blocklist = "com.shopee.id,com.lazada.android,com.tokopedia.tkpd",
};

bool n0kz_comm_blocked(const char *comm)
{
	int i;

	for (i = 0; i < n0kz_blocked_cnt; i++) {
		if (!strncmp(comm, n0kz_blocked[i], n0kz_blocked_len[i]))
			return true;
	}

	return false;
}
EXPORT_SYMBOL_GPL(n0kz_comm_blocked);

static void n0kz_rebuild_blocklist(char *buf)
{
	char *p = buf;
	char *token;

	n0kz_blocked_cnt = 0;
	while ((token = strsep(&p, ",")) &&
	       n0kz_blocked_cnt < N0KZ_MAX_BLOCKED) {
		if (!*token)
			continue;

		strscpy(n0kz_blocked[n0kz_blocked_cnt], token, TASK_COMM_LEN);

		n0kz_blocked_len[n0kz_blocked_cnt] =
			strlen(n0kz_blocked[n0kz_blocked_cnt]);

		pr_alert("N0KZ: blocking '%s'\n",
			 n0kz_blocked[n0kz_blocked_cnt]);
		n0kz_blocked_cnt++;
	}
	pr_alert("N0KZ: total blocked apps = %d\n", n0kz_blocked_cnt);
}

static ssize_t bg_blocklist_show(struct kobject *kobj,
				 struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", n0kz_data.bg_blocklist);
}

static ssize_t bg_blocklist_store(struct kobject *kobj,
				  struct kobj_attribute *attr, const char *buf,
				  size_t count)
{
	char tmp[N0KZ_BLOCKLIST_STRLEN];

	strscpy(tmp, buf, sizeof(tmp));
	strreplace(tmp, '\n', '\0');
	strscpy(n0kz_data.bg_blocklist, tmp, sizeof(n0kz_data.bg_blocklist));

	n0kz_rebuild_blocklist(tmp);

	return count;
}

static struct kobj_attribute bg_blocklist_attr =
	__ATTR(bg_blocklist, 0664, bg_blocklist_show, bg_blocklist_store);

#define N0KZ_ATTR_RW(name)                                                     \
	static ssize_t name##_show(struct kobject *kobj,                       \
				   struct kobj_attribute *attr, char *buf)     \
	{                                                                      \
		return sprintf(buf, "%d\n", n0kz_data.name);                   \
	}                                                                      \
	static ssize_t name##_store(struct kobject *kobj,                      \
				    struct kobj_attribute *attr,               \
				    const char *buf, size_t count)             \
	{                                                                      \
		int val;                                                       \
		if (kstrtoint(buf, 10, &val))                                  \
			return -EINVAL;                                        \
		n0kz_data.name = val;                                          \
		return count;                                                  \
	}                                                                      \
	static struct kobj_attribute name##_attr =                             \
		__ATTR(name, 0644, name##_show, name##_store);

N0KZ_ATTR_RW(kgsl_skip_zeroing);
N0KZ_ATTR_RW(avoid_dirty_pte);

static struct attribute *n0kz_attrs[] = { &kgsl_skip_zeroing_attr.attr,
					  &avoid_dirty_pte_attr.attr,
					  &bg_blocklist_attr.attr, NULL };

static struct attribute_group n0kz_attr_group = {
	.attrs = n0kz_attrs,
};

static struct kobject *n0kz_kobj;

static int __init n0kz_attributes_init(void)
{
	int retval;
	char tmp[N0KZ_BLOCKLIST_STRLEN];

	if (n0kz_data.bg_blocklist[0]) {
		strscpy(tmp, n0kz_data.bg_blocklist, sizeof(tmp));
		n0kz_rebuild_blocklist(tmp);
	}

	n0kz_kobj = kobject_create_and_add("n0kz_attributes", kernel_kobj);
	if (!n0kz_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(n0kz_kobj, &n0kz_attr_group);
	if (retval)
		kobject_put(n0kz_kobj);

	return retval;
}

static void __exit n0kz_attributes_exit(void)
{
	kobject_put(n0kz_kobj);
}

module_init(n0kz_attributes_init);
module_exit(n0kz_attributes_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kvsnr113 <kvsnrprojkt113@gmail.com>");
MODULE_DESCRIPTION("N0KZ kgsl skip zeroing attribute module");
