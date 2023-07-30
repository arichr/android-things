#include <stdio.h>

#include "android.h"

void get_os_version(unsigned short version[3], unsigned os_version) {
    version[0] = (os_version >> (32 - 7 * 1)) & ((1 << 7) - 1);  // Major
    version[1] = (os_version >> (32 - 7 * 2)) & ((1 << 7) - 1);  // Minor
    version[2] = (os_version >> (32 - 7 * 3)) & ((1 << 7) - 1);  // Patch
}

void get_patch_version(unsigned version[2], unsigned os_version) {
    version[0] = (os_version >> 4 & ((1 << 7) - 1)) + 2000;
    version[1] = os_version & ((1 << 4) - 1);
}

void print_v0(boot_img_hdr_v0 header) {
  unsigned short os_version[3];
  unsigned patch_version[2];
  get_os_version(os_version, header.os_version);
  get_patch_version(patch_version, header.os_version);

  printf(
    ITEM_PREFIX BOOT_NAME_FMT "\n",
    FORE_CYAN_UNDERLINE "Product name" STYLE_RESET,
    header.name
  );
  printf(ITEM_PREFIX BOOT_MAGIC_FMT "\n", "Magic", header.magic);
  printf(ITEM_PREFIX "0x%.8lx\n", "ID", (unsigned long)header.id);

  if (header.unused != 0)
    STDWARN("Unused field is not zero.\n");
  PRINT_HEX("Unused field", header.unused);

  printf(
    ITEM_PREFIX "%d.%d.%d\n",
    FORE_CYAN_UNDERLINE "OS version" STYLE_RESET,
    os_version[0],  // Major
    os_version[1],  // Minor
    os_version[2]   // Patch
  );
  printf(
    ITEM_PREFIX "%d.%.2d\n",
    "Security patch version",
    patch_version[0], // Year
    patch_version[1]  // Month
  );

  PRINT_HEX(FORE_CYAN_UNDERLINE "Kernel", header.kernel_addr);

  if (header.tags_addr < 0x10000000)
    STDWARN("Invalid kernel tags address.\n");
  PRINT_HEX("Kernel tags", header.tags_addr);

  PRINT_SIZE("Kernel size", header.kernel_size);

  printf(
    ITEM_PREFIX BOOT_ARGS_FMT "\n",
    FORE_CYAN_UNDERLINE "Cmdline" STYLE_RESET,
    header.cmdline
  );
  printf(ITEM_PREFIX BOOT_EXTRA_ARGS_FMT "\n", "Extra cmdline", header.extra_cmdline);

  PRINT_HEX(FORE_CYAN_UNDERLINE "Ramdisk", header.ramdisk_addr);
  PRINT_SIZE("Ramdisk size", header.ramdisk_size);

  PRINT_HEX(FORE_CYAN_UNDERLINE "Second", header.second_addr);

  if (header.second_size && header.second_addr < 0x10000000)
    STDWARN("Invalid second address.\n");
  PRINT_SIZE("Second size", header.second_size);

  PRINT_SIZE(FORE_CYAN_UNDERLINE "Flash page size", header.page_size);
}

void print_v1(boot_img_hdr_v1 header) {
  PRINT_SIZE(FORE_CYAN_UNDERLINE "Recovery DTBO size", header.recovery_dtbo_size);

  if (header.recovery_dtbo_size && header.recovery_dtbo_offset < 0x10000000)
    STDWARN("Invalid recovery DTBO offset.\n");
  PRINT_LONG_HEX("Recovery DTBO offset", header.recovery_dtbo_offset);

  PRINT_SIZE(FORE_CYAN_UNDERLINE "Header size", header.header_size);
}

void print_v2(boot_img_hdr_v2 header) {
  PRINT_SIZE(FORE_CYAN_UNDERLINE "DTBO image", header.dtb_size);

  if (header.dtb_size && header.dtb_addr < 0x10000000)
    STDWARN("Invalid DTBO image address.\n");
  PRINT_LONG_HEX("DTBO image address", header.dtb_addr);
}
