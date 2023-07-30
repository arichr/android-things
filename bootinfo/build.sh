#!/bin/sh

OUTDIR="build"
ARGS="-Wall -Wextra -Werror -Os -s"
set -xe

mkdir -p $OUTDIR
gcc $ARGS -DANSI_COLORS -o $OUTDIR/bootinfo main.c android.c
gcc $ARGS -o $OUTDIR/bootinfo-textonly main.c android.c
