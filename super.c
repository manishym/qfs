/*
 * =====================================================================================
 *
 *       Filename:  super.c
 *
 *    Description:  a superblock for a filesystem I call the quick file system.
 *
 *        Version:  1.0
 *        Created:  Monday 08 February 2016 05:51:09  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Manish M Yathnalli (ymm), manish.ym@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "qfs.h"

static struct inode_operation qfs_inode_ops = {
    .lookup = qfs_lookup ;
}

struct inode *qfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev) {
    struct inode * inode = new_inode(sb) ;
    if (inode) {
        inode -> i_ino = get_next_ino();
        inode_init_owner(inode, dir, mode) ;
        inode -> i_atime = inode -> i_mtime = inode -> i_ctime = CURRENT_TIME ;
    }
    return inode ;
}
static void qfs_put_super(struct super_block *sb)
{
    pr_debug("qfs super block destroyed\n");

}

static struct super_operations const qfs_super_ops = {
    .put_super = qfs_put_super,

};

static int qfs_fill_sb(struct super_block *sb, void *data, int silent)
{
    struct inode *inode = NULL;

    sb->s_magic = QFS_MAGIC_NUMBER;
    sb->s_op = &qfs_super_ops;

    inode = qfs_get_inode(sb, NULL, S_IFDIR, 0);
    inode -> i_op = &qfs_inode_ops ;
    inode -> i_fop = &qfs_file_operations ;
    if (!inode)
    {
        pr_err("inode allocation failed\n");
        return -ENOMEM;
    }

    sb->s_root = d_make_root(inode);

    if (!sb->s_root)
    {
        pr_err("root creation failed\n");
        return -ENOMEM;
    }

    return 0;
}

static struct dentry *qfs_mount(struct file_system_type *type, int flags,
        char const *dev, void *data)
{
    struct dentry *const entry = mount_bdev(type, flags, dev,
            data, qfs_fill_sb);
    if (IS_ERR(entry))
        pr_err("qfs mounting failed\n");
    else
        pr_debug("qfs mounted\n");
    return entry;
}

static struct file_system_type qfs_type = {
    .owner = THIS_MODULE,
    .name = "qfs",
    .mount = qfs_mount,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV, 
};


static int __init qfs_init(void)
{
    int ret = register_filesystem(&qfs_type);
    if(likely(ret == 0)){
        pr_debug("Successfully registered qfs filesystem\n") ;
    } else {
        printk(KERN_ERR "Failed to register qfs") ;
    }

    pr_debug("qfs module loaded\n");
    return 0;
}

static void __exit qfs_fini(void)
{
    pr_debug("qfs module unloaded\n");
    unregister_filesystem(&qfs_type) ;
}
module_init(qfs_init);
module_exit(qfs_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manish M Yathnalli");
