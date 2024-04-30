# Hand ins Practical Works 2 and 4


| Student Name     | Student ID |
|------------------|------------|
| Charlotte Heibig | 300855     |
| Ismael Frei      | 301225     |

## Practical Work 2
The complete folder path for the Verilog code from the virtual_prototype are the following: \

`modules/profile/verilog`
`modules/rgb565GrayscaleIse/verilog`

The complete folder path for the C code is:
`programms/grayscale`

Results | cycles without HW acceleration | cycles with HW acceleration | 
--- | --- | ---
CPU execution cycles | 30686434 | 24872108
CPU stall cycles | 19006087 | 18414161
CPU bus-idle cycles | 16930928 | 11858230


## Practical Work 4
The complete folder path for the Verilog code from the virtual_prototype are the following: \

`modules/ramDma/verilog`

The complete folder path for the C code is:
`programms/grayscale`

exerpt from CuteCom when running the program:

```
[02:01:18:195] CS-476 Embedded System Design
[02:01:18:195] Openrisc based virtual Prototype.
[02:01:18:195] Build version: Di 13 Feb 2024 10:27:52 CET
[02:01:18:195] 
[02:01:18:195] I am CPU 1 of 1 running at 74.24 MHz.
[02:01:18:195] 
[02:01:18:195] Known RS232 commands:
[02:01:18:195] $  Start the program loaded in target
[02:01:18:195] *p Set programming mode (default)
[02:01:18:195] *v Set verification mode
[02:01:18:195] *i Show info on program in target
[02:01:18:195] *t Toggle target between SDRam (default) and Flash
[02:01:18:195] *m Perform simple SDRam memcheck
[02:01:18:195] *s Check SPI-flash chip
[02:01:18:195] *e Erase SPI-flash chip
[02:01:18:195] *r Run program in SPI-flash
[02:01:18:195] *f Store program loaded in SDRAM to SPI-Flash
[02:01:18:216] *c Compare program loaded in SDRAM with SPI-Flash
[02:01:18:216] *h This helpscreen
[02:01:18:216] 
[02:01:20:920] Setting prog. mode
[02:01:23:310] Reading code table
[02:01:24:680] Download: done
[02:01:25:493] Executing loaded program...
[02:01:25:493] 
[02:01:25:493] ===== Read/Write directly to CI memory =====
[02:01:25:493] At SRAM address 67: Read before Write= 0, Read after Write 68
[02:01:25:493] At SRAM address 68: Read before Write= 0, Read after Write 69
[02:01:25:493] At SRAM address 69: Read before Write= 0, Read after Write 70
[02:01:25:493] At SRAM address 70: Read before Write= 0, Read after Write 71
[02:01:25:493] At SRAM address 71: Read before Write= 0, Read after Write 72
[02:01:25:493] At SRAM address 72: Read before Write= 0, Read after Write 73
[02:01:25:493] 
[02:01:25:493] ===== Single burst transfer from bus to CI-memory with blocksize = 5 =====
[02:01:25:536] Status register value: 1
[02:01:25:536] Status register value: 0
[02:01:25:536] Value at SRAM memory location 0x  2: 0x       2
[02:01:25:536] Value at SRAM memory location 0x  3: 0x       3
[02:01:25:536] Value at SRAM memory location 0x  4: 0x       4
[02:01:25:536] Value at SRAM memory location 0x  5: 0x       5
[02:01:25:536] Value at SRAM memory location 0x  6: 0x       6
[02:01:25:536] Status register value: 0
[02:01:25:536] 
[02:01:25:536] ===== Single burst transfer from bus to CI-memory with blocksize = 3 =====
[02:01:25:536] Status register value: 1
[02:01:25:536] Status register value: 0
[02:01:25:536] Value at SRAM memory location 0x 11: 0x      11
[02:01:25:585] Value at SRAM memory location 0x 12: 0x      12
[02:01:25:585] Value at SRAM memory location 0x 13: 0x      13
[02:01:25:585] Status register value: 0
[02:01:25:585] 
[02:01:25:585] ===== Multiple burst transfer from bus to CI-memory, blocksize = 7 =====
[02:01:25:585] Status register value: 1
[02:01:25:585] Status register value: 0
[02:01:25:585] Value at SRAM memory location 0x 21: 0x      21
[02:01:25:585] Value at SRAM memory location 0x 22: 0x      22
[02:01:25:585] Value at SRAM memory location 0x 23: 0x      23
[02:01:25:585] Value at SRAM memory location 0x 24: 0x      24
[02:01:25:585] Value at SRAM memory location 0x 25: 0x      25
[02:01:25:621] Value at SRAM memory location 0x 26: 0x      26
[02:01:25:621] Value at SRAM memory location 0x 27: 0x      21
[02:01:25:621] Status register value: 0
[02:01:25:621] Check burst size 5
[02:01:25:621] Check block size 7
[02:01:25:621] Check bus start address 8386664
[02:01:25:621] Check memory start address 32
[02:01:25:621] 
[02:01:25:621] ===== Single burst transfer from CI-memory to bus with blocksize = 5 =====
[02:01:25:621] Status of bus before transfer
[02:01:25:621] Value at bus location 128: 129
[02:01:25:621] Value at bus location 129: 130
[02:01:25:621] Value at bus location 130: 131
[02:01:25:621] Value at bus location 131: 132
[02:01:25:621] Value at bus location 132: 133
[02:01:25:621] Starting transfer
[02:01:25:665] Status register value: 1
[02:01:25:665] Status register value: 0
[02:01:25:665] Status of bus after transfer
[02:01:25:665] Value at bus location 128: 34
[02:01:25:665] Value at bus location 129: 35
[02:01:25:665] Value at bus location 130: 36
[02:01:25:665] Value at bus location 131: 37
[02:01:25:665] Value at bus location 132: 38
[02:01:25:665] 
[02:01:25:665] ===== Single burst transfer from CI-memory to bus with blocksize = 3 =====
[02:01:25:665] Status of bus before transfer
[02:01:25:665] Value at bus location 256: 257
[02:01:25:665] Value at bus location 257: 258
[02:01:25:665] Value at bus location 258: 259
[02:01:25:665] Starting transfer
[02:01:25:665] Status register value: 1
[02:01:25:665] Status register value: 0
[02:01:25:707] Status of bus after transfer
[02:01:25:707] Value at bus location 256: 34
[02:01:25:707] Value at bus location 257: 35
[02:01:25:707] Value at bus location 258: 36
[02:01:25:707] 
[02:01:25:707] ===== Multiple burst transfer from CI-memory to bus, blocksize = 7 =====
[02:01:25:707] Status of bus before transfer
[02:01:25:707] Value at bus location 384: 385
[02:01:25:707] Value at bus location 385: 386
[02:01:25:707] Value at bus location 386: 387
[02:01:25:707] Value at bus location 387: 388
[02:01:25:707] Value at bus location 388: 389
[02:01:25:707] Value at bus location 389: 390
[02:01:25:707] Starting transfer
[02:01:25:707] Status register value: 1
[02:01:25:707] Status register value: 0
[02:01:25:707] Status of bus after transfer
[02:01:25:753] Value at bus location 384: 34
[02:01:25:753] Value at bus location 385: 35
[02:01:25:753] Value at bus location 386: 36
[02:01:25:753] Value at bus location 387: 37
[02:01:25:753] Value at bus location 388: 38
[02:01:25:753] Value at bus location 389: 33
[02:01:25:753] End of program
```