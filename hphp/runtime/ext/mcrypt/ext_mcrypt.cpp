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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/ext/std/ext_std_math.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NON_FREE
#define MCRYPT2
#include <mcrypt.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class MCrypt : public SweepableResourceData {
public:
  explicit MCrypt(MCRYPT td) : m_td(td), m_init(false) {}

  ~MCrypt() {
    MCrypt::close();
  }

  void close() {
    if (m_td != MCRYPT_FAILED) {
      mcrypt_generic_deinit(m_td);
      mcrypt_module_close(m_td);
      m_td = MCRYPT_FAILED;
    }
  }

  CLASSNAME_IS("mcrypt");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  DECLARE_RESOURCE_ALLOCATION(MCrypt)

public:
  MCRYPT m_td;
  bool m_init;
};

IMPLEMENT_RESOURCE_ALLOCATION(MCrypt)

typedef enum {
  RANDOM = 0,
  URANDOM,
  RAND
} iv_source;

class mcrypt_data {
public:
  std::string algorithms_dir;
  std::string modes_dir;
};
static mcrypt_data s_globals;
#define MCG(n) (s_globals.n)
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#define MCRYPT_OPEN_MODULE_FAILED(str) \
 raise_warning("%s(): Module initialization failed", str);

static Variant php_mcrypt_do_crypt(const String& cipher, const String& key,
                                   const String& data, const String& mode,
                                   const String& iv, bool dencrypt,
                                   char *name) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)mode.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    MCRYPT_OPEN_MODULE_FAILED(name);
    return false;
  }

  /* Checking for key-length */
  int max_key_length = mcrypt_enc_get_key_size(td);
  if (key.size() > max_key_length) {
    raise_warning("Size of key is too large for this algorithm");
  }
  int count;
  int *key_length_sizes = mcrypt_enc_get_supported_key_sizes(td, &count);
  int use_key_length;
  char *key_s = nullptr;
  if (count == 0 && key_length_sizes == nullptr) { // all lengths 1 - k_l_s = OK
    use_key_length = key.size();
    key_s = (char*)malloc(use_key_length);
    memcpy(key_s, key.data(), use_key_length);
  } else if (count == 1) {  /* only m_k_l = OK */
    key_s = (char*)malloc(key_length_sizes[0]);
    memset(key_s, 0, key_length_sizes[0]);
    memcpy(key_s, key.data(), MIN(key.size(), key_length_sizes[0]));
    use_key_length = key_length_sizes[0];
  } else { /* dertermine smallest supported key > length of requested key */
    use_key_length = max_key_length; /* start with max key length */
    for (int i = 0; i < count; i++) {
      if (key_length_sizes[i] >= key.size() &&
          key_length_sizes[i] < use_key_length) {
        use_key_length = key_length_sizes[i];
      }
    }
    key_s = (char*)malloc(use_key_length);
    memset(key_s, 0, use_key_length);
    memcpy(key_s, key.data(), MIN(key.size(), use_key_length));
  }
  mcrypt_free(key_length_sizes);

  /* Check IV */
  char *iv_s = nullptr;
  int iv_size = mcrypt_enc_get_iv_size(td);

  /* IV is required */
  if (mcrypt_enc_mode_has_iv(td) == 1) {
    if (!iv.empty()) {
      if (iv_size != iv.size()) {
        raise_warning("%s(): The IV parameter must be as long as "
                      "the blocksize", name);
      } else {
        iv_s = (char*)malloc(iv_size + 1);
        memcpy(iv_s, iv.data(), iv_size);
      }
    } else {
      raise_warning("%s(): The IV parameter must be as long as "
                    "the blocksize", name);
      iv_s = (char*)malloc(iv_size + 1);
      memset(iv_s, 0, iv_size + 1);
    }
  }

  int block_size;
  unsigned long int data_size;
  String s;
  char *data_s;
  /* Check blocksize */
  if (mcrypt_enc_is_block_mode(td) == 1) { /* It's a block algorithm */
    block_size = mcrypt_enc_get_block_size(td);
    data_size = (((data.size() - 1) / block_size) + 1) * block_size;
    s = String(data_size, ReserveString);
    data_s = (char*)s.mutableData();
    memset(data_s, 0, data_size);
    memcpy(data_s, data.data(), data.size());
  } else { /* It's not a block algorithm */
    data_size = data.size();
    s = String(data_size, ReserveString);
    data_s = (char*)s.mutableData();
    memcpy(data_s, data.data(), data.size());
  }

  if (mcrypt_generic_init(td, key_s, use_key_length, iv_s) < 0) {
    raise_warning("Mcrypt initialisation failed");
    return false;
  }
  if (dencrypt) {
    mdecrypt_generic(td, data_s, data_size);
  } else {
    mcrypt_generic(td, data_s, data_size);
  }

  /* freeing vars */
  mcrypt_generic_end(td);
  if (key_s != nullptr) {
    free(key_s);
  }
  if (iv_s != nullptr) {
    free(iv_s);
  }
  s.setSize(data_size);
  return s;
}

