/*
 * kref.c - library routines for handling generic reference counted objects
 *
 * Copyright (C) 2004 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2004 IBM Corp.
 *
 * based on kobject.h which was:
 * Copyright (C) 2002-2003 Patrick Mochel <mochel@osdl.org>
 * Copyright (C) 2002-2003 Open Source Development Labs
 *
 * This file is released under the GPLv2.
 *
 */

#ifndef _KREF_H_
#define _KREF_H_

#include <linux/types.h>

struct kref {	/* 定义成结构体，以便于“类型检查” */
	atomic_t refcount;
};

void kref_set(struct kref *kref, int num);
void kref_init(struct kref *kref);										/* 初始化 “引用计数器 = 1”（当前共被被1个代码片引用） */
void kref_get(struct kref *kref);									    /* 引用某结构（引用计数器 +1） */
int kref_put(struct kref *kref, void (*release) (struct kref *kref)); /* 释放某结构（引用计数器 -1）*/

#endif /* _KREF_H_ */
