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

#include "hphp/runtime/ext/ext_hash.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/hash/hash_md.h"
#include "hphp/runtime/ext/hash/hash_sha.h"
#include "hphp/runtime/ext/hash/hash_ripemd.h"
#include "hphp/runtime/ext/hash/hash_whirlpool.h"
#include "hphp/runtime/ext/hash/hash_tiger.h"
#include "hphp/runtime/ext/hash/hash_snefru.h"
#include "hphp/runtime/ext/hash/hash_gost.h"
#include "hphp/runtime/ext/hash/hash_adler32.h"
#include "hphp/runtime/ext/hash/hash_crc32.h"
#include "hphp/runtime/ext/hash/hash_haval.h"
#include "hphp/runtime/ext/hash/hash_fnv1.h"
#include "hphp/runtime/ext/hash/hash_furc.h"
#include "hphp/runtime/ext/hash/hash_murmur.h"

#if defined(HPHP_OSS)
#define furc_hash furc_hash_internal
#else
#include "memcache/ch/hash.h"
#endif

namespace HPHP {

static class HashExtension : public Extension {
 public:
  HashExtension() : Extension("hash") { }
  virtual void moduleLoad(Hdf config) {
    HHVM_FE(hash);
    HHVM_FE(hash_algos);
    HHVM_FE(hash_file);
    HHVM_FE(hash_final);
    HHVM_FE(hash_init);
    HHVM_FE(hash_update);
    HHVM_FE(hash_copy);
    HHVM_FE(furchash_hphp_ext);
    HHVM_FE(hphp_murmurhash);
  }
} s_hash_extension;

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
    HashEngines["sha224"]     = HashEnginePtr(new hash_sha224());
    HashEngines["sha256"]     = HashEnginePtr(new hash_sha256());
    HashEngines["sha384"]     = HashEnginePtr(new hash_sha384());
    HashEngines["sha512"]     = HashEnginePtr(new hash_sha512());
    HashEngines["ripemd128"]  = HashEnginePtr(new hash_ripemd128());
    HashEngines["ripemd160"]  = HashEnginePtr(new hash_ripemd160());
    HashEngines["ripemd256"]  = HashEnginePtr(new hash_ripemd256());
    HashEngines["ripemd320"]  = HashEnginePtr(new hash_ripemd320());
    HashEngines["whirlpool"]  = HashEnginePtr(new hash_whirlpool());
#ifdef FACEBOOK
    HashEngines["tiger128,3-fb"]
                              = HashEnginePtr(new hash_tiger(true, 128, true));
    // Temporarily leave tiger128,3 algo inverting its hash output
    // to retain BC pending conversion of user code to correct endianness
    // sgolemon(2013-04-30)
    HashEngines["tiger128,3"] = HashEnginePtr(new hash_tiger(true, 128, true));
#else
    HashEngines["tiger128,3"] = HashEnginePtr(new hash_tiger(true, 128));
#endif
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
    HashEngines["fnv132"]     = HashEnginePtr(new hash_fnv132(false));
    HashEngines["fnv1a32"]    = HashEnginePtr(new hash_fnv132(true));
    HashEngines["fnv164"]     = HashEnginePtr(new hash_fnv164(false));
    HashEngines["fnv1a64"]    = HashEnginePtr(new hash_fnv164(true));
  }
};

static HashEngineMapInitializer s_engine_initializer;

///////////////////////////////////////////////////////////////////////////////
// hash context

class HashContext : public SweepableResourceData {
public:
  CLASSNAME_IS("Hash Context")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  HashContext(HashEnginePtr ops_, void *context_, int options_)
    : ops(ops_), context(context_), options(options_), key(nullptr) {
  }

  explicit HashContext(const HashContext* ctx) {
    assert(ctx->ops);
    assert(ctx->ops->context_size >= 0);
    ops = ctx->ops;
    context = malloc(ops->context_size);
    ops->hash_copy(context, ctx->context);
    options = ctx->options;
    key = ctx->key ? strdup(ctx->key) : nullptr;
  }

  ~HashContext() {
    HashContext::sweep();
  }

  void sweep() FOLLY_OVERRIDE {
    /* Just in case the algo has internally allocated resources */
    if (context) {
      assert(ops->digest_size >= 0);
      unsigned char dummy[ops->digest_size];
      ops->hash_final(dummy, context);
      free(context);
    }

    free(key);
  }

