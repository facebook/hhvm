/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

namespace HPHP {

static String to_full_path(const String& filename) {
  if (filename.charAt(0) == '/') {
    return filename;
  }
  return f_getcwd().toString() + String::FromChar('/') + filename;
}

// A wrapper for `zip_open` that prepares a full path
// file name to consider current working directory.
static zip* _zip_open(const String& filename, int _flags, int* zep) {
  return zip_open(to_full_path(filename).c_str(), _flags, zep);
}

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

  bool open(const String&, const String&) override { return false; }

  bool close() override {
    bool noError = true;
    if (!eof()) {
      if (zip_fclose(m_zipFile) != 0) {
        noError = false;
      }
      m_zipFile = nullptr;
    }
    return noError;
  }

  int64_t readImpl(char *buffer, int64_t length) override {
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

  int64_t writeImpl(const char *buffer, int64_t length) override { return 0; }

  bool eof() override { return m_zipFile == nullptr; }

 private:
  zip_file* m_zipFile;
};

void ZipStream::sweep() {
  close();
  File::sweep();
}

class ZipStreamWrapper : public Stream::Wrapper {
 public:
  virtual req::ptr<File> open(const String& filename,
                              const String& mode,
                              int options,
                              const req::ptr<StreamContext>& context) {
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
    auto z = _zip_open(path, 0, &err);
    if (z == nullptr) {
      return nullptr;
    }

    return req::make<ZipStream>(z, file);
  }
};

class ZipEntry : public SweepableResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION(ZipEntry);

  CLASSNAME_IS("ZipEntry");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

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
    StringBuffer sb(len);
    auto buf = sb.appendCursor(len);
    auto n   = zip_fread(m_zipFile, buf, len);
    if (n > 0) {
      sb.resize(n);
      return sb.detach();
    }
    return empty_string();
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
IMPLEMENT_RESOURCE_ALLOCATION(ZipEntry);

class ZipDirectory: public SweepableResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION(ZipDirectory);

  CLASSNAME_IS("ZipDirectory");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit ZipDirectory(zip *z) : m_zip(z),
                                  m_numFiles(zip_get_num_files(z)),
                                  m_curIndex(0) {}

  ~ZipDirectory() { close(); }

  bool close() {
    bool noError = true;
    if (isValid()) {
      if (zip_close(m_zip) != 0) {
        zip_discard(m_zip);
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

    auto zipEntry = req::make<ZipEntry>(m_zip, m_curIndex);

    if (!zipEntry->isValid()) {
      return false;
    }

    ++m_curIndex;

    return Variant(std::move(zipEntry));
  }

  zip* getZip() {
    return m_zip;
  }

 private:
  zip* m_zip;
  int  m_numFiles;
  int  m_curIndex;
};
IMPLEMENT_RESOURCE_ALLOCATION(ZipDirectory);

const StaticString s_ZipArchive("ZipArchive");

template<class T>
ALWAYS_INLINE
static req::ptr<T> getResource(ObjectData* obj, const char* varName) {
  auto var = obj->o_get(varName, true, s_ZipArchive);
  if (var.getType() == KindOfNull) {
    return nullptr;
  }
  return cast<T>(var);
}

ALWAYS_INLINE
static Variant setVariable(const Object& obj, const char* varName, const Variant& varValue) {
  return obj->o_set(varName, varValue, s_ZipArchive);
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
                  "Zip Directory resource", res->getId());              \
    return false;                                                       \
  }

#define FAIL_IF_INVALID_ZIPENTRY(func, res)                             \
  if (!res->isValid()) {                                                \
    raise_warning(#func "(): %d is not a valid Zip Entry resource",     \
                  res->getId());                                        \
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
        return empty_string_variant();
      default:
        return init_null();
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
      return this_->o_get("filename", true, s_ZipArchive).asCStrRef();
    }
    case 4:
    {
      int len;
      auto comment = zip_get_archive_comment(zipDir->getZip(), &len, 0);
      if (comment == nullptr) {
        return empty_string_variant();
      }
      return String(comment, len, CopyString);
    }
    default:
      return init_null();
  }
}