static Variant mcrypt_generic(const Resource& td, const String& data,
                              bool dencrypt) {
  auto pm = cast<MCrypt>(td);
  if (!pm->m_init) {
    raise_warning("Operation disallowed prior to mcrypt_generic_init().");
    return false;
  }

  if (data.empty()) {
    raise_warning("An empty string was passed");
    return false;
  }

  String s;
  unsigned char* data_s;
  int block_size, data_size;
  /* Check blocksize */
  if (mcrypt_enc_is_block_mode(pm->m_td) == 1) { /* It's a block algorithm */
    block_size = mcrypt_enc_get_block_size(pm->m_td);
    data_size = (((data.size() - 1) / block_size) + 1) * block_size;
    s = String(data_size, ReserveString);
    data_s = (unsigned char *)s.mutableData();
    memset(data_s, 0, data_size);
    memcpy(data_s, data.data(), data.size());
  } else { /* It's not a block algorithm */
    data_size = data.size();
    s = String(data_size, ReserveString);
    data_s = (unsigned char *)s.mutableData();
    memcpy(data_s, data.data(), data.size());
  }

  if (dencrypt) {
    mdecrypt_generic(pm->m_td, data_s, data_size);
  } else {
    mcrypt_generic(pm->m_td, data_s, data_size);
  }
  s.setSize(data_size);
  return s;
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(mcrypt_module_open, const String& algorithm,
                                          const String& algorithm_directory,
                             const String& mode, const String& mode_directory) {
  MCRYPT td = mcrypt_module_open
    ((char*)algorithm.data(),
     (char*)(algorithm_directory.empty() ? MCG(algorithms_dir).data() :
             algorithm_directory.data()),
     (char*)mode.data(),
     (char*)(mode_directory.empty() ? (char*)MCG(modes_dir).data() :
             mode_directory.data()));

  if (td == MCRYPT_FAILED) {
    raise_warning("Could not open encryption module");
    return false;
  }

  return Variant(req::make<MCrypt>(td));
}

bool HHVM_FUNCTION(mcrypt_module_close, const Resource& td) {
  cast<MCrypt>(td)->close();
  return true;
}

Array HHVM_FUNCTION(mcrypt_list_algorithms,
                    const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;

  int count = 0;
  char **modules = mcrypt_list_algorithms((char*)dir.data(), &count);
  if (count == 0) {
    raise_warning("No algorithms found in module dir");
  }
  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ret.append(String(modules[i], CopyString));
  }
  mcrypt_free_p(modules, count);
  return ret;
}

Array HHVM_FUNCTION(mcrypt_list_modes,
                    const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(modes_dir)) : lib_dir;

  int count = 0;
  char **modules = mcrypt_list_modes((char*)dir.data(), &count);
  if (count == 0) {
    raise_warning("No modes found in module dir");
  }
  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ret.append(String(modules[i], CopyString));
  }
  mcrypt_free_p(modules, count);
  return ret;
}

int64_t HHVM_FUNCTION(mcrypt_module_get_algo_block_size,
                                   const String& algorithm,
                                   const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_get_algo_block_size((char*)algorithm.data(),
                                           (char*)dir.data());
}

