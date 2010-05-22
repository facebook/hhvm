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

#include <runtime/ext/ext_hash.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/hash/hash_md.h>
#include <runtime/ext/hash/hash_sha.h>
#include <runtime/ext/hash/hash_ripemd.h>
#include <runtime/ext/hash/hash_whirlpool.h>
#include <runtime/ext/hash/hash_tiger.h>
#include <runtime/ext/hash/hash_snefru.h>
#include <runtime/ext/hash/hash_gost.h>
#include <runtime/ext/hash/hash_adler32.h>
#include <runtime/ext/hash/hash_crc32.h>
#include <runtime/ext/hash/hash_haval.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(hash);
///////////////////////////////////////////////////////////////////////////////
// hash engines

static HashEngineMap HashEngines;

class HashEngineMapInitializer {
public:
  HashEngineMapInitializer() {
    HashEngines["md2"]        = HashEnginePtr(new hash_md2());
    HashEngines["md4"]        = HashEnginePtr(new hash_md4());
    HashEngines["md5"]        = HashEnginePtr(new hash_md5());
    HashEngines["sha1"]       = HashEnginePtr(new hash_sha1());
    HashEngines["sha256"]     = HashEnginePtr(new hash_sha256());
    HashEngines["sha384"]     = HashEnginePtr(new hash_sha384());
    HashEngines["sha512"]     = HashEnginePtr(new hash_sha512());
    HashEngines["ripemd128"]  = HashEnginePtr(new hash_ripemd128());
    HashEngines["ripemd160"]  = HashEnginePtr(new hash_ripemd160());
    HashEngines["ripemd256"]  = HashEnginePtr(new hash_ripemd256());
    HashEngines["ripemd320"]  = HashEnginePtr(new hash_ripemd320());
    HashEngines["whirlpool"]  = HashEnginePtr(new hash_whirlpool());
    HashEngines["tiger128,3"] = HashEnginePtr(new hash_tiger(true, 128));
    HashEngines["tiger160,3"] = HashEnginePtr(new hash_tiger(true, 160));
    HashEngines["tiger192,3"] = HashEnginePtr(new hash_tiger(true, 192));
    HashEngines["tiger128,4"] = HashEnginePtr(new hash_tiger(false, 128));
    HashEngines["tiger160,4"] = HashEnginePtr(new hash_tiger(false, 160));
    HashEngines["tiger192,4"] = HashEnginePtr(new hash_tiger(false, 192));
    HashEngines["snefru"]     = HashEnginePtr(new hash_snefru());
    HashEngines["gost"]       = HashEnginePtr(new hash_gost());
    HashEngines["adler32"]    = HashEnginePtr(new hash_adler32());
    HashEngines["crc32"]      = HashEnginePtr(new hash_crc32(false));
    HashEngines["crc32b"]     = HashEnginePtr(new hash_crc32(true));
    HashEngines["haval128,3"] = HashEnginePtr(new hash_haval(3,128));
    HashEngines["haval160,3"] = HashEnginePtr(new hash_haval(3,160));
    HashEngines["haval192,3"] = HashEnginePtr(new hash_haval(3,192));
    HashEngines["haval224,3"] = HashEnginePtr(new hash_haval(3,224));
    HashEngines["haval256,3"] = HashEnginePtr(new hash_haval(3,256));
    HashEngines["haval128,4"] = HashEnginePtr(new hash_haval(4,128));
    HashEngines["haval160,4"] = HashEnginePtr(new hash_haval(4,160));
    HashEngines["haval192,4"] = HashEnginePtr(new hash_haval(4,192));
    HashEngines["haval224,4"] = HashEnginePtr(new hash_haval(4,224));
    HashEngines["haval256,4"] = HashEnginePtr(new hash_haval(4,256));
    HashEngines["haval128,5"] = HashEnginePtr(new hash_haval(5,128));
    HashEngines["haval160,5"] = HashEnginePtr(new hash_haval(5,160));
    HashEngines["haval192,5"] = HashEnginePtr(new hash_haval(5,192));
    HashEngines["haval224,5"] = HashEnginePtr(new hash_haval(5,224));
    HashEngines["haval256,5"] = HashEnginePtr(new hash_haval(5,256));
  }
};

static HashEngineMapInitializer s_engine_initializer;

///////////////////////////////////////////////////////////////////////////////
// hash context

class HashContext : public SweepableResourceData {
public:
  // overriding ResourceData
  const char *o_getClassName() const { return "Hash Context";}

