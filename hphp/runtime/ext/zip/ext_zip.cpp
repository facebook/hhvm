/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include <zip.h>

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_preg.h"

namespace HPHP {

class ZipStream : public File {
 public:
  DECLARE_RESOURCE_ALLOCATION(ZipStream);

  ZipStream(zip* z, const String& name) : m_zipFile(nullptr) {
    if (name.empty()) {
      return;
    }

    struct zip_stat zipStat;
    if (zip_stat(z, name.c_str(), 0, &zipStat) != 0) {
      return;
    }

    m_zipFile = zip_fopen(z, name.c_str(), 0);
  }


  virtual ~ZipStream() { close(); }

  virtual bool open(const String&, const String&) { return false; }

  virtual bool close() {
    bool noError = true;
    if (!eof()) {
      if (zip_fclose(m_zipFile) != 0) {
        noError = false;
      }
      m_zipFile = nullptr;
    }
    return noError;
  }

  virtual int64_t readImpl(char *buffer, int64_t length) {
    auto n = zip_fread(m_zipFile, buffer, length);
    if (n <= 0) {
      if (n == -1) {
        raise_warning("Zip stream error");
        n = 0;
      }
      close();
    }
    return n;
  }

  virtual int64_t writeImpl(const char *buffer, int64_t length) { return 0; }

  virtual bool eof() { return m_zipFile == nullptr; }

 private:
  zip_file* m_zipFile;
};

void ZipStream::sweep() {
  close();
  File::sweep();
}

class ZipStreamWrapper : public Stream::Wrapper {
 public:
  virtual File* open(const String& filename, const String& mode,
                     int options, CVarRef context) {
    std::string url(filename.c_str());
    auto pound = url.find('#');
    if (pound == std::string::npos) {
      return nullptr;
    }

    // 6 is the position after zip://
    auto path = url.substr(6, pound - 6);
    auto file = url.substr(pound + 1);

    if (path.empty() || file.empty()) {
      return nullptr;
    }

    int err;
    auto z = zip_open(path.c_str(), 0, &err);
    if (z == nullptr) {
      return nullptr;
    }

    return NEWOBJ(ZipStream)(z, file);
  }
};

class ZipEntry : public SweepableResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION(ZipEntry);

  CLASSNAME_IS("ZipEntry");
  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  ZipEntry(zip* z, int index) : m_zipFile(nullptr) {
    if (zip_stat_index(z, index, 0, &m_zipStat) == 0) {
      m_zipFile = zip_fopen_index(z, index, 0);
    }
  }

  ~ZipEntry() {
    close();
  }

  bool close() {
    bool noError = true;
    if (isValid()) {
      if (zip_fclose(m_zipFile) != 0) {
        noError = false;
      }
      m_zipFile = nullptr;
    }
    return noError;
  }

  bool isValid() {
    return m_zipFile != nullptr;
  }

  String read(int64_t len) {
    auto sb  = NEWOBJ(StringBuffer)(len);
    auto buf = sb->appendCursor(len);
    auto n   = zip_fread(m_zipFile, buf, len);
    if (n > 0) {
      sb->resize(n);
      return sb->detach();
    }
    return "";
  }

  uint64_t getCompressedSize() {
    return m_zipStat.comp_size;
  }

  String getCompressionMethod() {
    switch (m_zipStat.comp_method) {
      case 0:
        return "stored";
      case 1:
        return "shrunk";
      case 2:
      case 3:
      case 4:
      case 5:
        return "reduced";
      case 6:
        return "imploded";
      case 7:
        return "tokenized";
      case 8:
        return "deflated";
      case 9:
        return "deflatedX";
      case 10:
        return "implodedX";
      default:
        return false;
    }
  }

  String getName() {
    return m_zipStat.name;
  }

  uint64_t getSize() {
    return m_zipStat.size;
  }

 private:
  struct zip_stat m_zipStat;
  zip_file*       m_zipFile;
};
IMPLEMENT_OBJECT_ALLOCATION(ZipEntry);

class ZipDirectory: public SweepableResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION(ZipDirectory);

  CLASSNAME_IS("ZipDirectory");
  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  explicit ZipDirectory(zip *z) : m_zip(z),
                                  m_numFiles(zip_get_num_files(z)),
                                  m_curIndex(0) {}

  ~ZipDirectory() { close(); }

  bool close() {
    bool noError = true;
    if (isValid()) {
      if (zip_close(m_zip) != 0) {
        _zip_free(m_zip);
        noError = false;
      }
      m_zip = nullptr;
    }
    return noError;
  }

  bool isValid() const {
    return m_zip != nullptr;
  }

  Variant nextFile() {
    if (m_curIndex >= m_numFiles) {
      return false;
    }

    auto zipEntry = NEWOBJ(ZipEntry)(m_zip, m_curIndex);

    if (!zipEntry->isValid()) {
      return false;
    }

    ++m_curIndex;

    return Resource(zipEntry);
  }

  zip* getZip() {
    return m_zip;
  }

 private:
  zip* m_zip;
  int  m_numFiles;
  int  m_curIndex;
};
IMPLEMENT_OBJECT_ALLOCATION(ZipDirectory);

