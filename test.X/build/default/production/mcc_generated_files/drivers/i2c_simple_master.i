# 1 "mcc_generated_files/drivers/i2c_simple_master.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 288 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "/opt/microchip/xc8/v2.10/pic/include/language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "mcc_generated_files/drivers/i2c_simple_master.c" 2
# 27 "mcc_generated_files/drivers/i2c_simple_master.c"
# 1 "mcc_generated_files/drivers/../drivers/i2c_master.h" 1
# 26 "mcc_generated_files/drivers/../drivers/i2c_master.h"
# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/stdbool.h" 1 3
# 26 "mcc_generated_files/drivers/../drivers/i2c_master.h" 2

# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 1 3



# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/musl_xc8.h" 1 3
# 5 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 2 3
# 22 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 3
# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 1 3
# 127 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef unsigned long uintptr_t;
# 142 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef long intptr_t;
# 158 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef signed char int8_t;




typedef short int16_t;




typedef __int24 int24_t;




typedef long int32_t;





typedef long long int64_t;
# 188 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef long long intmax_t;





typedef unsigned char uint8_t;




typedef unsigned short uint16_t;




typedef __uint24 uint24_t;




typedef unsigned long uint32_t;





typedef unsigned long long uint64_t;
# 229 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef unsigned long long uintmax_t;
# 23 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 2 3

typedef int8_t int_fast8_t;

typedef int64_t int_fast64_t;


typedef int8_t int_least8_t;
typedef int16_t int_least16_t;

typedef int24_t int_least24_t;

typedef int32_t int_least32_t;

typedef int64_t int_least64_t;


typedef uint8_t uint_fast8_t;

typedef uint64_t uint_fast64_t;


typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;

typedef uint24_t uint_least24_t;

typedef uint32_t uint_least32_t;

typedef uint64_t uint_least64_t;
# 139 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 3
# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/stdint.h" 1 3
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
# 140 "/opt/microchip/xc8/v2.10/pic/include/c99/stdint.h" 2 3
# 27 "mcc_generated_files/drivers/../drivers/i2c_master.h" 2

# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 1 3
# 10 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 3
# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/features.h" 1 3
# 11 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 2 3
# 24 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 3
# 1 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 1 3





typedef void * va_list[1];




typedef void * __isoc_va_list[1];
# 122 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef unsigned size_t;
# 137 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef long ssize_t;
# 246 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef long long off_t;
# 399 "/opt/microchip/xc8/v2.10/pic/include/c99/bits/alltypes.h" 3
typedef struct _IO_FILE FILE;
# 25 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 2 3
# 52 "/opt/microchip/xc8/v2.10/pic/include/c99/stdio.h" 3
typedef union _G_fpos64_t {
 char __opaque[16];
 double __align;
} fpos_t;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;





FILE *fopen(const char *restrict, const char *restrict);
FILE *freopen(const char *restrict, const char *restrict, FILE *restrict);
int fclose(FILE *);

int remove(const char *);
int rename(const char *, const char *);

int feof(FILE *);
int ferror(FILE *);
int fflush(FILE *);
void clearerr(FILE *);

int fseek(FILE *, long, int);
long ftell(FILE *);
void rewind(FILE *);

int fgetpos(FILE *restrict, fpos_t *restrict);
int fsetpos(FILE *, const fpos_t *);

size_t fread(void *restrict, size_t, size_t, FILE *restrict);
size_t fwrite(const void *restrict, size_t, size_t, FILE *restrict);

int fgetc(FILE *);
int getc(FILE *);
int getchar(void);
int ungetc(int, FILE *);

int fputc(int, FILE *);
int putc(int, FILE *);
int putchar(int);

char *fgets(char *restrict, int, FILE *restrict);

char *gets(char *);


int fputs(const char *restrict, FILE *restrict);
int puts(const char *);

#pragma printf_check(printf) const
#pragma printf_check(vprintf) const
#pragma printf_check(sprintf) const
#pragma printf_check(snprintf) const
#pragma printf_check(vsprintf) const
#pragma printf_check(vsnprintf) const

int printf(const char *restrict, ...);
int fprintf(FILE *restrict, const char *restrict, ...);
int sprintf(char *restrict, const char *restrict, ...);
int snprintf(char *restrict, size_t, const char *restrict, ...);

