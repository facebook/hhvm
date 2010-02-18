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
#include <cpp/base/base_includes.h>
#include <cpp/base/variable_unserializer.h>
#include <cpp/ext/ext_fb.h>

#include "ext_php_mcc.h"
#include "types.h"

using namespace HPHP;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// serialization

bool phpmcc_fb_serialize_value(MccResourcePtr &phpmcc,
                               CStrRef key,
                               phpmcc_flags_t& flags,
                               CVarRef unserialized_value,
                               String& serialized_value) {
  bool result = false;

  /* Quick check for object - which are not handled by fb serialization. */
  if (unserialized_value.is(KindOfObject)) {
    return result;
  }

  bool use_fb = phpmcc->m_fb_serialize_enabled &&
    phpmcc->m_fb_serialize_available;

  if (use_fb) {
    Variant sval = f_fb_thrift_serialize(unserialized_value);
    if (sval.isNull()) {
      result = false;
    } else {
      serialized_value = sval;
      flags = (phpmcc_flags_t)(flags | phpmcc_fb_serialized);
      result = true;
    }
  }
  return result;
}

bool phpmcc_php_serialize_value(MccResourcePtr &phpmcc,
                                CStrRef key,
                                phpmcc_flags_t& flags,
                                CVarRef unserialized_value,
                                String& serialized_value) {
  Variant v = f_serialize(unserialized_value);
  if (v.isString()) {
    serialized_value = v.toString();
    flags = (phpmcc_flags_t) (flags | phpmcc_serialized);
    return true;
  }
  return false;
}

