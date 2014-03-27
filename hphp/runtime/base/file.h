/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_FILE_H_
#define incl_HPHP_FILE_H_

#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

struct stat;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StreamContext;

class FileData final : public RequestEventHandler {
public:
  FileData() : m_pcloseRet(0) {}
  void clear() { m_pcloseRet = 0; }
  void requestInit() override { clear(); }
  void requestShutdown() override { clear(); }
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
  static String TranslatePath(const String& filename);
  // Same as TranslatePath except doesn't make paths absolute
  static String TranslatePathKeepRelative(const String& filename);
  // Same as TranslatePath except checks the file cache on miss
  static String TranslatePathWithFileCache(const String& filename);
  static String TranslateCommand(const String& cmd);
  static Variant Open(const String& filename, const String& mode,
                      int options = 0, const Variant& context = uninit_null());

  static bool IsVirtualDirectory(const String& filename);
  static bool IsPlainFilePath(const String& filename);

public:
  static const int USE_INCLUDE_PATH;

  explicit File(bool nonblocking = true,
                const String& wrapper_type = null_string,
                const String& stream_type = empty_string);
  virtual ~File();

  static StaticString& classnameof() {
    static StaticString result("File");
    return result;
  }
  static StaticString s_resource_name;

  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }
  const String& o_getResourceName() const { return s_resource_name; }
  virtual bool isInvalid() const { return m_closed; }

  int fd() const { return m_fd;}
  bool valid() const { return m_fd >= 0;}
  const std::string getName() const { return m_name;}

  /**
   * How to open this type of file.
   */
  virtual bool open(const String& filename, const String& mode) = 0;

  /**
   * How to close this type of file.
   *
   * Your implementaitn should call invokeFiltersOnClose() before anything else
   * to make sure that any user-provided php_user_filter instances get to flush
   * and clean up.
   */
  virtual bool close() = 0;
  virtual bool isClosed() const { return m_closed;}

  /* Use:
   * - read() when fetching data to return to PHP
   * - readImpl() when you want raw unbuffered data; for example, if you use
   *   the Socket class to implement a network-based extension, use readImpl
   *   to avoid the internal buffer, stream filters, and so on
   * - filteredRead() (wrapper around readImpl()) to call user-supplied stream
   *   filters if you reimplement read()
   *
   * Stream filters are only supported for read() - the fgetc() and seek()
   * behavior in Zend is undocumented, surprising, and not supported
   * in HHVM.
   */

  /**
   * Read one chunk of input. Returns a null string on failure or eof.
   */
  virtual int64_t readImpl(char *buffer, int64_t length) = 0;
  virtual int getc();
  virtual String read(int64_t length);
  virtual String read();

  /* Use:
   * - write() in response to a PHP code that is documented as writing to a
   *   stream
   * - writeImpl() if you want C-like behavior, instead of PHP-like behavior;
   *   for example, if you write a network-based extension using Socket
   * - filteredWrite() if you re-implement write() to provide support for PHP
   *   user filters
   *
   * Stream filters are only supported for write() - the fputc() and seek()
   * behavior in Zend is undocumented, surprising, and not supported
   * in HHVM.
   */

  /**
   * Write one chunk of output. Returns bytes written.
   */
  virtual int64_t writeImpl(const char *buffer, int64_t length) = 0;
  virtual int64_t write(const String& str, int64_t length = 0);
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
  virtual bool stat(struct stat *sb);

  virtual Array getMetaData();
  virtual Array getWrapperMetaData() { return null_array; }
  String getWrapperType() const;
  String getStreamType() const { return m_streamType; }
  Resource &getStreamContext() { return m_streamContext; }
  void setStreamContext(Resource &context) { m_streamContext = context; }
  void appendReadFilter(Resource &filter);
  void appendWriteFilter(Resource &filter);
  void prependReadFilter(Resource &filter);
  void prependWriteFilter(Resource &filter);
  bool removeFilter(Resource &filter);

  int64_t bufferedLen() { return m_writepos - m_readpos; }

  std::string getMode() { return m_mode; }

  /**
   * Read one line a time. Returns a null string on failure or eof.
   */
  String readLine(int64_t maxlen = 0);

  /**
   * Read one record a time. Returns a null string on failure or eof.
   */
  String readRecord(const String& delimiter, int64_t maxlen = 0);

  /**
   * Read entire file and print it out.
   */
  int64_t print();

  /**
   * Write to file with specified format and arguments.
   */
  int64_t printf(const String& format, const Array& args);

  /**
   * Write one line of csv record.
   */
  int64_t writeCSV(const Array& fields, char delimiter = ',', char enclosure = '"');

  /**
   * Read one line of csv record.
   */
  Array readCSV(int64_t length = 0, char delimiter = ',', char enclosure = '"',
                char escape = '\\', const String* initial = nullptr);

  /**
   * Return the last error we know about
   */
  String getLastError();

  /**
   * Is this on the local disk?
   */
  bool m_isLocal;

protected:
  int m_fd;      // file descriptor
  bool m_closed; // whether close() was called
  bool m_nonblocking;

  // fields only useful for buffered reads
  int64_t m_writepos; // where we have read from lower level
  int64_t m_readpos;  // where we have given to upper level

  // fields useful for both reads and writes
  int64_t m_position; // the current cursor position
  bool m_eof;

  std::string m_name;
  std::string m_mode;

  StringData* m_wrapperType;
  StringData* m_streamType;
  Resource m_streamContext;
  smart::list<Resource> m_readFilters;
  smart::list<Resource> m_writeFilters;

  void invokeFiltersOnClose();
  void closeImpl();
  virtual void sweep() FOLLY_OVERRIDE;

  /**
   * call readImpl(m_buffer, CHUNK_SIZE), passing through stream filters if any.
   */
  int64_t filteredReadToBuffer();

  /**
   * call writeImpl, passing through stream filters if any.
   */
  int64_t filteredWrite(const char* buffer, int64_t length);
private:
  static const int CHUNK_SIZE = 8192;
  char *m_buffer;
  int64_t m_bufferSize;

  String applyFilters(const String& buffer,
                      smart::list<Resource>& filters,
                      bool closing);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FILE_H_
