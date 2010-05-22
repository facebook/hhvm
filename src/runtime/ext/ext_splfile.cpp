/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_splfile.h>
#include <runtime/ext/ext_file.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(SPL);
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION(SplFileInfo);
IMPLEMENT_OBJECT_ALLOCATION(SplFileObject);

static SplFileInfo *get_splfileinfo(CObjRef obj) {
  c_splfileinfo *c_splfi = obj.getTyped<c_splfileinfo>();
  return c_splfi->m_rsrc.toObject().getTyped<SplFileInfo>();
}

static SplFileObject *get_splfileobject(CObjRef obj) {
  c_splfileobject *c_splfo = obj.getTyped<c_splfileobject>();
  return c_splfo->m_rsrc.toObject().getTyped<SplFileObject>();
}

Object f_hphp_splfileinfo___construct(CObjRef obj, CStrRef file_name) {
  c_splfileinfo *c_splfi = obj.getTyped<c_splfileinfo>();
  c_splfi->m_rsrc = NEW(SplFileInfo)(file_name);
  return c_splfi;
}

int64 f_hphp_splfileinfo_getatime(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileatime(fileInfo->getFileName());
}

String f_hphp_splfileinfo_getbasename(CObjRef obj, CStrRef suffix) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_basename(fileInfo->getFileName(), suffix);
}

int64 f_hphp_splfileinfo_getctime(CObjRef obj) {
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

int64 f_hphp_splfileinfo_getgroup(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filegroup(fileInfo->getFileName());
}

int64 f_hphp_splfileinfo_getinode(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileinode(fileInfo->getFileName());
}

String f_hphp_splfileinfo_getlinktarget(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  String ret = f_readlink_internal(fileInfo->getFileName(), false);
  if (!ret.size())  {
    throw (Object)sp_exception(NEW(c_exception)())->create(Variant(
      "Unable to read link "+fileInfo->getFileName()
      +", error: no such file or directory"));
  }
  return ret;
}

int64 f_hphp_splfileinfo_getmtime(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_filemtime(fileInfo->getFileName());
}

int64 f_hphp_splfileinfo_getowner(CObjRef obj) {
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

int64 f_hphp_splfileinfo_getperms(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_fileperms(fileInfo->getFileName());
}

Variant f_hphp_splfileinfo_getrealpath(CObjRef obj) {
  SplFileInfo *fileInfo = get_splfileinfo(obj);
  return f_realpath(fileInfo->getFileName());
}

int64 f_hphp_splfileinfo_getsize(CObjRef obj) {
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
  return sp_splfileobject(sp_splfileobject(NEW(c_splfileobject)())->
           create(String(fileInfo->getFileName()),
                  open_mode, use_include_path, context));
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
  c_splfileobject *c_splfo = obj.getTyped<c_splfileobject>();
  c_splfo->m_rsrc = NEW(SplFileObject)(f);
  return c_splfo;
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

bool f_hphp_splfileobject_flock(CObjRef obj, Variant wouldblock) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_fpassthru(CObjRef obj) {
  throw NotImplementedException(__func__);
}

Variant f_hphp_splfileobject_fscanf(int64 _argc, CObjRef obj, CStrRef format, CVarRef _argv) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_fseek(CObjRef obj, int64 offset, int64 whence) {
  throw NotImplementedException(__func__);
}

Variant f_hphp_splfileobject_fstat(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_ftell(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_ftruncate(CObjRef obj, int64 size) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_fwrite(CObjRef obj, CStrRef str, int64 length) {
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

int64 f_hphp_splfileobject_getflags(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_getmaxlinelen(CObjRef obj) {
  throw NotImplementedException(__func__);
}

bool f_hphp_splfileobject_haschildren(CObjRef obj) {
  throw NotImplementedException(__func__);
}

int64 f_hphp_splfileobject_key(CObjRef obj) {
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

void f_hphp_splfileobject_seek(CObjRef obj, int64 line_pos) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setcsvcontrol(CObjRef obj, CStrRef delimiter, CStrRef enclosure, CStrRef escape) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setflags(CObjRef obj, int64 flags) {
  throw NotImplementedException(__func__);
}

void f_hphp_splfileobject_setmaxlinelen(CObjRef obj, int64 max_len) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////
}
