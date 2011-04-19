/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_FILE_H__
#define __HPHP_FILE_H__

#include <runtime/base/types.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/request_local.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class FileData : public RequestEventHandler {
public:
  FileData() : m_pcloseRet(0) {}
  void clear() { m_pcloseRet = 0; }
  virtual void requestInit() {
    clear();
  }
  virtual void requestShutdown() {
    clear();
  }
  int m_pcloseRet;
};

DECLARE_EXTERN_REQUEST_LOCAL(FileData, s_file_data);

/**
 * This is PHP's "stream", base class of plain file, gzipped file, directory
 * and sockets. We are not going to structure directories this way at all,
 * but we will have PlainFile, ZipFile and Socket derive from this base class,
 * so they can share some minimal functionalities.
 */
class File : public SweepableResourceData {
public:
  static String TranslatePath(CStrRef filename, bool useFileCache = false,
                              bool keepRelative = false);
  static String TranslateCommand(CStrRef cmd);
  static Variant Open(CStrRef filename, CStrRef mode,
                      CArrRef options = null_array);

  static bool IsVirtualDirectory(CStrRef filename);
  static bool IsPlainFilePath(CStrRef filename);

public:
  File(bool nonblocking = true);
  virtual ~File();

  static StaticString s_class_name;
  static StaticString s_resource_name;

  // overriding ResourceData
  CStrRef o_getClassName() const { return s_class_name; }
  CStrRef o_getResourceName() const { return s_resource_name; }
  int o_getResourceId() const {
    // This is different from Zend where each resource is assigned a unique id.
    return o_id;
  }
  virtual bool isResource() const { return !m_closed;}

  int fd() const { return m_fd;}
  bool valid() const { return m_fd >= 0;}
  const std::string getName() const { return m_name;}

  /**
   * How to open this type of file.
   */
  virtual bool open(CStrRef filename, CStrRef mode) = 0;

  /**
   * How to close this type of file.
   */
  virtual bool close() = 0;
  virtual bool isClosed() const { return m_closed;}

  /**
   * Read one chunk of input. Returns a null string on failure or eof.
   */
  virtual int64 readImpl(char *buffer, int64 length) = 0;
  virtual int getc();
  virtual String read(int64 length = 0);

  /**
   * Write one chunk of output. Returns bytes written.
   */
  virtual int64 writeImpl(const char *buffer, int64 length) = 0;
  virtual int64 write(CStrRef str, int64 length = 0);
  int putc(char c);

  /**
   * Optional virtual functions to implement.
   */
  virtual bool seekable() { return false;}
  virtual bool seek(int64 offset, int whence = SEEK_SET);
  virtual int64 tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int64 size);
  virtual bool lock(int operation);
  virtual bool lock(int operation, bool &wouldblock);

  virtual Array getMetaData();
  virtual const char *getStreamType() const { return "";}

  /**
   * Read one line a time. Returns a null string on failure or eof.
   */
  String readLine(int64 maxlen = 0);

  /**
   * Read one record a time. Returns a null string on failure or eof.
   */
  String readRecord(CStrRef delimiter, int64 maxlen = 0);

  /**
   * Read entire file and print it out.
   */
  int64 print();

  /**
   * Write to file with specified format and arguments.
   */
  int64 printf(CStrRef format, CArrRef args);

  /**
   * Write one line of csv record.
   */
  int64 writeCSV(CArrRef fields, char delimiter = ',', char enclosure = '"');

  /**
   * Read one line of csv record.
   */
  Array readCSV(int64 length = 0, char delimiter = ',', char enclosure = '"');

  /**
   * Return the last error we know about
   */
  String getLastError();

protected:
  int m_fd;      // file descriptor
  bool m_closed; // whether close() was called
  bool m_nonblocking;

  // fields only useful for buffered reads
  int64 m_writepos; // where we have read from lower level
  int64 m_readpos;  // where we have given to upper level

  // fields useful for both reads and writes
  int64 m_position; // the current cursor position

  std::string m_name;
  std::string m_mode;

  static Object OpenImpl(CStrRef filename, CStrRef mode, CArrRef options);
  void closeImpl();

private:
  static const int CHUNK_SIZE = 8192;
  char *m_buffer;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FILE_H__
