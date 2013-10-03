/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <memory>

#include "folly/ScopeGuard.h"

#include "hphp/runtime/base/data-stream-wrapper.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

File* DataStreamWrapper::open(const String& filename, const String& mode,
                              int options, CVarRef context) {

  // @todo: check allow_url_include?

  const char* data = filename.data();
  int data_len = filename.length();
  bool base64 = false;
  if (strncmp(data, "data:", sizeof("data:") - 1)) {
    return nullptr;
  }
  data += sizeof("data:") - 1;
  data_len -= sizeof("data:") - 1;

  // RFC 2397 specifies 'data:' as the prefix,
  // but zend's PHP supports 'data://' as well
  if (data_len >= 2 && data[0] == '/' && data[1] == '/') {
    data_len -= 2;
    data += 2;
  }

  char* comma = (char*)memchr(data, ',', data_len);
  if (comma == nullptr) {
    raise_warning("rfc2397: missing comma");
    return nullptr;
  }

  if (comma != data) {
    // we have meta
    ssize_t meta_len = comma - data;
    data_len -= meta_len;
    char* semi = (char*)memchr(data, ';', meta_len);
    char* slash = (char*)memchr(data, '/', meta_len);

    if (!slash && !semi) {
      raise_warning("rfc2397: invalid meta data");
      return nullptr;
    }

    if (!semi) {
      // only media type (type/subtype,data)
      ssize_t media_len = comma - data;
      meta_len -= media_len;
      data += media_len;
    } else if (slash && slash < semi) {
      // media type + param (type/subtype;param,data)
      ssize_t media_len = semi - data;
      meta_len -= media_len;
      data += media_len;
    } else {
      // no media type (;base64,data)
      if (semi != data // ex. foo;base64,data
          || meta_len != sizeof(";base64") - 1 // ex. ;something,data
          || memcmp(data, ";base64",
                    sizeof(";base64") - 1)) { // ex. ;base65,data
          raise_warning("rfc2397: invalid meta data");
          return nullptr;
        }
    }

    assert(data == comma || data == semi);
    // eat parameters, and figure out if we have ';base64'
    while (semi && (data == semi)) {
      data++;
      meta_len--;
      char* equals = (char*)memchr(data, '=', meta_len);
      semi = (char*)memchr(data, ';', meta_len);
      if (!equals || (semi && semi < data)) {
        // no equals, so either 'base64' or its bad
        if (meta_len != sizeof("base64") - 1 ||
            memcmp(data, "base64", sizeof("base64")-1)) {
          raise_warning("rfc2396: invalid parameter");
          return nullptr;
        }
        // it's "base64", we're done
        base64 = true;
        meta_len -= sizeof("base64") - 1;
        data += sizeof("base64") - 1;
        break;
      }
      // there's a parameter
      if (semi) {
        meta_len -= semi - data + 1;
        data = semi;
      } /* else, we're done with meta */
    }
  }
  data = comma + 1;
  data_len -= 1;
  std::unique_ptr<MemFile> file;
  int len = data_len;
  char* decoded = nullptr;

  SCOPE_EXIT {
    if (decoded) {
      free(decoded);
    }
  };

  if (base64) {
    decoded = string_base64_decode(data, len, true);
    if (!decoded) {
      raise_warning("unable to decode base64 data");
      return nullptr;
    }
  } else {
    decoded = url_decode(data, len);
  }
  file = std::unique_ptr<MemFile>(NEWOBJ(MemFile)(decoded, len));
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
