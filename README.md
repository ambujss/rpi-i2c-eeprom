# Raspberry-24Cxx

Two-Wire Serial EEPROM Access Command line tool for RaspberryPi.   

---

# Build
for 24C02   
cc -o i2ceeprom main.c 24cXX.c -DC02

for 24C04   
cc -o i2ceeprom main.c 24cXX.c -DC04

for 24C08   
cc -o i2ceeprom main.c 24cXX.c -DC08

for 24C16   
cc -o i2ceeprom main.c 24cXX.c -DC16

for 24C32   
cc -o i2ceeprom main.c 24cXX.c -DC32

for 24C64   
cc -o i2ceeprom main.c 24cXX.c -DC64

for 24C128   
cc -o i2ceeprom main.c 24cXX.c -DC128

for 24C256   
cc -o i2ceeprom main.c 24cXX.c -DC256

for 24C512   
cc -o i2ceeprom main.c 24cXX.c -DC512

---

# Usage

```
Usage: ./i2ceeprom <i2c_bus> <i2c_addr> [i2c_addr_end][FLAGS]
    i2c_bus        I2C bus number
    i2c_addr       I2C Address of EEPROM chip. If i2c_addr_end is specified, this is the first address
                   of the range of addresses from i2c_addr to i2c_addr_end.
    i2c_addr_end   End of Address range of EEPROMs. Only specify this  when reading.
    -w <contents>  Contents to be written to EEPROM
    -r [bytes]     Read contents from EEPROM. Outputs to stdout unless -o is specified.
                   Will read 'bytes' worth of data if specified, else till the first null byte (0) is encountered.
                   If i2c_addr_end is specified, all EEPROMs from i2c_addr to i2c_addr_end will be read. Contents of
                   each EEPROM will be output to a new line.
    -o <outfile>   Output filename. If used with -r, Contents of all EEPROMS will be written to <outfile>, separated by a newline.
                   If used with -a, contents of each EEPROM will be written to its own file
                   which will be named as <outfile>_<addr>
    -a             Read the entire EEPROM. -o needs to be specified with this flag.
                   Dumps output in binary format.
    -v             Verbose.
    -s <value>     Set all bytes in EEPROM to this value
One of -w, -r, -o, -a or -s must be specified. If both read and write operations are specified,
the read will take place after the write.
-s cannot be specified with -w.
```
