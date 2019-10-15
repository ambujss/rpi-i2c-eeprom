/*
  I forked from here.
  https://www.richud.com/wiki/Rasberry_Pi_I2C_EEPROM_Program

  24C02  cc -o main main.c 24cXX.c -DC02
  24C04  cc -o main main.c 24cXX.c -DC04
  24C08  cc -o main main.c 24cXX.c -DC08
  24C16  cc -o main main.c 24cXX.c -DC16
  24C32  cc -o main main.c 24cXX.c -DC32
  24C64  cc -o main main.c 24cXX.c -DC64
  24C128 cc -o main main.c 24cXX.c -DC128
  24C256 cc -o main main.c 24cXX.c -DC256
  24C512 cc -o main main.c 24cXX.c -DC512
*/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "24cXX.h"

void print_help(void) {
  printf("Usage: ./i2ceeprom <i2c_bus> <i2c_addr> [i2c_addr_end][FLAGS] \n"
         "    i2c_bus        I2C bus number\n"
         "    i2c_addr       I2C Address of EEPROM chip. If i2c_addr_end is specified, this is the first address\n"
         "                   of the range of addresses from i2c_addr to i2c_addr_end.\n"
         "    i2c_addr_end   End of Address range of EEPROMs. Only specify this  when reading.\n"
         "    -w <contents>  Contents to be written to EEPROM\n"
         "    -r [bytes]     Read contents from EEPROM. Outputs to stdout unless -o is specified.\n"
         "                   Will read 'bytes' worth of data if specified, else till the first null byte (0) is encountered.\n"
         "                   If i2c_addr_end is specified, all EEPROMs from i2c_addr to i2c_addr_end will be read. Contents of\n"
         "                   each EEPROM will be output to a new line.\n"
         "    -o <outfile>   Output filename. If used with -r, Contents of all EEPROMS will be written to <outfile>, separated by a newline.\n"
         "                   If used with -a, contents of each EEPROM will be written to its own file\n"
         "                   which will be named as <outfile>_<addr>\n"
         "    -a             Read the entire EEPROM. -o needs to be specified with this flag.\n"
         "                   Dumps output in binary format.\n"
         "    -v             Verbose.\n"
         "    -s <value>     Set all bytes in EEPROM to this value\n"
         "One of -w, -r, -o, -a or -s must be specified. If both read and write operations are specified, \n"
         "the read will take place after the write.\n"
         "-s cannot be specified with -w.\n");
}

