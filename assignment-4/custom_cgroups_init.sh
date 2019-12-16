#!/bin/bash

mkdir /sys/fs/cgroup/blkio/myapp

echo "8:0 1024" > /sys/fs/cgroup/blkio/myapp/blkio.throttle.read_bps_device
echo "8:0 1024" > /sys/fs/cgroup/blkio/myapp/blkio.throttle.write_bps_device

mkdir /sys/fs/cgroup/memory/myapp
echo 2048 | tee /sys/fs/cgroup/memory/myapp/memory.limit_in_bytes

mkdir /sys/fs/cgroup/cpu/myapp
echo 10000 > /sys/fs/cgroup/cpu/myapp/cpu.cfs_quota_us
echo 100000 > /sys/fs/cgroup/cpu/myapp/cpu.cfs_period_us