int phpmcc_unserialize_value(MccResourcePtr &phpmcc,
                             const phpmcc_flags_t flags,
                             const char *serialized_value,
                             int serialized_len,
                             Variant& unserialized_value) {
  // 0 is fail, 1 is success
  if (serialized_len == 0) {
    unserialized_value = null;
    return 0;
  }

  if (flags & phpmcc_fb_serialized) {
    int pos = 0;
    if ((fb_unserialize_from_buffer(unserialized_value,
                                    serialized_value,
                                    serialized_len,
                                    &pos))) {
      return 0;
    }
    return 1;
  }

  if (flags & phpmcc_serialized) {
    std::istringstream in(std::string(serialized_value, serialized_len));
    VariableUnserializer vu(in);
    try {
      unserialized_value = vu.unserialize();
    } catch (Exception &e) {
      return 0;
    }
    return 1;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// compression

#define NZLIB_MAGIC 0x6e7a6c69 /* nzli */
/* The new compression format stores a magic number and the size
   of the uncompressed object.  The magic number is stored to make sure
   bad values do not cause us to allocate bogus or extremely large amounts
   of memory when encountering an object with the new format. */
typedef struct _nzlib_format {
    uint32_t magic;
    uint32_t uncompressed_sz;
    Bytef buf[0];
} nzlib_format;

int phpmcc_zlib_compress_value(MccResourcePtr &phpmcc,
                               phpmcc_flags_t& flags,
                               CStrRef uncompressed_value,
                               String& compressed_value) {
  int result = 0;

  if (phpmcc->m_zlib_compression_enabled) {
    nzlib_format *compressed_object = NULL;
    uLong compressed_len;
    uLong internal_compressed_len;
    int status;
    char* compressed_buf;

    /* room for \0 and our header. taken from php_zlib */
    compressed_len = uncompressed_value.size() +
      (uncompressed_value.size() / 1000) + 15 + 5;
    compressed_object = (nzlib_format *)malloc(compressed_len);
    if (!compressed_object)
      goto epilogue;

    /* We want to steal the allocated compressed_value buf for the returned
       compressed_value zval, so compress into the compressed_object buf for
       nzlib compression or at the beginning of compressed_object itself if
       we aren't going to prepend a header. */

    if (phpmcc->m_nzlib_compression) {
      compressed_object->magic = htonl(NZLIB_MAGIC);
      compressed_object->uncompressed_sz = htonl(uncompressed_value.size());
      compressed_buf = (char*)compressed_object->buf;
      compressed_len -= sizeof(nzlib_format);
    } else {
      compressed_buf = (char*)compressed_object;
    }

    status = compress((Bytef*)compressed_buf, (uLongf *)&compressed_len,
                      (Bytef*)uncompressed_value.data(),
                      uncompressed_value.size());
    if (status != Z_OK) {
      goto epilogue;
    }

    /* XXX: Should we check for error? php_zlib does not. */
    internal_compressed_len = compressed_len +
      (phpmcc->m_nzlib_compression ? sizeof(*compressed_object) : 0);
    compressed_object = (nzlib_format*)realloc(compressed_object,
                                               internal_compressed_len + 1);
    ((char*)compressed_object)[internal_compressed_len] = '\0';
    compressed_value = String((char*)compressed_object,
                              internal_compressed_len, AttachString);
    flags =
      (phpmcc_flags_t)(flags |
                       ((phpmcc->m_nzlib_compression ? phpmcc_nzlib_compressed
                         : phpmcc_compressed)));

    /* We stole the buf from compressed object for compressed_value,
       so null it out so we don't attempt to free it. */

    compressed_object = NULL;

    result = 1;

  epilogue:

    if (!result) {
      if (compressed_object != NULL) {
        free(compressed_object);
      }
    }
  }

  return result;
}

int phpmcc_zlib_uncompress_value(MccResourcePtr &phpmcc,
                                 uint32_t flags,
                                 const char *compressed_value,
                                 int compressed_len,
                                 String& uncompressed_value) {
  int result = 1;
  nzlib_format *nzlib_object;
  Bytef *compressed = NULL;
  Bytef *uncompressed = NULL;
  uint32_t factor = 1;
  uint32_t maxfactor = 16;
  uLong compressed_sz;
  uLong uncompressed_sz = 0;
  uLong length;
  int status = 0;

  if (flags & phpmcc_nzlib_compressed) {
    nzlib_object  = (nzlib_format *) compressed_value;

    if (ntohl(nzlib_object->magic) != NZLIB_MAGIC) {
      result = 0;
      goto epilogue;
    }
    compressed = nzlib_object->buf;
    compressed_sz = compressed_len - sizeof(*nzlib_object);
    uncompressed_sz = ntohl(nzlib_object->uncompressed_sz);
  } else {
    compressed = (Bytef *)compressed_value;
    compressed_sz = compressed_len;
  }

  /* Taken from php/ext/zlib/zlib.c
     zlib::uncompress() wants to know the output data length
     if none was given as a parameter
     we try from input length * 2 up to input length   2^15
     doubling it whenever it wasn't big enough
     that should be enough for all real life cases */
  do {
    length = uncompressed_sz ? uncompressed_sz :
      compressed_sz * (1 << factor++);
    uncompressed = (Bytef*)realloc(uncompressed, length);
    status = uncompress(uncompressed, &length, compressed, compressed_sz);
  } while (status == Z_BUF_ERROR && !uncompressed_sz && factor < maxfactor);
  if (status != Z_OK) {
    result = 0;
    goto epilogue;
  }
  /* Make sure the uncompressed_sz matches the length returned by zlib */
  if (uncompressed_sz != 0 && uncompressed_sz != length) {
    result = 0;
    goto epilogue;
  }

  /* XXX: Should we check for error? php_zlib does not. */
  uncompressed = (Bytef*)realloc(uncompressed, length + 1); /* space for \0 */
  uncompressed[length] = '\0';

  if (uncompressed != NULL) {
    uncompressed_value = String((char *)uncompressed, length, AttachString);
    uncompressed = NULL;
  }

 epilogue:
  if (!result) {
    /* Free the uncompressed if it exists */
    if (uncompressed != NULL) {
      free(uncompressed);
    }
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}