const StaticString s_ZipArchive("ZipArchive");
const StaticString s_CREATE("CREATE");
const int64_t k_CREATE = 1;
const StaticString s_EXCL("EXCL");
const int64_t k_EXCL = 2;
const StaticString s_CHECKCONS("CHECKCONS");
const int64_t k_CHECKCONS = 4;
const StaticString s_OVERWRITE("OVERWRITE");
const int64_t k_OVERWRITE = 8;
const StaticString s_FL_NOCASE("FL_NOCASE");
const int64_t k_FL_NOCASE = 1;
const StaticString s_FL_NODIR("FL_NODIR");
const int64_t k_FL_NODIR = 2;
const StaticString s_FL_COMPRESSED("FL_COMPRESSED");
const int64_t k_FL_COMPRESSED = 4;
const StaticString s_FL_UNCHANGED("FL_UNCHANGED");
const int64_t k_FL_UNCHANGED = 8;
const StaticString s_FL_RECOMPRESS("FL_RECOMPRESS");
const int64_t k_FL_RECOMPRESS = 16;
const StaticString s_FL_ENCRYPTED("FL_ENCRYPTED");
const int64_t k_FL_ENCRYPTED = 32;
const StaticString s_ER_OK("ER_OK");
const int64_t k_ER_OK = 0;
const StaticString s_ER_MULTIDISK("ER_MULTIDISK");
const int64_t k_ER_MULTIDISK = 1;
const StaticString s_ER_RENAME("ER_RENAME");
const int64_t k_ER_RENAME = 2;
const StaticString s_ER_CLOSE("ER_CLOSE");
const int64_t k_ER_CLOSE = 3;
const StaticString s_ER_SEEK("ER_SEEK");
const int64_t k_ER_SEEK = 4;
const StaticString s_ER_READ("ER_READ");
const int64_t k_ER_READ = 5;
const StaticString s_ER_WRITE("ER_WRITE");
const int64_t k_ER_WRITE = 6;
const StaticString s_ER_CRC("ER_CRC");
const int64_t k_ER_CRC = 7;
const StaticString s_ER_ZIPCLOSED("ER_ZIPCLOSED");
const int64_t k_ER_ZIPCLOSED = 8;
const StaticString s_ER_NOENT("ER_NOENT");
const int64_t k_ER_NOENT = 9;
const StaticString s_ER_EXISTS("ER_EXISTS");
const int64_t k_ER_EXISTS = 10;
const StaticString s_ER_OPEN("ER_OPEN");
const int64_t k_ER_OPEN = 11;
const StaticString s_ER_TMPOPEN("ER_TMPOPEN");
const int64_t k_ER_TMPOPEN = 12;
const StaticString s_ER_ZLIB("ER_ZLIB");
const int64_t k_ER_ZLIB = 13;
const StaticString s_ER_MEMORY("ER_MEMORY");
const int64_t k_ER_MEMORY = 14;
const StaticString s_ER_CHANGED("ER_CHANGED");
const int64_t k_ER_CHANGED = 15;
const StaticString s_ER_COMPNOTSUPP("ER_COMPNOTSUPP");
const int64_t k_ER_COMPNOTSUPP = 16;
const StaticString s_ER_EOF("ER_EOF");
const int64_t k_ER_EOF = 17;
const StaticString s_ER_INVAL("ER_INVAL");
const int64_t k_ER_INVAL = 18;
const StaticString s_ER_NOZIP("ER_NOZIP");
const int64_t k_ER_NOZIP = 19;
const StaticString s_ER_INTERNAL("ER_INTERNAL");
const int64_t k_ER_INTERNAL = 20;
const StaticString s_ER_INCONS("ER_INCONS");
const int64_t k_ER_INCONS = 21;
const StaticString s_ER_REMOVE("ER_REMOVE");
const int64_t k_ER_REMOVE = 22;
const StaticString s_ER_DELETED("ER_DELETED");
const int64_t k_ER_DELETED = 23;
const StaticString s_ER_ENCRNOTSUPP("ER_ENCRNOTSUPP");
const int64_t k_ER_ENCRNOTSUPP = 24;
const StaticString s_ER_RDONLY("ER_RDONLY");
const int64_t k_ER_RDONLY = 25;
const StaticString s_ER_NOPASSWD("ER_NOPASSWD");
const int64_t k_ER_NOPASSWD = 26;
const StaticString s_ER_WRONGPASSWD("ER_WRONGPASSWD");
const int64_t k_ER_WRONGPASSWD = 27;
const StaticString s_CM_DEFAULT("CM_DEFAULT");
const int64_t k_CM_DEFAULT = -1;
const StaticString s_CM_STORE("CM_STORE");
const int64_t k_CM_STORE = 0;
const StaticString s_CM_SHRINK("CM_SHRINK");
const int64_t k_CM_SHRINK = 1;
const StaticString s_CM_REDUCE_1("CM_REDUCE_1");
const int64_t k_CM_REDUCE_1 = 2;
const StaticString s_CM_REDUCE_2("CM_REDUCE_2");
const int64_t k_CM_REDUCE_2 = 3;
const StaticString s_CM_REDUCE_3("CM_REDUCE_3");
const int64_t k_CM_REDUCE_3 = 4;
const StaticString s_CM_REDUCE_4("CM_REDUCE_4");
const int64_t k_CM_REDUCE_4= 5;
const StaticString s_CM_IMPLODE("CM_IMPLODE");
const int64_t k_CM_IMPLODE = 6;
const StaticString s_CM_DEFLATE("CM_DEFLATE");
const int64_t k_CM_DEFLATE = 8;
const StaticString s_CM_DEFLATE64("CM_DEFLATE64");
const int64_t k_CM_DEFLATE64 = 9;
const StaticString s_CM_PKWARE_IMPLODE("CM_PKWARE_IMPLODE");
const int64_t k_CM_PKWARE_IMPLODE = 10;
const StaticString s_CM_BZIP2("CM_BZIP2");
const int64_t k_CM_BZIP2 = 12;
const StaticString s_CM_LZMA("CM_LZMA");
const int64_t k_CM_LZMA = 14;
const StaticString s_CM_TERSE("CM_TERSE");
const int64_t k_CM_TERSE = 18;
const StaticString s_CM_LZ77("CM_LZ77");
const int64_t k_CM_LZ77 = 19;
const StaticString s_CM_WAVPACK("CM_WAVPACK");
const int64_t k_CM_WAVPACK = 97;
const StaticString s_CM_PPMD("CM_PPMD");
const int64_t k_CM_PPMD = 98;