  HashEnginePtr ops;
  void *context;
  int options;
  char *key;
};

///////////////////////////////////////////////////////////////////////////////
// hash functions

Array HHVM_FUNCTION(hash_algos) {
  Array ret;
  for (HashEngineMap::const_iterator iter = HashEngines.begin();
       iter != HashEngines.end(); ++iter) {
    ret.append(String(iter->first));
  }
  return ret;
}

static HashEnginePtr php_hash_fetch_ops(const String& algo) {
  HashEngineMap::const_iterator iter =
    HashEngines.find(f_strtolower(algo).data());
  if (iter == HashEngines.end()) {
    return HashEnginePtr();
  }
  return iter->second;
}

static Variant php_hash_do_hash(const String& algo, const String& data,
                                bool isfilename,
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
    for (Variant chunk = f_fread(f.toResource(), 1024);
         !is_empty_string(chunk);
         chunk = f_fread(f.toResource(), 1024)) {
      String schunk = chunk.toString();
      ops->hash_update(context, (unsigned char *)schunk.data(), schunk.size());
    }
  } else {
    ops->hash_update(context, (unsigned char *)data.data(), data.size());
  }

  String raw = String(ops->digest_size, ReserveString);
  char *digest = raw.bufferSlice().ptr;
  ops->hash_final((unsigned char *)digest, context);
  free(context);

  raw.setSize(ops->digest_size);
  if (raw_output) {
    return raw;
  }
  return f_bin2hex(raw);
}

Variant HHVM_FUNCTION(hash, const String& algo, const String& data,
                            bool raw_output /* = false */) {
  return php_hash_do_hash(algo, data, false, raw_output);
}

Variant HHVM_FUNCTION(hash_file, const String& algo, const String& filename,
                                 bool raw_output /* = false */) {
  return php_hash_do_hash(algo, filename, true, raw_output);
}

static char *prepare_hmac_key(HashEnginePtr ops, void *context,
                              const String& key) {
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

Variant HHVM_FUNCTION(hash_init, const String& algo,
                                 int64_t options /* = 0 */,
                                 const String& key /* = null_string */) {
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

  const auto hash = new HashContext(ops, context, options);
  if (options & k_HASH_HMAC) {
    hash->key = prepare_hmac_key(ops, context, key);
  }
  return Resource(hash);
}

bool HHVM_FUNCTION(hash_update, CResRef context, const String& data) {
  HashContext *hash = context.getTyped<HashContext>();
  hash->ops->hash_update(hash->context, (unsigned char *)data.data(),
                         data.size());
  return true;
}

String HHVM_FUNCTION(hash_final, CResRef context,
                                 bool raw_output /* = false */) {
  HashContext *hash = context.getTyped<HashContext>();

  String raw = String(hash->ops->digest_size, ReserveString);
  char *digest = raw.bufferSlice().ptr;
  hash->ops->hash_final((unsigned char *)digest, hash->context);
  if (hash->options & k_HASH_HMAC) {
    finalize_hmac_key(hash->key, hash->ops, hash->context, digest);
    hash->key = NULL;
  }
  free(hash->context);
  hash->context = NULL;

  raw.setSize(hash->ops->digest_size);
  if (raw_output) {
    return raw;
  }
  return f_bin2hex(raw);
}

Resource HHVM_FUNCTION(hash_copy, CResRef context) {
  HashContext *oldhash = context.getTyped<HashContext>();
  auto const hash = new HashContext(oldhash);
  return Resource(hash);
}

int64_t HHVM_FUNCTION(furchash_hphp_ext, const String& key,
                                         int64_t len, int64_t nPart) {
  len = std::max<int64_t>(std::min<int64_t>(len, key.size()), 0);
  return furc_hash(key.data(), len, nPart);
}

int64_t HHVM_FUNCTION(hphp_murmurhash, const String& key,
                                       int64_t len, int64_t seed) {
  len = std::max<int64_t>(std::min<int64_t>(len, key.size()), 0);
  return murmur_hash_64A(key.data(), len, seed);
}

///////////////////////////////////////////////////////////////////////////////
}
