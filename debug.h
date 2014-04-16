#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <stdio.h>
#define DEBUG_OUT printf

#else
#define DEBUG_OUT
#endif //DEBUG

#endif //DEBUG_H