template<class T>
ALWAYS_INLINE
static T* getResource(CObjRef obj, const char* varName) {
  auto var = obj->o_get(varName, true, s_ZipArchive.get());
  if (var.getType() == KindOfNull) {
    return nullptr;
  }
  return var.asCResRef().getTyped<T>();
}

ALWAYS_INLINE
static Variant setVariable(CObjRef obj, const char* varName, CVarRef varValue) {
  return obj->o_set(varName, varValue, s_ZipArchive.get());
}

#define FAIL_IF_EMPTY_STRING(func, str)                     \
  if (str.empty()) {                                        \
    raise_warning(#func "(): Empty string as source");      \
    return false;                                           \
  }

#define FAIL_IF_EMPTY_STRING_ZIPARCHIVE(func, str)                      \
  if (str.empty()) {                                                    \
    raise_warning("ZipArchive::" #func "(): Empty string as source");   \
    return false;                                                       \
  }

#define FAIL_IF_INVALID_INDEX(index)            \
  if (index < 0) {                              \
    return false;                               \
  }

#define FAIL_IF_INVALID_PTR(ptr)                \
  if (ptr == nullptr) {                         \
    return false;                               \
  }

#define FAIL_IF_INVALID_ZIPARCHIVE(func, res)                       \
  if (res == nullptr || !res->isValid()) {                          \
    raise_warning("ZipArchive::" #func                              \
                  "(): Invalid or uninitialized Zip object");       \
    return false;                                                   \
  }

#define FAIL_IF_INVALID_ZIPDIRECTORY(func, res)                         \
  if (!res->isValid()) {                                                \
    raise_warning(#func "(): %d is not a valid "                        \
                  "Zip Directory resource", res->o_getId());            \
    return false;                                                       \
  }

#define FAIL_IF_INVALID_ZIPENTRY(func, res)                             \
  if (!res->isValid()) {                                                \
    raise_warning(#func "(): %d is not a valid Zip Entry resource",     \
                  res->o_getId());                                      \
    return false;                                                       \
  }

//////////////////////////////////////////////////////////////////////////////
// class ZipArchive

static Variant HHVM_METHOD(ZipArchive, getProperty, int64_t property) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  if (zipDir == nullptr) {
    switch (property) {
      case 0:
      case 1:
      case 2:
        return 0;
      case 3:
      case 4:
        return "";
      default:
        return null_variant;
    }
  }

  switch (property) {
    case 0:
    case 1:
    {
      int zep, sys;
      zip_error_get(zipDir->getZip(), &zep, &sys);
      if (property == 0) {
        return zep;
      }
      return sys;
    }
    case 2:
    {
      return zip_get_num_files(zipDir->getZip());
    }
    case 3:
    {
      return this_->o_get("filename", true, s_ZipArchive.get()).asCStrRef();
    }
    case 4:
    {
      int len;
      auto comment = zip_get_archive_comment(zipDir->getZip(), &len, 0);
      if (comment == nullptr) {
        return "";
      }
      return String(comment, len, CopyString);
    }
    default:
      return null_variant;
  }
}

static bool HHVM_METHOD(ZipArchive, addEmptyDir, const String& dirname) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addEmptyDir, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addEmptyDir, dirname);

  std::string dirStr(dirname.c_str());
  if (dirStr[dirStr.length() - 1] != '/') {
    dirStr.push_back('/');
  }

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), dirStr.c_str(), 0, &zipStat) != -1) {
    return false;
  }

  if (zip_add_dir(zipDir->getZip(), dirStr.c_str()) == -1) {
    return false;
  }

  return true;
}

