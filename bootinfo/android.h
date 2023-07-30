/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *   Copyright 2007, The Android Open Source Project
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#pragma once

#include <stdint.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024

#define BOOT_MAGIC_FMT "%." STR(BOOT_MAGIC_SIZE) "s"
#define BOOT_NAME_FMT "%." STR(BOOT_NAME_SIZE) "s"
#define BOOT_ARGS_FMT "%." STR(BOOT_ARGS_SIZE) "s"
#define BOOT_EXTRA_ARGS_FMT "%." STR(BOOT_EXTRA_ARGS_SIZE) "s"


#ifdef ANSI_COLORS
#  define STYLE_RESET         "\x1b[0m"
#  define FORE_RED            "\x1b[31m"
#  define FORE_YELLOW         "\x1b[33m"
#  define FORE_BLUE           "\x1b[34m"
#  define FORE_CYAN           "\x1b[36m"
#  define FORE_CYAN_UNDERLINE "\x1b[36;4m"
#else
#  define STYLE_RESET         ""
#  define FORE_RED            ""
#  define FORE_YELLOW         ""
#  define FORE_BLUE           ""
#  define FORE_CYAN           ""
#  define FORE_CYAN_UNDERLINE ""
#endif

#define ITEM_PREFIX "  ▸ " FORE_CYAN "%s" STYLE_RESET ": "

#define STDWARN(args...) fprintf(stderr, FORE_YELLOW "Warning: " STYLE_RESET args)
#define STDERR(args...) fprintf(stderr, FORE_RED "Error: " STYLE_RESET args)

// 1048576 = 1024*1024
#define PRINT_SIZE(title, bytes) \
  printf(ITEM_PREFIX "%d bytes", title, bytes); \
  printf(" (%.1f KB / %.1f MB)\n", (float)bytes / 1024, (float)bytes / 1048576)

#define PRINT_HEX(title, addr) printf(ITEM_PREFIX "0x%x\n", title STYLE_RESET, addr)
#define PRINT_LONG_HEX(title, offset) printf(ITEM_PREFIX "0x%lx\n", title STYLE_RESET, offset)

typedef struct {
  uint8_t magic[BOOT_MAGIC_SIZE];
  uint32_t kernel_size;  /* size in bytes */
  uint32_t kernel_addr;  /* physical load addr */
  uint32_t ramdisk_size; /* size in bytes */
  uint32_t ramdisk_addr; /* physical load addr */
  uint32_t second_size;  /* size in bytes */
  uint32_t second_addr;  /* physical load addr */
  uint32_t tags_addr;    /* physical addr for kernel tags */
  uint32_t page_size;    /* flash page size we assume */
  uint32_t unused;       /* reserved for future expansion: MUST be 0 */
  /* operating system version and security patch level; for
    * version "A.B.C" and patch level "Y-M-D":
    * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
    * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
    * os_version = ver << 11 | lvl */
  uint32_t os_version;
  uint8_t name[BOOT_NAME_SIZE]; /* asciiz product name */
  uint8_t cmdline[BOOT_ARGS_SIZE];
  uint32_t id[8]; /* timestamp / checksum / sha1 / etc */
  /* Supplemental command line data; kept here to maintain
    * binary compatibility with older versions of mkbootimg */
  uint8_t extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
} boot_img_hdr_v0;

typedef struct {
  boot_img_hdr_v0 v0_base;
  uint32_t recovery_dtbo_size;   /* size in bytes for recovery DTBO/ACPIO image */
  uint64_t recovery_dtbo_offset; /* offset to recovery dtbo/acpio in boot image */
  uint32_t header_size;
} boot_img_hdr_v1;

typedef struct {
  boot_img_hdr_v1 v1_base;
  uint32_t dtb_size; /* size in bytes for DTB image */
  uint64_t dtb_addr; /* physical load address for DTB image */
} boot_img_hdr_v2;

void get_os_version(unsigned short version[3], unsigned os_version);
void get_patch_version(unsigned version[2], unsigned os_version);
void print_v0(boot_img_hdr_v0 header);
void print_v1(boot_img_hdr_v1 header);
void print_v2(boot_img_hdr_v2 header);
