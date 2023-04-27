/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/FSDetect.h"
#include <folly/portability/GTest.h>
#include <string>

TEST(FSType, fstype) {
  auto mount_data =
      "sysfs /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0\n"
      "devtmpfs /dev devtmpfs rw,seclabel,nosuid,size=58706680k,nr_inodes=14676670,mode=755 0 0\n"
      "securityfs /sys/kernel/security securityfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "tmpfs /dev/shm tmpfs rw,seclabel,nosuid,nodev 0 0\n"
      "devpts /dev/pts devpts rw,seclabel,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0\n"
      "tmpfs /run tmpfs rw,seclabel,nosuid,nodev,mode=755 0 0\n"
      "cgroup2 /sys/fs/cgroup cgroup2 rw,nosuid,nodev,noexec,relatime,nsdelegate 0 0\n"
      "pstore /sys/fs/pstore pstore rw,nosuid,nodev,noexec,relatime 0 0\n"
      "bpf /sys/fs/bpf bpf rw,relatime,mode=700 0 0\n"
      "configfs /sys/kernel/config configfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "/dev/vda3 / btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/ 0 0\n"
      "/dev/vda1 /boot ext3 rw,seclabel,relatime 0 0\n"
      "/etc/auto.home /home autofs rw,relatime,fd=40,pgrp=2446760,timeout=600,minproto=5,maxproto=5,indirect 0 0\n"
      "tmpfs /run/user/5537 tmpfs rw,seclabel,nosuid,nodev,relatime,size=11743612k,mode=700,uid=5537,gid=100 0 0\n"
      "/dev/vda3 /home/wez btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/dotsync-home 0 0\n"
      "edenfs /data/users/wez/fbsource fuse rw,nosuid,relatime,user_id=5537,group_id=100,default_permissions,allow_other 0 0\n"
      "squashfuse_ll /mnt/xarfuse/uid-5537/326c1234-ns-4026531840 fuse.squashfuse_ll rw,nosuid,nodev,relatime,user_id=5537,group_id=100 0 0\n"
      "/dev/vda3 /data/users/wez/fbsource/buck-out btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/scratch/dataZusersZwezZovrsource/edenfsZredirectionsZbuck-out 0 0\n";

  EXPECT_EQ(
      std::nullopt, find_fstype_in_linux_proc_mounts("bleet", mount_data));
  EXPECT_EQ(
      w_string("btrfs"), find_fstype_in_linux_proc_mounts("/", mount_data));
  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts("/foo/bar", mount_data));
  EXPECT_EQ(
      w_string("edenfs"),
      find_fstype_in_linux_proc_mounts("/data/users/wez/fbsource", mount_data));
  EXPECT_EQ(
      w_string("edenfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsource/something", mount_data));
  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsourcenoslash", mount_data));
  EXPECT_EQ(
      w_string("fuse.squashfuse_ll"),
      find_fstype_in_linux_proc_mounts(
          "/mnt/xarfuse/uid-5537/326c1234-ns-4026531840", mount_data));
  EXPECT_EQ(
      w_string("fuse.squashfuse_ll"),
      find_fstype_in_linux_proc_mounts(
          "/mnt/xarfuse/uid-5537/326c1234-ns-4026531840/", mount_data));
}

