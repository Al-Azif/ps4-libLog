// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

// License: LGPL-3.0

#ifndef LIBLOG_H
#define LIBLOG_H

#ifdef __cplusplus
#include <cstdio>
#include <ostream>
#include <sstream>

#ifdef __LIBLOG_PC__
#include <iostream>
#endif

extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __LIBLOG_PC__
#include <orbis/libkernel.h>
#endif

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

#define logPrint(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Print, true, __FILE__, __LINE__, __VA_ARGS__)
#define logPrintUnformatted(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Print, false, __FILE__, __LINE__, __VA_ARGS__)
#define logPrintHexdump(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Print, true, __FILE__, __LINE__, p_Pointer, p_Length)
#define logPrintHexdumpUnformatted(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Print, false, __FILE__, __LINE__, p_Pointer, p_Length)

#define logKernel(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Kernel, true, __FILE__, __LINE__, __VA_ARGS__)
#define logKernelUnformatted(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Kernel, false, __FILE__, __LINE__, __VA_ARGS__)
#define logKernelHexdump(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Kernel, true, __FILE__, __LINE__, p_Pointer, p_Length)
#define logKernelHexdumpUnformatted(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Kernel, false, __FILE__, __LINE__, p_Pointer, p_Length)

#define logSocket(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Socket, true, __FILE__, __LINE__, __VA_ARGS__)
#define logSocketUnformatted(p_LogLevel, ...) _logPrint(p_LogLevel, PT_Socket, false, __FILE__, __LINE__, __VA_ARGS__)
#define logSocketHexdump(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Socket, true, __FILE__, __LINE__, p_Pointer, p_Length)
#define logSocketHexdumpUnformatted(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_Socket, false, __FILE__, __LINE__, p_Pointer, p_Length)

#define logFile(p_LogLevel, ...) _logPrint(p_LogLevel, PT_File, true, __FILE__, __LINE__, __VA_ARGS__)
#define logFileUnformatted(p_LogLevel, ...) _logPrint(p_LogLevel, PT_File, false, __FILE__, __LINE__, __VA_ARGS__)
#define logFileHexdump(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_File, true, __FILE__, __LINE__, p_Pointer, p_Length)
#define logFileHexdumpUnformatted(p_LogLevel, p_Pointer, p_Length) _logPrintHex(p_LogLevel, PT_File, false, __FILE__, __LINE__, p_Pointer, p_Length)

#define logSocketBindump(p_LogLevel, p_IpAddress, p_Port, p_Pointer, p_Length) _logPrintBin(p_LogLevel, PT_Socket, p_IpAddress, p_Port, p_Pointer, p_Length)
#define logFileBindump(p_LogLevel, p_Path, p_Pointer, p_Length) _logPrintBin(p_LogLevel, PT_File, p_Path, 0, p_Pointer, p_Length)

#ifdef __cplusplus
}

extern FILE *g_LogFilePointer;

void _sendSocket(const char *s_Buffer);

// C++ Bindings

class PrintLog {
public:
  PrintLog(const char *p_File, int32_t p_Line, bool p_Format) {
    m_Format = p_Format;
    if (m_Format) {
      if (logGetLogLevel() <= LL_None) {
        m_Skip = true;
        return;
      }

      if (logPrintGetLogLevel() > logGetLogLevel()) {
        m_Skip = true;
        return;
      }

      const char *s_LevelString = "None";
      const char *s_LevelColor = KNRM;

      switch (logPrintGetLogLevel()) {
      case LL_Info:
        s_LevelString = "Info";
        s_LevelColor = KGRN;
        break;
      case LL_Warn:
        s_LevelString = "Warn";
        s_LevelColor = KYEL;
        break;
      case LL_Error:
        s_LevelString = "Error";
        s_LevelColor = KRED;
        break;
      case LL_Debug:
        s_LevelString = "Debug";
        s_LevelColor = KGRY;
        break;
      case LL_None:
      default:
        s_LevelString = "None";
        s_LevelColor = KNRM;
        break;
      }
      m_LogStream << s_LevelColor << "[" << s_LevelString << "] " << p_File << ":" << p_Line << ": ";
    }
  }

  template <class T>
  PrintLog &operator<<(const T &v) {
    m_LogStream << v;
    return *this;
  }

  PrintLog &operator<<(std::ostream&(*f)(std::ostream&)) {
    m_LogStream << f;
    return *this;
  }

  ~PrintLog() {
    if (m_Format) {
      if (m_Skip) {
        return;
      }
      m_LogStream << KNRM << std::endl;
    }

    std::cout << m_LogStream.str();
    m_LogStream.str("");
  }

private:
  bool m_Skip = false;
  bool m_Format = false;
  std::stringstream m_LogStream;
};

class KernelLog {
public:
  KernelLog(const char *p_File, int32_t p_Line, bool p_Format) {
    m_Format = p_Format;
    if (m_Format) {
      if (logGetLogLevel() <= LL_None) {
        m_Skip = true;
        return;
      }

      if (logKernelGetLogLevel() > logGetLogLevel()) {
        m_Skip = true;
        return;
      }

      const char *s_LevelString = "None";
      const char *s_LevelColor = KNRM;

      switch (logKernelGetLogLevel()) {
      case LL_Info:
        s_LevelString = "Info";
        s_LevelColor = KGRN;
        break;
      case LL_Warn:
        s_LevelString = "Warn";
        s_LevelColor = KYEL;
        break;
      case LL_Error:
        s_LevelString = "Error";
        s_LevelColor = KRED;
        break;
      case LL_Debug:
        s_LevelString = "Debug";
        s_LevelColor = KGRY;
        break;
      case LL_None:
      default:
        s_LevelString = "None";
        s_LevelColor = KNRM;
        break;
      }
      m_LogStream << s_LevelColor << "[" << s_LevelString << "] " << p_File << ":" << p_Line << ": ";
    }
  }

