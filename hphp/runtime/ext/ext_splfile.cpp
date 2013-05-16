/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/ext/ext_splfile.h"
#include "hphp/runtime/ext/ext_file.h"

#include "hphp/system/lib/systemlib.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(SPL);
///////////////////////////////////////////////////////////////////////////////

StaticString SplFileInfo::s_class_name("splfileinfo");
StaticString SplFileObject::s_class_name("splfileobject");

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION(SplFileInfo)
IMPLEMENT_OBJECT_ALLOCATION(SplFileObject)

static SplFileInfo *get_splfileinfo(CObjRef obj) {
  if (!obj->o_instanceof("SplFileInfo")) {
    throw InvalidObjectTypeException(obj->o_getClassName().c_str());
  }
  CObjRef rsrc = obj->o_get("rsrc", true, "SplFileInfo");
  return rsrc.getTyped<SplFileInfo>();
}

static SplFileObject *get_splfileobject(CObjRef obj) {
  if (!obj->o_instanceof("SplFileObject")) {
    throw InvalidObjectTypeException(obj->o_getClassName().c_str());
  }
  // "SplFileInfo" as context -- rsrc is a private property
  CObjRef rsrc = obj->o_get("rsrc", true, "SplFileInfo");
  return rsrc.getTyped<SplFileObject>();
}

Object f_hphp_splfileinfo___construct(CObjRef obj, CStrRef file_name) {
  int len = file_name.size();
  const char *data = file_name.data();
  ObjectData *fi;
  if (len && data[len-1] == '/') {
    do {
      len--;
    } while (len && data[len-1] == '/');
    fi = NEWOBJ(SplFileInfo)(String(data, len, CopyString));
  } else {
    fi = NEWOBJ(SplFileInfo)(file_name);
  }
  obj->o_set("rsrc", fi, "SplFileInfo");
  return obj;
}

int64_t f_hphp_splfileinfo_getatime(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileatime(fileInfo->getFileName());
}

String f_hphp_splfileinfo_getbasename(CObjRef obj, CStrRef suffix) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_basename(fileInfo->getFileName(), suffix);
}

int64_t f_hphp_splfileinfo_getctime(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filectime(fileInfo->getFileName());
}

Object f_hphp_splfileinfo_getfileinfo(CObjRef obj, CStrRef class_name) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileinfo_getfilename(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_basename(fileInfo->getFileName());
}

int64_t f_hphp_splfileinfo_getgroup(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filegroup(fileInfo->getFileName());
}

int64_t f_hphp_splfileinfo_getinode(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileinode(fileInfo->getFileName());
}

String f_hphp_splfileinfo_getlinktarget(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  String ret = f_readlink_internal(fileInfo->getFileName(), false);
  if (!ret.size())  {
    throw Object(SystemLib::AllocExceptionObject(Variant(
      "Unable to read link "+std::string(fileInfo->getFileName()) +
      ", error: no such file or directory")));
  }
  return ret;
}

int64_t f_hphp_splfileinfo_getmtime(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filemtime(fileInfo->getFileName());
}

int64_t f_hphp_splfileinfo_getowner(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileowner(fileInfo->getFileName());
}

String f_hphp_splfileinfo_getpath(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  const char *fileName = fileInfo->getFileName().c_str();
  const char *p1 = strrchr(fileName, '/');
  if (!p1) return "";
  return String(fileName, p1 - fileName, CopyString);
}

Object f_hphp_splfileinfo_getpathinfo(CObjRef obj, CStrRef class_name) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileinfo_getpathname(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return fileInfo->getFileName();
}

int64_t f_hphp_splfileinfo_getperms(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileperms(fileInfo->getFileName());
}

Variant f_hphp_splfileinfo_getrealpath(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_realpath(fileInfo->getFileName());
}

int64_t f_hphp_splfileinfo_getsize(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filesize(fileInfo->getFileName());
}

String f_hphp_splfileinfo_gettype(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filetype(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_isdir(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_dir(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_isexecutable(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_executable(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_isfile(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_file(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_islink(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_link(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_isreadable(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_readable(fileInfo->getFileName());
}

bool f_hphp_splfileinfo_iswritable(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_is_writable(fileInfo->getFileName());
}

Object f_hphp_splfileinfo_openfile(CObjRef obj, CStrRef open_mode, bool use_include_path, CVarRef context) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return SystemLib::AllocSplFileObjectObject(
    String(fileInfo->getFileName()), open_mode, use_include_path, context);
}

void f_hphp_splfileinfo_setfileclass(CObjRef obj, CStrRef class_name) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileinfo_setinfoclass(CObjRef obj, CStrRef class_name) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileinfo___tostring(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return fileInfo->getFileName();
}

Object f_hphp_splfileobject___construct(CObjRef obj, CStrRef filename, CStrRef open_mode, bool use_include_path, CVarRef context) {
  Variant f = f_fopen(filename, open_mode, use_include_path,
                      context.isNull() ? null_object : context.toObject());
  obj->o_set("rsrc", NEWOBJ(SplFileObject)(f), "SplFileInfo");
  return obj;
}

Variant f_hphp_splfileobject_current(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_eof(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_fflush(CObjRef obj) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileobject_fgetc(CObjRef obj) {
  throw NotImplementedException(__func__);
}

Variant f_hphp_splfileobject_fgetcsv(CObjRef obj, CStrRef delimiter, CStrRef enclosure, CStrRef escape) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileobject_fgets(CObjRef obj) {
  throw NotImplementedException(__func__);
}

String f_hphp_splfileobject_fgetss(CObjRef obj, CStrRef allowable_tags) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_flock(CObjRef obj, VRefParam wouldblock) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_fpassthru(CObjRef obj) {
  throw NotImplementedException(__func__);
}

Variant f_hphp_splfileobject_fscanf(int64_t _argc, CObjRef obj, CStrRef format, CVarRef _argv) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_fseek(CObjRef obj, int64_t offset, int64_t whence) {
  throw NotImplementedException(__func__);
}

Variant f_hphp_splfileobject_fstat(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_ftell(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_ftruncate(CObjRef obj, int64_t size) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_fwrite(CObjRef obj, CStrRef str, int64_t length) {
  SplFileObject *fileObject = get_splfileobject(obj);
  Object file = fileObject->getFile();
  if (!file.isNull()) {
    File *f = file.getTyped<File>();
    return f->write(str, length);
  }
  return -1;
}

Variant f_hphp_splfileobject_getcvscontrol(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_getflags(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_getmaxlinelen(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_haschildren(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64_t f_hphp_splfileobject_key(CObjRef obj) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_next(CObjRef obj) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_rewind(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_valid(CObjRef obj) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_seek(CObjRef obj, int64_t line_pos) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setcsvcontrol(CObjRef obj, CStrRef delimiter, CStrRef enclosure, CStrRef escape) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setflags(CObjRef obj, int64_t flags) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setmaxlinelen(CObjRef obj, int64_t max_len) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
