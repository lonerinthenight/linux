/*
 * fs/sysfs/sysfs.h - sysfs internal header file
 *
 * Copyright (c) 2001-3 Patrick Mochel
 * Copyright (c) 2007 SUSE Linux Products GmbH
 * Copyright (c) 2007 Tejun Heo <teheo@suse.de>
 *
 * This file is released under the GPLv2.
 */

#include <linux/fs.h>

struct sysfs_open_dirent;

/* type-specific structures for sysfs_dirent->s_* union members */
struct sysfs_elem_dir {
	struct kobject		*kobj;
	/* children list starts here and goes through sd->s_sibling */
	struct sysfs_dirent	*children;
};

struct sysfs_elem_symlink {
	struct sysfs_dirent	*target_sd;
};

struct sysfs_elem_attr {
	struct attribute	*attr;
	struct sysfs_open_dirent *open;
};

struct sysfs_elem_bin_attr {
	struct bin_attribute	*bin_attr;
	struct hlist_head	buffers;
};

struct sysfs_inode_attrs {
	struct iattr	ia_iattr;		/* -> *file -> f_dentry -> inode -> f_data    */
									/*       |---> f_mapping (page_cache_of_file) */
	void			*ia_secdata;
	u32				ia_secdata_len;
};

/*
 * sysfs_dirent - the building block of sysfs hierarchy.  Each and
 * every sysfs node is represented by single sysfs_dirent.
 *
 * As long as s_count reference is held, the sysfs_dirent itself is
 * accessible.  Dereferencing s_elem or any other outer entity
 * requires s_active reference.
 */
 /* sysfs：以“文件系统”形式呈现给用户的“设备（含伪设备）管理接口”。初衷是便于“device model的调试”，
 	1. “sysfs目录项” 与 kobject 相结合
	2. sysfs 文件系统树结构（ 父亲-兄弟 形式表示） 
	扩展：https://www.freedesktop.org/wiki/Software/hal/     */
struct sysfs_dirent {		
	atomic_t				s_count;
	atomic_t				s_active;
	struct sysfs_dirent		*s_parent;
	struct sysfs_dirent		*s_sibling;
	const char				*s_name;

	union {
		struct sysfs_elem_dir		s_dir;
		struct sysfs_elem_symlink	s_symlink;
		struct sysfs_elem_attr		s_attr;
		struct sysfs_elem_bin_attr	s_bin_attr;
	};

	unsigned int		s_flags;
	ino_t				s_ino;
	umode_t				s_mode;
	struct sysfs_inode_attrs *s_iattr;	/* sysfs目录项与kobject结合的关键：*file(f_dentry<inode、data>, f_mapping)*/
};

#define SD_DEACTIVATED_BIAS		INT_MIN

#define SYSFS_TYPE_MASK			0x00ff
#define SYSFS_DIR			0x0001
#define SYSFS_KOBJ_ATTR			0x0002
#define SYSFS_KOBJ_BIN_ATTR		0x0004
#define SYSFS_KOBJ_LINK			0x0008
#define SYSFS_COPY_NAME			(SYSFS_DIR | SYSFS_KOBJ_LINK)

#define SYSFS_FLAG_MASK			~SYSFS_TYPE_MASK
#define SYSFS_FLAG_REMOVED		0x0200

static inline unsigned int sysfs_type(struct sysfs_dirent *sd)
{
	return sd->s_flags & SYSFS_TYPE_MASK;
}

/*
 * Context structure to be used while adding/removing nodes.
 */
struct sysfs_addrm_cxt {
	struct sysfs_dirent	*parent_sd;
	struct inode		*parent_inode;
	struct sysfs_dirent	*removed;
	int			cnt;
};

/*
 * mount.c
 */
extern struct sysfs_dirent sysfs_root;
extern struct super_block *sysfs_sb;
extern struct kmem_cache *sysfs_dir_cachep;

/*
 * dir.c
 */
extern struct mutex sysfs_mutex;
extern struct mutex sysfs_rename_mutex;
extern spinlock_t sysfs_assoc_lock;

extern const struct file_operations sysfs_dir_operations;
extern const struct inode_operations sysfs_dir_inode_operations;

struct dentry *sysfs_get_dentry(struct sysfs_dirent *sd);
struct sysfs_dirent *sysfs_get_active_two(struct sysfs_dirent *sd);
void sysfs_put_active_two(struct sysfs_dirent *sd);
void sysfs_addrm_start(struct sysfs_addrm_cxt *acxt,
		       struct sysfs_dirent *parent_sd);
int __sysfs_add_one(struct sysfs_addrm_cxt *acxt, struct sysfs_dirent *sd);
int sysfs_add_one(struct sysfs_addrm_cxt *acxt, struct sysfs_dirent *sd);
void sysfs_remove_one(struct sysfs_addrm_cxt *acxt, struct sysfs_dirent *sd);
void sysfs_addrm_finish(struct sysfs_addrm_cxt *acxt);

struct sysfs_dirent *sysfs_find_dirent(struct sysfs_dirent *parent_sd,
				       const unsigned char *name);
struct sysfs_dirent *sysfs_get_dirent(struct sysfs_dirent *parent_sd,
				      const unsigned char *name);
struct sysfs_dirent *sysfs_new_dirent(const char *name, umode_t mode, int type);

void release_sysfs_dirent(struct sysfs_dirent *sd);

int sysfs_create_subdir(struct kobject *kobj, const char *name,
			struct sysfs_dirent **p_sd);
void sysfs_remove_subdir(struct sysfs_dirent *sd);

static inline struct sysfs_dirent *__sysfs_get(struct sysfs_dirent *sd)
{
	if (sd) {
		WARN_ON(!atomic_read(&sd->s_count));
		atomic_inc(&sd->s_count);
	}
	return sd;
}
#define sysfs_get(sd) __sysfs_get(sd)

static inline void __sysfs_put(struct sysfs_dirent *sd)
{
	if (sd && atomic_dec_and_test(&sd->s_count))
		release_sysfs_dirent(sd);
}
#define sysfs_put(sd) __sysfs_put(sd)

/*
 * inode.c
 */
struct inode *sysfs_get_inode(struct sysfs_dirent *sd);
void sysfs_delete_inode(struct inode *inode);
int sysfs_setattr(struct dentry *dentry, struct iattr *iattr);
int sysfs_setxattr(struct dentry *dentry, const char *name, const void *value,
		size_t size, int flags);
int sysfs_hash_and_remove(struct sysfs_dirent *dir_sd, const char *name);
int sysfs_inode_init(void);

/*
 * file.c
 */
extern const struct file_operations sysfs_file_operations;

int sysfs_add_file(struct sysfs_dirent *dir_sd,
		   const struct attribute *attr, int type);

int sysfs_add_file_mode(struct sysfs_dirent *dir_sd,
			const struct attribute *attr, int type, mode_t amode);
/*
 * bin.c
 */
extern const struct file_operations bin_fops;
void unmap_bin_file(struct sysfs_dirent *attr_sd);

/*
 * symlink.c
 */
extern const struct inode_operations sysfs_symlink_inode_operations;