int64_t HHVM_FUNCTION(mcrypt_module_get_algo_key_size, const String& algorithm,
                                   const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_get_algo_key_size((char*)algorithm.data(),
                                         (char*)dir.data());
}

Array HHVM_FUNCTION(mcrypt_module_get_supported_key_sizes,
                    const String& algorithm,
                    const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;

  int count = 0;
  int *key_sizes = mcrypt_module_get_algo_supported_key_sizes
    ((char*)algorithm.data(), (char*)dir.data(), &count);

  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ret.append(key_sizes[i]);
  }
  mcrypt_free(key_sizes);
  return ret;
}

bool HHVM_FUNCTION(mcrypt_module_is_block_algorithm_mode, const String& mode,
                                  const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(modes_dir)) : lib_dir;
  return mcrypt_module_is_block_algorithm_mode((char*)mode.data(),
                                               (char*)dir.data()) == 1;
}

bool HHVM_FUNCTION(mcrypt_module_is_block_algorithm, const String& algorithm,
                                  const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_is_block_algorithm((char*)algorithm.data(),
                                          (char*)dir.data()) == 1;
}

bool HHVM_FUNCTION(mcrypt_module_is_block_mode, const String& mode,
                                   const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(modes_dir)) : lib_dir;
  return mcrypt_module_is_block_mode((char*)mode.data(),
                                     (char*)dir.data()) == 1;
}

bool HHVM_FUNCTION(mcrypt_module_self_test, const String& algorithm,
                               const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_self_test((char*)algorithm.data(),
                                 (char*)dir.data()) == 0;
}

Variant HHVM_FUNCTION(mcrypt_create_iv, int size, int source /* = 0 */) {
  if (size <= 0 || size >= INT_MAX) {
    raise_warning("Can not create an IV with a size of less than 1 or "
                    "greater than %d", INT_MAX);
    return false;
  }

  int n = 0;
  char *iv = (char*)calloc(size + 1, 1);
  if (source == RANDOM || source == URANDOM) {
    int fd = open(source == RANDOM ? "/dev/random" : "/dev/urandom", O_RDONLY);
    if (fd < 0) {
      free(iv);
      raise_warning("Cannot open source device");
      return false;
    }
    int read_bytes;
    for (read_bytes = 0; read_bytes < size && n >= 0; read_bytes += n) {
      n = read(fd, iv + read_bytes, size - read_bytes);
    }
    n = read_bytes;
    close(fd);
    if (n < size) {
      free(iv);
      raise_warning("Could not gather sufficient random data");
      return false;
    }
  } else {
    n = size;
    while (size) {
      // Use userspace rand() function because it handles auto-seeding
      iv[--size] = (char)HHVM_FN(rand)(0, 255);
    }
  }
  return String(iv, n, AttachString);
}

Variant HHVM_FUNCTION(mcrypt_encrypt, const String& cipher, const String& key,
                                      const String& data, const String& mode,
                                      const Variant& viv /* = null_string */) {
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, mode, iv, false,
                             "mcrypt_encrypt");
}

Variant HHVM_FUNCTION(mcrypt_decrypt, const String& cipher, const String& key,
                                      const String& data, const String& mode,
                                      const Variant& viv /* = null_string */) {
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, mode, iv, true,
                             "mcrypt_decrypt");
}

Variant HHVM_FUNCTION(mcrypt_cbc, const String& cipher, const String& key,
                                  const String& data, const Variant& mode,
                                  const Variant& viv /* = null_string */) {
  raise_deprecated("Function mcrypt_cbc() is deprecated");
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, "cbc", iv, mode.toInt32(),
                             "mcrypt_cbc");
}

Variant HHVM_FUNCTION(mcrypt_cfb, const String& cipher, const String& key,
                                  const String& data, const Variant& mode,
                                  const Variant& viv /* = null_string */) {
  raise_deprecated("Function mcrypt_cfb() is deprecated");
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, "cfb", iv, mode.toInt32(),
                             "mcrypt_cfb");
}

