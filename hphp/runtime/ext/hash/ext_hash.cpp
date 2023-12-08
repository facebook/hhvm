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

#include "hphp/runtime/ext/hash/ext_hash.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/ext/hash/hash_adler32.h"
#include "hphp/runtime/ext/hash/hash_crc32.h"
#include "hphp/runtime/ext/hash/hash_fnv1.h"
#include "hphp/runtime/ext/hash/hash_furc.h"
#include "hphp/runtime/ext/hash/hash_gost.h"
#include "hphp/runtime/ext/hash/hash_haval.h"
#include "hphp/runtime/ext/hash/hash_joaat.h"
#include "hphp/runtime/ext/hash/hash_keccak.h"
#include "hphp/runtime/ext/hash/hash_md.h"
#include "hphp/runtime/ext/hash/hash_murmur.h"
#include "hphp/runtime/ext/hash/hash_ripemd.h"
#include "hphp/runtime/ext/hash/hash_sha.h"
#include "hphp/runtime/ext/hash/hash_snefru.h"
#include "hphp/runtime/ext/hash/hash_tiger.h"
#include "hphp/runtime/ext/hash/hash_whirlpool.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/hash/hash_blake3.h"
#include "hphp/runtime/ext/hash/hash_keyed_blake3.h"

#include <algorithm>
#include <memory>

#if defined(HPHP_OSS)
#define furc_hash(a, b, c) furc_hash_internal(                          \
    (a), (b),                                                           \
    uint64_t(c) > furc_maximum_pool_size() ? furc_maximum_pool_size() : c)
#else
#include "mcrouter/lib/fbi/hash.h" // @nolint
#endif

namespace HPHP {

static struct HashExtension final : Extension {
  HashExtension() : Extension("hash", "1.0", NO_ONCALL_YET) { }
  void moduleInit() override;
} s_hash_extension;

///////////////////////////////////////////////////////////////////////////////
// hash engines

static HashEngineMap HashEngines;
using HashEnginePtr = std::shared_ptr<HashEngine>;

struct HashEngineMapInitializer {
  HashEngineMapInitializer() {
    HashEngines["md2"]          = HashEnginePtr(new hash_md2());
    HashEngines["md4"]          = HashEnginePtr(new hash_md4());
    HashEngines["md5"]          = HashEnginePtr(new hash_md5());
    HashEngines["sha1"]         = HashEnginePtr(new hash_sha1());
    HashEngines["sha224"]       = HashEnginePtr(new hash_sha224());
    HashEngines["sha256"]       = HashEnginePtr(new hash_sha256());
    HashEngines["sha384"]       = HashEnginePtr(new hash_sha384());
    HashEngines["sha512"]       = HashEnginePtr(new hash_sha512());
    HashEngines["ripemd128"]    = HashEnginePtr(new hash_ripemd128());
    HashEngines["ripemd160"]    = HashEnginePtr(new hash_ripemd160());
    HashEngines["ripemd256"]    = HashEnginePtr(new hash_ripemd256());
    HashEngines["ripemd320"]    = HashEnginePtr(new hash_ripemd320());
    HashEngines["whirlpool"]    = HashEnginePtr(new hash_whirlpool());
#ifdef HHVM_FACEBOOK
    // The original version of tiger got the endianness backwards
    // This fb-specific version remains for backward compatibility
    HashEngines["tiger128,3-fb"]
                                = HashEnginePtr(new hash_tiger(true, 128, true));
#endif
    HashEngines["tiger128,3"]   = HashEnginePtr(new hash_tiger(true, 128));
    HashEngines["tiger160,3"]   = HashEnginePtr(new hash_tiger(true, 160));
    HashEngines["tiger192,3"]   = HashEnginePtr(new hash_tiger(true, 192));
    HashEngines["tiger128,4"]   = HashEnginePtr(new hash_tiger(false, 128));
    HashEngines["tiger160,4"]   = HashEnginePtr(new hash_tiger(false, 160));
    HashEngines["tiger192,4"] = HashEnginePtr(new hash_tiger(false, 192));

    HashEngines["snefru"]       = HashEnginePtr(new hash_snefru());
    HashEngines["gost"]         = HashEnginePtr(new hash_gost());
    HashEngines["joaat"]        = HashEnginePtr(new hash_joaat());
#ifdef HHVM_FACEBOOK
    HashEngines["adler32-fb"]   = HashEnginePtr(new hash_adler32(true));
#endif
    HashEngines["adler32"]      = HashEnginePtr(new hash_adler32(false));
    HashEngines["crc32"]        = HashEnginePtr(new hash_crc32(Crc32Variant::Crc32));
    HashEngines["crc32b"]       = HashEnginePtr(new hash_crc32(Crc32Variant::Crc32B));
    HashEngines["crc32c"]       = HashEnginePtr(new hash_crc32(Crc32Variant::Crc32C));
    HashEngines["haval128,3"]   = HashEnginePtr(new hash_haval(3,128));
    HashEngines["haval160,3"]   = HashEnginePtr(new hash_haval(3,160));
    HashEngines["haval192,3"]   = HashEnginePtr(new hash_haval(3,192));
    HashEngines["haval224,3"]   = HashEnginePtr(new hash_haval(3,224));
    HashEngines["haval256,3"]   = HashEnginePtr(new hash_haval(3,256));
    HashEngines["haval128,4"]   = HashEnginePtr(new hash_haval(4,128));
    HashEngines["haval160,4"]   = HashEnginePtr(new hash_haval(4,160));
    HashEngines["haval192,4"]   = HashEnginePtr(new hash_haval(4,192));
    HashEngines["haval224,4"]   = HashEnginePtr(new hash_haval(4,224));
    HashEngines["haval256,4"]   = HashEnginePtr(new hash_haval(4,256));
    HashEngines["haval128,5"]   = HashEnginePtr(new hash_haval(5,128));
    HashEngines["haval160,5"]   = HashEnginePtr(new hash_haval(5,160));
    HashEngines["haval192,5"]   = HashEnginePtr(new hash_haval(5,192));
    HashEngines["haval224,5"]   = HashEnginePtr(new hash_haval(5,224));
    HashEngines["haval256,5"]   = HashEnginePtr(new hash_haval(5,256));
    HashEngines["fnv132"]       = HashEnginePtr(new hash_fnv132(false));
    HashEngines["fnv1a32"]      = HashEnginePtr(new hash_fnv132(true));
    HashEngines["fnv164"]       = HashEnginePtr(new hash_fnv164(false));
    HashEngines["fnv1a64"]      = HashEnginePtr(new hash_fnv164(true));
    HashEngines["sha3-224"]     = HashEnginePtr(new hash_keccak( 448, 28));
    HashEngines["sha3-256"]     = HashEnginePtr(new hash_keccak( 512, 32));
    HashEngines["sha3-384"]     = HashEnginePtr(new hash_keccak( 768, 48));
    HashEngines["sha3-512"]     = HashEnginePtr(new hash_keccak(1024, 64));
    HashEngines["blake3"]       = HashEnginePtr(new hash_blake3());
  #ifndef HPHP_OSS
    HashEngines["keyed-blake3"] = HashEnginePtr(new hash_keyed_blake3());
  #endif
  }
};

static HashEngineMapInitializer s_engine_initializer;

///////////////////////////////////////////////////////////////////////////////
// hash context

struct HashContext : SweepableResourceData {
  HashContext(HashEnginePtr ops_, void *context_, int options_)
    : ops(ops_), context(context_), options(options_), key(nullptr) {
  }