static bool HHVM_METHOD(ZipArchive, addEmptyDir, const String& dirname) {
  if (dirname.empty()) {
    return false;
  }

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

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool addFile(zip* zipStruct, const char* source, const char* dest,
                    int64_t start = 0, int64_t length = 0) {
  if (!HHVM_FN(is_file)(source)) {
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

  zip_error_clear(zipStruct);
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

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool addPattern(zip* zipStruct, const String& pattern, const Array& options,
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
    auto match = HHVM_FN(glob)(pattern, flags);
    if (match.isArray()) {
      files = match.toArrRef();
    } else {
      return false;
    }
  } else {
    if (path[path.size() - 1] != '/') {
      path.push_back('/');
    }
    auto allFiles = HHVM_FN(scandir)(path);
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
    if (HHVM_FN(is_dir)(source)) {
      continue;
    }

    if (!glob) {
      auto var = preg_match(pattern, source);
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

  zip_error_clear(zipStruct);
  return true;
}

static bool HHVM_METHOD(ZipArchive, addGlob, const String& pattern,
                        int64_t flags, const Array& options) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addGlob, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addGlob, pattern);

  return addPattern(zipDir->getZip(), pattern, options, "", flags, true);
}

static bool HHVM_METHOD(ZipArchive, addPattern, const String& pattern,
                        const String& path, const Array& options) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(addPattern, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(addPattern, pattern);

  return addPattern(zipDir->getZip(), pattern, options, path.c_str(), 0, false);
}

static bool HHVM_METHOD(ZipArchive, close) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(close, zipDir);

  bool ret = zipDir->close();

  setVariable(Object{this_}, "zipDir", null_resource);

  return ret;
}

static bool HHVM_METHOD(ZipArchive, deleteIndex, int64_t index) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(deleteIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  if (zip_delete(zipDir->getZip(), index) != 0) {
    return false;
  }

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool extractFileTo(zip* zip, const std::string &file, std::string& to,
                          char* buf, size_t len) {
  auto sep = file.rfind('/');
  if (sep != std::string::npos) {
    auto path = to + file.substr(0, sep);
    if (!HHVM_FN(is_dir)(path) && !HHVM_FN(mkdir)(path, 0777, true)) {
      return false;
    }

    if (sep == file.length() - 1) {
      return true;
    }
  }

  to.append(file);
  struct zip_stat zipStat;
  if (zip_stat(zip, file.c_str(), 0, &zipStat) != 0) {
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
                        const Variant& entries) {
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

  if (!HHVM_FN(is_dir)(to) && !HHVM_FN(mkdir)(to)) {
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

  zip_error_clear(zipDir->getZip());
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
    return empty_string_variant();
  }

  struct zip_stat zipStat;
  if (zip_stat_index(zipDir->getZip(), index, 0, &zipStat) != 0) {
    return false;
  }

  if (zipStat.size < 1) {
    return empty_string_variant();
  }

  auto zipFile = zip_fopen_index(zipDir->getZip(), index, flags);
  FAIL_IF_INVALID_PTR(zipFile);

  if (length == 0)  {
    length = zipStat.size;
  }

  StringBuffer sb(length);
  auto buf = sb.appendCursor(length);
  auto n   = zip_fread(zipFile, buf, length);
  if (n > 0) {
    sb.resize(n);
    return sb.detach();
  }
  return empty_string_variant();
}

static Variant HHVM_METHOD(ZipArchive, getFromName, const String& name,
                           int64_t length, int64_t flags) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(getFromName, zipDir);
  FAIL_IF_EMPTY_STRING_ZIPARCHIVE(getFromName, name);

  if (length < 0) {
    return empty_string_variant();
  }

  struct zip_stat zipStat;
  if (zip_stat(zipDir->getZip(), name.c_str(), flags, &zipStat) != 0) {
    return false;
  }

  if (zipStat.size < 1) {
    return empty_string_variant();
  }

  auto zipFile = zip_fopen(zipDir->getZip(), name.c_str(), flags);
  FAIL_IF_INVALID_PTR(zipFile);

  if (length == 0)  {
    length = zipStat.size;
  }

  StringBuffer sb(length);
  auto buf = sb.appendCursor(length);
  auto n   = zip_fread(zipFile, buf, length);
  if (n > 0) {
    sb.resize(n);
    return sb.detach();
  }
  return empty_string_variant();
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

  auto zipStream = req::make<ZipStream>(zipDir->getZip(), name);
  if (zipStream->eof()) {
    return false;
  }
  return Variant(std::move(zipStream));
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
  auto z = _zip_open(filename, flags, &err);
  if (z == nullptr) {
    return err;
  }

  auto zipDir = req::make<ZipDirectory>(z);

  setVariable(Object{this_}, "zipDir", Variant(zipDir));
  setVariable(Object{this_}, "filename", filename);

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool HHVM_METHOD(ZipArchive, setArchiveComment, const String& comment) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(setArchiveComment, zipDir);

  if (zip_set_archive_comment(zipDir->getZip(), comment.c_str(),
                              comment.length()) != 0) {
    return false;
  }

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
  return true;
}

const StaticString s_name("name");
const StaticString s_index("index");
const StaticString s_crc("crc");
const StaticString s_size("size");
const StaticString s_mtime("mtime");
const StaticString s_comp_size("comp_size");
const StaticString s_comp_method("comp_method");

ALWAYS_INLINE
static Array zipStatToArray(struct zip_stat* zipStat) {
  if (zipStat == nullptr) {
    return Array();
  }

  return make_map_array(
    s_name,        String(zipStat->name),
    s_index,       VarNR(zipStat->index),
    s_crc,         VarNR(static_cast<int64_t>(zipStat->crc)),
    s_size,        VarNR(zipStat->size),
    s_mtime,       VarNR(zipStat->mtime),
    s_comp_size,   VarNR(zipStat->comp_size),
    s_comp_method, VarNR(zipStat->comp_method)
  );
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

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool HHVM_METHOD(ZipArchive, unchangeArchive) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeArchive, zipDir);

  if (zip_unchange_archive(zipDir->getZip()) != 0) {
    return false;
  }

  zip_error_clear(zipDir->getZip());
  return true;
}

static bool HHVM_METHOD(ZipArchive, unchangeIndex, int64_t index) {
  auto zipDir = getResource<ZipDirectory>(this_, "zipDir");

  FAIL_IF_INVALID_ZIPARCHIVE(unchangeIndex, zipDir);
  FAIL_IF_INVALID_INDEX(index);

  if (zip_unchange(zipDir->getZip(), index) != 0) {
    return false;
  }

  zip_error_clear(zipDir->getZip());
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

  zip_error_clear(zipDir->getZip());
  return true;
}

//////////////////////////////////////////////////////////////////////////////
// functions

static Variant HHVM_FUNCTION(zip_close, const Resource& zip) {
  auto zipDir = cast<ZipDirectory>(zip);

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_close, zipDir);

  zipDir->close();

  return init_null();
}

