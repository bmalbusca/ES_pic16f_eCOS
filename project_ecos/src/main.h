#ifndef MAIN_H
#define MAIN_H

#include <cyg/kernel/kapi.h>
#include <cyg/error/codes.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <stdio.h>
#include <string.h>
#include "threads.h"
#include "proc.h" // proc tasks
#include "monitor.h" // user interface

#define NRBUF 100
#define LM_SIZE (5*NRBUF)
#define CYG_SERIAL_FLAGS_RTSCTS 0x0001

// macros
#define DELTA(X,Y)      ((X) >= (Y) ? (X) - (Y) : (Y) - (X))
#define RINGDELTA(X,Y)  ((Y) >= (X) ? (Y) - (X) + 1 : LM_SIZE - (X) + (Y) + 1) // X,Y are indexes
#define ABS(X)          ((X) < 0 ? -(X) : (X))


#endif
