/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/ext/fileinfo/libmagic/magic.h"

#include <folly/portability/Unistd.h>

namespace HPHP {
const StaticString s_finfo("finfo");

struct FileinfoResource : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(FileinfoResource)
  CLASSNAME_IS("file_info")
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit FileinfoResource(struct magic_set* magic) : m_magic(magic) {}
  ~FileinfoResource() override { close(); }
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

  return Variant(req::make<FileinfoResource>(magic));
}

static bool HHVM_FUNCTION(finfo_close, const OptResource& finfo) {
  cast<FileinfoResource>(finfo)->close();
  return true;
}

static bool HHVM_FUNCTION(finfo_set_flags, const OptResource& finfo, int64_t options) {
  auto magic = cast<FileinfoResource>(finfo)->getMagic();
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

static Variant
php_finfo_get_type(const OptResource& object, const Variant& what, int64_t options,
                   const Variant& /*context*/, int mode, int mimetype_emu) {
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
  } else if (object) {
    buffer = what.toString();
    magic = cast<FileinfoResource>(object)->getMagic();
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
      auto stream = cast<File>(what);
      if (!stream) {
        goto common;
      }

      auto streampos = stream->tell(); // remember stream position
      stream->seek(0, SEEK_SET);

      ret_val = magic_stream(magic, stream.get());

      stream->seek(streampos, SEEK_SET);
      break;
    }

    case FILEINFO_MODE_FILE:
    {
      if (buffer.empty()) {
        raise_warning("Empty filename or path");
        ret_val.reset();
        goto clean;
      }

      auto stream = File::Open(buffer, "rb");
      if (!stream) {
        ret_val.reset();
        goto clean;
      }

      struct stat st;
      if (stream->stat(&st)) {
        if (st.st_mode & S_IFDIR) {
          ret_val = mime_directory;
        } else {
          ret_val = magic_stream(magic, stream.get());
        }
      }
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
    const OptResource& finfo, const Variant& string,
    int64_t options, const Variant& context) {

  String s;
  if (!string.isNull()) {
    s = string.toString();
  }
  return php_finfo_get_type(
      finfo, s, options, context,
      FILEINFO_MODE_BUFFER, 0).toString();
}

static String HHVM_FUNCTION(finfo_file,
    const OptResource& finfo, const Variant& file_name,
    int64_t options, const Variant& context) {

  String fn;
  if (!file_name.isNull()) {
    fn = file_name.toString();
  }
  return php_finfo_get_type(
      finfo, fn, options, context,
      FILEINFO_MODE_FILE, 0).toString();
}

static String HHVM_FUNCTION(mime_content_type, const Variant& filename) {
  return php_finfo_get_type(
    OptResource{}, filename, 0, uninit_null(), -1, 1
  ).toString();
}

//////////////////////////////////////////////////////////////////////////////

struct fileinfoExtension final : Extension {
  fileinfoExtension() : Extension("fileinfo", "1.0.5-dev", NO_ONCALL_YET) {}
  void moduleInit() override {
    HHVM_RC_INT(FILEINFO_NONE, MAGIC_NONE);
    HHVM_RC_INT(FILEINFO_SYMLINK, MAGIC_SYMLINK);
    HHVM_RC_INT(FILEINFO_MIME, MAGIC_MIME);
    HHVM_RC_INT(FILEINFO_MIME_TYPE, MAGIC_MIME_TYPE);
    HHVM_RC_INT(FILEINFO_MIME_ENCODING,MAGIC_MIME_ENCODING);
    HHVM_RC_INT(FILEINFO_DEVICES, MAGIC_DEVICES);
    HHVM_RC_INT(FILEINFO_CONTINUE, MAGIC_CONTINUE);
    HHVM_RC_INT(FILEINFO_PRESERVE_ATIME, MAGIC_PRESERVE_ATIME);
    HHVM_RC_INT(FILEINFO_RAW, MAGIC_RAW);
    HHVM_FE(finfo_open);
    HHVM_FE(finfo_buffer);
    HHVM_FE(finfo_file);
    HHVM_FE(finfo_set_flags);
    HHVM_FE(finfo_close);
    HHVM_FE(mime_content_type);
  }
} s_fileinfo_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(fileinfo);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