static bool addFile(zip* zipStruct, const char* source, const char* dest,
                    int64_t start = 0, int64_t length = 0) {
  if (!f_is_file(source)) {
    return false;
  }

  auto zipSource = zip_source_file(zipStruct, source, start, length);
  FAIL_IF_INVALID_PTR(zipSource);

  auto index = zip_name_locate(zipStruct, dest, 0);
  if (index < 0) {
    if (zip_add(zipStruct, dest, zipSource) == -1) {
      zip_source_free(zipSource);
      return false;
    }
  } else {
    if (zip_replace(zipStruct, index, zipSource) == -1) {
      zip_source_free(zipSource);
      return false;
    }
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, addFile, const String& filename,
                        const String& localname, int64_t start,
                        int64_t length) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addFile, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addFile, filename);

  return addFile(zipDir->getZip(), filename.c_str(),
                 localname.empty() ? filename.c_str() : localname.c_str(),
                 start, length);
}

static bool HHVM_METHOD(ZipArchive, addFromString, const String& localname,
                        const String& contents) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addFromString, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addFromString, localname);

  auto data = malloc(contents.length());
  FAIL_IF_INVALID_PTR(data);

  memcpy(data, contents.c_str(), contents.length());

  auto zipSource = zip_source_buffer(zipDir->getZip(), data, contents.length(),
                                     1); // this will free data ptr
  if (zipSource == nullptr) {
    free(data);
    return false;
  }

  auto index = zip_name_locate(zipDir->getZip(), localname.c_str(), 0);
  if (index < 0) {
    if (zip_add(zipDir->getZip(), localname.c_str(), zipSource) == -1) {
      zip_source_free(zipSource);
      return false;
    }
  } else {
    if (zip_replace(zipDir->getZip(), index, zipSource) == -1) {
      zip_source_free(zipSource);
      return false;
    }
  }

  return true;
}

static bool addPattern(zip* zipStruct, const String& pattern, CArrRef options,
                       std::string path, int64_t flags, bool glob) {
  std::string removePath;
  if (options->exists(String("remove_path"))) {
    auto var = options->get(String("remove_path"));
    if (var.isString()) {
      removePath.append(var.asCStrRef().c_str());
    }
  }

  bool removeAllPath = false;
  if (options->exists(String("remove_all_path"))) {
    auto var = options->get(String("remove_all_path"));
    if (var.isBoolean()) {
      removeAllPath = var.asBooleanVal();
    }
  }

  std::string addPath;
  if (options->exists(String("add_path"))) {
    auto var = options->get(String("add_path"));
    if (var.isString()) {
      addPath.append(var.asCStrRef().c_str());
    }
  }

  Array files;
  if (glob) {
    auto match = f_glob(pattern, flags);
    if (match.isArray()) {
      files = match.toArrRef();
    } else {
      return false;
    }
  } else {
    if (path[path.size() - 1] != '/') {
      path.push_back('/');
    }
    auto allFiles = f_scandir(path);
    if (allFiles.isArray()) {
      files = allFiles.toArrRef();
    } else {
      return false;
    }
  }

  std::string dest;
  auto pathLen = path.size();

  for (ArrayIter it(files); it; ++it) {
    auto var = it.second();
    if (!var.isString()) {
      return false;
    }

    auto source = var.asCStrRef();
    if (f_is_dir(source)) {
      continue;
    }

    if (!glob) {
      auto var = f_preg_match(pattern, source);
      if (var.isInteger()) {
        if (var.asInt64Val() == 0) {
          continue;
        }
      } else {
        return false;
      }
    }

    dest.resize(0);
    dest.append(source.c_str());

    if (removeAllPath) {
      auto index = dest.rfind('/');
      if (index != std::string::npos) {
        dest.erase(0, index + 1);
      }
    } else if (!removePath.empty()) {
      auto index = dest.find(removePath);
      if (index == 0) {
        dest.erase(0, removePath.size());
      }
    }

    if (!addPath.empty()) {
      dest.insert(0, addPath);
    }

    path.resize(pathLen);
    path.append(source.c_str());

    if (!addFile(zipStruct, path.c_str(), dest.c_str())) {
      return false;
    }
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, addGlob, const String& pattern,
                        int64_t flags, CArrRef options) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addGlob, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addGlob, pattern);

  return addPattern(zipDir->getZip(), pattern, options, "", flags, true);
}

static bool HHVM_METHOD(ZipArchive, addPattern, const String& pattern,
                        const String& path, CArrRef options) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addPattern, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addPattern, pattern);

  return addPattern(zipDir->getZip(), pattern, options, path.c_str(), 0, false);
}

