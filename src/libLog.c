// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

// License: LGPL-3.0

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef __LIBLOG_PC__
#include <orbis/libkernel.h>
#endif

#include "libLog.h"

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KLBLU "\x1B[94m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"
#define KGRY "\x1b[90m"

// Globals
enum LogLevels g_LogLevel = LL_All;
enum LogLevels g_PrintLogLevel = LL_Trace;
enum LogLevels g_KernelLogLevel = LL_Trace;
enum LogLevels g_SocketLogLevel = LL_Trace;
enum LogLevels g_FileLogLevel = LL_Trace;

int g_Socket = -1;
const char *g_SocketLogIpAddress = "000.000.000.000";
uint16_t g_SocketLogPort = 0;

const char *g_LogFileName = "";
FILE *g_LogFilePointer;

static const char *_formatOutput(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, bool p_Meta, const char *p_File, int p_Line, const char *p_Format, va_list p_Args) {
  if (p_File == NULL || p_Format == NULL) {
    return NULL;
  }

  size_t s_MessageSize = vsnprintf(NULL, 0, p_Format, p_Args);
  if (s_MessageSize <= 0) {
    return NULL;
  }
  s_MessageSize++; // Null terminator

  char *s_Message = calloc(s_MessageSize, sizeof(char));
  if (s_Message == NULL) {
    return NULL;
  }

  vsnprintf(s_Message, s_MessageSize, p_Format, p_Args);

  if (!p_Meta) {
    return s_Message;
  }

  const char *s_LevelString = "None ";
  const char *s_LevelColor = KNRM;
  const char *s_LevelResetColor = KNRM;

  switch (p_LogLevel) {
  case LL_None:
    break;
  case LL_Fatal:
    s_LevelString = "Fatal";
    s_LevelColor = KMAG;
    break;
  case LL_Error:
    s_LevelString = "Error";
    s_LevelColor = KRED;
    break;
  case LL_Warn:
    s_LevelString = "Warn";
    s_LevelColor = KYEL;
    break;
  case LL_Info:
    s_LevelString = "Info";
    s_LevelColor = KGRN;
    break;
  case LL_Debug:
    s_LevelString = "Debug";
    s_LevelColor = KCYN;
    break;
  case LL_Trace:
    s_LevelString = "Trace";
    s_LevelColor = KLBLU;
    break;
  case LL_All:
  default:
    break;
  }

  if (p_PrintType == PT_File) {
    s_LevelColor = "\0";
    s_LevelResetColor = "\0";
  }

  size_t s_OutputSize = snprintf(NULL, 0, "%s[%-5s] %s:%d: %s%s\n", s_LevelColor, s_LevelString, p_File, p_Line, s_Message, s_LevelResetColor);
  if (s_OutputSize <= 0) {
    return NULL;
  }
  s_OutputSize++; // Null terminator

  char *s_Output = calloc(s_OutputSize, sizeof(char));
  if (s_Output == NULL) {
    free((void *)s_Message);
    return NULL;
  }

  snprintf(s_Output, s_OutputSize, "%s[%-5s] %s:%d: %s%s\n", s_LevelColor, s_LevelString, p_File, p_Line, s_Message, s_LevelResetColor);
  free((void *)s_Message);

  return s_Output;
}

static size_t _hexdumpSize(int p_Input) {
  int s_Lines = p_Input / 0x10;
  if (s_Lines == 0) {
    s_Lines++;
  }

  int s_Remain = p_Input % 0x10;
  if (s_Remain > 0) {
    s_Lines++;
  }

  int s_Total = s_Lines * 77 + 1;
  if (s_Remain != 0) {
    s_Total -= 0x10 - s_Remain;
  }

  return s_Total + 1; // + 1 for new line in formatted hexdumps
}

