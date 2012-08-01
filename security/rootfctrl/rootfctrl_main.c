/*
 *  HTC - ROOT File Control
 *
 *  This file contains the rootfctrl hook function implementations.
 *
 *  Authors:  Mike Wu <mike_wu@htc.com>
 *
 *  Copyright (C) 2011 HTC Corp, Mike Wu <mike_wu@htc.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2,
 *  as published by the Free Software Foundation.
 */


#include <linux/security.h>
#include <linux/namei.h>

#define RTFCTL_NORMAL_MODE      1
#define RTFCTL_TRACKING_MODE    2
#define RTFCTL_TEST_MODE        3
#define RTFCTL_RUN_MODE         RTFCTL_NORMAL_MODE

#define ROOTFCTRL_DEBUG         0

#if ROOTFCTRL_DEBUG
#define RTFCTL_MSG(s...) printk("[RTFCTL]" s)
#else
#define RTFCTL_MSG(s...) do {}while(0)
#endif

char *buf;
int need_chk = 0;
pid_t zygote_pid = -1, adbd_pid = -1, cur_pid = -1, flc_daemon_pid = -1, flc_agent_pid = -1, installd_pid = -1;

#define UID_FELICA_LOCK_APP         9990
#define UID_MOBILE_FELICA_CLIENT    9989

#define RTFCTL_FELICA_UID_NUM    2
uid_t felica_uid[16] = {UID_FELICA_LOCK_APP, UID_MOBILE_FELICA_CLIENT};

#define RTFCTL_FELICA_FILE_NUM    0
char *felica_file[16] = {"/data/data/jp.co.fsi.felicalock", "/data/data/com.felicanetworks.mfc",
    "/data/dalvik-cache/system@app@KDDI_Felicalock.apk@classes.dex",
    "/data/dalvik-cache/system@app@MobileFeliCaClient.apk@classes.dex"};

#define RTFCTL_FELICA_CFG_NUM    4
char *felica_cfg[16] = {"/system/etc/felica/bookmark.cfg", "/system/etc/felica/common.cfg",
    "/system/etc/felica/mfm.cfg", "/system/etc/felica/mfs.cfg"};

#define RTFCTL_FELICA_APK_NUM    8
char *felica_apk[16] = {"/system/app/MobileFeliCaClient.apk", "/system/app/MobileFeliCaSettingApp.apk",
    "/system/app/MobileFeliCaMenuApp.apk", "/system/app/MobileFeliCaWebPluginBoot.apk",
    "/system/app/MobileFeliCaClient.odex", "/system/app/MobileFeliCaSettingApp.odex",
    "/system/app/MobileFeliCaMenuApp.odex", "/system/app/MobileFeliCaWebPluginBoot.odex"};

#define RTFCTL_FELICA_DEV_NUM    8
char *felica_dev[16] = {"/dev/felica", "/dev/felica_pon", "/dev/felica_cen", "/dev/felica_rfs", "/dev/felica_rws",
            "/dev/felica_int", "/dev/felica_int_poll", "/dev/felica_uid"};
unsigned int felica_dev_t[16][2] = {{91, 0}, {92, 0}, {93, 0}, {94, 0}, {95, 0}, {96, 0}, {97, 0}, {98, 0}};
unsigned int felica_dev_mode[16] = {0666, 0666, 0666, 0444, 0666, 0444, 0400, 0222}; //TBD


#if 0   //(RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
char *felica_file_name[16] = {"felica", "felica8", "felica3"};
static int is_felica_file_name(const char *name)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_FILE_NUM; i++) {
        if (!strcmp(name, felica_file_name[i]))
            return true;
    }

    return false;
}
#endif

static char *get_full_path(struct path *path, struct dentry *dentry, char *buf)
{
    char *full_path = 0;

    /* get full or parent path */
    memset(buf, 0, PATH_MAX);
    full_path = d_path(path, buf, PATH_MAX);
    if (!full_path) {
        printk(KERN_WARNING"[ERR] %s: d_path fail...\n", __FUNCTION__);
        return 0;
    }

    if (dentry) {
        strcat(full_path, "/");
        strcat(full_path, dentry->d_name.name);
    }

    return full_path;
}