static bool HHVM_METHOD(ZipArchive, close) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(close, zipDir);

  bool ret = zipDir->close();

  setVariable(this_, "zipDir", null_resource);

  return ret;
}

static bool HHVM_METHOD(ZipArchive, deleteIndex, int64_t index) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(deleteIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  if (zip_delete(zipDir->getZip(), index) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, deleteName, const String& name) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(deleteName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(deleteName, name);

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), 0, &zipStat) != 0) {
    return false;
  }

  if (zip_delete(zipDir->getZip(), zipStat.index) != 0) {
    return false;
  }

  return true;
}

static bool extractFileTo(zip* zip, const char* file, std::string& to,
                          char* buf, size_t len) {
  to.append(file);
  if (to[to.size() - 1] == '/') {
    return f_is_dir(to) || f_mkdir(to);
  }

  struct zip_stat zipStat;
  if (zip_stat(zip, file, 0, &zipStat) != 0) {
    return false;
  }

  auto zipFile = zip_fopen_index(zip, zipStat.index, 0);
  FAIL_IF_INVALID_PTR(zipFile);

  auto outFile = fopen(to.c_str(), "wb");
  if (outFile == nullptr) {
    zip_fclose(zipFile);
    return false;
  }

  for (auto n = zip_fread(zipFile, buf, len); n != 0;
       n = zip_fread(zipFile, buf, len)) {
    if (n < 0 || fwrite(buf, sizeof(char), n, outFile) != n) {
      zip_fclose(zipFile);
      fclose(outFile);
      remove(to.c_str());
      return false;
    }
  }

  zip_fclose(zipFile);
  if (fclose(outFile) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, extractTo, const String& destination,
                        CVarRef entries) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(extractTo, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(extractTo, destination);

  auto fileCount = zip_get_num_files(zipDir->getZip());
  if (fileCount == -1) {
    raise_warning("Illegal archive");
    return false;
  }

  std::string to(destination.c_str());
  if (to[to.size() - 1] != '/') {
    to.push_back('/');
  }

  if (!f_is_dir(to) && !f_mkdir(to)) {
    return false;
  }

  char buf[1024];
  auto toSize = to.size();

  if (entries.isString()) {
    // extract only this file
    if (!extractFileTo(zipDir->getZip(), entries.asCStrRef().c_str(),
                       to, buf, sizeof(buf))) {
      return false;
    }
  } else if (entries.isArray() && entries.asCArrRef().size() != 0) {
    // extract ones in the array
    for (ArrayIter it(entries.asCArrRef()); it; ++it) {
      auto var = it.second();
      if (!var.isString() || !extractFileTo(zipDir->getZip(),
                                            var.asCStrRef().c_str(),
                                            to, buf, sizeof(buf))) {
        return false;
      }
      to.resize(toSize);
    }
  } else {
    // extract all files
    for (decltype(fileCount) index = 0; index < fileCount; ++index) {
      auto file = zip_get_name(zipDir->getZip(), index, ZIP_FL_UNCHANGED);
      if (!extractFileTo(zipDir->getZip(), file, to, buf, sizeof(buf))) {
        return false;
      }
      to.resize(toSize);
    }
  }

  return true;
}

static Variant HHVM_METHOD(ZipArchive, getArchiveComment, int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getArchiveComment, zipDir);

  int len;
  auto comment = zip_get_archive_comment(zipDir->getZip(), &len, flags);
  FAIL_IF_INVALID_PTR(comment);

  return String(comment, len, CopyString);
}

static Variant HHVM_METHOD(ZipArchive, getCommentIndex, int64_t index,
                          int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getCommentIndex, zipDir);

  struct zip_stat zipStat;
  if (zip_stat_index(zipDir->getZip(), index, 0, &zipStat) != 0) {
    return false;
  }

  int len;
  auto comment = zip_get_file_comment(zipDir->getZip(), index, &len, flags);
  FAIL_IF_INVALID_PTR(comment);

  return String(comment, len, CopyString);
}

static Variant HHVM_METHOD(ZipArchive, getCommentName, const String& name,
                          int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getCommentName, zipDir);
  if (name.empty()) {
    raise_notice("ZipArchive::getCommentName(): Empty string as source");
    return false;
  }

  int index = zip_name_locate(zipDir->getZip(), name.c_str(), 0);
  if (index != 0) {
    return false;
  }

  int len;
  auto comment = zip_get_file_comment(zipDir->getZip(), index, &len, flags);
  FAIL_IF_INVALID_PTR(comment);

  return String(comment, len, CopyString);
}

