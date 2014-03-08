/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/ext/fileinfo/libmagic/magic.h"

namespace HPHP {
const StaticString s_finfo("finfo");

class FileinfoResource : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(FileinfoResource)
  CLASSNAME_IS("file_info")
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  explicit FileinfoResource(struct magic_set* magic) : m_magic(magic) {}
  virtual ~FileinfoResource() { close(); }
  void close() {
    magic_close(m_magic);
    m_magic = nullptr;
  }
  struct magic_set* getMagic() { return m_magic; }

private:
  struct magic_set* m_magic;
};

void FileinfoResource::sweep() {
  close();
}

static Variant HHVM_FUNCTION(finfo_open,
    int64_t options,
    const Variant& magic_file) {
  auto magic = magic_open(options);
  if (magic == nullptr) {
    raise_warning("Invalid mode '%" PRId64 "'.", options);
    return false;
  }

  String mf;
  if (!magic_file.isNull()) {
    mf = magic_file.toString();
  }

  auto fn = mf.empty() ? nullptr : mf.data();
  if (magic_load(magic, fn) == -1) {
    raise_warning("Failed to load magic database at '%s'.", fn);
    magic_close(magic);
    return false;
  }

  return NEWOBJ(FileinfoResource)(magic);
}

static bool HHVM_FUNCTION(finfo_close, const Resource& finfo) {
  auto res = finfo.getTyped<FileinfoResource>();
  if (!res) {
    return false;
  }
  res->close();
  return true;
}

static bool HHVM_FUNCTION(finfo_set_flags, const Resource& finfo, int64_t options) {
  auto magic = finfo.getTyped<FileinfoResource>()->getMagic();
  if (magic_setflags(magic, options) == -1) {
    raise_warning(
      "Failed to set option '%" PRId64 "' %d:%s",
      options,
      magic_errno(magic),
      magic_error(magic)
    );
    return false;
  }
  return true;
}

#define FILEINFO_MODE_BUFFER 0
#define FILEINFO_MODE_STREAM 1
#define FILEINFO_MODE_FILE 2

static Variant php_finfo_get_type(
    const Resource& object, const Variant& what,
    int64_t options, const Variant& context, int mode, int mimetype_emu)
{
  String ret_val;
  String buffer;
  char mime_directory[] = "directory";
  struct magic_set *magic = NULL;

  if (mimetype_emu) {
    if (what.isString()) {
      buffer = what.toString();
      mode = FILEINFO_MODE_FILE;
    } else if (what.isResource()) {
      mode = FILEINFO_MODE_STREAM;
    } else {
      raise_warning("Can only process string or stream arguments");
    }

    magic = magic_open(MAGIC_MIME_TYPE);
    if (magic_load(magic, NULL) == -1) {
      raise_warning("Failed to load magic database.");
      goto common;
    }
  } else if (object.get()) {
    buffer = what.toString();
    magic = object.getTyped<FileinfoResource>()->getMagic();
  } else {
    // if we want to support finfo as a resource as well, do it here
    not_reached();
  }

  // Set options for the current file/buffer.
  if (options) {
    HHVM_FN(finfo_set_flags)(object, options);
  }

  switch (mode) {
    case FILEINFO_MODE_BUFFER:
    {
      ret_val = magic_buffer(magic, buffer.data(), buffer.size());
      break;
    }

    case FILEINFO_MODE_STREAM:
    {
      auto stream = what.toResource().getTyped<File>();
      if (!stream) {
        goto common;
      }

      auto streampos = stream->tell(); // remember stream position
      stream->seek(0, SEEK_SET);

      ret_val = magic_stream(magic, stream);

      stream->seek(streampos, SEEK_SET);
      break;
    }

    case FILEINFO_MODE_FILE:
    {
      if (buffer.empty()) {
        raise_warning("Empty filename or path");
        ret_val = null_string;
        goto clean;
      }

      auto wrapper = Stream::getWrapperFromURI(buffer);
      auto stream = wrapper->open(buffer, "rb", 0, HPHP::Variant());

      if (!stream) {
        ret_val = null_string;
        goto clean;
      }

      struct stat st;
      if (wrapper->stat(buffer, &st) == 0) {
        if (st.st_mode & S_IFDIR) {
          ret_val = mime_directory;
        } else {
          ret_val = magic_stream(magic, stream);
        }
      }

      stream->close();
      break;
    }

    default:
      raise_warning("Can only process string or stream arguments");
  }

common:
  if (!ret_val) {
    raise_warning(
      "Failed identify data %d:%s",
      magic_errno(magic),
      magic_error(magic)
    );
  }

clean:
  if (mimetype_emu) {
    magic_close(magic);
  }

  // Restore options
  if (options) {
    HHVM_FN(finfo_set_flags)(object, options);
  }

  if (ret_val) {
    return ret_val;
  }
  return false;
}

static String HHVM_FUNCTION(finfo_buffer,
    const Resource& finfo, const Variant& string,
    int64_t options, const Variant& context) {

  String s;
  if (!string.isNull()) {
    s = string.toString();
  }
  return php_finfo_get_type(
      finfo, s, options, context,
      FILEINFO_MODE_BUFFER, 0);
}

static String HHVM_FUNCTION(finfo_file,
    const Resource& finfo, const Variant& file_name,
    int64_t options, const Variant& context) {

  String fn;
  if (!file_name.isNull()) {
    fn = file_name.toString();
  }
  return php_finfo_get_type(
      finfo, fn, options, context,
      FILEINFO_MODE_FILE, 0);
}

static String HHVM_FUNCTION(mime_content_type, const Variant& filename) {
  return php_finfo_get_type(nullptr, filename, 0, uninit_null(), -1, 1);
}

//////////////////////////////////////////////////////////////////////////////

const StaticString s_FILEINFO_NONE("FILEINFO_NONE");
const StaticString s_FILEINFO_SYMLINK("FILEINFO_SYMLINK");
const StaticString s_FILEINFO_MIME("FILEINFO_MIME");
const StaticString s_FILEINFO_MIME_TYPE("FILEINFO_MIME_TYPE");
const StaticString s_FILEINFO_MIME_ENCODING("FILEINFO_MIME_ENCODING");
const StaticString s_FILEINFO_DEVICES("FILEINFO_DEVICES");
const StaticString s_FILEINFO_CONTINUE("FILEINFO_CONTINUE");
const StaticString s_FILEINFO_PRESERVE_ATIME("FILEINFO_PRESERVE_ATIME");
const StaticString s_FILEINFO_RAW("FILEINFO_RAW");

class fileinfoExtension : public Extension {
 public:
  fileinfoExtension() : Extension("fileinfo", "1.0.5-dev") {}
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_NONE.get(), MAGIC_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_SYMLINK.get(), MAGIC_SYMLINK
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_MIME.get(), MAGIC_MIME
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_MIME_TYPE.get(), MAGIC_MIME_TYPE
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_MIME_ENCODING.get(),MAGIC_MIME_ENCODING
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_DEVICES.get(), MAGIC_DEVICES
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_CONTINUE.get(), MAGIC_CONTINUE
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_PRESERVE_ATIME.get(), MAGIC_PRESERVE_ATIME
    );
    Native::registerConstant<KindOfInt64>(
      s_FILEINFO_RAW.get(), MAGIC_RAW
    );
    HHVM_FE(finfo_open);
    HHVM_FE(finfo_buffer);
    HHVM_FE(finfo_file);
    HHVM_FE(finfo_set_flags);
    HHVM_FE(finfo_close);
    HHVM_FE(mime_content_type);
    loadSystemlib();
  }
} s_fileinfo_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(fileinfo);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