// Hexdump based on: https://stackoverflow.com/a/29865
static const char *_hexdump(bool p_Meta, const void *p_Pointer, int p_Length) {
  if (p_Pointer == NULL) {
    return NULL;
  }

  unsigned char *s_Buffer = (unsigned char *)p_Pointer;

  size_t s_HexdumpStringSize = _hexdumpSize(p_Length);
  char *s_Ret = (char *)calloc(s_HexdumpStringSize, sizeof(char));
  if (s_Ret == NULL) {
    return NULL;
  }

  size_t s_Offset = 0;

  if (p_Meta) {
    if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
      free((void *)s_Ret);
      return NULL;
    }
    s_Offset += snprintf(&s_Ret[s_Offset], 2, "%s", "\n"); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term
  }

  for (int i = 0; i < p_Length; i += 0x10) {
    if (s_Offset + 10 + 1 > s_HexdumpStringSize) {
      free((void *)s_Ret);
      return NULL;
    }
    s_Offset += snprintf(&s_Ret[s_Offset], 11, "%08X: ", i); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%08X: `. Copy size is 10 + null term
    for (int j = 0; j < 0x10; j++) {
      if (i + j < p_Length) {
        if (s_Offset + 3 + 1 > s_HexdumpStringSize) {
          free((void *)s_Ret);
          return NULL;
        }
        s_Offset += snprintf(&s_Ret[s_Offset], 4, "%02X ", s_Buffer[i + j]); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%02X `. Copy size is 3 + null term
      } else {
        if (s_Offset + 3 + 1 > s_HexdumpStringSize) {
          free((void *)s_Ret);
          return NULL;
        }
        s_Offset += snprintf(&s_Ret[s_Offset], 4, "%s", "   "); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 3 + null term
      }
      if (j == 7) {
        if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
          free((void *)s_Ret);
          return NULL;
        }
        s_Offset += snprintf(&s_Ret[s_Offset], 2, "%s", " "); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term
      }
    }

    if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
      free((void *)s_Ret);
      return NULL;
    }
    s_Offset += snprintf(&s_Ret[s_Offset], 2, "%s", " "); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term

    for (int j = 0; j < 0x10; j++) {
      if (i + j < p_Length) {
        if (isprint(s_Buffer[i + j])) {
          if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
            free((void *)s_Ret);
            return NULL;
          }
          s_Offset += snprintf(&s_Ret[s_Offset], 2, "%c", s_Buffer[i + j]); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%c`. Copy size is 1 + null term
        } else {
          if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
            free((void *)s_Ret);
            return NULL;
          }
          s_Offset += snprintf(&s_Ret[s_Offset], 2, "%s", "."); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term
        }
      }
    }

    if (i + 0x10 < p_Length) {
      if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
        free((void *)s_Ret);
        return NULL;
      }
      s_Offset += snprintf(&s_Ret[s_Offset], 2, "%s", "\n"); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term
    }
  }

  if (!p_Meta) {
    if (s_Offset + 1 + 1 > s_HexdumpStringSize) {
      free((void *)s_Ret);
      return NULL;
    }
    snprintf(&s_Ret[s_Offset], 2, "%s", "\n"); // Flawfinder: Ignore. Format cannot be controlled externally here... it's `%s`. Copy size is 1 + null term
  }

  return s_Ret;
}

static const char *_bindump(const void *p_Pointer, int p_Length) {
  if (p_Pointer == NULL) {
    return NULL;
  }

  unsigned char *s_Buffer = (unsigned char *)p_Pointer;

  char *s_Output = (char *)calloc(p_Length + 1, sizeof(char));
  if (s_Output == NULL) {
    return NULL;
  }

  for (int i = 0; i < p_Length; i++) {
    s_Output[i] = s_Buffer[i];
  }

  return s_Output;
}

// https://www.tutorialspoint.com/c-program-to-validate-an-ip-address
static bool _validNumber(char *p_Str) {
  while (*p_Str) {
    if (!isdigit(*p_Str)) {
      return false;
    }
    p_Str++;
  }

  return true;
}

// https://www.tutorialspoint.com/c-program-to-validate-an-ip-address
static bool _validIPAddress(const char *p_IpAddress) {
  if (!p_IpAddress) {
    return false;
  }

  char *s_TempIp = strdup(p_IpAddress);

  int s_Dots = 0;

  char *s_Ptr;
  s_Ptr = strtok(s_TempIp, ".");
  if (!s_Ptr) {
    free((void *)s_TempIp);
    return false;
  }

  while (s_Ptr) {
    if (!_validNumber(s_Ptr)) {
      free((void *)s_TempIp);
      return false;
    }
    int s_Num = atoi(s_Ptr); // Flawfinder: ignore. Value is checked below
    if (s_Num >= 0 && s_Num <= 255) {
      s_Ptr = strtok(NULL, ".");
      if (s_Ptr) {
        s_Dots++;
      }
    } else {
      free((void *)s_TempIp);
      return false;
    }
  }
  if (s_Dots != 3) {
    free((void *)s_TempIp);
    return false;
  }

  free((void *)s_TempIp);

  return true;
}

static void _send_socket(const char *s_Buffer) {
  if (s_Buffer == NULL) {
    return;
  }

  if (g_Socket < 0) {
    return;
  }

  if (!_validIPAddress(g_SocketLogIpAddress) || g_SocketLogPort == 0) {
    return;
  }

  struct sockaddr_in s_Servaddr = {
      .sin_family = AF_INET,
      .sin_addr = {
          .s_addr = inet_addr(g_SocketLogIpAddress),
      },
      .sin_port = htons(g_SocketLogPort),
  };

  sendto(g_Socket, s_Buffer, strlen(s_Buffer), 0, (struct sockaddr *)&s_Servaddr, sizeof(s_Servaddr));
}

static void _write_file(const char *s_Buffer) {
  if (s_Buffer == NULL || g_LogFilePointer == NULL) {
    return;
  }

  fprintf(g_LogFilePointer, "%s", s_Buffer);
}

static bool _checkLogLevelByType(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType) {
  enum LogLevels s_TypeLogLevel;
  switch (p_PrintType) {
  case PT_Print:
    s_TypeLogLevel = g_PrintLogLevel;
    break;
  case PT_Kernel:
    s_TypeLogLevel = g_KernelLogLevel;
    break;
  case PT_Socket:
    s_TypeLogLevel = g_SocketLogLevel;
    break;
  case PT_File:
    s_TypeLogLevel = g_FileLogLevel;
    break;
  default:
    return false;
    break;
  }

  if (g_LogLevel >= g_PrintLogLevel && s_TypeLogLevel >= p_LogLevel) {
    return true;
  }

  return false;
}

static void _printByType(enum PrintTypes p_PrintType, const char *s_Buffer) {
  if (p_PrintType == PT_Print) {
    printf("%s", s_Buffer);
  } else if (p_PrintType == PT_Kernel) {
#ifndef __LIBLOG_PC__
    sceKernelDebugOutText(0, s_Buffer);
#else
    // Not the same as `sceKernelDebugOutText` but it workings as a way to tell if it's even getting here and the data input into it
    printf("sceKernelDebugOutText(0, %s);\n", s_Buffer);
#endif
  } else if (p_PrintType == PT_Socket) {
    _send_socket(s_Buffer);
  } else if (p_PrintType == PT_File) {
    _write_file(s_Buffer);
  }
}

void _logPrint(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, bool p_Meta, const char *p_File, int p_Line, const char *p_Format, ...) {
  if (p_File == NULL || p_Format == NULL) {
    return;
  }

  if (!_checkLogLevelByType(p_LogLevel, p_PrintType)) {
    return;
  }

  va_list p_Args;
  va_start(p_Args, p_Format);
  const char *s_Output = _formatOutput(p_LogLevel, p_PrintType, p_Meta, p_File, p_Line, p_Format, p_Args);
  va_end(p_Args);
  if (s_Output == NULL) {
    return;
  }

  _printByType(p_PrintType, s_Output);

  free((void *)s_Output);
}

void _logPrintHex(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, bool p_Meta, const char *p_File, int p_Line, const void *p_Pointer, int p_Length) {
  if (p_Pointer == NULL) {
    return;
  }

  if (p_Length == 0) {
    return;
  }

  if (!_checkLogLevelByType(p_LogLevel, p_PrintType)) {
    return;
  }

  const char *s_Output = _hexdump(p_Meta, p_Pointer, p_Length);
  if (s_Output == NULL) {
    return;
  }

  _logPrint(p_LogLevel, p_PrintType, p_Meta, p_File, p_Line, "%s", s_Output);

  free((void *)s_Output);
}

void _logPrintBin(enum LogLevels p_LogLevel, enum PrintTypes p_PrintType, const char *p_Input, uint16_t p_Port, const void *p_Pointer, int p_Length) {
  if (p_Input == NULL || p_Pointer == NULL) {
    return;
  }

  if (p_Length == 0) {
    return;
  }

  if (p_PrintType != PT_Socket && p_PrintType != PT_File) {
    return;
  }

  if (!_checkLogLevelByType(p_LogLevel, p_PrintType)) {
    return;
  }

  const char *s_Output = _bindump(p_Pointer, p_Length);
  if (s_Output == NULL) {
    return;
  }

  if (p_PrintType == PT_Socket) {
    if (!_validIPAddress(p_Input) || p_Port == 0) {
      free((void *)s_Output);
      return;
    }

    int s_Socket = -1;
    if ((s_Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      free((void *)s_Output);
      return;
    }

    struct sockaddr_in s_Servaddr = {
        .sin_family = AF_INET,
        .sin_addr = {
            .s_addr = inet_addr(p_Input),
        },
        .sin_port = htons(p_Port),
    };

    sendto(s_Socket, s_Output, p_Length, 0, (struct sockaddr *)&s_Servaddr, sizeof(s_Servaddr));

    close(s_Socket);
  } else if (p_PrintType == PT_File) {
    FILE *s_FilePointer;

    s_FilePointer = fopen(p_Input, "wb");

    if (s_FilePointer == NULL) {
      free((void *)s_Output);
      return;
    }

    fwrite(s_Output, sizeof(char), p_Length, s_FilePointer);

    fclose(s_FilePointer);
  }

  free((void *)s_Output);
}

bool logSocketOpen(const char *p_IpAddress, uint16_t p_Port) {
  if (p_IpAddress == NULL) {
    return false;
  }

  if (!_validIPAddress(p_IpAddress)) {
    return false;
  }

  if (g_Socket < 0) {
    if ((g_Socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      return false;
    }
  }

  g_SocketLogIpAddress = p_IpAddress;
  g_SocketLogPort = p_Port;

  return true;
}

bool logSocketClose() {
  if (g_Socket < 0) {
    return true;
  }

  if (close(g_Socket) < 0) {
    return false;
  }

  g_Socket = -1;

  return true;
}

bool logSocketIsOpen() {
  if (g_Socket < 0) {
    return false;
  }

  return true;
}

const char *logSocketGetIpAddress() {
  return g_SocketLogIpAddress;
}

uint16_t logSocketGetPort() {
  return g_SocketLogPort;
}

bool logFileOpen(const char *p_Path) {
  if (p_Path == NULL) {
    return false;
  }

  if (g_LogFilePointer != NULL) {
    // We should either close the currently open file and open the new file, or return false to "fail"
    return false; // or logFileClose();
  }

  g_LogFilePointer = fopen(p_Path, "a");

  if (g_LogFilePointer == NULL) {
    return false;
  }

  g_LogFileName = p_Path;

  return true;
}

bool logFileClose() {
  if (g_LogFilePointer == NULL) {
    g_LogFileName = "";
    return true;
  }

  fclose(g_LogFilePointer);
  g_LogFilePointer = NULL;
  g_LogFileName = "";

  return true;
}

const char *logFileGetFilename() {
  return g_LogFileName;
}

void logSetLogLevel(enum LogLevels p_LogLevel) {
  g_LogLevel = p_LogLevel;
}

enum LogLevels logGetLogLevel() {
  return g_LogLevel;
}

void logPrintSetLogLevel(enum LogLevels p_LogLevel) {
  g_PrintLogLevel = p_LogLevel;
}

enum LogLevels logPrintGetLogLevel() {
  return g_PrintLogLevel;
}

void logKernelSetLogLevel(enum LogLevels p_LogLevel) {
  g_KernelLogLevel = p_LogLevel;
}

enum LogLevels logKernelGetLogLevel() {
  return g_KernelLogLevel;
}

void logFileSetLogLevel(enum LogLevels p_LogLevel) {
  g_FileLogLevel = p_LogLevel;
}

enum LogLevels logFileGetLogLevel() {
  return g_FileLogLevel;
}

void logSocketSetLogLevel(enum LogLevels p_LogLevel) {
  g_SocketLogLevel = p_LogLevel;
}

enum LogLevels logSocketGetLogLevel() {
  return g_SocketLogLevel;
}
