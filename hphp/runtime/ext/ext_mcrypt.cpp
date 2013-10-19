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

#include "hphp/runtime/ext/ext_mcrypt.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define NON_FREE
#define MCRYPT2
#include <mcrypt.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(mcrypt);
///////////////////////////////////////////////////////////////////////////////

class MCrypt : public SweepableResourceData {
public:
  explicit MCrypt(MCRYPT td) : m_td(td), m_init(false) {
  }

  ~MCrypt() {
    MCrypt::close();
  }

  void sweep() FOLLY_OVERRIDE {
    close();
  }

  void close() {
    if (m_td != MCRYPT_FAILED) {
      mcrypt_generic_deinit(m_td);
      mcrypt_module_close(m_td);
      m_td = MCRYPT_FAILED;
    }
  }

  CLASSNAME_IS("MCrypt");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  MCRYPT m_td;
  bool m_init;
};

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
#define MCRYPT_OPEN_MODULE_FAILED "Module initialization failed"

static Variant php_mcrypt_do_crypt(const String& cipher, const String& key, const String& data,
                                   const String& mode, const String& iv, bool dencrypt) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)mode.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    raise_warning(MCRYPT_OPEN_MODULE_FAILED);
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
  char *key_s = NULL;
  if (count == 0 && key_length_sizes == NULL) { // all lengths 1 - k_l_s = OK
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
  char *iv_s = NULL;
  int iv_size = mcrypt_enc_get_iv_size(td);

  /* IV is required */
  if (mcrypt_enc_mode_has_iv(td) == 1) {
    if (!iv.empty()) {
      if (iv_size != iv.size()) {
        raise_warning("The IV parameter must be as long as the blocksize");
      } else {
        iv_s = (char*)malloc(iv_size + 1);
        memcpy(iv_s, iv.data(), iv_size);
      }
    } else {
      raise_warning("Attempt to use an empty IV, which is NOT recommended");
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
    data_s = (char*)s.bufferSlice().ptr;
    memset(data_s, 0, data_size);
    memcpy(data_s, data.data(), data.size());
  } else { /* It's not a block algorithm */
    data_size = data.size();
    s = String(data_size, ReserveString);
    data_s = (char*)s.bufferSlice().ptr;
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
  if (key_s != NULL) {
    free(key_s);
  }
  if (iv_s != NULL) {
    free(iv_s);
  }
  return s.setSize(data_size);
}

static Variant mcrypt_generic(CResRef td, const String& data, bool dencrypt) {
  MCrypt *pm = td.getTyped<MCrypt>();
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
    data_s = (unsigned char *)s.bufferSlice().ptr;
    memset(data_s, 0, data_size);
    memcpy(data_s, data.data(), data.size());
  } else { /* It's not a block algorithm */
    data_size = data.size();
    s = String(data_size, ReserveString);
    data_s = (unsigned char *)s.bufferSlice().ptr;
    memcpy(data_s, data.data(), data.size());
  }

  if (dencrypt) {
    mdecrypt_generic(pm->m_td, data_s, data_size);
  } else {
    mcrypt_generic(pm->m_td, data_s, data_size);
  }
  return s.setSize(data_size);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_mcrypt_module_open(const String& algorithm, const String& algorithm_directory,
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

  return Resource(new MCrypt(td));
}

bool f_mcrypt_module_close(CResRef td) {
  td.getTyped<MCrypt>()->close();
  return true;
}

Array f_mcrypt_list_algorithms(const String& lib_dir /* = null_string */) {
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

Array f_mcrypt_list_modes(const String& lib_dir /* = null_string */) {
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

int64_t f_mcrypt_module_get_algo_block_size(const String& algorithm,
                                        const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_get_algo_block_size((char*)algorithm.data(),
                                           (char*)dir.data());
}

int64_t f_mcrypt_module_get_algo_key_size(const String& algorithm,
                                      const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_get_algo_key_size((char*)algorithm.data(),
                                         (char*)dir.data());
}

Array f_mcrypt_module_get_supported_key_sizes(const String& algorithm,
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

bool f_mcrypt_module_is_block_algorithm_mode(const String& mode,
                                             const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(modes_dir)) : lib_dir;
  return mcrypt_module_is_block_algorithm_mode((char*)mode.data(),
                                               (char*)dir.data()) == 1;
}

bool f_mcrypt_module_is_block_algorithm(const String& algorithm,
                                        const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_is_block_algorithm((char*)algorithm.data(),
                                          (char*)dir.data()) == 1;
}

bool f_mcrypt_module_is_block_mode(const String& mode,
                                   const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(modes_dir)) : lib_dir;
  return mcrypt_module_is_block_mode((char*)mode.data(),
                                     (char*)dir.data()) == 1;
}

bool f_mcrypt_module_self_test(const String& algorithm,
                               const String& lib_dir /* = null_string */) {
  String dir = lib_dir.empty() ? String(MCG(algorithms_dir)) : lib_dir;
  return mcrypt_module_self_test((char*)algorithm.data(),
                                 (char*)dir.data()) == 0;
}

Variant f_mcrypt_create_iv(int size, int source /* = 0 */) {
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
      iv[--size] = (char)(255.0 * rand() / RAND_MAX);
    }
  }
  return String(iv, n, AttachString);
}

Variant f_mcrypt_encrypt(const String& cipher, const String& key, const String& data,
                         const String& mode, const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, mode, iv, false);
}

Variant f_mcrypt_decrypt(const String& cipher, const String& key, const String& data,
                         const String& mode, const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, mode, iv, true);
}