static Variant HHVM_METHOD(ZipArchive, getFromIndex, int64_t index,
                           int64_t length, int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getFromIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  if (length < 0) {
    return "";
  }

  struct zip_stat zipStat;
  if (zip_stat_index(zipDir->getZip(), index, 0, &zipStat) != 0) {
    return false;
  }

  if (zipStat.size < 1) {
    return "";
  }

  auto zipFile = zip_fopen_index(zipDir->getZip(), index, flags);
  FAIL_IF_INVALID_PTR(zipFile);

  if (length == 0)  {
    length = zipStat.size;
  }

  auto sb  = NEWOBJ(StringBuffer)(length);
  auto buf = sb->appendCursor(length);
  auto n   = zip_fread(zipFile, buf, length);
  if (n > 0) {
    sb->resize(n);
    return sb->detach();
  }
  return "";
}

static Variant HHVM_METHOD(ZipArchive, getFromName, const String& name,
                           int64_t length, int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getFromName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(getFromName, name);

  if (length < 0) {
    return "";
  }

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), flags, &zipStat) != 0) {
    return false;
  }

  if (zipStat.size < 1) {
    return "";
  }

  auto zipFile = zip_fopen(zipDir->getZip(), name.c_str(), flags);
  FAIL_IF_INVALID_PTR(zipFile);

  if (length == 0)  {
    length = zipStat.size;
  }

  auto sb  = NEWOBJ(StringBuffer)(length);
  auto buf = sb->appendCursor(length);
  auto n   = zip_fread(zipFile, buf, length);
  if (n > 0) {
    sb->resize(n);
    return sb->detach();
  }
  return "";
}

static Variant HHVM_METHOD(ZipArchive, getNameIndex, int64_t index,
                          int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getNameIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  auto name = zip_get_name(zipDir->getZip(), index, flags);
  FAIL_IF_INVALID_PTR(name);

  return String(name, CopyString);
}

static Variant HHVM_METHOD(ZipArchive, getStatusString) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getStatusString, zipDir);

  int zep, sep, len;
  zip_error_get(zipDir->getZip(), &zep, &sep);

  char error_string[128];
  len = zip_error_to_str(error_string, 128, zep, sep);

  return String(error_string, len, CopyString);
}

static Variant HHVM_METHOD(ZipArchive, getStream, const String& name) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getStream, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(getStream, name);

  auto zipStream    = NEWOBJ(ZipStream)(zipDir->getZip(), name);
  auto zipStreamRes = Resource(zipStream);
  if (zipStream->eof()) {
    return false;
  }

  return zipStreamRes;
}

static Variant HHVM_METHOD(ZipArchive, locateName, const String& name,
                           int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(locateName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(locateName, name);

  auto index = zip_name_locate(zipDir->getZip(), name.c_str(), flags);
  FAIL_IF_INVALID_INDEX(index);

  return index;
}

static Variant HHVM_METHOD(ZipArchive, open, const String& filename,
                           int64_t flags) {
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(open, filename);

  int  err;
  auto z = zip_open(filename.c_str(), flags, &err);
  if (z == nullptr) {
    return err;
  }

  auto zipDir = NEWOBJ(ZipDirectory)(z);

  setVariable(this_, "zipDir", Resource(zipDir));
  setVariable(this_, "filename", filename);

  return true;
}

static bool HHVM_METHOD(ZipArchive, renameIndex, int64_t index,
                        const String& newname) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(renameIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(renameIndex, newname);

  if (zip_rename(zipDir->getZip(), index, newname.c_str()) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, renameName, const String& name,
                        const String& newname) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(renameName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(renameName, newname);

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), 0, &zipStat) != 0) {
    return false;
  }

  if (zip_rename(zipDir->getZip(), zipStat.index, newname.c_str()) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, setArchiveComment, const String& comment) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(setArchiveComment, zipDir);

  if (zip_set_archive_comment(zipDir->getZip(), comment.c_str(),
                              comment.length()) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, setCommentIndex, int64_t index,
                        const String& comment) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(setCommentIndex, zipDir);

  struct zip_stat zipStat;
  if (zip_stat_index(zipDir->getZip(), index, 0, &zipStat) != 0) {
    return false;
  }

  if (zip_set_file_comment(zipDir->getZip(), index, comment.c_str(),
                           comment.length()) != 0 ) {
    return false;
  }
  return true;
}

static bool HHVM_METHOD(ZipArchive, setCommentName, const String& name,
                        const String& comment) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(setCommentName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(setCommentName, name);

  int index = zip_name_locate(zipDir->getZip(), name.c_str(), 0);
  FAIL_IF_INVALID_INDEX(index);

  if (zip_set_file_comment(zipDir->getZip(), index, comment.c_str(),
                           comment.length()) != 0 ) {
    return false;
  }
  return true;
}

ALWAYS_INLINE
static Array zipStatToArray(struct zip_stat* zipStat) {
  if (zipStat == nullptr) {
    return Array();
  }

  ArrayInit stat(7);
  stat.add(String("name"),        String(zipStat->name));
  stat.add(String("index"),       VarNR(zipStat->index));
  stat.add(String("crc"),         VarNR(static_cast<int64_t>(zipStat->crc)));
  stat.add(String("size"),        VarNR(zipStat->size));
  stat.add(String("mtime"),       VarNR(zipStat->mtime));
  stat.add(String("comp_size"),   VarNR(zipStat->comp_size));
  stat.add(String("comp_method"), VarNR(zipStat->comp_method));

  return stat.create();
}