static bool HHVM_FUNCTION(zip_entry_close, const Resource& zip_entry) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_close, zipEntry);

  return zipEntry->close();
}

static Variant HHVM_FUNCTION(zip_entry_compressedsize, const Resource& zip_entry) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_compressedsize, zipEntry);

  return zipEntry->getCompressedSize();
}

static Variant HHVM_FUNCTION(zip_entry_compressionmethod, const Resource& zip_entry) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_compressionmethod, zipEntry);

  return zipEntry->getCompressionMethod();
}

static Variant HHVM_FUNCTION(zip_entry_filesize, const Resource& zip_entry) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_filesize, zipEntry);

  return zipEntry->getSize();
}

static Variant HHVM_FUNCTION(zip_entry_name, const Resource& zip_entry) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_name, zipEntry);

  return zipEntry->getName();
}

static bool HHVM_FUNCTION(zip_entry_open, const Resource& zip, const Resource& zip_entry,
                          const String& mode) {
  auto zipDir   = cast<ZipDirectory>(zip);
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_entry_open, zipDir);
  FAIL_IF_INVALID_ZIPENTRY(zip_entry_open, zipEntry);

  zip_error_clear(zipDir->getZip());
  return true;
}

static Variant HHVM_FUNCTION(zip_entry_read, const Resource& zip_entry,
                             int64_t length) {
  auto zipEntry = cast<ZipEntry>(zip_entry);

  FAIL_IF_INVALID_ZIPENTRY(zip_entry_read, zipEntry);

  return zipEntry->read(length > 0 ? length : 1024);
}