Variant HHVM_FUNCTION(mcrypt_ecb, const String& cipher, const String& key,
                                  const String& data, const Variant& mode,
                                  const Variant& viv /* = null_string */) {
  raise_deprecated("Function mcrypt_ecb() is deprecated");
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, "ecb", iv, mode.toInt32(),
                             "mcrypt_ecb");
}

Variant HHVM_FUNCTION(mcrypt_ofb, const String& cipher, const String& key,
                                  const String& data, const Variant& mode,
                                  const Variant& viv /* = null_string */) {
  raise_deprecated("Function mcrypt_ofb() is deprecated");
  String iv = viv.toString();
  return php_mcrypt_do_crypt(cipher, key, data, "ofb", iv, mode.toInt32(),
                             "mcrypt_ofb");
}

Variant HHVM_FUNCTION(mcrypt_get_block_size, const String& cipher,
                                    const Variant& module /* = null_string */) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)module.asCStrRef().data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    MCRYPT_OPEN_MODULE_FAILED("mcrypt_get_block_size");
    return false;
  }

  int64_t ret = mcrypt_enc_get_block_size(td);
  mcrypt_module_close(td);
  return ret;
}

Variant HHVM_FUNCTION(mcrypt_get_cipher_name, const String& cipher) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)"ecb",
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    td = mcrypt_module_open((char*)cipher.data(),
                            (char*)MCG(algorithms_dir).data(),
                            (char*)"stream",
                            (char*)MCG(modes_dir).data());
    if (td == MCRYPT_FAILED) {
      MCRYPT_OPEN_MODULE_FAILED("mcrypt_get_cipher_name");
      return false;
    }
  }

  char *cipher_name = mcrypt_enc_get_algorithms_name(td);
  mcrypt_module_close(td);
  String ret(cipher_name, CopyString);
  mcrypt_free(cipher_name);
  return ret;
}

Variant HHVM_FUNCTION(mcrypt_get_iv_size, const String& cipher,
                                          const String& mode) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)mode.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    MCRYPT_OPEN_MODULE_FAILED("mcrypt_get_iv_size");
    return false;
  }

  int64_t ret = mcrypt_enc_get_iv_size(td);
  mcrypt_module_close(td);
  return ret;
}

Variant HHVM_FUNCTION(mcrypt_get_key_size, const String& cipher,
                                           const String& module) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)module.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    MCRYPT_OPEN_MODULE_FAILED("mcrypt_get_key_size");
    return false;
  }

  int64_t ret = mcrypt_enc_get_key_size(td);
  mcrypt_module_close(td);
  return ret;
}

String HHVM_FUNCTION(mcrypt_enc_get_algorithms_name, const Resource& td) {
  char *name = mcrypt_enc_get_algorithms_name(cast<MCrypt>(td)->m_td);
  String ret(name, CopyString);
  mcrypt_free(name);
  return ret;
}

int64_t HHVM_FUNCTION(mcrypt_enc_get_block_size, const Resource& td) {
  return mcrypt_enc_get_block_size(cast<MCrypt>(td)->m_td);
}

int64_t HHVM_FUNCTION(mcrypt_enc_get_iv_size, const Resource& td) {
  return mcrypt_enc_get_iv_size(cast<MCrypt>(td)->m_td);
}

int64_t HHVM_FUNCTION(mcrypt_enc_get_key_size, const Resource& td) {
  return mcrypt_enc_get_key_size(cast<MCrypt>(td)->m_td);
}

String HHVM_FUNCTION(mcrypt_enc_get_modes_name, const Resource& td) {
  char *name = mcrypt_enc_get_modes_name(cast<MCrypt>(td)->m_td);
  String ret(name, CopyString);
  mcrypt_free(name);
  return ret;
}

Array HHVM_FUNCTION(mcrypt_enc_get_supported_key_sizes, const Resource& td) {
  int count = 0;
  int *key_sizes =
    mcrypt_enc_get_supported_key_sizes(cast<MCrypt>(td)->m_td, &count);

  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ret.append(key_sizes[i]);
  }
  mcrypt_free(key_sizes);
  return ret;
}