  HashContext(HashEnginePtr ops_, void *context_, int options_)
    : ops(ops_), context(context_), options(options_), key(NULL) {
  }

  ~HashContext() {
    /* Just in case the algo has internally allocated resources */
    if (context) {
      unsigned char *dummy = (unsigned char *)malloc(ops->digest_size);
      ops->hash_final(dummy, context);
      free(dummy);
      free(context);
    }

    if (key) {
      memset(key, 0, ops->block_size);
      free(key);
    }
  }

  HashEnginePtr ops;
  void *context;
  int options;
  char *key;
};

///////////////////////////////////////////////////////////////////////////////
// hash functions

Array f_hash_algos() {
  Array ret;
  for (HashEngineMap::const_iterator iter = HashEngines.begin();
       iter != HashEngines.end(); ++iter) {
    ret.append(String(iter->first));
  }
  return ret;
}

static HashEnginePtr php_hash_fetch_ops(CStrRef algo) {
  HashEngineMap::const_iterator iter =
    HashEngines.find(StringUtil::ToLower(algo).data());
  if (iter == HashEngines.end()) {
    return HashEnginePtr();
  }
  return iter->second;
}

static Variant php_hash_do_hash(CStrRef algo, CStrRef data, bool isfilename,
                                bool raw_output) {
  HashEnginePtr ops = php_hash_fetch_ops(algo);
  if (!ops) {
    raise_warning("Unknown hashing algorithm: %s", algo.data());
    return false;
  }
  Variant f;
  if (isfilename) {
    f = f_fopen(data, "rb");
    if (same(f, false)) {
      return false;
    }
  }

  void *context = malloc(ops->context_size);
  ops->hash_init(context);

  if (isfilename) {
    for (Variant chunk = f_fread(f, 1024); !same(chunk, "");
         chunk = f_fread(f, 1024)) {
      String schunk = chunk.toString();
      ops->hash_update(context, (unsigned char *)schunk.data(), schunk.size());
    }
  } else {
    ops->hash_update(context, (unsigned char *)data.data(), data.size());
  }

  char *digest = (char*)malloc(ops->digest_size + 1);
  ops->hash_final((unsigned char *)digest, context);
  free(context);

  digest[ops->digest_size] = '\0';
  String raw(digest, ops->digest_size, AttachString);
  if (raw_output) {
    return raw;
  }
  return StringUtil::HexEncode(raw);
}

Variant f_hash(CStrRef algo, CStrRef data, bool raw_output /* = false */) {
  return php_hash_do_hash(algo, data, false, raw_output);
}

Variant f_hash_file(CStrRef algo, CStrRef filename,
                   bool raw_output /* = false */) {
  return php_hash_do_hash(algo, filename, true, raw_output);
}

static char *prepare_hmac_key(HashEnginePtr ops, void *context, CStrRef key) {
  char *K = (char*)malloc(ops->block_size);
  memset(K, 0, ops->block_size);
  if (key.size() > ops->block_size) {
    /* Reduce the key first */
    ops->hash_update(context, (unsigned char *)key.data(), key.size());
    ops->hash_final((unsigned char *)K, context);
    /* Make the context ready to start over */
    ops->hash_init(context);
  } else {
    memcpy(K, key.data(), key.size());
  }

  /* XOR ipad */
  for (int i = 0; i < ops->block_size; i++) {
    K[i] ^= 0x36;
  }
  ops->hash_update(context, (unsigned char *)K, ops->block_size);
  return K;
}

static void finalize_hmac_key(char *K, HashEnginePtr ops, void *context,
                              char *digest) {
  /* Convert K to opad -- 0x6A = 0x36 ^ 0x5C */
  for (int i = 0; i < ops->block_size; i++) {
    K[i] ^= 0x6A;
  }

  /* Feed this result into the outter hash */
  ops->hash_init(context);
  ops->hash_update(context, (unsigned char *)K, ops->block_size);
  ops->hash_update(context, (unsigned char *)digest, ops->digest_size);
  ops->hash_final((unsigned char *)digest, context);

  /* Zero the key */
  memset(K, 0, ops->block_size);
  free(K);
}