int main(int argc, char *argv[])
{
  int i2c_addr = -1;
  int i2c_bus = -1;

  if (argc < 3) {
    fprintf(stderr, "Incorrect arguments\n");
    print_help();
    return 1;
  }

  // i2c bus is the first argument
  i2c_bus = strtol(argv[1],NULL,16);
  // i2c addr is the second argument
  i2c_addr = strtol(argv[2],NULL,16);



  // start from arg 3
  int i = 3;

  int i2c_addr_end = -1;
  // check if addr range provided
  if (argc > 3 && argv[3][0] != '-') {
    // if more than 2 positional arguments provided, take 3rd argument as
    // end of address range
    i2c_addr_end = strtol(argv[3], NULL, 16);
    // bump up start index of flags
    i++;
  }

  if (i2c_addr_end >=0 && i2c_addr >= i2c_addr_end) {
    // End address must be greater than start address
    fprintf(stderr, "End of address range must be greater than start of address range.\n");
    print_help();
    return 1;
  }

  int do_write = 0;
  int do_read = 0;
  int read_bytes = -1;
  int do_read_all = 0;
  int verbose = 0;
  int set = 0;
  char* setstr = NULL;
  char* contents = NULL;
  char* outfile = NULL;

  while (i < argc) {
    char* v = argv[i];
    if (v[0] == '-')
    { int j = 1;
      int arglen = strlen(v);
      while (v[j]) {
        switch(v[j]) {
          case 'r':
            do_read = 1;
            // this flag has an optional argument, check for it
            if (!v[j+1] && i+1 < argc && argv[i+1][0] != '-') {
              // if the flags list ended here and we have a non-flag argument after it,
              // set read_bytes
              read_bytes = strtol(argv[i+1], NULL, 16);
              if (read_bytes == 0) {
                fprintf(stderr, "Please provide a number greater than 0 for -r\n");
                print_help();
                return 1;
              }
              i++;
            }
            break;
          case 'a':
            do_read_all = 1;
            break;
          case 'v':
            verbose = 1;
            break;
          case 'w':
            // this flag requires an argument, check for it
            if (v[j+1] || i+1 >= argc) {
              // if the flags list didn't end here or we reached end or arg list, error
              fprintf(stderr, "-w flag requires an argument\n", v[j]);  
              print_help();
              return 1;
            }
            contents = argv[i+1];
            do_write = 1;
            i++;
            break;
          case 's':
            // this flag requires an argument, check for it
            if (v[j+1] || i+1 >= argc) {
              // if the flags list didn't end here or we reached end or arg list, error
              fprintf(stderr, "-s flag requires an argument\n", v[j]);  
              print_help();
              return 1;
            }
            set = 1;
            setstr = argv[i+1];
            i++;
            break;
          case 'o':
            // this flag requires an argument, check for it
            if (v[j+1] || i+1 >= argc) {
              // if the flags list didn't end here or we reached end or arg list, error
              fprintf(stderr, "-o flag requires an argument\n", v[j]);  
              print_help();
              return 1;
            }
            outfile = argv[i+1];
            i++;
            break;
          default:
            fprintf(stderr, "Invalid flag option %c\n", v[j]);
            print_help();
            return 1;
        }
        j++;
      }
    } else {
      fprintf(stderr, "Unrecognized parameter %s\n", v);
      print_help();
      return 1;
    }
    i++;
  }

  if (outfile) {
    do_read = 1;
  }

  if (!do_write && !do_read && !do_read_all) {
    fprintf(stderr, "One of -w, -r or -a must be specified\n");
    print_help();
    return 1;
  }

  if (do_read_all && !outfile) {
    fprintf(stderr, "-o must be specified with -a\n");
    print_help();
    return 1;
  }

  if (do_write && set) {
    fprintf(stderr, "-s cannot be specifed with -w\n");
    print_help();
    return 1;
  }

  if ((do_write || set) && i2c_addr_end >= 0) {
    fprintf(stderr, "Writing to EEPROM only works with a single EEPROM. Please do not provide a range.");
    print_help();
    return 1;
  }

  char device[20];
  struct stat st;
  int ret;

  // set device file name
  snprintf(device, 19, "/dev/i2c-%d", i2c_bus);
  ret = stat(device, &st); 
  if (ret != 0) {
      fprintf(stderr, "i2c device %d at %s not found\n",i2c_bus, device);
      return 2;
  }
  if (verbose) {
    printf("i2c device %s\n", device);
    if (i2c_addr_end >= 0) {
      printf("i2c start address = 0x%x\n",i2c_addr);
      printf("i2c end address   = 0x%x\n",i2c_addr_end);
    } else {
      printf("i2c address=0x%x\n",i2c_addr);
    }
  }

  // set EEPROM memory size
  int eeprom_bits = 0;
#ifdef C02
  eeprom_bits = 2;
#endif
#ifdef C04
  eeprom_bits = 4;
#endif
#ifdef C08
  eeprom_bits = 8;
#endif
#ifdef C16
  eeprom_bits = 16;
#endif
#ifdef C32
  eeprom_bits = 32;
#endif
#ifdef C64
  eeprom_bits = 64;
#endif
#ifdef C128
  eeprom_bits = 128;
#endif
#ifdef C256
  eeprom_bits = 256;
#endif
#ifdef C512
  eeprom_bits = 512;
#endif
  if (verbose)
    printf("eeprom_bits=%d\n",eeprom_bits);
  if (eeprom_bits == 0) {
    fprintf(stderr, "EEPROM model not found\n");
    return 2;
  }

  // open device
  int write_cycle_time = 3;
  struct eeprom e;
  ret = eeprom_open(device, i2c_addr, eeprom_bits, write_cycle_time, &e);
  if (ret == -1)
    return 3;

  // get EEPROM size(byte)
  uint16_t eeprom_bytes = getEEPROMbytes(&e);
  if (verbose)
    printf("EEPROM chip=24C%.02d bytes=%dByte\n", eeprom_bits, eeprom_bytes);

  uint16_t mem_addr;
  uint8_t data;
  uint8_t rdata[256];

  if (do_write) {
    if (-1 == eeprom_detect(&e, i2c_addr)) {
      fprintf(stderr, "No i2c device found at address 0x%02x on bus %d", i2c_addr, i2c_bus);
      return 6;
    }
    int len = strlen(contents);
    len = len >= eeprom_bytes ? eeprom_bytes - 1: len;
    for(i=0; i<len; i++) {
      mem_addr = i;
      data = (uint8_t)contents[i];
      ret = eeprom_write_byte(&e, mem_addr, data);
      if (ret != 0) {
        fprintf(stderr, "eeprom_write_byte ret=%d at %x %x\n", ret, mem_addr, data);
      }
    }
    ret = eeprom_write_byte(&e, len, 0);
    if (ret != 0) {
      fprintf(stderr, "eeprom_write_byte ret=%d at %x %x\n", ret, mem_addr, data);
    }
  }

  else if (set) {
    if (-1 == eeprom_detect(&e, i2c_addr)) {
      fprintf(stderr, "No i2c device found at address 0x%02x on bus %d", i2c_addr, i2c_bus);
      return 6;
    }
    int val = strtol(setstr, NULL, 16);
    if (val & 0xff != val) {
      fprintf(stderr, "Value to set must be between 0 and 255\n");
      return 5;
    }
    uint8_t setval = val & 0xff;
    for(i=0; i<eeprom_bytes; i++) {
      mem_addr = i;
      ret = eeprom_write_byte(&e, mem_addr, setval);
      if (ret != 0) {
        fprintf(stderr, "eeprom_write_byte ret=%d at %x %x\n", ret, mem_addr, data);
      }
    }
  }

  // memory for holding filenames.
  char fname[64];

  if (do_read_all) {
    // read data in blocks of 256 bytes. 
    int addr;
    int end_addr = i2c_addr_end >= 0 ? i2c_addr_end : i2c_addr;
    for (addr = i2c_addr; addr <= end_addr; addr++) {
      int j;
      if (-1 == eeprom_detect(&e, addr)) {
        fprintf(stderr, "No i2c device found at address 0x%02x on bus %d", addr, i2c_bus);
        continue;
      }
      snprintf(fname, 64, "%s_0x%x", outfile, addr);
      FILE* fptr = fopen(fname, "wb");
      eeprom_set_addr(&e, addr);
      for (j=0; j<eeprom_bytes; j+=256) {
        memset(rdata, 0, sizeof(rdata));
        for(i=0;i<256;i++) {
          mem_addr = j+i;
          ret = eeprom_read_byte(&e, mem_addr);
          //ret = eeprom_24c32_read_byte(&e, mem_addr);
          if (ret == -1) fprintf(stderr, "eeprom_read_byte ret=%d at %x\n",ret,mem_addr);
          else rdata[i] = ret;
        }
        if (256 != fwrite(rdata, 1, 256, fptr)) {
          fprintf(stderr, "Write to file %s failed\n", fname);
          return 4;
        }
      }
      fclose(fptr);
    }
  }
  else if (do_read) {
    // read data in blocks of 256 bytes. 
    int addr;
    int end_addr = i2c_addr_end >= 0 ? i2c_addr_end : i2c_addr;
    FILE* fptr = stdout;
    if (outfile) {
      fptr = fopen(outfile, "w");
    }
    for (addr = i2c_addr; addr <= end_addr; addr++) {
      int j;
      int done = 0;
      int bytes_read = 0;
      if (-1 == eeprom_detect(&e, addr)) {
        fprintf(stderr, "No i2c device found at address 0x%02x on bus %d\n", addr, i2c_bus);
        fprintf(fptr, "--\n");
        continue;
      }
      eeprom_set_addr(&e, addr);
      for (j=0; j<eeprom_bytes && !done; j+=256) {
        memset(rdata, 0, sizeof(rdata));
        for(i=0;i<256;i++) {
          mem_addr = j+i;
          ret = eeprom_read_byte(&e, mem_addr);
          //ret = eeprom_24c32_read_byte(&e, mem_addr);
          if (ret == -1) fprintf(stderr, "eeprom_read_byte ret=%d at %x\n",ret,mem_addr);
          else rdata[i] = ret;
          bytes_read++;
          if (read_bytes > 0) {
            if (bytes_read == read_bytes) {
              done = 1;
              break;
            }
          } else if (ret == 0) {
            done = 1;
            break;
          }
        }
        if (i != fwrite(rdata, 1, i, fptr)) {
          fprintf(stderr, "Write to file %s failed\n", outfile);
          return 4;
        }
      }
      fwrite("\n", 1, 1, fptr);
    }
    if (fptr != stdout)
      fclose(fptr);
  }
  eeprom_close(&e);
}