int vprintf(const char *restrict, __isoc_va_list);
int vfprintf(FILE *restrict, const char *restrict, __isoc_va_list);
int vsprintf(char *restrict, const char *restrict, __isoc_va_list);
int vsnprintf(char *restrict, size_t, const char *restrict, __isoc_va_list);

int scanf(const char *restrict, ...);
int fscanf(FILE *restrict, const char *restrict, ...);
int sscanf(const char *restrict, const char *restrict, ...);
int vscanf(const char *restrict, __isoc_va_list);
int vfscanf(FILE *restrict, const char *restrict, __isoc_va_list);
int vsscanf(const char *restrict, const char *restrict, __isoc_va_list);

void perror(const char *);

int setvbuf(FILE *restrict, char *restrict, int, size_t);
void setbuf(FILE *restrict, char *restrict);

char *tmpnam(char *);
FILE *tmpfile(void);




FILE *fmemopen(void *restrict, size_t, const char *restrict);
FILE *open_memstream(char **, size_t *);
FILE *fdopen(int, const char *);
FILE *popen(const char *, const char *);
int pclose(FILE *);
int fileno(FILE *);
int fseeko(FILE *, off_t, int);
off_t ftello(FILE *);
int dprintf(int, const char *restrict, ...);
int vdprintf(int, const char *restrict, __isoc_va_list);
void flockfile(FILE *);
int ftrylockfile(FILE *);
void funlockfile(FILE *);
int getc_unlocked(FILE *);
int getchar_unlocked(void);
int putc_unlocked(int, FILE *);
int putchar_unlocked(int);
ssize_t getdelim(char **restrict, size_t *restrict, int, FILE *restrict);
ssize_t getline(char **restrict, size_t *restrict, FILE *restrict);
int renameat(int, const char *, int, const char *);
char *ctermid(char *);







char *tempnam(const char *, const char *);
# 28 "mcc_generated_files/drivers/../drivers/i2c_master.h" 2

# 1 "mcc_generated_files/drivers/i2c_types.h" 1
# 29 "mcc_generated_files/drivers/i2c_types.h"
typedef enum {
    I2C_NOERR,
    I2C_BUSY,
    I2C_FAIL


} i2c_error_t;

typedef enum
{
    i2c_stop=1,
    i2c_restart_read,
    i2c_restart_write,
    i2c_continue,
    i2c_reset_link
} i2c_operations_t;

typedef i2c_operations_t (*i2c_callback)(void *p);

typedef uint8_t i2c_address_t;


i2c_operations_t i2c_returnStop(void *p);
i2c_operations_t i2c_returnReset(void *p);
i2c_operations_t i2c_restartWrite(void *p);
i2c_operations_t i2c_restartRead(void *p);
# 29 "mcc_generated_files/drivers/../drivers/i2c_master.h" 2




i2c_error_t i2c_open(i2c_address_t address);
void i2c_setAddress(i2c_address_t address);
i2c_error_t i2c_close(void);
i2c_error_t i2c_masterOperation(_Bool read);
i2c_error_t i2c_masterWrite(void);
i2c_error_t i2c_masterRead(void);

void i2c_setTimeOut(uint8_t to);
void i2c_setBuffer(void *buffer, size_t bufferSize);


void i2c_setDataCompleteCallback(i2c_callback cb, void *p);
void i2c_setWriteCollisionCallback(i2c_callback cb, void *p);
void i2c_setAddressNACKCallback(i2c_callback cb, void *p);
void i2c_setDataNACKCallback(i2c_callback cb, void *p);
void i2c_setTimeOutCallback(i2c_callback cb, void *p);


void i2c_ISR(void);
void i2c_busCollisionISR(void);
# 27 "mcc_generated_files/drivers/i2c_simple_master.c" 2

# 1 "mcc_generated_files/drivers/i2c_simple_master.h" 1
# 30 "mcc_generated_files/drivers/i2c_simple_master.h"
uint8_t i2c_read1ByteRegister(i2c_address_t address, uint8_t reg);
uint16_t i2c_read2ByteRegister(i2c_address_t address, uint8_t reg);
void i2c_write1ByteRegister(i2c_address_t address, uint8_t reg, uint8_t data);
void i2c_write2ByteRegister(i2c_address_t address, uint8_t reg, uint16_t data);