static Variant HHVM_METHOD(ZipArchive, statIndex, int64_t index,
                           int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(statIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  struct zip_stat zipStat;
  if (zip_stat_index(zipDir->getZip(), index, flags, &zipStat) != 0) {
    return false;
  }

  return zipStatToArray(&zipStat);
}

static Variant HHVM_METHOD(ZipArchive, statName, const String& name,
                           int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(statName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(statName, name);

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), flags, &zipStat) != 0) {
    return false;
  }

  return zipStatToArray(&zipStat);
}

static bool HHVM_METHOD(ZipArchive, unchangeAll) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeAll, zipDir);

  if (zip_unchange_all(zipDir->getZip()) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, unchangeArchive) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeArchive, zipDir);

  if (zip_unchange_archive(zipDir->getZip()) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, unchangeIndex, int64_t index) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  if (zip_unchange(zipDir->getZip(), index) != 0) {
    return false;
  }

  return true;
}

static bool HHVM_METHOD(ZipArchive, unchangeName, const String& name) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(unchangeName, name);

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), 0, &zipStat) != 0) {
    return false;
  }

  if (zip_unchange(zipDir->getZip(), zipStat.index) != 0) {
    return false;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////
// functions

static Variant HHVM_FUNCTION(zip_close, CResRef zip) {
  auto zipDir = zip.getTyped<ZipDirectory>();

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_close, zipDir);

  zipDir->close();

  return null_variant;
}

static bool HHVM_FUNCTION(zip_entry_close, CResRef zip_entry) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_close, zipEntry);

  return zipEntry->close();
}

static Variant HHVM_FUNCTION(zip_entry_compressedsize, CResRef zip_entry) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_compressedsize, zipEntry);

  return zipEntry->getCompressedSize();
}

static Variant HHVM_FUNCTION(zip_entry_compressionmethod, CResRef zip_entry) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_compressionmethod, zipEntry);

  return zipEntry->getCompressionMethod();
}

static Variant HHVM_FUNCTION(zip_entry_filesize, CResRef zip_entry) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_filesize, zipEntry);

  return zipEntry->getSize();
}

static Variant HHVM_FUNCTION(zip_entry_name, CResRef zip_entry) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_name, zipEntry);

  return zipEntry->getName();
}

static bool HHVM_FUNCTION(zip_entry_open, CResRef zip, CResRef zip_entry,
                          const String& mode) {
  auto zipDir   = zip.getTyped<ZipDirectory>();
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_entry_open, zipDir);
  FAIL_IF_INVALID_ZIPENTRY(zip_entry_open, zipEntry);

  return true;
}

static Variant HHVM_FUNCTION(zip_entry_read, CResRef zip_entry,
                             int64_t length) {
  auto zipEntry = zip_entry.getTyped<ZipEntry>();

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_read, zipEntry);

  return zipEntry->read(length > 0 ? length : 1024);
}

static Variant HHVM_FUNCTION(zip_open, const String& filename) {
  FAIL_IF_EMPTY_STRING(zip_open, filename);

  int  err;
  auto z = zip_open(filename.c_str(), 0, &err);
  if (z == nullptr) {
    return err;
  }

  auto zipDir = NEWOBJ(ZipDirectory)(z);

  return Resource(zipDir);
}

static Variant HHVM_FUNCTION(zip_read, CResRef zip) {
  auto zipDir = zip.getTyped<ZipDirectory>();

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_read, zipDir);

  return zipDir->nextFile();
}

//////////////////////////////////////////////////////////////////////////////

