#!/bin/bash

RUN_CONTAINER_CMD="chroot jailed /bin/bash"

cgexec -g memory,cpu,blkio:myapp $RUN_CONTAINER_CMD