#define RTFCTL_FILE_TYPE_NONE   0
#define RTFCTL_FILE_TYPE_NORMAL 1
#define RTFCTL_FILE_TYPE_DEVICE 2
#define RTFCTL_FILE_TYPE_APK    3
#define RTFCTL_FILE_TYPE_CFG    4
static int is_felica_file(const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_FILE_NUM; i++) {
        if (!strcmp(full_path, felica_file[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_NORMAL;
        }
    }

    for (i = 0; i < RTFCTL_FELICA_CFG_NUM; i++) {
        if (!strcmp(full_path, felica_cfg[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_CFG;
        }
    }

    for (i = 0; i < RTFCTL_FELICA_APK_NUM; i++) {
        if (!strcmp(full_path, felica_apk[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_APK;
        }
    }

    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        if (!strcmp(full_path, felica_dev[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_DEVICE;
        }
    }

    return RTFCTL_FILE_TYPE_NONE;
}

static int is_felica_RWP_file(const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        if (!strcmp(full_path, felica_dev[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_DEVICE;
        }
    }

    return RTFCTL_FILE_TYPE_NONE;
}

static int is_felica_WP_file(const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_FILE_NUM; i++) {
        if (!strcmp(full_path, felica_file[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_NORMAL;
        }
    }

    for (i = 0; i < RTFCTL_FELICA_CFG_NUM; i++) {
        if (!strcmp(full_path, felica_cfg[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_CFG;
        }
    }

    for (i = 0; i < RTFCTL_FELICA_APK_NUM; i++) {
        if (!strcmp(full_path, felica_apk[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            return RTFCTL_FILE_TYPE_APK;
        }
    }

    return RTFCTL_FILE_TYPE_NONE;
}

static int is_felica_uid(uid_t uid)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_UID_NUM; i++) {
        if (felica_uid[i] == uid)
            return true;
    }
    return false;
}

static int is_non_felica_root(uid_t uid, pid_t pid)
{
    if (uid == 0) {
        if(pid != flc_daemon_pid && pid != flc_agent_pid)
            return true;
        else {
            RTFCTL_MSG("Felica Root: %s\n", (pid == flc_daemon_pid)? "felica_daemon": "felica_agent");
            return false;
        }
    }

    return false;
}

#define RTFCTL_FELICA_DEV_NONE      0
#define RTFCTL_FELICA_DEV_VALID     1
#define RTFCTL_FELICA_DEV_INVALID   -1
#define RTFCTL_FELICA_DEV_FAKE      -2
static int is_felica_dev(unsigned int major,unsigned int minor, const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        if (!strcmp(full_path, felica_dev[i])) {
            RTFCTL_MSG("file: %s (%u, %u)\n", full_path, major, minor);
            if (felica_dev_t[i][0] == major && felica_dev_t[i][1] == minor)
                return RTFCTL_FELICA_DEV_VALID;
            else
                return RTFCTL_FELICA_DEV_INVALID;
        }
    }
#if 1 /* major number is the same as others */
    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        /* kernel find the driver according to major number */
        if(felica_dev_t[i][0] == major) {
            RTFCTL_MSG("fake file: %s (%u, %u)\n", full_path, major, minor);
            return RTFCTL_FELICA_DEV_FAKE;
        }
    }
#endif
    return RTFCTL_FELICA_DEV_NONE;
}

static int is_felica_mode_valid(mode_t mode, const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_FILE_NUM; i++) {
        if (!strcmp(full_path, felica_file[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if (!(mode & 002) && current_uid() == 0)
                return true;
            else {
                RTFCTL_MSG("uid: %d\n", current_uid());
                RTFCTL_MSG("mode: %o\n", mode);
                return false;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_CFG_NUM; i++) {
        if (!strcmp(full_path, felica_cfg[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if (mode == 0444 && current_uid() == 0)
                return true;
            else {
                RTFCTL_MSG("uid: %d\n", current_uid());
                RTFCTL_MSG("mode: %o\n", mode);
                return false;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_APK_NUM; i++) {
        if (!strcmp(full_path, felica_apk[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if (!(mode & 007) && current_uid() == 0)
                return true;
            else {
                RTFCTL_MSG("uid: %d\n", current_uid());
                RTFCTL_MSG("mode: %o\n", mode);
                return false;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        if (!strcmp(full_path, felica_dev[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if ((felica_dev_mode[i] == -1 || mode == felica_dev_mode[i]) && current_uid() == 0)
                return true;
            else {
                RTFCTL_MSG("uid: %d\n", current_uid());
                RTFCTL_MSG("mode: %o\n", mode);
                return false;
            }
        }
    }

    return true;
}

static int is_felica_owner_valid(uid_t uid, gid_t gid, const char *full_path)
{
    int i;

    for (i = 0; i < RTFCTL_FELICA_FILE_NUM; i++) {
        if (!strcmp(full_path, felica_file[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if ((uid == -1 || is_felica_uid(uid)) || (gid == -1 || is_felica_uid(gid)))
                return 1;
            else {
                RTFCTL_MSG("uid: %d\n", uid);
                RTFCTL_MSG("gid: %d\n", gid);
                return -1;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_CFG_NUM; i++) {
        if (!strcmp(full_path, felica_cfg[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if ((uid == -1 || uid == 0) && (gid == -1 || gid == 0))
                return 1;
            else {
                RTFCTL_MSG("uid: %d\n", uid);
                RTFCTL_MSG("gid: %d\n", gid);
                return -1;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_APK_NUM; i++) {
        if (!strcmp(full_path, felica_apk[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if ((uid == -1 || is_felica_uid(uid)) && (gid == -1 || is_felica_uid(gid)))
                return 1;
            else {
                RTFCTL_MSG("uid: %d\n", uid);
                RTFCTL_MSG("gid: %d\n", gid);
                return -1;
            }
        }
    }

    for (i = 0; i < RTFCTL_FELICA_DEV_NUM; i++) {
        if (!strcmp(full_path, felica_dev[i])) {
            RTFCTL_MSG("file: %s\n", full_path);
            if ((uid == -1 || is_felica_uid(uid)) && (gid == -1 || is_felica_uid(gid)))
                return 1;
            else {
                RTFCTL_MSG("uid: %d\n", uid);
                RTFCTL_MSG("gid: %d\n", gid);
                return -1;
            }
        }
    }

    return 0;
}

/**
 * Security hook for dentry
 */
static int rootfctrl_dentry_open(struct file *file, const struct cred *cred)
{
    char *full_path;
    pid_t pid;

    full_path = get_full_path(&file->f_path, NULL, buf);
    if (is_felica_RWP_file(full_path)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("pid: %d\n", task_tgid_vnr(current));
        RTFCTL_MSG("uid: %d\n", current_uid());
        RTFCTL_MSG("euid: %d\n", current_euid());
        RTFCTL_MSG("suid: %d\n", current_suid());
#endif
        /* root or access through setuid program are denied */
        if (is_non_felica_root(current_uid(), task_tgid_vnr(current)) || (current_uid() != current_euid())) {
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            char tcomm[sizeof(current->comm)];
            get_task_comm(tcomm, current);
            RTFCTL_MSG("pid: %d (%s)\n", task_tgid_vnr(current), tcomm);
            RTFCTL_MSG("uid: %d\n", current_uid());
            RTFCTL_MSG("euid: %d\n", current_euid());
            RTFCTL_MSG("suid: %d\n", current_suid());
#endif
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            return -EACCES;
#endif
        }
    } else if (is_felica_WP_file(full_path)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("pid: %d\n", task_tgid_vnr(current));
        RTFCTL_MSG("uid: %d\n", current_uid());
        RTFCTL_MSG("euid: %d\n", current_euid());
        RTFCTL_MSG("suid: %d\n", current_suid());
#endif
        pid = task_tgid_vnr(current);
        if (pid == adbd_pid) {
            RTFCTL_MSG(KERN_INFO "ADB Rejected...\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            return -EACCES;
#endif
        }
    }
    return 0;
}

/**
 *  Security hooks for file operations
 */
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
static int rootfctrl_file_permission(struct file *file, int mask)
{
    if ((mask & 2) && is_felica_file(get_full_path(&file->f_path, NULL, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("mask: %d\n", mask);

        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Want to Rejected (but Can't)...\n");
            //return -EACCES;

        }
    }
    return 0;
}

static int rootfctrl_file_ioctrl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (is_felica_file(get_full_path(&file->f_path, NULL, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("cmd: %x, arg: %lx\n", cmd, arg);

        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Want to Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
}

static int rootfctrl_file_fcntl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (is_felica_file(get_full_path(&file->f_path, NULL, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("cmd: %x, arg: %lx\n", cmd, arg);

        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Want to Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
}

static int rootfctrl_file_set_fowner(struct file *file)
{
    if (is_felica_file(get_full_path(&file->f_path, NULL, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Want to Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
}
#endif
/**
 *  Security hooks for path operations
 */
static int rootfctrl_path_unlink(struct path *dir, struct dentry *dentry)
{
    if (is_felica_file(get_full_path(dir, dentry, buf))) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
#endif
        if (is_non_felica_root(current_uid(), task_tgid_vnr(current))) {
            char tcomm[sizeof(current->comm)];
            get_task_comm(tcomm, current);
            RTFCTL_MSG("pid: %d (%s)\n", task_tgid_vnr(current), tcomm);
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            return -EACCES;
#endif
        }
    }
    return 0;
}

#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
static int rootfctrl_path_mkdir(struct path *dir, struct dentry *dentry, int mode)
{
    if (is_felica_file(get_full_path(dir, dentry, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("mode: %o\n", mode);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
}
static int rootfctrl_path_rmdir(struct path *dir, struct dentry *dentry)
{
    if (is_felica_file(get_full_path(dir, dentry, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
}
#endif

static int rootfctrl_path_mknod(struct path *dir, struct dentry *dentry, int mode, unsigned int dev)
{
    dev_t dev_num;
    int ret, format = mode & S_IFMT;

    if (format == S_IFCHR || format == S_IFBLK) {
        dev_num = new_decode_dev(dev);
        ret = is_felica_dev(MAJOR(dev_num), MINOR(dev_num), get_full_path(dir, dentry, buf));

        /* can't make node */
        if (ret < 0) {
            RTFCTL_MSG("########## %s(device node) ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
            RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
            RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
            RTFCTL_MSG("major:%d minor:%d\n", MAJOR(dev_num), MINOR(dev_num));
            RTFCTL_MSG("mode: %o\n", mode);
#else
            RTFCTL_MSG(KERN_INFO "Node not match, Rejected...(%d)\n", ret);
            return -EACCES;
#endif
        }

        if (ret && !is_felica_uid(current_uid()) && current_uid() != 0) {
            RTFCTL_MSG(KERN_INFO "Felica Node, current UID wrong, Rejected...\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            return -EACCES;
#endif
        }
    }
    else if (is_felica_file(get_full_path(dir, dentry, buf))) {
        RTFCTL_MSG("########## %s(regular file) ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        dev_num = new_decode_dev(dev);
        RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("major:%d minor:%d\n", MAJOR(dev_num), MINOR(dev_num));
        RTFCTL_MSG("mode: %o\n", mode);
#else
        RTFCTL_MSG(KERN_INFO "Can't create Felica file dynamicly, Rejected...\n");
        return -EACCES;
#endif
    }

    return 0;
}

#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
static int rootfctrl_path_truncate(struct path *path)
{
    if (is_felica_file(get_full_path(path, NULL, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("path: %s\n", get_full_path(path, NULL, buf));
    }
    return 0;
}

static int rootfctrl_path_symlink(struct path *dir, struct dentry *dentry, const char *old_name)
{
    struct path path;

    path.mnt = dir->mnt;
    path.dentry = dentry;

    if (!strcmp(old_name, "../felica")) {
    //if (is_felica_file(construct_full_path(dir, old_name, buf))) {
        RTFCTL_MSG("*****>>>>> %s <<<<<*****\n", __FUNCTION__);
        RTFCTL_MSG("path name: %s\n", get_full_path(&path, NULL, buf));
        RTFCTL_MSG("parent: %s\n", get_full_path(dir, NULL, buf));
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("old_name: %s\n", old_name);

        /* can't create link to felica file anyway (or we can't control it...) */
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        //return -EACCES;

    }
    return 0;
}
#endif

static int rootfctrl_path_link(struct dentry *old_dentry, struct path *new_dir, struct dentry *new_dentry)
{
    struct path path;

    path.mnt = new_dir->mnt;
    path.dentry = old_dentry;

    if (is_felica_file(get_full_path(&path, NULL, buf))) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("parent: %s\n", get_full_path(new_dir, NULL, buf));
        RTFCTL_MSG("old_dentry: %s\n", old_dentry->d_name.name);
        RTFCTL_MSG("new_dentry: %s\n", new_dentry->d_name.name);
#else
        /* can't create link to felica file anyway (or we can't control it...) */
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        return -EACCES;
#endif
    }
    return 0;
}

static int rootfctrl_path_rename(struct path *old_dir, struct dentry *old_dentry,
                    struct path *new_dir, struct dentry *new_dentry)
{
#if (RTFCTL_RUN_MODE == RTFCTL_TEST_MODE)
    if (is_felica_file(get_full_path(old_dir, old_dentry, buf)))
#else
    if (is_felica_file(get_full_path(old_dir, old_dentry, buf))
        || is_felica_file(get_full_path(new_dir, new_dentry, buf)))
#endif
    {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("old_parent: %s\n", get_full_path(old_dir, NULL, buf));
        RTFCTL_MSG("old_dentry: %s\n", old_dentry->d_name.name);
        RTFCTL_MSG("new_parent: %s\n", get_full_path(new_dir, NULL, buf));
        RTFCTL_MSG("new_dentry: %s\n", new_dentry->d_name.name);
#else
        /* can't rename anyway (or we can't control it...) */
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        return -EACCES;
#endif
    }
    return 0;
}

static int rootfctrl_path_chmod(struct dentry *dentry, struct vfsmount *mnt, mode_t mode)
{
    struct path path;

    path.mnt = mnt;
    path.dentry = dentry;
    if (!is_felica_mode_valid(mode, get_full_path(&path, NULL, buf))) {
        char tcomm[sizeof(current->comm)];
        get_task_comm(tcomm, current);
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("path: %s\n", get_full_path(&path, NULL, buf));
        RTFCTL_MSG("mode: %o\n", mode);
#else
        RTFCTL_MSG("pid: %d (%s)\n", task_tgid_vnr(current), tcomm);
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        return -EACCES;
#endif
    }

    return 0;
}

static int rootfctrl_path_chown(struct path *path, uid_t uid, gid_t gid)
{
    int ret;

    ret = is_felica_owner_valid(uid, gid, get_full_path(path, NULL, buf));
    if (ret != 0) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("path: %s\n", get_full_path(path, NULL, buf));
        RTFCTL_MSG("uid: %d gid: %d\n", uid, gid);
#endif
        if (ret < 0) {
            RTFCTL_MSG("File path: %s\n", get_full_path(path, NULL, buf));
            RTFCTL_MSG(KERN_INFO "Change Felica file UID/GID Rejected...\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            return -EACCES;
#endif
        }
    } else {
        if ((uid != -1 && is_felica_uid(uid)) || (gid != -1 && is_felica_uid(gid))) {
            RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
            RTFCTL_MSG("File path: %s\n", get_full_path(path, NULL, buf));
            RTFCTL_MSG(KERN_INFO "Change other file to Felica UID/GID\n");
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
            if(task_tgid_vnr(current) != installd_pid) {
                RTFCTL_MSG(KERN_INFO "Rejected...\n");
                return -EACCES;
            }
#endif
        }
    }
    return 0;
}

/**
 * Security hooks for inode operations
 */
#if 0   //(RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
static int rootfctrl_inode_create (struct inode *dir, struct dentry *dentry, int mode)
{
    if (is_felica_file_name(dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_create <<<<<*****\n");
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("mode: %o\n", mode);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }
    return 0;
};

static int rootfctrl_inode_link(struct dentry *old_dentry, struct inode *dir, struct dentry *new_dentry)
{
    if (is_felica_file_name(old_dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_link <<<<<*****\n");
        RTFCTL_MSG("old_dentry: %s\n", old_dentry->d_name.name);

        /* can't create link to felica file currently */
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        //return -EACCES;
    }

    return 0;
}

static int rootfctrl_inode_unlink(struct inode *dir, struct dentry *dentry)
{
    if (is_felica_file_name(dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_unlink <<<<<*****\n");
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_symlink(struct inode *dir, struct dentry *dentry, const char *name)
{
    if (is_felica_file_name(name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_symlink <<<<<*****\n");
        RTFCTL_MSG("%s -> %s\n", dentry->d_name.name, name);

        /* can't create link to felica file currently */
        RTFCTL_MSG(KERN_INFO "Rejected...\n");
        //return -EACCES;
    }

    return 0;
}

static int rootfctrl_inode_mkdir(struct inode *dir, struct dentry *dentry, int mask)
{
    if (is_felica_file_name(dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_mkdir <<<<<*****\n");
        RTFCTL_MSG("mask: %d\n", mask);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_rmdir(struct inode *dir, struct dentry *dentry)
{
    if (is_felica_file_name(dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_rmdir <<<<<*****\n");
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
    int ret = 0;

    ret = is_felica_dev(MAJOR(dev), MINOR(dev), dentry->d_name.name);
    /* can't make node */
    if (ret < 0) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_mknod <<<<<*****\n");
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("major:%d minor:%d\n", MAJOR(dev), MINOR(dev));
        RTFCTL_MSG(KERN_INFO "Not Felica Node Rejected...(%d)\n", ret);
        //return -EACCES;
    }

    if (ret && !is_felica_uid(current_uid()) && current_uid() != 0) {
        RTFCTL_MSG(KERN_INFO "Felica Node but Not Root Rejected...\n");
        //return -EACCES;
    }

    return 0;
}

static int rootfctrl_inode_rename(struct inode *old_inode, struct dentry *old_dentry,
				struct inode *new_inode, struct dentry *new_dentry)
{
    if (is_felica_file_name(old_dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_rename <<<<<*****\n");
        RTFCTL_MSG("old_dentry: %s\n", old_dentry->d_name.name);
        RTFCTL_MSG("new_dentry: %s\n", new_dentry->d_name.name);
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_readlink(struct dentry *dentry)
{
    if (is_felica_file_name(dentry->d_name.name)) { // link name, not linked file name
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_readlink <<<<<*****\n");
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_follow_link(struct dentry *dentry, struct nameidata *nameidata)
{
    if (is_felica_file_name(dentry->d_name.name)) { // link name, not linked file name
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_follow_link <<<<<*****\n");
        RTFCTL_MSG("name: %s\n", get_full_path(&nameidata->path, NULL, buf));
        if (current_uid() == 0) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            //return -EACCES;
        }
    }

    return 0;
}

static int rootfctrl_inode_setattr(struct dentry *dentry, struct iattr *iattr)
{
    int ret = 0;

    if (is_felica_file_name(dentry->d_name.name)) {
        RTFCTL_MSG("*****>>>>> rootfctrl_inode_setattr <<<<<*****\n");
        RTFCTL_MSG("dentry: %s\n", dentry->d_name.name);
        RTFCTL_MSG("iattr->ia_valid = %d\n", iattr->ia_valid);
        RTFCTL_MSG("iattr->ia_mode = %d\n", iattr->ia_mode);
        RTFCTL_MSG("iattr->ia_uid = %d\n", iattr->ia_uid);
        RTFCTL_MSG("iattr->ia_gid = %d\n", iattr->ia_gid);

        if (current_uid()!=0) {
            //ret = -EACCES;
            RTFCTL_MSG(KERN_INFO "Rejected...non-root user cannot set attribute of %s\n", dentry->d_name.name);
            goto OUT;
        }
        else {
            if ((iattr->ia_valid & ATTR_MODE) && (iattr->ia_mode == 33200)) { /* set to 0660 (33152 for 600) */
                RTFCTL_MSG(KERN_INFO "set %s to %o\n", dentry->d_name.name, iattr->ia_mode);
            }
            else if ((iattr->ia_valid & (ATTR_UID|ATTR_GID)) == (ATTR_UID|ATTR_GID)) { /* set owner + group */
                if (!is_felica_uid(iattr->ia_uid) || !is_felica_uid(iattr->ia_gid)) {
                    RTFCTL_MSG(KERN_INFO "Reject to set owner and group of %s to %d\n", dentry->d_name.name, iattr->ia_uid);
                    //ret = -EACCES;
                }
                else {
                    RTFCTL_MSG(KERN_INFO "set owner and group of %s to %d\n", dentry->d_name.name, iattr->ia_uid);
                }
            }
            else if (iattr->ia_valid & ATTR_UID) { /*set owner */
                if (!is_felica_uid(iattr->ia_uid)) {
                    RTFCTL_MSG(KERN_INFO "Reject to set owner of %s to %d\n", dentry->d_name.name, iattr->ia_uid);
                    //ret = -EACCES;
                }
                else {
                    RTFCTL_MSG(KERN_INFO "set owner of %s to %d\n", dentry->d_name.name, iattr->ia_uid);
                }
            } else { /* operation which is not allowed */
                //ret = -EACCES;
                RTFCTL_MSG(KERN_INFO "The operation is not allowed\n");
            }

        }
    }
    OUT:
    return ret;
}
#endif

/**
 *  Security hooks for task operations
 */
static int rootfctrl_task_create(unsigned long clone_flags)
{
    pid_t ppid = task_tgid_vnr(current->real_parent);
    char tcomm[sizeof(current->comm)];

    get_task_comm(tcomm, current);

    if (zygote_pid == -1) {
        if (!strcmp("zygote", tcomm) && current_uid() == 0 && ppid == 1) {
            RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
            RTFCTL_MSG("########## current name: %s\n", current->comm);
            RTFCTL_MSG("########## current pid: %d\n", task_tgid_vnr(current));
            RTFCTL_MSG("########## current uid: %d\n", current_uid());
#endif
            zygote_pid = task_tgid_vnr(current);
            RTFCTL_MSG("Current zygote_pid: %d\n", zygote_pid);
        }
    }

    if (flc_daemon_pid == -1) {
        if (!strcmp("felica_daemon", tcomm) && current_uid() == 0 && ppid == 1) {
            RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
            RTFCTL_MSG("########## current name: %s\n", current->comm);
            RTFCTL_MSG("########## current pid: %d\n", task_tgid_vnr(current));
            RTFCTL_MSG("########## current uid: %d\n", current_uid());
#endif
            flc_daemon_pid = task_tgid_vnr(current);
            RTFCTL_MSG("Current flc_daemon_pid: %d\n", flc_daemon_pid);
        }
    }

    if (flc_agent_pid == -1) {
        if (!strcmp("felica_agent", tcomm) && current_uid() == 0 && ppid == 1) {
            RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
            RTFCTL_MSG("########## current name: %s\n", current->comm);
            RTFCTL_MSG("########## current pid: %d\n", task_tgid_vnr(current));
            RTFCTL_MSG("########## current uid: %d\n", current_uid());
#endif
            flc_agent_pid = task_tgid_vnr(current);
            RTFCTL_MSG("Current flc_agent_pid: %d\n", flc_agent_pid);
        }
    }

    if (installd_pid == -1) {
        if (!strcmp("installd", tcomm) && current_uid() == 0 && ppid == 1) {
            RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
            RTFCTL_MSG("########## current name: %s\n", current->comm);
            RTFCTL_MSG("########## current pid: %d\n", task_tgid_vnr(current));
            RTFCTL_MSG("########## current uid: %d\n", current_uid());
#endif
            installd_pid = task_tgid_vnr(current);
            RTFCTL_MSG("Current installd_pid: %d\n", installd_pid);
        }
    }

    if (!strcmp("adbd", tcomm) && ppid == 1) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
        RTFCTL_MSG("########## current name: %s\n", current->comm);
        RTFCTL_MSG("########## current pid: %d\n", task_tgid_vnr(current));
        RTFCTL_MSG("########## current uid: %d\n", current_uid());
#endif
        adbd_pid = task_tgid_vnr(current);
        RTFCTL_MSG("Current adbd_pid: %d\n", adbd_pid);
    }

    return 0;
}

int rootfctrl_task_kill(struct task_struct *p, struct siginfo *info,
			int sig, u32 secid)
{
    char tcomm[sizeof(p->comm)];
    char tcomm2[sizeof(current->comm)];

    get_task_comm(tcomm2, current);
    get_task_comm(tcomm, p);

    /* "zygote" is not one-shot service, if it's killed, we need to recognize again */
    if (!strcmp("zygote", tcomm) && (task_tgid_vnr(p) == zygote_pid)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
        RTFCTL_MSG("%s (pid=%d) kill %s (pid=%d)\n", tcomm2, task_tgid_vnr(current), tcomm, task_tgid_vnr(p));
        RTFCTL_MSG("info: %x, isFromKernel: %d\n", (unsigned int)info, SI_FROMKERNEL(info));
        RTFCTL_MSG("sig: %d, secid: %d\n", sig, secid);

        zygote_pid = -1;
    }

    /* "installd" is not one-shot service, if it's killed, we need to recognize again */
    if (!strcmp("installd", tcomm) && (task_tgid_vnr(p) == installd_pid)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
        RTFCTL_MSG("%s (pid=%d) kill %s (pid=%d)\n", tcomm2, task_tgid_vnr(current), tcomm, task_tgid_vnr(p));
        RTFCTL_MSG("info: %x, isFromKernel: %d\n", (unsigned int)info, SI_FROMKERNEL(info));
        RTFCTL_MSG("sig: %d, secid: %d\n", sig, secid);

        installd_pid = -1;
    }

    return 0;
}

static int rootfctrl_task_fix_setuid (struct cred *new, const struct cred *old, int flags)
{
    pid_t ppid = task_tgid_vnr(current->real_parent);
    pid_t pid;
    char tcomm[sizeof(current->comm)];

    pid = task_tgid_vnr(current);
    get_task_comm(tcomm, current);

#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
    if (is_felica_uid(new->uid)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
        RTFCTL_MSG("old->uid = %d, old->gid = %d\n", old->uid, old->gid);
        RTFCTL_MSG("old->suid = %d, old->sgid = %d\n", old->suid, old->sgid);
        RTFCTL_MSG("old->euid = %d, old->egid = %d\n", old->euid, old->egid);

        RTFCTL_MSG("new->uid = %d, new->gid = %d\n", new->uid, new->gid);
        RTFCTL_MSG("new->suid = %d, new->sgid = %d\n", new->suid, new->sgid);
        RTFCTL_MSG("new->euid = %d, new->egid = %d\n", new->euid, new->egid);
        RTFCTL_MSG("Zygote pid: %d\n", zygote_pid);
    }
#endif

    if (ppid != zygote_pid && !is_felica_uid(old->uid) && is_felica_uid(new->uid)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
        RTFCTL_MSG("Old UID: %d -> New UID: %d\n", old->uid, new->uid);
        RTFCTL_MSG("pid: %d (%s) ppid: %d\n", pid, tcomm, ppid);
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
        if(pid != installd_pid && ppid != installd_pid) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            return -EACCES;
        }
#endif
    }
    if (ppid != zygote_pid && !is_felica_uid(old->gid) && is_felica_uid(new->gid)) {
        RTFCTL_MSG("########## %s ##########\n", __FUNCTION__);
        RTFCTL_MSG("Old GID: %d -> New GID: %d\n", old->gid, new->gid);
        RTFCTL_MSG("pid: %d (%s) ppid: %d\n", pid, tcomm, ppid);
#if (RTFCTL_RUN_MODE != RTFCTL_TRACKING_MODE)
        if(pid != installd_pid && ppid != installd_pid) {
            RTFCTL_MSG(KERN_INFO "Rejected...\n");
            return -EACCES;
        }
#endif
    }

    return 0;
}

static struct security_operations rootfctrl_ops = {
    .name   =   "rootfctrl",
    .dentry_open        =       rootfctrl_dentry_open,
#if (RTFCTL_RUN_MODE == RTFCTL_TRACKING_MODE)
    .file_set_fowner    =       rootfctrl_file_set_fowner,
    .file_fcntl         =       rootfctrl_file_fcntl,
    .file_ioctl         =       rootfctrl_file_ioctrl,
    .file_permission    =       rootfctrl_file_permission,
#if 0
    .inode_create       =       rootfctrl_inode_create,
    .inode_link         =       rootfctrl_inode_link,
    .inode_unlink       =       rootfctrl_inode_unlink,
    .inode_symlink      =       rootfctrl_inode_symlink,
    .inode_mkdir        =       rootfctrl_inode_mkdir,
    .inode_rmdir        =       rootfctrl_inode_rmdir,
    .inode_mknod        =       rootfctrl_inode_mknod,
    .inode_rename       =       rootfctrl_inode_rename,
    .inode_readlink     =       rootfctrl_inode_readlink,
    .inode_follow_link  =       rootfctrl_inode_follow_link,
    .inode_setattr      =       rootfctrl_inode_setattr,
#endif
    .path_mkdir         =       rootfctrl_path_mkdir,
    .path_rmdir         =       rootfctrl_path_rmdir,
    .path_truncate      =       rootfctrl_path_truncate,
    .path_symlink       =       rootfctrl_path_symlink,
#endif
    .path_unlink        =       rootfctrl_path_unlink,
    .path_mknod         =       rootfctrl_path_mknod,
    .path_link          =       rootfctrl_path_link,
    .path_rename        =       rootfctrl_path_rename,
    .path_chmod         =       rootfctrl_path_chmod,
    .path_chown         =       rootfctrl_path_chown,

    .task_create        =       rootfctrl_task_create,
    .task_kill          =       rootfctrl_task_kill,
    .task_fix_setuid    =       rootfctrl_task_fix_setuid,
};

static __init int rootfctrl_init(void)
{
    if (!security_module_enable(&rootfctrl_ops)) {
        printk(KERN_WARNING"[ERR] ROOTFCtrl is not enabled!");
        return 0;
    }

    RTFCTL_MSG(KERN_INFO "rootfctrl:  Initializing.....\n");

    buf = kmalloc(PATH_MAX+1, GFP_KERNEL);
    if (!buf) {
        printk(KERN_WARNING"[ERR] out of memory...");
        return 0;
    }

    if (register_security(&rootfctrl_ops))
        panic("rootfctrl: Unable to register with kernel.\n");

    return 0;
}

security_initcall(rootfctrl_init);