static Variant php_hash_do_hash_hmac(CStrRef algo, CStrRef data,
                                     bool isfilename, CStrRef key,
                                     bool raw_output /* = false */) {
  HashEnginePtr ops = php_hash_fetch_ops(algo);
  if (!ops) {
    raise_warning("Unknown hashing algorithm: %s", algo.data());
    return false;
  }
  Variant f;
  if (isfilename) {
    f = f_fopen(data, "rb");
    if (same(f, false)) {
      return false;
    }
  }

  void *context = malloc(ops->context_size);
  ops->hash_init(context);

  char *K = prepare_hmac_key(ops, context, key);

  if (isfilename) {
    for (Variant chunk = f_fread(f, 1024); !same(chunk, "");
         chunk = f_fread(f, 1024)) {
      String schunk = chunk.toString();
      ops->hash_update(context, (unsigned char *)schunk.data(), schunk.size());
    }
  } else {
    ops->hash_update(context, (unsigned char *)data.data(), data.size());
  }

  char *digest = (char*)malloc(ops->digest_size + 1);
  ops->hash_final((unsigned char *)digest, context);
  finalize_hmac_key(K, ops, context, digest);
  free(context);

  digest[ops->digest_size] = '\0';
  String raw(digest, ops->digest_size, AttachString);
  if (raw_output) {
    return raw;
  }
  return StringUtil::HexEncode(raw);
}

Variant f_hash_hmac(CStrRef algo, CStrRef data, CStrRef key,
                   bool raw_output /* = false */) {
  return php_hash_do_hash_hmac(algo, data, false, key, raw_output);
}

Variant f_hash_hmac_file(CStrRef algo, CStrRef filename, CStrRef key,
                        bool raw_output /* = false */) {
  return php_hash_do_hash_hmac(algo, filename, true, key, raw_output);
}

Variant f_hash_init(CStrRef algo, int options /* = 0 */,
                   CStrRef key /* = null_string */) {
  HashEnginePtr ops = php_hash_fetch_ops(algo);
  if (!ops) {
    raise_warning("Unknown hashing algorithm: %s", algo.data());
    return false;
  }

  if ((options & k_HASH_HMAC) && key.empty()) {
    raise_warning("HMAC requested without a key");
    return false;
  }

  void *context = malloc(ops->context_size);
  ops->hash_init(context);

  HashContext *hash = new HashContext(ops, context, options);
  if (options & k_HASH_HMAC) {
    hash->key = prepare_hmac_key(ops, context, key);
  }
  return Object(hash);
}

bool f_hash_update(CObjRef context, CStrRef data) {
  HashContext *hash = context.getTyped<HashContext>();
  hash->ops->hash_update(hash->context, (unsigned char *)data.data(),
                         data.size());
  return true;
}

bool f_hash_update_file(CObjRef init_context, CStrRef filename,
                        CObjRef stream_context /* = null */) {
  Variant f = f_fopen(filename, "rb");
  if (same(f, false)) {
    return false;
  }

  HashContext *hash = init_context.getTyped<HashContext>();
  for (Variant chunk = f_fread(f, 1024); !same(chunk, "");
       chunk = f_fread(f, 1024)) {
    String schunk = chunk.toString();
    hash->ops->hash_update(hash->context, (unsigned char *)schunk.data(),
                           schunk.size());
  }
  return true;
}

int f_hash_update_stream(CObjRef context, CObjRef handle,
                         int length /* = -1 */) {
  HashContext *hash = context.getTyped<HashContext>();
  int didread = 0;
  while (length) {
    Variant chunk = f_fread(handle, length > 0 ? length : 1024);
    if (same(chunk, "")) {
      return didread;
    }
    String schunk = chunk.toString();
    hash->ops->hash_update(hash->context, (unsigned char *)schunk.data(),
                           schunk.size());
    didread += schunk.size();
    length -= schunk.size();
  }
  return didread;
}

String f_hash_final(CObjRef context, bool raw_output /* = false */) {
  HashContext *hash = context.getTyped<HashContext>();

  char *digest = (char*)malloc(hash->ops->digest_size + 1);
  hash->ops->hash_final((unsigned char *)digest, hash->context);
  if (hash->options & k_HASH_HMAC) {
    finalize_hmac_key(hash->key, hash->ops, hash->context, digest);
    hash->key = NULL;
  }
  free(hash->context);
  hash->context = NULL;

  digest[hash->ops->digest_size] = '\0';
  String raw(digest, hash->ops->digest_size, AttachString);
  if (raw_output) {
    return raw;
  }
  return StringUtil::HexEncode(raw);
}

///////////////////////////////////////////////////////////////////////////////
}