bool HHVM_FUNCTION(mcrypt_enc_is_block_algorithm_mode, const Resource& td) {
  return mcrypt_enc_is_block_algorithm_mode(cast<MCrypt>(td)->m_td) == 1;
}

bool HHVM_FUNCTION(mcrypt_enc_is_block_algorithm, const Resource& td) {
  return mcrypt_enc_is_block_algorithm(cast<MCrypt>(td)->m_td) == 1;
}

bool HHVM_FUNCTION(mcrypt_enc_is_block_mode, const Resource& td) {
  return mcrypt_enc_is_block_mode(cast<MCrypt>(td)->m_td) == 1;
}

int64_t HHVM_FUNCTION(mcrypt_enc_self_test, const Resource& td) {
  return mcrypt_enc_self_test(cast<MCrypt>(td)->m_td);
}

int64_t HHVM_FUNCTION(mcrypt_generic_init, const Resource& td,
                                           const String& key,
                                           const String& iv) {
  auto pm = cast<MCrypt>(td);
  int max_key_size = mcrypt_enc_get_key_size(pm->m_td);
  int iv_size = mcrypt_enc_get_iv_size(pm->m_td);

  if (key.empty()) {
    raise_warning("Key size is 0");
  }

  unsigned char *key_s = (unsigned char *)malloc(key.size());
  memset(key_s, 0, key.size());

  unsigned char *iv_s = (unsigned char *)malloc(iv_size + 1);
  memset(iv_s, 0, iv_size + 1);

  int key_size;
  if (key.size() > max_key_size) {
    raise_warning("Key size too large; supplied length: %d, max: %d",
                    key.size(), max_key_size);
    key_size = max_key_size;
  } else {
    key_size = key.size();
  }
  memcpy(key_s, key.data(), key.size());

  if (iv.size() != iv_size) {
    raise_warning("Iv size incorrect; supplied length: %d, needed: %d",
                    iv.size(), iv_size);
  }
  memcpy(iv_s, iv.data(), std::min(iv_size, iv.size()));

  mcrypt_generic_deinit(pm->m_td);
  int result = mcrypt_generic_init(pm->m_td, key_s, key_size, iv_s);

  /* If this function fails, close the mcrypt module to prevent crashes
   * when further functions want to access this resource */
  if (result < 0) {
    pm->close();
    switch (result) {
    case -3:
      raise_warning("Key length incorrect");
      break;
    case -4:
      raise_warning("Memory allocation error");
      break;
    case -1:
    default:
      raise_warning("Unknown error");
      break;
    }
  } else {
    pm->m_init = true;
  }

  free(iv_s);
  free(key_s);
  return result;
}

Variant HHVM_FUNCTION(mcrypt_generic, const Resource& td, const String& data) {
  return mcrypt_generic(td, data, false);
}

Variant HHVM_FUNCTION(mdecrypt_generic, const Resource& td,
                                        const String& data) {
  return mcrypt_generic(td, data, true);
}

bool HHVM_FUNCTION(mcrypt_generic_deinit, const Resource& td) {
  auto pm = cast<MCrypt>(td);
  if (mcrypt_generic_deinit(pm->m_td) < 0) {
    raise_warning("Could not terminate encryption specifier");
    return false;
  }
  pm->m_init = false;
  return true;
}

