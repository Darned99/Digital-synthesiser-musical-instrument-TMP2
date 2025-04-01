#ifndef klaw_h
#define klaw_h

#include "MKL05Z4.h"

#define klaw_port PORTA

#define R1 5
#define R2 6
#define R3 7
#define R4 8

#define C1 9
#define C2 10
#define C3 11
#define C4 12

void klaw_Init(void);
char klaw_read(void);

#endif