void i2c_writeNBytes(i2c_address_t address, void* data, size_t len);
void i2c_readDataBlock(i2c_address_t address, uint8_t reg, void *data, size_t len);
void i2c_readNBytes(i2c_address_t address, void *data, size_t len);
# 28 "mcc_generated_files/drivers/i2c_simple_master.c" 2



static i2c_operations_t wr1RegCompleteHandler(void *p)
{
    i2c_setBuffer(p,1);
    i2c_setDataCompleteCallback(((void*)0),((void*)0));
    return i2c_continue;
}

void i2c_write1ByteRegister(i2c_address_t address, uint8_t reg, uint8_t data)
{
    while(!i2c_open(address));
    i2c_setDataCompleteCallback(wr1RegCompleteHandler,&data);
    i2c_setBuffer(&reg,1);
    i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
    i2c_masterWrite();
    while(I2C_BUSY == i2c_close());
}

void i2c_writeNBytes(i2c_address_t address, void* data, size_t len)
{
    while(!i2c_open(address));
    i2c_setBuffer(data,len);
    i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
    i2c_masterWrite();
    while(I2C_BUSY == i2c_close());
}


static i2c_operations_t rd1RegCompleteHandler(void *p)
{
    i2c_setBuffer(p,1);
    i2c_setDataCompleteCallback(((void*)0),((void*)0));
    return i2c_restart_read;
}

uint8_t i2c_read1ByteRegister(i2c_address_t address, uint8_t reg)
{
    uint8_t d2=42;
    i2c_error_t e;
    int x;

    for(x = 2; x != 0; x--)
    {
        while(!i2c_open(address));
        i2c_setDataCompleteCallback(rd1RegCompleteHandler,&d2);
        i2c_setBuffer(&reg,1);
        i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
        i2c_masterWrite();
        while(I2C_BUSY == (e = i2c_close()));
        if(e==I2C_NOERR) break;
    }


    return d2;
}


static i2c_operations_t rd2RegCompleteHandler(void *p)
{
    i2c_setBuffer(p,2);
    i2c_setDataCompleteCallback(((void*)0),((void*)0));
    return i2c_restart_read;
}

uint16_t i2c_read2ByteRegister(i2c_address_t address, uint8_t reg)
{

    uint16_t result;

    while(!i2c_open(address));
    i2c_setDataCompleteCallback(rd2RegCompleteHandler,&result);
    i2c_setBuffer(&reg,1);
    i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
    i2c_masterWrite();
    while(I2C_BUSY == i2c_close());

    return (result << 8 | result >> 8);
}


static i2c_operations_t wr2RegCompleteHandler(void *p)
{
    i2c_setBuffer(p,2);
    i2c_setDataCompleteCallback(((void*)0),((void*)0));
    return i2c_continue;
}

void i2c_write2ByteRegister(i2c_address_t address, uint8_t reg, uint16_t data)
{
    while(!i2c_open(address));
    i2c_setDataCompleteCallback(wr2RegCompleteHandler,&data);
    i2c_setBuffer(&reg,1);
    i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
    i2c_masterWrite();
    while(I2C_BUSY == i2c_close());
}


typedef struct
{
    size_t len;
    char *data;
}buf_t;

static i2c_operations_t rdBlkRegCompleteHandler(void *p)
{
    i2c_setBuffer(((buf_t *)p)->data,((buf_t*)p)->len);
    i2c_setDataCompleteCallback(((void*)0),((void*)0));
    return i2c_restart_read;
}

void i2c_readDataBlock(i2c_address_t address, uint8_t reg, void *data, size_t len)
{

    buf_t d;
    d.data = data;
    d.len = len;

    while(!i2c_open(address));
    i2c_setDataCompleteCallback(rdBlkRegCompleteHandler,&d);
    i2c_setBuffer(&reg,1);
    i2c_setAddressNACKCallback(i2c_restartWrite,((void*)0));
    i2c_masterWrite();
    while(I2C_BUSY == i2c_close());
}

void i2c_readNBytes(i2c_address_t address, void *data, size_t len)
{
    while(!i2c_open(address));
    i2c_setBuffer(data,len);
    i2c_masterRead();
    while(I2C_BUSY == i2c_close());
}
