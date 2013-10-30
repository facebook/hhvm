#include "hphp/runtime/base/base-includes.h"
namespace HPHP {
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

//////////////////////////////////////////////////////////////////////////////
// class ZipArchive

static bool HHVM_METHOD(ZipArchive, addEmptyDir, const String& dirname) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, addFile, const String& filename,
                        const String& localname, int64_t start,
                        int64_t length) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, addFromString, const String& localname,
                        const String& contents) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, addGlob, const String& pattern,
                        int64_t flags, CArrRef options) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, addPattern, const String& pattern,
                        const String& path, CArrRef options) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, close) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, deleteIndex, int64_t index) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, deleteName, const String& name) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, extractTo, const String& destination,
                        CVarRef entries) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getArchiveComment, int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getCommentIndex, int64_t index,
                          int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getCommentName, const String& name,
                          int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getFromIndex, int64_t index,
                          int64_t length, int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getFromName, const String& name,
                          int64_t length, int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getNameIndex, int64_t index,
                          int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_METHOD(ZipArchive, getStatusString) {
  throw NotImplementedException("Not implemented");
}

static Resource HHVM_METHOD(ZipArchive, getStream, const String& name) {
  throw NotImplementedException("Not implemented");
}

static int64_t HHVM_METHOD(ZipArchive, locateName, const String& name,
                           int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static Variant HHVM_METHOD(ZipArchive, open, const String& filename,
                           int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, renameIndex, int64_t index,
                        const String& newname) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, renameName, const String& name,
                        const String& newname) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, setArchiveComment, const String& comment) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, setCommentIndex, int64_t index,
                        const String& comment) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, setCommentName, const String& name,
                        const String& comment) {
  throw NotImplementedException("Not implemented");
}

static Array HHVM_METHOD(ZipArchive, statIndex, int64_t index, int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static Array HHVM_METHOD(ZipArchive, statName, const String& name,
                         int64_t flags) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, unchangeAll) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, unchangeArchive) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, unchangeIndex, int64_t index) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_METHOD(ZipArchive, unchangeName, const String& name) {
  throw NotImplementedException("Not implemented");
}

//////////////////////////////////////////////////////////////////////////////
// functions

static void HHVM_FUNCTION(zip_close, CResRef zip) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_FUNCTION(zip_entry_close, CResRef zip_entry) {
  throw NotImplementedException("Not implemented");
}

static int64_t HHVM_FUNCTION(zip_entry_compressedsize, CResRef zip_entry) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_FUNCTION(zip_entry_compressionmethod, CResRef zip_entry) {
  throw NotImplementedException("Not implemented");
}

static int64_t HHVM_FUNCTION(zip_entry_filesize, CResRef zip_entry) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_FUNCTION(zip_entry_name, CResRef zip_entry) {
  throw NotImplementedException("Not implemented");
}

static bool HHVM_FUNCTION(zip_entry_open, CResRef zip, CResRef zip_entry,
                          const String& mode) {
  throw NotImplementedException("Not implemented");
}

static String HHVM_FUNCTION(zip_entry_read, CResRef zip_entry, int64_t length) {
  throw NotImplementedException("Not implemented");
}

static Resource HHVM_FUNCTION(zip_open, const String& filename) {
  throw NotImplementedException("Not implemented");
}

static Resource HHVM_FUNCTION(zip_read, CResRef zip) {
  throw NotImplementedException("Not implemented");
}

//////////////////////////////////////////////////////////////////////////////

class zipExtension : public Extension {
 public:
  zipExtension() : Extension("zip") {}
  virtual void moduleInit() {
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
    loadSystemlib();
  }
} s_zip_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(zip);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