bool HHVM_FUNCTION(mcrypt_generic_end, const Resource& td) {
  return HHVM_FUNCTION(mcrypt_generic_deinit, td);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_MCRYPT_3DES("MCRYPT_3DES");
const StaticString s_MCRYPT_ARCFOUR("MCRYPT_ARCFOUR");
const StaticString s_MCRYPT_ARCFOUR_IV("MCRYPT_ARCFOUR_IV");
const StaticString s_MCRYPT_BLOWFISH("MCRYPT_BLOWFISH");
const StaticString s_MCRYPT_BLOWFISH_COMPAT("MCRYPT_BLOWFISH_COMPAT");
const StaticString s_MCRYPT_CAST_128("MCRYPT_CAST_128");
const StaticString s_MCRYPT_CAST_256("MCRYPT_CAST_256");
const StaticString s_MCRYPT_CRYPT("MCRYPT_CRYPT");
const StaticString s_MCRYPT_DECRYPT("MCRYPT_DECRYPT");
const StaticString s_MCRYPT_DES("MCRYPT_DES");
const StaticString s_MCRYPT_DEV_RANDOM("MCRYPT_DEV_RANDOM");
const StaticString s_MCRYPT_DEV_URANDOM("MCRYPT_DEV_URANDOM");
const StaticString s_MCRYPT_ENCRYPT("MCRYPT_ENCRYPT");
const StaticString s_MCRYPT_ENIGNA("MCRYPT_ENIGNA");
const StaticString s_MCRYPT_GOST("MCRYPT_GOST");
const StaticString s_MCRYPT_IDEA("MCRYPT_IDEA");
const StaticString s_MCRYPT_LOKI97("MCRYPT_LOKI97");
const StaticString s_MCRYPT_MARS("MCRYPT_MARS");
const StaticString s_MCRYPT_MODE_CBC("MCRYPT_MODE_CBC");
const StaticString s_MCRYPT_MODE_CFB("MCRYPT_MODE_CFB");
const StaticString s_MCRYPT_MODE_ECB("MCRYPT_MODE_ECB");
const StaticString s_MCRYPT_MODE_NOFB("MCRYPT_MODE_NOFB");
const StaticString s_MCRYPT_MODE_OFB("MCRYPT_MODE_OFB");
const StaticString s_MCRYPT_MODE_STREAM("MCRYPT_MODE_STREAM");
const StaticString s_MCRYPT_PANAMA("MCRYPT_PANAMA");
const StaticString s_MCRYPT_RAND("MCRYPT_RAND");
const StaticString s_MCRYPT_RC2("MCRYPT_RC2");
const StaticString s_MCRYPT_RC6("MCRYPT_RC6");
const StaticString s_MCRYPT_RIJNDAEL_128("MCRYPT_RIJNDAEL_128");
const StaticString s_MCRYPT_RIJNDAEL_192("MCRYPT_RIJNDAEL_192");
const StaticString s_MCRYPT_RIJNDAEL_256("MCRYPT_RIJNDAEL_256");
const StaticString s_MCRYPT_SAFER128("MCRYPT_SAFER128");
const StaticString s_MCRYPT_SAFER64("MCRYPT_SAFER64");
const StaticString s_MCRYPT_SAFERPLUS("MCRYPT_SAFERPLUS");
const StaticString s_MCRYPT_SERPENT("MCRYPT_SERPENT");
const StaticString s_MCRYPT_SKIPJACK("MCRYPT_SKIPJACK");
const StaticString s_MCRYPT_THREEWAY("MCRYPT_THREEWAY");
const StaticString s_MCRYPT_TRIPLEDES("MCRYPT_TRIPLEDES");
const StaticString s_MCRYPT_TWOFISH("MCRYPT_TWOFISH");
const StaticString s_MCRYPT_WAKE("MCRYPT_WAKE");
const StaticString s_MCRYPT_XTEA("MCRYPT_XTEA");

class McryptExtension final : public Extension {
 public:
  McryptExtension() : Extension("mcrypt") {}
  void moduleInit() override {
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_3DES.get(), StaticString("tripledes").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_ARCFOUR.get(), StaticString("arcfour").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_ARCFOUR_IV.get(), StaticString("arcfour-iv").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_BLOWFISH.get(), StaticString("blowfish").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_BLOWFISH_COMPAT.get(), StaticString("blowfish-compat").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_CAST_128.get(), StaticString("cast-128").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_CAST_256.get(), StaticString("cast-256").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_CRYPT.get(), StaticString("crypt").get()
    );
    Native::registerConstant<KindOfInt64>(
      s_MCRYPT_DECRYPT.get(), 1
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_DES.get(), StaticString("des").get()
    );
    Native::registerConstant<KindOfInt64>(
      s_MCRYPT_DEV_RANDOM.get(), RANDOM
    );
    Native::registerConstant<KindOfInt64>(
      s_MCRYPT_DEV_URANDOM.get(), URANDOM
    );
    Native::registerConstant<KindOfInt64>(
      s_MCRYPT_ENCRYPT.get(), 0
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_ENIGNA.get(), StaticString("crypt").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_GOST.get(), StaticString("gost").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_IDEA.get(), StaticString("idea").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_LOKI97.get(), StaticString("loki97").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MARS.get(), StaticString("mars").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_CBC.get(), StaticString("cbc").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_CFB.get(), StaticString("cfb").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_ECB.get(), StaticString("ecb").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_NOFB.get(), StaticString("nofb").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_OFB.get(), StaticString("ofb").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_MODE_STREAM.get(), StaticString("stream").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_PANAMA.get(), StaticString("panama").get()
    );
    Native::registerConstant<KindOfInt64>(
      s_MCRYPT_RAND.get(), RAND
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_RC2.get(), StaticString("rc2").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_RC6.get(), StaticString("rc6").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_RIJNDAEL_128.get(), StaticString("rijndael-128").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_RIJNDAEL_192.get(), StaticString("rijndael-192").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_RIJNDAEL_256.get(), StaticString("rijndael-256").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_SAFER128.get(), StaticString("safer-sk128").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_SAFER64.get(), StaticString("safer-sk64").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_SAFERPLUS.get(), StaticString("saferplus").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_SERPENT.get(), StaticString("serpent").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_SKIPJACK.get(), StaticString("skipjack").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_THREEWAY.get(), StaticString("threeway").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_TRIPLEDES.get(), StaticString("tripledes").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_TWOFISH.get(), StaticString("twofish").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_WAKE.get(), StaticString("wake").get()
    );
    Native::registerConstant<KindOfStaticString>(
      s_MCRYPT_XTEA.get(), StaticString("xtea").get()
    );

    HHVM_FE(mcrypt_module_open);
    HHVM_FE(mcrypt_module_close);
    HHVM_FE(mcrypt_list_algorithms);
    HHVM_FE(mcrypt_list_modes);
    HHVM_FE(mcrypt_module_get_algo_block_size);
    HHVM_FE(mcrypt_module_get_algo_key_size);
    HHVM_FE(mcrypt_module_get_supported_key_sizes);
    HHVM_FE(mcrypt_module_is_block_algorithm_mode);
    HHVM_FE(mcrypt_module_is_block_algorithm);
    HHVM_FE(mcrypt_module_is_block_mode);
    HHVM_FE(mcrypt_module_self_test);
    HHVM_FE(mcrypt_create_iv);
    HHVM_FE(mcrypt_encrypt);
    HHVM_FE(mcrypt_decrypt);
    HHVM_FE(mcrypt_cbc);
    HHVM_FE(mcrypt_cfb);
    HHVM_FE(mcrypt_ecb);
    HHVM_FE(mcrypt_ofb);
    HHVM_FE(mcrypt_get_block_size);
    HHVM_FE(mcrypt_get_cipher_name);
    HHVM_FE(mcrypt_get_iv_size);
    HHVM_FE(mcrypt_get_key_size);
    HHVM_FE(mcrypt_enc_get_algorithms_name);
    HHVM_FE(mcrypt_enc_get_block_size);
    HHVM_FE(mcrypt_enc_get_iv_size);
    HHVM_FE(mcrypt_enc_get_key_size);
    HHVM_FE(mcrypt_enc_get_modes_name);
    HHVM_FE(mcrypt_enc_get_supported_key_sizes);
    HHVM_FE(mcrypt_enc_is_block_algorithm_mode);
    HHVM_FE(mcrypt_enc_is_block_algorithm);
    HHVM_FE(mcrypt_enc_is_block_mode);
    HHVM_FE(mcrypt_enc_self_test);
    HHVM_FE(mcrypt_generic_init);
    HHVM_FE(mcrypt_generic);
    HHVM_FE(mdecrypt_generic);
    HHVM_FE(mcrypt_generic_deinit);
    HHVM_FE(mcrypt_generic_end);

    loadSystemlib();
  }
} s_mcrypt_extension;

///////////////////////////////////////////////////////////////////////////////

}
