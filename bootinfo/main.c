#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "android.h"

#define VERSION "v1.0.0"

#define READ_HEADER \
  if (!fread(&header, sizeof(header), 1, fp)) { \
    STDERR("Unable to read the file.\n"); \
    code = EXIT_FAILURE; \
    break; \
  }

int main(int argc, const char* argv[]) {
  unsigned short code = EXIT_SUCCESS;

  if (argc < 3) {
    fprintf(stderr, FORE_BLUE "Android Things/" FORE_RED "BootInfo " STYLE_RESET VERSION "\n");
    fprintf(stderr, "An utility for parsing the header section of Android boot images.\n\n");
    fprintf(stderr, FORE_CYAN "Usage:\n" STYLE_RESET);
    fprintf(stderr, "   %s <boot.img> <boot header version>\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE *fp;
  if ((fp = fopen(argv[1], "r")) == NULL) {
    STDERR("Unable to open %s: %s.\n", argv[1], strerror(errno));
    return errno;
  }

  printf(FORE_BLUE "%s" STYLE_RESET ":\n", argv[1]);

  switch (atoi(argv[2])) {
    case 0: {
      boot_img_hdr_v0 header;
      READ_HEADER;
      print_v0(header);
      break;
    }
    case 1: {
      boot_img_hdr_v1 header;
      READ_HEADER;
      print_v0(header.v0_base);
      print_v1(header);
      break;
    }
    case 2:{
      boot_img_hdr_v2 header;
      READ_HEADER;
      print_v0(header.v1_base.v0_base);
      print_v1(header.v1_base);
      print_v2(header);
      break;
    }
    default:
      STDERR("Invalid boot header version: 0 <= header_version <= 2.\n");
      code = EXIT_FAILURE;
  }

  fclose(fp);
  return code;
}