  explicit HashContext(req::ptr<HashContext>&& ctx) {
    assertx(ctx->ops);
    assertx(ctx->ops->context_size >= 0);
    ops = ctx->ops;
    context = req::malloc_noptrs(ops->context_size);
    ops->hash_copy(context, ctx->context);
    options = ctx->options;
    if (ctx->key) {
      key = static_cast<char*>(req::malloc_noptrs(ops->block_size));
      memcpy(key, ctx->key, ops->block_size);
    } else {
      key = nullptr;
    }
  }

  ~HashContext() override {
    /* Just in case the algo has internally allocated resources */
    if (context) {
      assertx(ops->digest_size >= 0);
      unsigned char* dummy = (unsigned char*)alloca(
        sizeof(unsigned char) * ops->digest_size);
      ops->hash_final(dummy, context);
      req::free(context);
    }

    req::free(key);
  }

  bool isInvalid() const override {
    return context == nullptr;
  }

  CLASSNAME_IS("Hash Context")
  DECLARE_RESOURCE_ALLOCATION(HashContext)

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  HashEnginePtr ops;
  void *context;
  int options;
  char *key;

  TYPE_SCAN_IGNORE_FIELD(context);
};

IMPLEMENT_RESOURCE_ALLOCATION(HashContext)

///////////////////////////////////////////////////////////////////////////////

static req::ptr<HashContext> get_valid_hash_context_resource(const OptResource& context,
                                                             const char* func_name) {
  auto hash = dyn_cast_or_null<HashContext>(context);
  if (hash == nullptr || hash->isInvalid()) {
    raise_warning(
      "%s(): supplied resource is not a valid Hash Context resource",
      func_name + 2
    );
    return nullptr;
  }
  return hash;
}

///////////////////////////////////////////////////////////////////////////////
// hash functions

Array HHVM_FUNCTION(hash_algos) {
  VecInit ret(HashEngines.size());
  for (auto const& engine : HashEngines) {
    ret.append(String(engine.first));
  }
  return ret.toArray();
}

static HashEnginePtr php_hash_fetch_ops(const String& algo) {
  HashEngineMap::const_iterator iter =
    HashEngines.find(HHVM_FN(strtolower)(algo).data());
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
    f = HHVM_FN(fopen)(data, "rb");
    if (same(f, false)) {
      return false;
    }
  }

  void *context = req::malloc_noptrs(ops->context_size);
  ops->hash_init(context);

