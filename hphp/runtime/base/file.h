/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_FILE_H_
#define incl_HPHP_FILE_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/resource_data.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/request_local.h"

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
  static String TranslatePath(CStrRef filename);
  // Same as TranslatePath except doesn't make paths absolute
  static String TranslatePathKeepRelative(CStrRef filename);
  // Same as TranslatePath except checks the file cache on miss
  static String TranslatePathWithFileCache(CStrRef filename);
  static String TranslateCommand(CStrRef cmd);
  static Variant Open(CStrRef filename, CStrRef mode,
                      int options = 0, CVarRef context = uninit_null());

  static bool IsVirtualDirectory(CStrRef filename);
  static bool IsPlainFilePath(CStrRef filename);

public:
  static const int USE_INCLUDE_PATH;

  explicit File(bool nonblocking = true);
  virtual ~File();

  static StaticString s_class_name;
  static StaticString s_resource_name;

  // overriding ResourceData
  CStrRef o_getClassNameHook() const { return s_class_name; }
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
  virtual int64_t readImpl(char *buffer, int64_t length) = 0;
  virtual int getc();
  virtual String read(int64_t length = 0);

  /**
   * Write one chunk of output. Returns bytes written.
   */
  virtual int64_t writeImpl(const char *buffer, int64_t length) = 0;
  virtual int64_t write(CStrRef str, int64_t length = 0);
  int putc(char c);

  /**
   * Optional virtual functions to implement.
   */
  virtual bool seekable() { return false;}
  virtual bool seek(int64_t offset, int whence = SEEK_SET);
  virtual int64_t tell();
  virtual bool eof();
  virtual bool rewind();
  virtual bool flush();
  virtual bool truncate(int64_t size);
  virtual bool lock(int operation);
  virtual bool lock(int operation, bool &wouldblock);

  virtual Array getMetaData();
  virtual Array getWrapperMetaData() { return null_array; }
  virtual const char *getStreamType() const { return "";}

  std::string getMode() { return m_mode; }

  /**
   * Read one line a time. Returns a null string on failure or eof.
   */
  String readLine(int64_t maxlen = 0);

  /**
   * Read one record a time. Returns a null string on failure or eof.
   */
  String readRecord(CStrRef delimiter, int64_t maxlen = 0);

  /**
   * Read entire file and print it out.
   */
  int64_t print();

  /**
   * Write to file with specified format and arguments.
   */
  int64_t printf(CStrRef format, CArrRef args);

  /**
   * Write one line of csv record.
   */
  int64_t writeCSV(CArrRef fields, char delimiter = ',', char enclosure = '"');

  /**
   * Read one line of csv record.
   */
  Array readCSV(int64_t length = 0, char delimiter = ',', char enclosure = '"',
                char escape = '\\');

  /**
   * Return the last error we know about
   */
  String getLastError();

protected:
  int m_fd;      // file descriptor
  bool m_closed; // whether close() was called
  bool m_nonblocking;

  // fields only useful for buffered reads
  int64_t m_writepos; // where we have read from lower level
  int64_t m_readpos;  // where we have given to upper level

  // fields useful for both reads and writes
  int64_t m_position; // the current cursor position

  std::string m_name;
  std::string m_mode;

  void closeImpl();

private:
  static const int CHUNK_SIZE = 8192;
  char *m_buffer;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FILE_H_