TEST(FSType, fstype_two_entries) {
  auto mount_data =
      "sysfs /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0\n"
      "devtmpfs /dev devtmpfs rw,seclabel,nosuid,size=58706680k,nr_inodes=14676670,mode=755 0 0\n"
      "securityfs /sys/kernel/security securityfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "tmpfs /dev/shm tmpfs rw,seclabel,nosuid,nodev 0 0\n"
      "devpts /dev/pts devpts rw,seclabel,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0\n"
      "tmpfs /run tmpfs rw,seclabel,nosuid,nodev,mode=755 0 0\n"
      "cgroup2 /sys/fs/cgroup cgroup2 rw,nosuid,nodev,noexec,relatime,nsdelegate 0 0\n"
      "pstore /sys/fs/pstore pstore rw,nosuid,nodev,noexec,relatime 0 0\n"
      "bpf /sys/fs/bpf bpf rw,relatime,mode=700 0 0\n"
      "configfs /sys/kernel/config configfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "/dev/vda3 / btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/ 0 0\n"
      "/dev/vda1 /boot ext3 rw,seclabel,relatime 0 0\n"
      "/etc/auto.home /home autofs rw,relatime,fd=40,pgrp=2446760,timeout=600,minproto=5,maxproto=5,indirect 0 0\n"
      "tmpfs /run/user/5537 tmpfs rw,seclabel,nosuid,nodev,relatime,size=11743612k,mode=700,uid=5537,gid=100 0 0\n"
      "/dev/vda3 /home/wez btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/dotsync-home 0 0\n"
      "/dev/vda3 /data/users/wez/fbsource btrfs rw,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/scratch/fbsource 0 0\n"
      "edenfs /data/users/wez/fbsource fuse rw,nosuid,relatime,user_id=5537,group_id=100,default_permissions,allow_other 0 0\n"
      "squashfuse_ll /mnt/xarfuse/uid-5537/326c1234-ns-4026531840 fuse.squashfuse_ll rw,nosuid,nodev,relatime,user_id=5537,group_id=100 0 0\n"
      "/dev/vda3 /data/users/wez/fbsource/buck-out btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/scratch/dataZusersZwezZovrsource/edenfsZredirectionsZbuck-out 0 0\n";

  // in some systems there are bind mounts under the eden fbsource mount for
  // reasons. We want the mount that was added last, which will be the current
  // mount that the user is interacting with to be the mount we detect.
  EXPECT_EQ(
      w_string("edenfs"),
      find_fstype_in_linux_proc_mounts("/data/users/wez/fbsource", mount_data));
  EXPECT_EQ(
      w_string("edenfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsource/something", mount_data));
  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsourcenoslash", mount_data));

  auto mount_data_btrfs =
      "sysfs /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0\n"
      "devtmpfs /dev devtmpfs rw,seclabel,nosuid,size=58706680k,nr_inodes=14676670,mode=755 0 0\n"
      "securityfs /sys/kernel/security securityfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "tmpfs /dev/shm tmpfs rw,seclabel,nosuid,nodev 0 0\n"
      "devpts /dev/pts devpts rw,seclabel,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000 0 0\n"
      "tmpfs /run tmpfs rw,seclabel,nosuid,nodev,mode=755 0 0\n"
      "cgroup2 /sys/fs/cgroup cgroup2 rw,nosuid,nodev,noexec,relatime,nsdelegate 0 0\n"
      "pstore /sys/fs/pstore pstore rw,nosuid,nodev,noexec,relatime 0 0\n"
      "bpf /sys/fs/bpf bpf rw,relatime,mode=700 0 0\n"
      "configfs /sys/kernel/config configfs rw,nosuid,nodev,noexec,relatime 0 0\n"
      "/dev/vda3 / btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/ 0 0\n"
      "/dev/vda1 /boot ext3 rw,seclabel,relatime 0 0\n"
      "/etc/auto.home /home autofs rw,relatime,fd=40,pgrp=2446760,timeout=600,minproto=5,maxproto=5,indirect 0 0\n"
      "tmpfs /run/user/5537 tmpfs rw,seclabel,nosuid,nodev,relatime,size=11743612k,mode=700,uid=5537,gid=100 0 0\n"
      "/dev/vda3 /home/wez btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/dotsync-home 0 0\n"
      "edenfs /data/users/wez/fbsource fuse rw,nosuid,relatime,user_id=5537,group_id=100,default_permissions,allow_other 0 0\n"
      "/dev/vda3 /data/users/wez/fbsource btrfs rw,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/scratch/fbsource 0 0\n"
      "squashfuse_ll /mnt/xarfuse/uid-5537/326c1234-ns-4026531840 fuse.squashfuse_ll rw,nosuid,nodev,relatime,user_id=5537,group_id=100 0 0\n"
      "/dev/vda3 /data/users/wez/fbsource/buck-out btrfs rw,seclabel,relatime,compress-force=zstd:3,discard,space_cache,subvolid=5,subvol=/data/users/wez/scratch/dataZusersZwezZovrsource/edenfsZredirectionsZbuck-out 0 0\n";

  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsource", mount_data_btrfs));
  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsource/something", mount_data_btrfs));
  EXPECT_EQ(
      w_string("btrfs"),
      find_fstype_in_linux_proc_mounts(
          "/data/users/wez/fbsourcenoslash", mount_data_btrfs));
}
