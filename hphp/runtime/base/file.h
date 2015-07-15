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
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

struct stat;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StreamContext;
class StreamFilter;

extern int __thread s_pcloseRet;

// This structure holds the request allocated data members of File.  The
// purpose of the class is to allow File (and subclasses) to be managed by
// the request heap while also allowing their underlying OS handles to be
// persisted beyond the lifetime of a request.
//
// The FileData is stored in a shared_ptr and managed by new/delete, so it is
// safe to store in an object whose lifetime is longer than a request.
// A File (or subclass) can be reconstructed using a shared_ptr to a FileData.
// Note that subclasses of File that need to be persisted must subclass
// FileData to add any persistent data members, e.g. see Socket.
// Classes in the FileData hierarchy may not contain request-allocated data.
struct FileData {
  static const int CHUNK_SIZE;

  FileData() { }
  explicit FileData(bool nonblocking);
  virtual bool closeImpl();
  virtual ~FileData();

 protected:
  bool valid() const { return m_fd >= 0;}
  bool isClosed() const { return m_closed; }
  void setIsClosed(bool closed) { m_closed = closed; }
  void setFd(int fd) { m_fd = fd; }
  int getFd() { return m_fd; }

 private:
  friend class File;
  int m_fd{-1};      // file descriptor
  bool m_isLocal{false}; // is this on the local disk?
  bool m_closed{false}; // whether close() was called
  const bool m_nonblocking{true};

  // fields useful for both reads and writes
  bool m_eof{false};
  int64_t m_position{0}; // the current cursor position

  // fields only useful for buffered reads
  int64_t m_writepos{0}; // where we have read from lower level
  int64_t m_readpos{0};  // where we have given to upper level

  std::string m_name;
  std::string m_mode;

  char *m_buffer{nullptr};
  int64_t m_bufferSize{CHUNK_SIZE};
};

/**
 * This is PHP's "stream", base class of plain file, gzipped file, directory
 * and sockets. We are not going to structure directories this way at all,
 * but we will have PlainFile, ZipFile and Socket derive from this base class,
 * so they can share some minimal functionalities.
 */
struct File : SweepableResourceData {
  static const int CHUNK_SIZE;

  static String TranslatePath(const String& filename);
  // Same as TranslatePath except doesn't make paths absolute
  static String TranslatePathKeepRelative(const String& filename);
  // Same as TranslatePath except checks the file cache on miss
  static String TranslatePathWithFileCache(const String& filename);
  static String TranslateCommand(const String& cmd);
  static req::ptr<File> Open(
    const String& filename, const String& mode,
    int options = 0, const req::ptr<StreamContext>& context = nullptr);

  static bool IsVirtualDirectory(const String& filename);
  static bool IsVirtualFile(const String& filename);
  static bool IsPlainFilePath(const String& filename) {
    return filename.find("://") == String::npos;
  }

  static const int USE_INCLUDE_PATH;

  explicit File(bool nonblocking = true,
                const String& wrapper_type = null_string,
                const String& stream_type = empty_string_ref);
  virtual ~File();

  static StaticString& classnameof() {
    static StaticString result("File");
    return result;
  }
  static StaticString s_resource_name;

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }
  const String& o_getResourceName() const override;
  bool isInvalid() const override { return m_data->m_closed; }

  virtual int fd() const { return m_data->m_fd;}
  bool valid() const { return m_data && m_data->m_fd >= 0; }
  std::string getName() const { return m_data->m_name;}

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
  virtual bool isClosed() const { return !m_data || m_data->m_closed; }

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

  virtual Object await(uint16_t events, double timeout);

  virtual Array getMetaData();
  virtual Variant getWrapperMetaData() { return Variant(); }
  String getWrapperType() const;
  String getStreamType() const { return m_streamType; }
  const req::ptr<StreamContext>& getStreamContext() { return m_streamContext; }
  void setStreamContext(const req::ptr<StreamContext>& context) {
    m_streamContext = context;
  }
  void appendReadFilter(const req::ptr<StreamFilter>& filter);
  void appendWriteFilter(const req::ptr<StreamFilter>& filter);
  void prependReadFilter(const req::ptr<StreamFilter>& filter);
  void prependWriteFilter(const req::ptr<StreamFilter>& filter);
  bool removeFilter(const req::ptr<StreamFilter>& filter);

  int64_t bufferedLen() { return m_data->m_writepos - m_data->m_readpos; }

  std::string getMode() { return m_data->m_mode; }

  /**
   * Read one line a time. Returns a null string on failure or eof.
   */
  String readLine(int64_t maxlen = 0);

  /**
   * Read one record a time. Returns a false on failure or eof.
   */
  Variant readRecord(const String& delimiter, int64_t maxlen = 0);

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
  int64_t writeCSV(const Array& fields, char delimiter = ',',
                   char enclosure = '"');

  /**
   * Read one line of csv record.
   */
  Array readCSV(int64_t length = 0, char delimiter = ',', char enclosure = '"',
                char escape = '\\', const String* initial = nullptr);

  /**
   * Return the last error we know about
   */
  String getLastError();

  bool isLocal() const { return m_data->m_isLocal; }

  std::shared_ptr<FileData> getData() const { return m_data; }

protected:
  void invokeFiltersOnClose();
  bool closeImpl();
  virtual void sweep() override;

  void setIsLocal(bool isLocal) { m_data->m_isLocal = isLocal; }
  void setIsClosed(bool closed) { m_data->m_closed = closed; }

  bool getEof() const { return m_data->m_eof; }
  void setEof(bool eof) { m_data->m_eof = eof; }

  int64_t getPosition() const { return m_data->m_position; }
  void setPosition(int64_t pos) { m_data->m_position = pos; }

  int64_t getWritePosition() const { return m_data->m_writepos; }
  void setWritePosition(int64_t wpos) { m_data->m_writepos = wpos; }

  int64_t getReadPosition() const { return m_data->m_readpos; }
  void setReadPosition(int64_t rpos) { m_data->m_readpos = rpos; }

  int getFd() const { return m_data->m_fd; }
  void setFd(int fd) { m_data->m_fd = fd; }

  void setName(std::string name) { m_data->m_name = name; }

  void setStreamType(const StaticString& streamType) {
    m_streamType = streamType.get();
  }

  /**
   * call readImpl(m_buffer, CHUNK_SIZE), passing through stream filters if any.
   */
  int64_t filteredReadToBuffer();

  /**
   * call writeImpl, passing through stream filters if any.
   */
  int64_t filteredWrite(const char* buffer, int64_t length);

  FileData* getFileData() { return m_data.get(); }
  const FileData* getFileData() const { return m_data.get(); }

protected:
  explicit File(std::shared_ptr<FileData> data,
                const String& wrapper_type = null_string,
                const String& stream_type = empty_string_ref);

private:
  template <typename F> friend void scan(const File& this_, F& mark);
  template<class ResourceList>
  String applyFilters(const String& buffer,
                      ResourceList& filters,
                      bool closing);

  std::shared_ptr<FileData> m_data;
  StringData* m_wrapperType;
  StringData* m_streamType;
  req::ptr<StreamContext> m_streamContext;
  req::list<req::ptr<StreamFilter>> m_readFilters;
  req::list<req::ptr<StreamFilter>> m_writeFilters;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FILE_H_
