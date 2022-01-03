// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

// License: LGPL-3.0

#ifdef __LIBLOG_PC__
// This example code itself should work on the PS4 as well, as long as the library is initialized

#include <string.h>

#include "libLog.h"

int main() {
  char test[0x20];
  memset(test, 0xCC, sizeof(test));

#ifdef __VALGRIND__
  for (int i = 0; i < 1000; i++) {
#endif
    logPrint(LL_Error, "This is a logPrint call");
    logPrint(LL_Error, "This is a logPrint call #%i", 2);
    logPrintUnformatted(LL_Error, "This is a logPrintUnformatted call\n");
    logPrintUnformatted(LL_Error, "This is a logPrintUnformatted call #%i\n", 2);
    logPrintHexdump(LL_Error, test, sizeof(test));
    logPrintHexdumpUnformatted(LL_Error, test, sizeof(test));

    logSocketOpen("192.0.2.2", 9023);
    logSocket(LL_Error, "This is a logSocket call");
    logSocket(LL_Error, "This is a logSocket call #%i", 2);
    logSocketUnformatted(LL_Error, "This is a logSocketUnformatted call\n");
    logSocketUnformatted(LL_Error, "This is a logSocketUnformatted call #%i\n", 2);
    logSocketHexdump(LL_Error, test, sizeof(test));
    logSocketHexdumpUnformatted(LL_Error, test, sizeof(test));
    logSocketClose();

    logFileOpen("./test.log");
    logFile(LL_Error, "This is a logFile call");
    logFile(LL_Error, "This is a logFile call #%i", 2);
    logFileUnformatted(LL_Error, "This is a logFileUnformatted call\n");
    logFileUnformatted(LL_Error, "This is a logFileUnformatted call #%i\n", 2);
    logFileHexdump(LL_Error, test, sizeof(test));
    logFileHexdumpUnformatted(LL_Error, test, sizeof(test));
    logFileClose();

    logSocketBindump(LL_Error, "192.0.2.2", 9024, test, sizeof(test));
    logFileBindump(LL_Error, "./test.bin", test, sizeof(test));
#ifdef __VALGRIND__
  }
#endif

  return 0;
}
#else
// ISO C requires a translation unit to contain at least one declaration
int empty_declaration;
#endif