class zipExtension : public Extension {
 public:
  zipExtension() : Extension("zip") {}
  virtual void moduleInit() {
    HHVM_ME(ZipArchive, getProperty);
    HHVM_ME(ZipArchive, addEmptyDir);
    HHVM_ME(ZipArchive, addFile);
    HHVM_ME(ZipArchive, addFromString);
    HHVM_ME(ZipArchive, addGlob);
    HHVM_ME(ZipArchive, addPattern);
    HHVM_ME(ZipArchive, close);
    HHVM_ME(ZipArchive, deleteIndex);
    HHVM_ME(ZipArchive, deleteName);
    HHVM_ME(ZipArchive, extractTo);
    HHVM_ME(ZipArchive, getArchiveComment);
    HHVM_ME(ZipArchive, getCommentIndex);
    HHVM_ME(ZipArchive, getCommentName);
    HHVM_ME(ZipArchive, getFromIndex);
    HHVM_ME(ZipArchive, getFromName);
    HHVM_ME(ZipArchive, getNameIndex);
    HHVM_ME(ZipArchive, getStatusString);
    HHVM_ME(ZipArchive, getStream);
    HHVM_ME(ZipArchive, locateName);
    HHVM_ME(ZipArchive, open);
    HHVM_ME(ZipArchive, renameIndex);
    HHVM_ME(ZipArchive, renameName);
    HHVM_ME(ZipArchive, setArchiveComment);
    HHVM_ME(ZipArchive, setCommentIndex);
    HHVM_ME(ZipArchive, setCommentName);
    HHVM_ME(ZipArchive, statIndex);
    HHVM_ME(ZipArchive, statName);
    HHVM_ME(ZipArchive, unchangeAll);
    HHVM_ME(ZipArchive, unchangeArchive);
    HHVM_ME(ZipArchive, unchangeIndex);
    HHVM_ME(ZipArchive, unchangeName);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CREATE.get(), k_CREATE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_EXCL.get(), k_EXCL);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CHECKCONS.get(), k_CHECKCONS);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_OVERWRITE.get(), k_OVERWRITE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_NOCASE.get(), k_FL_NOCASE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_NODIR.get(), k_FL_NODIR);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_COMPRESSED.get(),
                                               k_FL_COMPRESSED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_UNCHANGED.get(),
                                               k_FL_UNCHANGED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_RECOMPRESS.get(),
                                               k_FL_RECOMPRESS);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_FL_ENCRYPTED.get(),
                                               k_FL_ENCRYPTED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_OK.get(), k_ER_OK);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_MULTIDISK.get(),
                                               k_ER_MULTIDISK);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_RENAME.get(), k_ER_RENAME);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_CLOSE.get(), k_ER_CLOSE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_SEEK.get(), k_ER_SEEK);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_READ.get(), k_ER_READ);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_WRITE.get(), k_ER_WRITE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_CRC.get(), k_ER_CRC);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_ZIPCLOSED.get(),
                                               k_ER_ZIPCLOSED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_NOENT.get(), k_ER_NOENT);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_EXISTS.get(), k_ER_EXISTS);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_OPEN.get(), k_ER_OPEN);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_TMPOPEN.get(),
                                               k_ER_TMPOPEN);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_ZLIB.get(), k_ER_ZLIB);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_MEMORY.get(), k_ER_MEMORY);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_CHANGED.get(),
                                               k_ER_CHANGED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_COMPNOTSUPP.get(),
                                               k_ER_COMPNOTSUPP);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_EOF.get(), k_ER_EOF);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_INVAL.get(), k_ER_INVAL);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_NOZIP.get(), k_ER_NOZIP);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_INTERNAL.get(),
                                               k_ER_INTERNAL);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_INCONS.get(), k_ER_INCONS);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_REMOVE.get(), k_ER_REMOVE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_DELETED.get(),
                                               k_ER_DELETED);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_ENCRNOTSUPP.get(),
                                               k_ER_ENCRNOTSUPP);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_RDONLY.get(), k_ER_RDONLY);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_NOPASSWD.get(),
                                               k_ER_NOPASSWD);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_ER_WRONGPASSWD.get(),
                                               k_ER_WRONGPASSWD);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_DEFAULT.get(),
                                               k_CM_DEFAULT);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_STORE.get(), k_CM_STORE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_SHRINK.get(), k_CM_SHRINK);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_REDUCE_1.get(),
                                               k_CM_REDUCE_1);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_REDUCE_2.get(),
                                               k_CM_REDUCE_2);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_REDUCE_3.get(),
                                               k_CM_REDUCE_3);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_REDUCE_4.get(),
                                               k_CM_REDUCE_4);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_IMPLODE.get(),
                                               k_CM_IMPLODE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_DEFLATE.get(),
                                               k_CM_DEFLATE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_DEFLATE64.get(),
                                               k_CM_DEFLATE64);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_PKWARE_IMPLODE.get(),
                                               k_CM_PKWARE_IMPLODE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_BZIP2.get(),
                                               k_CM_BZIP2);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_LZMA.get(),
                                               k_CM_LZMA);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_TERSE.get(),
                                               k_CM_TERSE);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_LZ77.get(),
                                               k_CM_LZ77);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_WAVPACK.get(),
                                               k_CM_WAVPACK);
    Native::registerClassConstant<KindOfInt64>(s_ZipArchive.get(),
                                               s_CM_PPMD.get(),
                                               k_CM_PPMD);

    HHVM_FE(zip_close);
    HHVM_FE(zip_entry_close);
    HHVM_FE(zip_entry_compressedsize);
    HHVM_FE(zip_entry_compressionmethod);
    HHVM_FE(zip_entry_filesize);
    HHVM_FE(zip_entry_name);
    HHVM_FE(zip_entry_open);
    HHVM_FE(zip_entry_read);
    HHVM_FE(zip_open);
    HHVM_FE(zip_read);

    auto wrapper = NEWOBJ(ZipStreamWrapper)();
    if (wrapper == nullptr || !Stream::registerWrapper("zip", wrapper)) {
      raise_warning("Couldn't register Zip wrapper");
    }

    loadSystemlib();
  }
} s_zip_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(zip);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
