// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

// License: LGPL-3.0

#ifndef LIBLOG_H
#define LIBLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum LogLevels {
  LL_None,
  LL_Fatal,
  LL_Error,
  LL_Warn,
  LL_Info,
  LL_Debug,
  LL_Trace,
  LL_All
};

enum PrintTypes {
  PT_Print,
  PT_Kernel,
  PT_Socket,
  PT_File,
};

void _logPrint(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, bool p_Meta, const char *p_File, int p_Line, const char *p_Format, ...);
void _logPrintHex(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, bool p_Meta, const char *p_File, int p_Line, const void *p_Pointer, int p_Length);
void _logPrintBin(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, const char *p_Input, uint16_t p_Port, const void *p_Pointer, int p_Length);

bool logSocketOpen(const char *p_IpAddress, uint16_t p_Port);
bool logSocketClose();
bool logSocketIsOpen();

const char *logSocketGetIpAddress();
uint16_t logSocketGetPort();

bool logFileOpen(const char *p_Path);
bool logFileClose();
const char *logFileGetFilename();

void logSetLogLevel(enum LogLevels p_LogLevel);
enum LogLevels logGetLogLevel();

void logPrintSetLogLevel(enum LogLevels p_LogLevel);
enum LogLevels logPrintGetLogLevel();

void logKernelSetLogLevel(enum LogLevels p_LogLevel);
enum LogLevels logKernelGetLogLevel();

void logFileSetLogLevel(enum LogLevels p_LogLevel);
enum LogLevels logFileGetLogLevel();

void logSocketSetLogLevel(enum LogLevels p_LogLevel);
enum LogLevels logSocketGetLogLevel();

#ifdef __cplusplus
}
#endif

#endif