Variant f_mcrypt_cbc(const String& cipher, const String& key, const String& data, int mode,
                     const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, "cbc", iv, mode);
}

Variant f_mcrypt_cfb(const String& cipher, const String& key, const String& data, int mode,
                     const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, "cfb", iv, mode);
}

Variant f_mcrypt_ecb(const String& cipher, const String& key, const String& data, int mode,
                     const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, "ecb", iv, mode);
}

Variant f_mcrypt_ofb(const String& cipher, const String& key, const String& data, int mode,
                     const String& iv /* = null_string */) {
  return php_mcrypt_do_crypt(cipher, key, data, "ofb", iv, mode);
}

Variant f_mcrypt_get_block_size(const String& cipher, const String& module /* = null_string */) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)module.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    raise_warning(MCRYPT_OPEN_MODULE_FAILED);
    return false;
  }

  int64_t ret = mcrypt_enc_get_block_size(td);
  mcrypt_module_close(td);
  return ret;
}

Variant f_mcrypt_get_cipher_name(const String& cipher) {
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
      raise_warning(MCRYPT_OPEN_MODULE_FAILED);
      return false;
    }
  }

  char *cipher_name = mcrypt_enc_get_algorithms_name(td);
  mcrypt_module_close(td);
  String ret(cipher_name, CopyString);
  mcrypt_free(cipher_name);
  return ret;
}

Variant f_mcrypt_get_iv_size(const String& cipher, const String& mode) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)mode.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    raise_warning(MCRYPT_OPEN_MODULE_FAILED);
    return false;
  }

  int64_t ret = mcrypt_enc_get_iv_size(td);
  mcrypt_module_close(td);
  return ret;
}

int64_t f_mcrypt_get_key_size(const String& cipher, const String& module) {
  MCRYPT td = mcrypt_module_open((char*)cipher.data(),
                                 (char*)MCG(algorithms_dir).data(),
                                 (char*)module.data(),
                                 (char*)MCG(modes_dir).data());
  if (td == MCRYPT_FAILED) {
    raise_warning(MCRYPT_OPEN_MODULE_FAILED);
    return false;
  }

  int64_t ret = mcrypt_enc_get_key_size(td);
  mcrypt_module_close(td);
  return ret;
}

String f_mcrypt_enc_get_algorithms_name(CResRef td) {
  char *name = mcrypt_enc_get_algorithms_name(td.getTyped<MCrypt>()->m_td);
  String ret(name, CopyString);
  mcrypt_free(name);
  return ret;
}

int64_t f_mcrypt_enc_get_block_size(CResRef td) {
  return mcrypt_enc_get_block_size(td.getTyped<MCrypt>()->m_td);
}

int64_t f_mcrypt_enc_get_iv_size(CResRef td) {
  return mcrypt_enc_get_iv_size(td.getTyped<MCrypt>()->m_td);
}

int64_t f_mcrypt_enc_get_key_size(CResRef td) {
  return mcrypt_enc_get_key_size(td.getTyped<MCrypt>()->m_td);
}

String f_mcrypt_enc_get_modes_name(CResRef td) {
  char *name = mcrypt_enc_get_modes_name(td.getTyped<MCrypt>()->m_td);
  String ret(name, CopyString);
  mcrypt_free(name);
  return ret;
}

Array f_mcrypt_enc_get_supported_key_sizes(CResRef td) {
  int count = 0;
  int *key_sizes =
    mcrypt_enc_get_supported_key_sizes(td.getTyped<MCrypt>()->m_td, &count);

  Array ret = Array::Create();
  for (int i = 0; i < count; i++) {
    ret.append(key_sizes[i]);
  }
  mcrypt_free(key_sizes);
  return ret;
}

bool f_mcrypt_enc_is_block_algorithm_mode(CResRef td) {
  return mcrypt_enc_is_block_algorithm_mode(td.getTyped<MCrypt>()->m_td) == 1;
}

bool f_mcrypt_enc_is_block_algorithm(CResRef td) {
  return mcrypt_enc_is_block_algorithm(td.getTyped<MCrypt>()->m_td) == 1;
}

bool f_mcrypt_enc_is_block_mode(CResRef td) {
  return mcrypt_enc_is_block_mode(td.getTyped<MCrypt>()->m_td) == 1;
}

int64_t f_mcrypt_enc_self_test(CResRef td) {
  return mcrypt_enc_self_test(td.getTyped<MCrypt>()->m_td);
}

int64_t f_mcrypt_generic_init(CResRef td, const String& key, const String& iv) {
  MCrypt *pm = td.getTyped<MCrypt>();
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
  memcpy(iv_s, iv.data(), iv_size);

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
  }
  pm->m_init = true;
  free(iv_s);
  free(key_s);
  return result;
}

Variant f_mcrypt_generic(CResRef td, const String& data) {
  return mcrypt_generic(td, data, false);
}

Variant f_mdecrypt_generic(CResRef td, const String& data) {
  return mcrypt_generic(td, data, true);
}

bool f_mcrypt_generic_deinit(CResRef td) {
  MCrypt *pm = td.getTyped<MCrypt>();
  if (mcrypt_generic_deinit(pm->m_td) < 0) {
    raise_warning("Could not terminate encryption specifier");
    return false;
  }
  pm->m_init = false;
  return true;
}

bool f_mcrypt_generic_end(CResRef td) {
  return f_mcrypt_generic_deinit(td);
}

///////////////////////////////////////////////////////////////////////////////
}