  if (isfilename) {
    for (Variant chunk = HHVM_FN(fread)(f.toResource(), 1024);
         !is_empty_string(chunk.asTypedValue());
         chunk = HHVM_FN(fread)(f.toResource(), 1024)) {
      String schunk = chunk.toString();
      ops->hash_update(context, (unsigned char *)schunk.data(), schunk.size());
    }
  } else {
    ops->hash_update(context, (unsigned char *)data.data(), data.size());
  }

  String raw = String(ops->digest_size, ReserveString);
  char *digest = raw.mutableData();
  ops->hash_final((unsigned char *)digest, context);
  req::free(context);

  raw.setSize(ops->digest_size);
  if (raw_output) {
    return raw;
  }
  return HHVM_FN(bin2hex)(raw);
}

Variant HHVM_FUNCTION(hash, const String& algo, const String& data,
                            bool raw_output /* = false */) {
  return php_hash_do_hash(algo, data, false, raw_output);
}

Variant HHVM_FUNCTION(hash_file, const String& algo, const String& filename,
                                 bool raw_output /* = false */) {
  if (filename.size() != strlen(filename.data())) {
    raise_warning(
     "hash_file() expects parameter 2 to be a valid path, string given"
    );
    return uninit_variant;
  }
  return php_hash_do_hash(algo, filename, true, raw_output);
}

static char *prepare_hmac_key(HashEnginePtr ops, void *context,
                              const String& key) {
  char *K = (char*)req::malloc_noptrs(ops->block_size);
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
  req::free(K);
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

  void *context = req::malloc_noptrs(ops->context_size);
  ops->hash_init(context);

  const auto hash = req::make<HashContext>(ops, context, options);
  if (options & k_HASH_HMAC) {
    hash->key = prepare_hmac_key(ops, context, key);
  }
  return Variant(std::move(hash));
}

bool HHVM_FUNCTION(hash_update, const OptResource& context, const String& data) {
  auto hash = get_valid_hash_context_resource(context, __FUNCTION__);
  if (!hash) {
    return false;
  }
  hash->ops->hash_update(hash->context, (unsigned char *)data.data(),
                         data.size());
  return true;
}

Variant HHVM_FUNCTION(hash_final, const OptResource& context,
                                 bool raw_output /* = false */) {
  auto hash = get_valid_hash_context_resource(context, __FUNCTION__);
  if (!hash) {
    return false;
  }

  String raw = String(hash->ops->digest_size, ReserveString);
  char *digest = raw.mutableData();
  hash->ops->hash_final((unsigned char *)digest, hash->context);
  if (hash->options & k_HASH_HMAC) {
    finalize_hmac_key(hash->key, hash->ops, hash->context, digest);
    hash->key = NULL;
  }
  req::free(hash->context);
  hash->context = NULL;

  raw.setSize(hash->ops->digest_size);
  if (raw_output) {
    return raw;
  }
  return HHVM_FN(bin2hex)(raw);
}

Variant HHVM_FUNCTION(hash_copy, const OptResource& context) {
  auto oldhash = get_valid_hash_context_resource(context, __FUNCTION__);
  if (!oldhash) {
    return false;
  }
  return OptResource(req::make<HashContext>(std::move(oldhash)));
}

/**
 * It is important that the run time of this function is dependent
 * only on the length of the user-supplied string.
 *
 * The only branch in the code below *should* result in non-branching
 * machine code.
 *
 * Do not try to optimize this function.
 */
bool HHVM_FUNCTION(hash_equals, const Variant& known, const Variant& user) {
  if (!known.isString()) {
    raise_warning(
      "hash_equals(): Expected known_string to be a string, %s given",
      getDataTypeString(known.getType()).c_str()
    );
    return false;
  }
  if (!user.isString()) {
    raise_warning(
      "hash_equals(): Expected user_string to be a string, %s given",
      getDataTypeString(user.getType()).c_str()
    );
    return false;
  }
  String known_str = known.toString();
  String user_str = user.toString();
  const auto known_len = known_str.size();
  const auto known_limit = known_len - 1;
  const auto user_len = user_str.size();
  int64_t result = known_len ^ user_len;

  int64_t ki = 0;
  for (int64_t ui = 0; ui < user_len; ++ui) {
    result |= user_str[ui] ^ known_str[ki];
    if (ki < known_limit) {
      ++ki;
    }
  }
  return (result == 0);
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

void HashExtension::moduleInit() {
  HHVM_FE(hash);
  HHVM_FE(hash_algos);
  HHVM_FE(hash_file);
  HHVM_FE(hash_final);
  HHVM_FE(hash_init);
  HHVM_FE(hash_update);
  HHVM_FE(hash_copy);
  HHVM_FE(hash_equals);
  HHVM_FE(furchash_hphp_ext);
  HHVM_FE(hphp_murmurhash);

  HHVM_RC_INT(HASH_HMAC, k_HASH_HMAC);
}

///////////////////////////////////////////////////////////////////////////////
}