static Variant HHVM_FUNCTION(zip_open, const String& filename) {
  FAIL_IF_EMPTY_STRING(zip_open, filename);

  int  err;
  auto z = _zip_open(filename, 0, &err);
  if (z == nullptr) {
    return err;
  }

  return Variant(req::make<ZipDirectory>(z));
}

static Variant HHVM_FUNCTION(zip_read, const Resource& zip) {
  auto zipDir = cast<ZipDirectory>(zip);

  FAIL_IF_INVALID_ZIPDIRECTORY(zip_read, zipDir);

  return zipDir->nextFile();
}

//////////////////////////////////////////////////////////////////////////////

class zipExtension final : public Extension {
 public:
  zipExtension() : Extension("zip", "1.12.4-dev") {}
  void moduleInit() override {
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

    HHVM_RCC_INT(ZipArchive, CREATE, ZIP_CREATE);
    HHVM_RCC_INT(ZipArchive, EXCL, ZIP_EXCL);
    HHVM_RCC_INT(ZipArchive, CHECKCONS, ZIP_CHECKCONS);
    HHVM_RCC_INT(ZipArchive, OVERWRITE, ZIP_TRUNCATE);
    HHVM_RCC_INT(ZipArchive, FL_NOCASE, ZIP_FL_NOCASE);
    HHVM_RCC_INT(ZipArchive, FL_NODIR, ZIP_FL_NODIR);
    HHVM_RCC_INT(ZipArchive, FL_COMPRESSED, ZIP_FL_COMPRESSED);
    HHVM_RCC_INT(ZipArchive, FL_UNCHANGED, ZIP_FL_UNCHANGED);
    HHVM_RCC_INT(ZipArchive, FL_RECOMPRESS, ZIP_FL_RECOMPRESS);
    HHVM_RCC_INT(ZipArchive, FL_ENCRYPTED, ZIP_FL_ENCRYPTED);
    HHVM_RCC_INT(ZipArchive, ER_OK, ZIP_ER_OK);
    HHVM_RCC_INT(ZipArchive, ER_MULTIDISK, ZIP_ER_MULTIDISK);
    HHVM_RCC_INT(ZipArchive, ER_RENAME, ZIP_ER_RENAME);
    HHVM_RCC_INT(ZipArchive, ER_CLOSE, ZIP_ER_CLOSE);
    HHVM_RCC_INT(ZipArchive, ER_SEEK, ZIP_ER_SEEK);
    HHVM_RCC_INT(ZipArchive, ER_READ, ZIP_ER_READ);
    HHVM_RCC_INT(ZipArchive, ER_WRITE, ZIP_ER_WRITE);
    HHVM_RCC_INT(ZipArchive, ER_CRC, ZIP_ER_CRC);
    HHVM_RCC_INT(ZipArchive, ER_ZIPCLOSED, ZIP_ER_ZIPCLOSED);
    HHVM_RCC_INT(ZipArchive, ER_NOENT, ZIP_ER_NOENT);
    HHVM_RCC_INT(ZipArchive, ER_EXISTS, ZIP_ER_EXISTS);
    HHVM_RCC_INT(ZipArchive, ER_OPEN, ZIP_ER_OPEN);
    HHVM_RCC_INT(ZipArchive, ER_TMPOPEN, ZIP_ER_TMPOPEN);
    HHVM_RCC_INT(ZipArchive, ER_ZLIB, ZIP_ER_ZLIB);
    HHVM_RCC_INT(ZipArchive, ER_MEMORY, ZIP_ER_MEMORY);
    HHVM_RCC_INT(ZipArchive, ER_CHANGED, ZIP_ER_CHANGED);
    HHVM_RCC_INT(ZipArchive, ER_COMPNOTSUPP, ZIP_ER_COMPNOTSUPP);
    HHVM_RCC_INT(ZipArchive, ER_EOF, ZIP_ER_EOF);
    HHVM_RCC_INT(ZipArchive, ER_INVAL, ZIP_ER_INVAL);
    HHVM_RCC_INT(ZipArchive, ER_NOZIP, ZIP_ER_NOZIP);
    HHVM_RCC_INT(ZipArchive, ER_INTERNAL, ZIP_ER_INTERNAL);
    HHVM_RCC_INT(ZipArchive, ER_INCONS, ZIP_ER_INCONS);
    HHVM_RCC_INT(ZipArchive, ER_REMOVE, ZIP_ER_REMOVE);
    HHVM_RCC_INT(ZipArchive, ER_DELETED, ZIP_ER_DELETED);
    HHVM_RCC_INT(ZipArchive, ER_ENCRNOTSUPP, ZIP_ER_ENCRNOTSUPP);
    HHVM_RCC_INT(ZipArchive, ER_RDONLY, ZIP_ER_RDONLY);
    HHVM_RCC_INT(ZipArchive, ER_NOPASSWD, ZIP_ER_NOPASSWD);
    HHVM_RCC_INT(ZipArchive, ER_WRONGPASSWD, ZIP_ER_WRONGPASSWD);
    HHVM_RCC_INT(ZipArchive, CM_DEFAULT, ZIP_CM_DEFAULT);
    HHVM_RCC_INT(ZipArchive, CM_STORE, ZIP_CM_STORE);
    HHVM_RCC_INT(ZipArchive, CM_SHRINK, ZIP_CM_SHRINK);
    HHVM_RCC_INT(ZipArchive, CM_REDUCE_1, ZIP_CM_REDUCE_1);
    HHVM_RCC_INT(ZipArchive, CM_REDUCE_2, ZIP_CM_REDUCE_2);
    HHVM_RCC_INT(ZipArchive, CM_REDUCE_3, ZIP_CM_REDUCE_3);
    HHVM_RCC_INT(ZipArchive, CM_REDUCE_4, ZIP_CM_REDUCE_4);
    HHVM_RCC_INT(ZipArchive, CM_IMPLODE, ZIP_CM_IMPLODE);
    HHVM_RCC_INT(ZipArchive, CM_DEFLATE, ZIP_CM_DEFLATE);
    HHVM_RCC_INT(ZipArchive, CM_DEFLATE64, ZIP_CM_DEFLATE64);
    HHVM_RCC_INT(ZipArchive, CM_PKWARE_IMPLODE, ZIP_CM_PKWARE_IMPLODE);
    HHVM_RCC_INT(ZipArchive, CM_BZIP2, ZIP_CM_BZIP2);
    HHVM_RCC_INT(ZipArchive, CM_LZMA, ZIP_CM_LZMA);
    HHVM_RCC_INT(ZipArchive, CM_TERSE, ZIP_CM_TERSE);
    HHVM_RCC_INT(ZipArchive, CM_LZ77, ZIP_CM_LZ77);
    HHVM_RCC_INT(ZipArchive, CM_WAVPACK, ZIP_CM_WAVPACK);
    HHVM_RCC_INT(ZipArchive, CM_PPMD, ZIP_CM_PPMD);

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

    auto wrapper = new ZipStreamWrapper();
    if (wrapper == nullptr || !Stream::registerWrapper("zip", wrapper)) {
      delete wrapper;
      raise_warning("Couldn't register Zip wrapper");
    }

    loadSystemlib();
  }
} s_zip_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(zip);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