  template <class T>
  KernelLog &operator<<(const T &v) {
    m_LogStream << v;
    return *this;
  }

  KernelLog &operator<<(std::ostream&(*f)(std::ostream&)) {
    m_LogStream << f;
    return *this;
  }

  ~KernelLog() {
    if (m_Format) {
      if (m_Skip) {
        return;
      }
      m_LogStream << KNRM << std::endl;
    }

#ifndef __LIBLOG_PC__
    sceKernelDebugOutText(0, m_LogStream.str().c_str());
#else
    // Not the same as `sceKernelDebugOutText` but it workings as a way to tell if it's even getting here and the data input into it
    std::cout << m_LogStream.str();
#endif
    m_LogStream.str("");
  }

private:
  bool m_Skip = false;
  bool m_Format = false;
  std::stringstream m_LogStream;
};

class SocketLog {
public:
  SocketLog(const char *p_File, int32_t p_Line, bool p_Format) {
    m_Format = p_Format;
    if (m_Format) {
      if (logGetLogLevel() <= LL_None) {
        m_Skip = true;
        return;
      }

      if (logSocketGetLogLevel() > logGetLogLevel()) {
        m_Skip = true;
        return;
      }

      const char *s_LevelString = "None";
      const char *s_LevelColor = KNRM;

      switch (logSocketGetLogLevel()) {
      case LL_Info:
        s_LevelString = "Info";
        s_LevelColor = KGRN;
        break;
      case LL_Warn:
        s_LevelString = "Warn";
        s_LevelColor = KYEL;
        break;
      case LL_Error:
        s_LevelString = "Error";
        s_LevelColor = KRED;
        break;
      case LL_Debug:
        s_LevelString = "Debug";
        s_LevelColor = KGRY;
        break;
      case LL_None:
      default:
        s_LevelString = "None";
        s_LevelColor = KNRM;
        break;
      }
      m_LogStream << s_LevelColor << "[" << s_LevelString << "] " << p_File << ":" << p_Line << ": ";
    }
  }

  template <class T>
  SocketLog &operator<<(const T &v) {
    m_LogStream << v;
    return *this;
  }

  SocketLog &operator<<(std::ostream&(*f)(std::ostream&)) {
    m_LogStream << f;
    return *this;
  }

  ~SocketLog() {
    if (m_Format) {
      if (m_Skip) {
        return;
      }
      m_LogStream << KNRM << std::endl;
    }

    //TODO: _sendSocket(m_LogStream.str().c_str());
    m_LogStream.str("");
  }

private:
  bool m_Skip = false;
  bool m_Format = false;
  std::stringstream m_LogStream;
};

class FileLog {
public:
  FileLog(const char *p_File, int32_t p_Line, bool p_Format) {
    m_Format = p_Format;
    if (m_Format) {
      if (logGetLogLevel() <= LL_None) {
        m_Skip = true;
        return;
      }

      if (logFileGetLogLevel() > logGetLogLevel()) {
        m_Skip = true;
        return;
      }

      const char *s_LevelString = "None";

      switch (logFileGetLogLevel()) {
      case LL_Info:
        s_LevelString = "Info";
        break;
      case LL_Warn:
        s_LevelString = "Warn";
        break;
      case LL_Error:
        s_LevelString = "Error";
        break;
      case LL_Debug:
        s_LevelString = "Debug";
        break;
      case LL_None:
      default:
        s_LevelString = "None";
        break;
      }
      m_LogStream << "[" << s_LevelString << "] " << p_File << ":" << p_Line << ": ";
    }
  }

  template <class T>
  FileLog &operator<<(const T &v) {
    m_LogStream << v;
    return *this;
  }

  FileLog &operator<<(std::ostream&(*f)(std::ostream&)) {
    m_LogStream << f;
    return *this;
  }

  ~FileLog() {
    if (m_Format) {
      if (m_Skip) {
        return;
      }
      m_LogStream << std::endl;
    }

    fprintf(g_LogFilePointer, "%s", m_LogStream.str().c_str());
    m_LogStream.str("");
  }

private:
  bool m_Skip = false;
  bool m_Format = false;
  std::stringstream m_LogStream;
};

#define PRINTLOG PrintLog(__FILE__, __LINE__, true)
#define KERNELLOG KernelLog(__FILE__, __LINE__, true)
#define SOCKETLOG SocketLog(__FILE__, __LINE__, true)
#define FILELOG FileLog(__FILE__, __LINE__, true)

#define PRINTLOG_UNFORMATTED PrintLog(__FILE__, __LINE__, false)
#define KERNELLOG_UNFORMATTED KernelLog(__FILE__, __LINE__, false)
#define SOCKETLOG_UNFORMATTED SocketLog(__FILE__, __LINE__, false)
#define FILELOG_UNFORMATTED FileLog(__FILE__, __LINE__, false)

#endif

#endif
