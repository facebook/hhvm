/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_openssl.h>
#include <runtime/base/file/ssl_socket.h>
#include <runtime/base/zend/zend_string.h>
#include <util/logger.h>

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/pkcs12.h>

using namespace std;

namespace HPHP {

#define MIN_KEY_LENGTH          384

#define OPENSSL_ALGO_SHA1       1
#define OPENSSL_ALGO_MD5        2
#define OPENSSL_ALGO_MD4        3
#ifdef HAVE_OPENSSL_MD2_H
#define OPENSSL_ALGO_MD2        4
#endif
#define OPENSSL_ALGO_DSS1       5

enum php_openssl_key_type {
  OPENSSL_KEYTYPE_RSA,
  OPENSSL_KEYTYPE_DSA,
  OPENSSL_KEYTYPE_DH,
  OPENSSL_KEYTYPE_DEFAULT = OPENSSL_KEYTYPE_RSA,
#ifdef EVP_PKEY_EC
  OPENSSL_KEYTYPE_EC = OPENSSL_KEYTYPE_DH +1
#endif
};
enum php_openssl_cipher_type {
  PHP_OPENSSL_CIPHER_RC2_40,
  PHP_OPENSSL_CIPHER_RC2_128,
  PHP_OPENSSL_CIPHER_RC2_64,
  PHP_OPENSSL_CIPHER_DES,
  PHP_OPENSSL_CIPHER_3DES,
  PHP_OPENSSL_CIPHER_DEFAULT = PHP_OPENSSL_CIPHER_RC2_40
};

// bitfields
const int64 k_OPENSSL_RAW_DATA = 1;
const int64 k_OPENSSL_ZERO_PADDING = 2;

static char default_ssl_conf_filename[PATH_MAX];

class OpenSSLInitializer {
public:
  OpenSSLInitializer() {
    SSL_library_init();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_algorithms();

    ERR_load_ERR_strings();
    ERR_load_crypto_strings();
    ERR_load_EVP_strings();

    /* Determine default SSL configuration file */
    char *config_filename = getenv("OPENSSL_CONF");
    if (config_filename == NULL) {
      config_filename = getenv("SSLEAY_CONF");
    }

    /* default to 'openssl.cnf' if no environment variable is set */
    if (config_filename == NULL) {
      snprintf(default_ssl_conf_filename, sizeof(default_ssl_conf_filename),
               "%s/%s", X509_get_default_cert_area(), "openssl.cnf");
    } else {
      strncpy(default_ssl_conf_filename, config_filename,
              sizeof(default_ssl_conf_filename));
    }
  }

  ~OpenSSLInitializer() {
    EVP_cleanup();
  }
};
static OpenSSLInitializer s_openssl_initializer;

IMPLEMENT_DEFAULT_EXTENSION(openssl);
///////////////////////////////////////////////////////////////////////////////
// resource classes

class Key : public SweepableResourceData {
public:
  EVP_PKEY *m_key;
  Key(EVP_PKEY *key) : m_key(key) { ASSERT(m_key);}
  ~Key() { if (m_key) EVP_PKEY_free(m_key);}

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

  bool isPrivate() {
    ASSERT(m_key);
    switch (m_key->type) {
#ifndef NO_RSA
    case EVP_PKEY_RSA:
    case EVP_PKEY_RSA2:
      ASSERT(m_key->pkey.rsa);
      if (!m_key->pkey.rsa->p || !m_key->pkey.rsa->q) {
        return false;
      }
      break;
#endif
#ifndef NO_DSA
    case EVP_PKEY_DSA:
    case EVP_PKEY_DSA1:
    case EVP_PKEY_DSA2:
    case EVP_PKEY_DSA3:
    case EVP_PKEY_DSA4:
      ASSERT(m_key->pkey.dsa);
      if (!m_key->pkey.dsa->p || !m_key->pkey.dsa->q ||
          !m_key->pkey.dsa->priv_key) {
        return false;
      }
      break;
#endif
#ifndef NO_DH
    case EVP_PKEY_DH:
      ASSERT(m_key->pkey.dh);
      if (!m_key->pkey.dh->p || !m_key->pkey.dh->priv_key) {
        return false;
      }
      break;
#endif
    default:
      raise_warning("key type not supported in this PHP build!");
      break;
    }
    return true;
  }

  /**
   * Given a variant, coerce it into a EVP_PKEY object. It can be:
   *
   *   1. private key resource from openssl_get_privatekey()
   *   2. X509 resource -> public key will be extracted from it
   *   3. if it starts with file:// interpreted as path to key file
   *   4. interpreted as the data from the cert/key file and interpreted in
   *      same way as openssl_get_privatekey()
   *   5. an array(0 => [items 2..4], 1 => passphrase)
   *   6. if val is a string (possibly starting with file:///) and it is not
   *      an X509 certificate, then interpret as public key
   *
   * NOTE: If you are requesting a private key but have not specified a
   *   passphrase, you should use an empty string rather than NULL for the
   *   passphrase - NULL causes a passphrase prompt to be emitted in
   *   the Apache error log!
   */
  static Object Get(CVarRef var, bool public_key,
                    const char *passphrase = NULL) {
    Object ocert;
    EVP_PKEY *key = NULL;

    if (var.isResource()) {
      Certificate *cert = var.toObject().getTyped<Certificate>(true, true);
      Key *key = var.toObject().getTyped<Key>(true, true);
      if (cert == NULL && key == NULL) {
        return Object();
      }
      if (key) {
        bool is_priv = key->isPrivate();
        if (!public_key && !is_priv) {
          raise_warning("supplied key param is a public key");
          return Object();
        }
        if (public_key && is_priv) {
          raise_warning("Don't know how to get public key from "
                          "this private key");
          return Object();
        }
        return var.toObject();
      }
      ocert = cert;
    } else {
      /* it's an X509 file/cert of some kind, and we need to extract
         the data from that */
      if (public_key) {
        ocert = Certificate::Get(var);
        if (ocert.isNull()) {
          /* not a X509 certificate, try to retrieve public key */
          BIO *in = Certificate::ReadData(var);
          if (in == NULL) return Object();
          key = PEM_read_bio_PUBKEY(in, NULL,NULL, NULL);
          BIO_free(in);
        }
      } else {
        String zphrase;
        if (var.is(KindOfArray)) {
          Array arr = var.toArray();
          if (!arr.exists(0LL) || !arr.exists(1LL)) {
            raise_warning("key array must be of the form "
                            "array(0 => key, 1 => phrase)");
            return Object();
          }
          zphrase = arr[1].toString();
          passphrase = zphrase.data();
        }
        /* we want the private key */
        BIO *in = Certificate::ReadData(var);
        if (in == NULL) return Object();
        key = PEM_read_bio_PrivateKey(in, NULL,NULL, (void*)passphrase);
        BIO_free(in);
      }
    }

    if (public_key && !ocert.isNull() && key == NULL) {
      /* extract public key from X509 cert */
      key = (EVP_PKEY *)X509_get_pubkey(ocert.getTyped<Certificate>()->m_cert);
    }

    if (key) {
      return Object(new Key(key));
    }
    return Object();
  }
};

StaticString Key::s_class_name("OpenSSL key");

/**
 * Certificate Signing Request
 */
class CSRequest : public SweepableResourceData {
public:
  X509_REQ *m_csr;
  CSRequest(X509_REQ *csr) : m_csr(csr) { ASSERT(m_csr);}
  ~CSRequest() { if (m_csr) X509_REQ_free(m_csr);}

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

  static X509_REQ *Get(CVarRef var, Object &ocsr) {
    ocsr = Get(var);
    CSRequest *csr = ocsr.getTyped<CSRequest>(true);
    if (csr == NULL || csr->m_csr == NULL) {
      raise_warning("cannot get CSR");
      return NULL;
    }
    return csr->m_csr;
  }

  static Object Get(CVarRef var) {
    if (var.isResource()) {
      return var.toObject();
    }
    if (var.isString() || var.isObject()) {
      BIO *in = Certificate::ReadData(var);
      if (in == NULL) return Object();

      X509_REQ *csr = PEM_read_bio_X509_REQ(in, NULL,NULL,NULL);
      BIO_free(in);
      if (csr) {
        return Object(new CSRequest(csr));
      }
    }
    return Object();
  }
};

StaticString CSRequest::s_class_name("OpenSSL X.509 CSR");

class php_x509_request {
public:
#if OPENSSL_VERSION_NUMBER >= 0x10000002L
  LHASH_OF(CONF_VALUE) * global_config; /* Global SSL config */
  LHASH_OF(CONF_VALUE) * req_config;    /* SSL config for this request */
#else
  LHASH *global_config;  /* Global SSL config */
  LHASH *req_config;     /* SSL config for this request */
#endif
  const EVP_MD *md_alg;
  const EVP_MD *digest;
  const char *section_name;
  const char *config_filename;
  const char *digest_name;
  const char *extensions_section;
  const char *request_extensions_section;
  int priv_key_bits;
  int priv_key_type;
  int priv_key_encrypt;
  EVP_PKEY *priv_key;

  static bool load_rand_file(const char *file, int *egdsocket, int *seeded) {
    char buffer[PATH_MAX];

    *egdsocket = 0;
    *seeded = 0;
    if (file == NULL) {
      file = RAND_file_name(buffer, sizeof(buffer));
    } else if (RAND_egd(file) > 0) {
      /* if the given filename is an EGD socket, don't
       * write anything back to it */
      *egdsocket = 1;
      return true;
    }

    if (file == NULL || !RAND_load_file(file, -1)) {
      if (RAND_status() == 0) {
        raise_warning("unable to load random state; not enough data!");
        return false;
      }
      return false;
    }
    *seeded = 1;
    return true;
  }

  static bool write_rand_file(const char *file, int egdsocket, int seeded) {
    char buffer[PATH_MAX];

    if (egdsocket || !seeded) {
      /* if we did not manage to read the seed file, we should not write
       * a low-entropy seed file back */
      return false;
    }
    if (file == NULL) {
      file = RAND_file_name(buffer, sizeof(buffer));
    }
    if (file == NULL || !RAND_write_file(file)) {
      raise_warning("unable to write random state");
      return false;
    }
    return true;
  }

  bool generatePrivateKey() {
    ASSERT(priv_key == NULL);

    if (priv_key_bits < MIN_KEY_LENGTH) {
      raise_warning("private key length is too short; it needs to be "
                      "at least %d bits, not %d",
                      MIN_KEY_LENGTH, priv_key_bits);
      return false;
    }

    char *randfile = CONF_get_string(req_config, section_name, "RANDFILE");
    int egdsocket, seeded;
    load_rand_file(randfile, &egdsocket, &seeded);

    bool ret = false;
    if ((priv_key = EVP_PKEY_new()) != NULL) {
      switch (priv_key_type) {
      case OPENSSL_KEYTYPE_RSA:
        if (EVP_PKEY_assign_RSA
            (priv_key, RSA_generate_key(priv_key_bits, 0x10001, NULL, NULL))) {
          ret = true;
        }
        break;
#if !defined(NO_DSA) && defined(HAVE_DSA_DEFAULT_METHOD)
      case OPENSSL_KEYTYPE_DSA:
        {
          DSA *dsapar = DSA_generate_parameters(priv_key_bits, NULL, 0, NULL,
                                                NULL, NULL, NULL);
          if (dsapar) {
            DSA_set_method(dsapar, DSA_get_default_method());
            if (DSA_generate_key(dsapar)) {
              if (EVP_PKEY_assign_DSA(priv_key, dsapar)) {
                ret = true;
              }
            } else {
              DSA_free(dsapar);
            }
          }
        }
        break;
#endif
      default:
        raise_warning("Unsupported private key type");
      }
    }

    write_rand_file(randfile, egdsocket, seeded);
    return ret;
  }
};

///////////////////////////////////////////////////////////////////////////////
// utilities

static void add_assoc_name_entry(Array &ret, const char *key,
                                 X509_NAME *name, bool shortname) {
  Array subitem_data;
  Array &subitem = key ? subitem_data : ret;

  for (int i = 0; i < X509_NAME_entry_count(name); i++) {
    X509_NAME_ENTRY *ne = X509_NAME_get_entry(name, i);
    ASN1_OBJECT *obj = X509_NAME_ENTRY_get_object(ne);
    int nid = OBJ_obj2nid(obj);
    int obj_cnt = 0;

    char *sname;
    if (shortname) {
      sname = (char *)OBJ_nid2sn(nid);
    } else {
      sname = (char *)OBJ_nid2ln(nid);
    }

    Array subentries;
    int last = -1;
    int j;
    ASN1_STRING *str = NULL;
    unsigned char *to_add = NULL;
    int to_add_len = 0;
    for (;;) {
      j = X509_NAME_get_index_by_OBJ(name, obj, last);
      if (j < 0) {
        if (last != -1) break;
      } else {
        obj_cnt++;
        ne  = X509_NAME_get_entry(name, j);
        str = X509_NAME_ENTRY_get_data(ne);
        if (ASN1_STRING_type(str) != V_ASN1_UTF8STRING) {
          to_add_len = ASN1_STRING_to_UTF8(&to_add, str);
          if (to_add_len != -1) {
            subentries.append(String((char*)to_add, to_add_len, AttachString));
          }
        } else {
          to_add = ASN1_STRING_data(str);
          to_add_len = ASN1_STRING_length(str);
          subentries.append(String((char*)to_add, to_add_len, CopyString));
        }
      }
      last = j;
    }
    i = last;

    if (obj_cnt > 1) {
      subitem.set(String(sname, CopyString), subentries);
    } else if (obj_cnt && str && to_add_len > -1) {
      subitem.set(String(sname, CopyString),
                  String((char*)to_add, to_add_len, CopyString));
    }
  }

  if (key) {
    ret.set(String(key, CopyString), subitem);
  }
}

static const char *read_string(CArrRef args, const char *key, const char *def,
                               vector<String> &strings) {
  if (args.exists(key)) {
    String value = args[key].toString();
    strings.push_back(value);
    return (char*)value.data();
  }
  return def;
}

static int64 read_integer(CArrRef args, const char *key, int64 def) {
  if (args.exists(key)) {
    return args[key].toInt64();
  }
  return def;
}

static bool add_oid_section(struct php_x509_request *req) {
  char *str = CONF_get_string(req->req_config, NULL, "oid_section");
  if (str == NULL) {
    return true;
  }

  STACK_OF(CONF_VALUE) *sktmp = CONF_get_section(req->req_config, str);
  if (sktmp == NULL) {
    raise_warning("problem loading oid section %s", str);
    return false;
  }

  for (int i = 0; i < sk_CONF_VALUE_num(sktmp); i++) {
    CONF_VALUE *cnf = sk_CONF_VALUE_value(sktmp, i);
    if (OBJ_create(cnf->value, cnf->name, cnf->name) == NID_undef) {
      raise_warning("problem creating object %s=%s", cnf->name, cnf->value);
      return false;
    }
  }
  return true;
}

#if OPENSSL_VERSION_NUMBER >= 0x10000002L
static inline bool php_openssl_config_check_syntax
(const char *section_label, const char *config_filename, const char *section,
 LHASH_OF(CONF_VALUE) *config) {
#else
static inline bool php_openssl_config_check_syntax
(const char *section_label, const char *config_filename, const char *section,
 LHASH *config) {
#endif

  X509V3_CTX ctx;
  X509V3_set_ctx_test(&ctx);
  X509V3_set_conf_lhash(&ctx, config);
  if (!X509V3_EXT_add_conf(config, &ctx, (char*)section, NULL)) {
    raise_warning("Error loading %s section %s of %s",
                    section_label, section, config_filename);
    return false;
  }
  return true;
}

static bool php_openssl_parse_config(struct php_x509_request *req,
                                     CArrRef args, vector<String> &strings) {
  req->config_filename =
    read_string(args, "config", default_ssl_conf_filename, strings);
  req->section_name =
    read_string(args, "config_section_name", "req", strings);
  req->global_config = CONF_load(NULL, default_ssl_conf_filename, NULL);
  req->req_config = CONF_load(NULL, req->config_filename, NULL);
  if (req->req_config == NULL) {
    return false;
  }

  /* read in the oids */
  char *str = CONF_get_string(req->req_config, NULL, "oid_file");
  if (str) {
    BIO *oid_bio = BIO_new_file(str, "r");
    if (oid_bio) {
      OBJ_create_objects(oid_bio);
      BIO_free(oid_bio);
    }
  }
  if (!add_oid_section(req)) {
    return false;
  }

  req->digest_name =
    read_string(args, "digest_alg",
                CONF_get_string(req->req_config, req->section_name,
                                "default_md"),
                strings);

  req->extensions_section =
    read_string(args, "x509_extensions",
                CONF_get_string(req->req_config, req->section_name,
                                "x509_extensions"),
                strings);

  req->request_extensions_section =
    read_string(args, "req_extensions",
                CONF_get_string(req->req_config, req->section_name,
                                "req_extensions"),
                strings);

  req->priv_key_bits =
    read_integer(args, "private_key_bits",
                 CONF_get_number(req->req_config, req->section_name,
                                 "default_bits"));

  req->priv_key_type =
    read_integer(args, "private_key_type", OPENSSL_KEYTYPE_DEFAULT);

  if (args.exists("encrypt_key")) {
    bool value = args["encrypt_key"].toBoolean();
    req->priv_key_encrypt = value ? 1 : 0;
  } else {
    str = CONF_get_string(req->req_config, req->section_name,
                          "encrypt_rsa_key");
    if (str == NULL) {
      str = CONF_get_string(req->req_config, req->section_name, "encrypt_key");
    }
    if (str && strcmp(str, "no") == 0) {
      req->priv_key_encrypt = 0;
    } else {
      req->priv_key_encrypt = 1;
    }
  }

  /* digest alg */
  if (req->digest_name == NULL) {
    req->digest_name = CONF_get_string(req->req_config, req->section_name,
                                       "default_md");
  }
  if (req->digest_name) {
    req->digest = req->md_alg = EVP_get_digestbyname(req->digest_name);
  }
  if (req->md_alg == NULL) {
    req->md_alg = req->digest = EVP_md5();
  }

  if (req->extensions_section &&
      !php_openssl_config_check_syntax
      ("extensions_section", req->config_filename, req->extensions_section,
       req->req_config)) {
    return false;
  }

  /* set the string mask */
  str = CONF_get_string(req->req_config, req->section_name, "string_mask");
  if (str && !ASN1_STRING_set_default_mask_asc(str)) {
    raise_warning("Invalid global string mask setting %s", str);
    return false;
  }

  if (req->request_extensions_section &&
      !php_openssl_config_check_syntax
      ("request_extensions_section", req->config_filename,
       req->request_extensions_section, req->req_config)) {
    return false;
  }

  return true;
}

static void php_openssl_dispose_config(struct php_x509_request *req) {
  if (req->global_config) {
    CONF_free(req->global_config);
    req->global_config = NULL;
  }
  if (req->req_config) {
    CONF_free(req->req_config);
    req->req_config = NULL;
  }
}

static STACK_OF(X509) *load_all_certs_from_file(const char *certfile) {
  STACK_OF(X509_INFO) *sk = NULL;
  STACK_OF(X509) *stack = NULL, *ret = NULL;
  BIO *in = NULL;
  X509_INFO *xi;

  if (!(stack = sk_X509_new_null())) {
    raise_warning("memory allocation failure");
    goto end;
  }

  if (!(in = BIO_new_file(certfile, "r"))) {
    raise_warning("error opening the file, %s", certfile);
    sk_X509_free(stack);
    goto end;
  }

  /* This loads from a file, a stack of x509/crl/pkey sets */
  if (!(sk = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL))) {
    raise_warning("error reading the file, %s", certfile);
    sk_X509_free(stack);
    goto end;
  }

  /* scan over it and pull out the certs */
  while (sk_X509_INFO_num(sk)) {
    xi = sk_X509_INFO_shift(sk);
    if (xi->x509 != NULL) {
      sk_X509_push(stack, xi->x509);
      xi->x509 = NULL;
    }
    X509_INFO_free(xi);
  }
  if (!sk_X509_num(stack)) {
    raise_warning("no certificates in file, %s", certfile);
    sk_X509_free(stack);
    goto end;
  }
  ret = stack;

end:
  BIO_free(in);
  sk_X509_INFO_free(sk);

  return ret;
}

/**
 * calist is an array containing file and directory names.  create a
 * certificate store and add those certs to it for use in verification.
 */
static X509_STORE *setup_verify(CArrRef calist) {
  X509_STORE *store = X509_STORE_new();
  if (store == NULL) {
    return NULL;
  }

  X509_LOOKUP *dir_lookup, *file_lookup;
  int ndirs = 0, nfiles = 0;
  for (ArrayIter iter(calist); iter; ++iter) {
    String item = iter.second().toString();

    struct stat sb;
    if (stat(item.data(), &sb) == -1) {
      raise_warning("unable to stat %s", item.data());
      continue;
    }

    if ((sb.st_mode & S_IFREG) == S_IFREG) {
      file_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
      if (file_lookup == NULL ||
          !X509_LOOKUP_load_file(file_lookup, item.data(),
                                 X509_FILETYPE_PEM)) {
        raise_warning("error loading file %s", item.data());
      } else {
        nfiles++;
      }
      file_lookup = NULL;
    } else {
      dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
      if (dir_lookup == NULL ||
          !X509_LOOKUP_add_dir(dir_lookup, item.data(), X509_FILETYPE_PEM)) {
        raise_warning("error loading directory %s", item.data());
      } else {
        ndirs++;
      }
      dir_lookup = NULL;
    }
  }
  if (nfiles == 0) {
    file_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
    if (file_lookup) {
      X509_LOOKUP_load_file(file_lookup, NULL, X509_FILETYPE_DEFAULT);
    }
  }
  if (ndirs == 0) {
    dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
    if (dir_lookup) {
      X509_LOOKUP_add_dir(dir_lookup, NULL, X509_FILETYPE_DEFAULT);
    }
  }
  return store;
}

///////////////////////////////////////////////////////////////////////////////

static bool add_entries(X509_NAME *subj, CArrRef items) {
  for (ArrayIter iter(items); iter; ++iter) {
    String index = iter.first();
    String item = iter.second();
    int nid = OBJ_txt2nid(index.data());
    if (nid != NID_undef) {
      if (!X509_NAME_add_entry_by_NID(subj, nid, MBSTRING_ASC,
                                      (unsigned char*)item.data(),
                                      -1, -1, 0)) {
        raise_warning("dn: add_entry_by_NID %d -> %s (failed)",
                        nid, item.data());
        return false;
      }
    } else {
      raise_warning("dn: %s is not a recognized name", index.data());
    }
  }
  return true;
}

static bool php_openssl_make_REQ(struct php_x509_request *req, X509_REQ *csr,
                                 CArrRef dn, CArrRef attribs) {
  char *dn_sect = CONF_get_string(req->req_config, req->section_name,
                                  "distinguished_name");
  if (dn_sect == NULL) return false;

  STACK_OF(CONF_VALUE) *dn_sk = CONF_get_section(req->req_config, dn_sect);
  if (dn_sk == NULL) return false;

  char *attr_sect = CONF_get_string(req->req_config, req->section_name,
                                    "attributes");
  STACK_OF(CONF_VALUE) *attr_sk = NULL;
  if (attr_sect) {
    attr_sk = CONF_get_section(req->req_config, attr_sect);
    if (attr_sk == NULL) {
      return false;
    }
  }

  /* setup the version number: version 1 */
  if (X509_REQ_set_version(csr, 0L)) {
    X509_NAME *subj = X509_REQ_get_subject_name(csr);
    if (!add_entries(subj, dn)) return false;

    /* Finally apply defaults from config file */
    for (int i = 0; i < sk_CONF_VALUE_num(dn_sk); i++) {
      CONF_VALUE *v = sk_CONF_VALUE_value(dn_sk, i);
      char *type = v->name;

      int len = strlen(type);
      if (len < (int)sizeof("_default")) {
        continue;
      }
      len -= sizeof("_default") - 1;
      if (strcmp("_default", type + len) != 0) {
        continue;
      }
      if (len > 200) {
        len = 200;
      }
      char buffer[200 + 1]; /*200 + \0 !*/
      memcpy(buffer, type, len);
      buffer[len] = '\0';
      type = buffer;

      /* Skip past any leading X. X: X, etc to allow for multiple instances */
      for (char *str = type; *str; str++) {
        if (*str == ':' || *str == ',' || *str == '.') {
          str++;
          if (*str) {
            type = str;
          }
          break;
        }
      }
      /* if it is already set, skip this */
      int nid = OBJ_txt2nid(type);
      if (X509_NAME_get_index_by_NID(subj, nid, -1) >= 0) {
        continue;
      }
      if (!X509_NAME_add_entry_by_txt(subj, type, MBSTRING_ASC,
                                      (unsigned char*)v->value, -1, -1, 0)) {
        raise_warning("add_entry_by_txt %s -> %s (failed)", type, v->value);
        return false;
      }
      if (!X509_NAME_entry_count(subj)) {
        raise_warning("no objects specified in config file");
        return false;
      }
    }

    if (!add_entries(subj, attribs)) return false;

    if (attr_sk) {
      for (int i = 0; i < sk_CONF_VALUE_num(attr_sk); i++) {
        CONF_VALUE *v = sk_CONF_VALUE_value(attr_sk, i);
        /* if it is already set, skip this */
        int nid = OBJ_txt2nid(v->name);
        if (X509_REQ_get_attr_by_NID(csr, nid, -1) >= 0) {
          continue;
        }
        if (!X509_REQ_add1_attr_by_txt(csr, v->name, MBSTRING_ASC,
                                       (unsigned char*)v->value, -1)) {
          /**
           * hzhao: mismatched version of conf file may have attributes that
           * are not recognizable, and I don't think it should be treated as
           * fatal errors.
           */
          Logger::Verbose("add1_attr_by_txt %s -> %s (failed)", v->name,
                          v->value);
          // return false;
        }
      }
    }
  }

  X509_REQ_set_pubkey(csr, req->priv_key);
  return true;
}

bool f_openssl_csr_export_to_file(CVarRef csr, CStrRef outfilename,
                                  bool notext /* = true */) {
  Object ocsr;
  X509_REQ *pcsr = CSRequest::Get(csr, ocsr);
  if (pcsr == NULL) return false;

  BIO *bio_out = BIO_new_file((char*)outfilename.data(), "w");
  if (bio_out == NULL) {
    raise_warning("error opening file %s", outfilename.data());
    return false;
  }

  if (!notext) {
    X509_REQ_print(bio_out, pcsr);
  }
  PEM_write_bio_X509_REQ(bio_out, pcsr);
  BIO_free(bio_out);
  return true;
}

bool f_openssl_csr_export(CVarRef csr, VRefParam out, bool notext /* = true */) {
  Object ocsr;
  X509_REQ *pcsr = CSRequest::Get(csr, ocsr);
  if (pcsr == NULL) return false;

  BIO *bio_out = BIO_new(BIO_s_mem());
  if (!notext) {
    X509_REQ_print(bio_out, pcsr);
  }

  if (PEM_write_bio_X509_REQ(bio_out, pcsr)) {
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    out = String((char*)bio_buf->data, bio_buf->length, CopyString);
    BIO_free(bio_out);
    return true;
  }

  BIO_free(bio_out);
  return false;
}

Variant f_openssl_csr_get_public_key(CVarRef csr) {
  Object ocsr;
  X509_REQ *pcsr = CSRequest::Get(csr, ocsr);
  if (pcsr == NULL) return false;

  return Object(new Key(X509_REQ_get_pubkey(pcsr)));
}

Variant f_openssl_csr_get_subject(CVarRef csr,
                                  bool use_shortnames /* = true */) {
  Object ocsr;
  X509_REQ *pcsr = CSRequest::Get(csr, ocsr);
  if (pcsr == NULL) return false;

  X509_NAME *subject = X509_REQ_get_subject_name(pcsr);
  Array ret = Array::Create();
  add_assoc_name_entry(ret, NULL, subject, use_shortnames);
  return ret;
}

Variant f_openssl_csr_new(CArrRef dn, VRefParam privkey,
                          CVarRef configargs /* = null_variant */,
                          CVarRef extraattribs /* = null_variant */) {
  Variant ret = false;
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));

  Object okey;
  X509_REQ *csr = NULL;
  vector<String> strings;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    /* Generate or use a private key */
    if (!privkey.isNull()) {
      okey = Key::Get(privkey, false);
      if (!okey.isNull()) {
        req.priv_key = okey.getTyped<Key>()->m_key;
      }
    }
    if (req.priv_key == NULL) {
      req.generatePrivateKey();
      if (req.priv_key) {
        okey = Object(new Key(req.priv_key));
      }
    }
    if (req.priv_key == NULL) {
      raise_warning("Unable to generate a private key");
    } else {
      csr = X509_REQ_new();
      if (csr && php_openssl_make_REQ(&req, csr, dn, extraattribs.toArray())) {
        X509V3_CTX ext_ctx;
        X509V3_set_ctx(&ext_ctx, NULL, NULL, csr, NULL, 0);
        X509V3_set_conf_lhash(&ext_ctx, req.req_config);

        /* Add extensions */
        if (req.request_extensions_section &&
            !X509V3_EXT_REQ_add_conf(req.req_config, &ext_ctx,
                                     (char*)req.request_extensions_section, csr)) {
          raise_warning("Error loading extension section %s",
                          req.request_extensions_section);
        } else {
          ret = true;
          if (X509_REQ_sign(csr, req.priv_key, req.digest)) {
            ret = Object(new CSRequest(csr));
            csr = NULL;
          } else {
            raise_warning("Error signing request");
          }

          privkey = okey;
        }
      }
    }
  }
  if (csr) {
    X509_REQ_free(csr);
  }
  php_openssl_dispose_config(&req);
  return ret;
}

Variant f_openssl_csr_sign(CVarRef csr, CVarRef cacert, CVarRef priv_key,
                           int days, CVarRef configargs /* = null_variant */,
                           int serial /* = 0 */) {
  Object ocsr;
  X509_REQ *pcsr = CSRequest::Get(csr, ocsr);
  if (pcsr == NULL) return false;

  Object ocert;
  if (!cacert.isNull()) {
    ocert = Certificate::Get(cacert);
    if (ocert.isNull()) {
      raise_warning("cannot get cert from parameter 2");
      return false;
    }
  }
  Object okey = Key::Get(priv_key, false);
  if (okey.isNull()) {
    raise_warning("cannot get private key from parameter 3");
    return false;
  }
  X509 *cert = NULL;
  if (!ocert.isNull()) {
    cert = ocert.getTyped<Certificate>()->m_cert;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  if (cert && !X509_check_private_key(cert, pkey)) {
    raise_warning("private key does not correspond to signing cert");
    return false;
  }

  Object onewcert;
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));
  Variant ret = false;
  vector<String> strings;
  if (!php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    goto cleanup;
  }

  /* Check that the request matches the signature */
  EVP_PKEY *key;
  key = X509_REQ_get_pubkey(pcsr);
  if (key == NULL) {
    raise_warning("error unpacking public key");
    goto cleanup;
  }
  int i;
  i = X509_REQ_verify(pcsr, key);
  if (i < 0) {
    raise_warning("Signature verification problems");
    goto cleanup;
  }
  if (i == 0) {
    raise_warning("Signature did not match the certificate request");
    goto cleanup;
  }

  /* Now we can get on with it */
  X509 *new_cert;
  new_cert = X509_new();
  if (new_cert == NULL) {
    raise_warning("No memory");
    goto cleanup;
  }
  onewcert = Object(new Certificate(new_cert));
  /* Version 3 cert */
  if (!X509_set_version(new_cert, 2)) {
    goto cleanup;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(new_cert), serial);
  X509_set_subject_name(new_cert, X509_REQ_get_subject_name(pcsr));

  if (cert == NULL) {
    cert = new_cert;
  }
  if (!X509_set_issuer_name(new_cert, X509_get_subject_name(cert))) {
    goto cleanup;
  }
  X509_gmtime_adj(X509_get_notBefore(new_cert), 0);
  X509_gmtime_adj(X509_get_notAfter(new_cert), (long)60 * 60 * 24 * days);
  i = X509_set_pubkey(new_cert, key);
  if (!i) {
    goto cleanup;
  }
  if (req.extensions_section) {
    X509V3_CTX ctx;
    X509V3_set_ctx(&ctx, cert, new_cert, pcsr, NULL, 0);
    X509V3_set_conf_lhash(&ctx, req.req_config);
    if (!X509V3_EXT_add_conf(req.req_config, &ctx, (char*)req.extensions_section,
                             new_cert)) {
      goto cleanup;
    }
  }

  /* Now sign it */
  if (!X509_sign(new_cert, pkey, req.digest)) {
    raise_warning("failed to sign it");
    goto cleanup;
  }

  /* Succeeded; lets return the cert */
  ret = onewcert;

 cleanup:
  php_openssl_dispose_config(&req);
  return ret;
}

Variant f_openssl_error_string() {
  char buf[512];
  unsigned long val = ERR_get_error();
  if (val) {
    return String(ERR_error_string(val, buf), CopyString);
  }
  return false;
}

void f_openssl_free_key(CObjRef key) {
  return f_openssl_pkey_free(key);
}

bool f_openssl_open(CStrRef sealed_data, VRefParam open_data, CStrRef env_key,
                    CVarRef priv_key_id) {
  Object okey = Key::Get(priv_key_id, false);
  if (okey.isNull()) {
    raise_warning("unable to coerce parameter 4 into a private key");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;

  unsigned char *buf = (unsigned char *)malloc(sealed_data.size() + 1);

  EVP_CIPHER_CTX ctx;
  int len1, len2;
  if (!EVP_OpenInit(&ctx, EVP_rc4(), (unsigned char *)env_key.data(),
                    env_key.size(), NULL, pkey) ||
      !EVP_OpenUpdate(&ctx, buf, &len1, (unsigned char *)sealed_data.data(),
                      sealed_data.size()) ||
      !EVP_OpenFinal(&ctx, buf + len1, &len2) ||
      len1 + len2 == 0) {
    free(buf);
    return false;
  }
  buf[len1 + len2] = '\0';
  open_data = String((char*)buf, AttachString);
  return true;
}

static STACK_OF(X509) *php_array_to_X509_sk(CVarRef certs) {
  STACK_OF(X509) *pcerts = sk_X509_new_null();
  Array arrCerts;
  if (certs.is(KindOfArray)) {
    arrCerts = certs.toArray();
  } else {
    arrCerts.append(certs);
  }
  for (ArrayIter iter(arrCerts); iter; ++iter) {
    Object ocert = Certificate::Get(iter.second());
    if (ocert.isNull()) {
      break;
    }
    sk_X509_push(pcerts, ocert.getTyped<Certificate>()->m_cert);
  }
  return pcerts;
}

static bool openssl_pkcs12_export_impl(CVarRef x509, BIO *bio_out,
                                       CVarRef priv_key, CStrRef pass,
                                       CVarRef args /* = null_variant */) {
  Object ocert = Certificate::Get(x509);
  if (ocert.isNull()) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  Object okey = Key::Get(priv_key, false);
  if (okey.isNull()) {
    raise_warning("cannot get private key from parameter 3");
    return false;
  }
  X509 *cert = ocert.getTyped<Certificate>()->m_cert;
  EVP_PKEY *key = okey.getTyped<Key>()->m_key;
  if (cert && !X509_check_private_key(cert, key)) {
    raise_warning("private key does not correspond to cert");
    return false;
  }

  Array arrArgs = args.toArray();

  String friendly_name;
  if (arrArgs.exists("friendly_name")) {
    friendly_name = arrArgs["friendly_name"].toString();
  }

  STACK_OF(X509) *ca = NULL;
  if (arrArgs.exists("extracerts")) {
    ca = php_array_to_X509_sk(arrArgs["extracerts"]);
  }

  PKCS12 *p12 = PKCS12_create
    ((char*)pass.data(),
     (char*)(friendly_name.empty() ? NULL : friendly_name.data()),
     key, cert, ca, 0, 0, 0, 0, 0);

  ASSERT(bio_out);
  bool ret = i2d_PKCS12_bio(bio_out, p12);
  PKCS12_free(p12);
  sk_X509_free(ca);
  return ret;
}

bool f_openssl_pkcs12_export_to_file(CVarRef x509, CStrRef filename,
                                     CVarRef priv_key, CStrRef pass,
                                     CVarRef args /* = null_variant */) {
  BIO *bio_out = BIO_new_file(filename.data(), "w");
  if (bio_out == NULL) {
    raise_warning("error opening file %s", filename.data());
    return false;
  }
  bool ret = openssl_pkcs12_export_impl(x509, bio_out, priv_key, pass, args);
  BIO_free(bio_out);
  return ret;
}

bool f_openssl_pkcs12_export(CVarRef x509, VRefParam out, CVarRef priv_key,
                             CStrRef pass, CVarRef args /* = null_variant */) {
  BIO *bio_out = BIO_new(BIO_s_mem());
  bool ret = openssl_pkcs12_export_impl(x509, bio_out, priv_key, pass, args);
  if (ret) {
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    out = String((char*)bio_buf->data, bio_buf->length, CopyString);
  }
  BIO_free(bio_out);
  return ret;
}

bool f_openssl_pkcs12_read(CStrRef pkcs12, VRefParam certs, CStrRef pass) {
  Variant &vcerts = certs;
  bool ret = false;
  PKCS12 *p12 = NULL;

  BIO *bio_in = BIO_new(BIO_s_mem());
  if (!BIO_write(bio_in, pkcs12.data(), pkcs12.size())) {
    goto cleanup;
  }

  if (d2i_PKCS12_bio(bio_in, &p12)) {
    EVP_PKEY *pkey = NULL;
    X509 *cert = NULL;
    STACK_OF(X509) *ca = NULL;
    if (PKCS12_parse(p12, pass.data(), &pkey, &cert, &ca)) {
      vcerts = Array::Create();
      BIO *bio_out = BIO_new(BIO_s_mem());
      if (PEM_write_bio_X509(bio_out, cert)) {
        BUF_MEM *bio_buf;
        BIO_get_mem_ptr(bio_out, &bio_buf);
        vcerts.set("cert", String((char*)bio_buf->data, bio_buf->length,
                                 CopyString));
      }
      BIO_free(bio_out);

      bio_out = BIO_new(BIO_s_mem());
      if (PEM_write_bio_PrivateKey(bio_out, pkey, NULL, NULL, 0, 0, NULL)) {
        BUF_MEM *bio_buf;
        BIO_get_mem_ptr(bio_out, &bio_buf);
        vcerts.set("pkey", String((char*)bio_buf->data, bio_buf->length,
                                 CopyString));
      }
      BIO_free(bio_out);

      Array extracerts;
      for (X509 *aCA = sk_X509_pop(ca); aCA; aCA = sk_X509_pop(ca)) {
        bio_out = BIO_new(BIO_s_mem());
        if (PEM_write_bio_X509(bio_out, aCA)) {
          BUF_MEM *bio_buf;
          BIO_get_mem_ptr(bio_out, &bio_buf);
          extracerts.append(String((char*)bio_buf->data, bio_buf->length,
                                   CopyString));
        }
        BIO_free(bio_out);
        X509_free(aCA);
      }
      if (ca) {
        sk_X509_free(ca);
        vcerts.set("extracerts", extracerts);
      }
      ret = true;
      PKCS12_free(p12);
    }
  }

 cleanup:
  if (bio_in) {
    BIO_free(bio_in);
  }

  return ret;
}

bool f_openssl_pkcs7_decrypt(CStrRef infilename, CStrRef outfilename,
                             CVarRef recipcert,
                             CVarRef recipkey /* = null_variant */) {
  bool ret = false;
  BIO *in = NULL, *out = NULL, *datain = NULL;
  PKCS7 *p7 = NULL;
  Object okey;

  Object ocert = Certificate::Get(recipcert);
  if (ocert.isNull()) {
    raise_warning("unable to coerce parameter 3 to x509 cert");
    goto clean_exit;
  }

  okey = Key::Get(recipkey.isNull() ? recipcert : recipkey, false);
  if (okey.isNull()) {
    raise_warning("unable to get private key");
    goto clean_exit;
  }

  in = BIO_new_file(infilename.data(), "r");
  if (in == NULL) {
    raise_warning("error opening the file, %s", infilename.data());
    goto clean_exit;
  }
  out = BIO_new_file(outfilename.data(), "w");
  if (out == NULL) {
    raise_warning("error opening the file, %s", outfilename.data());
    goto clean_exit;
  }

  p7 = SMIME_read_PKCS7(in, &datain);
  if (p7 == NULL) {
    goto clean_exit;
  }
  ASSERT(okey.getTyped<Key>()->m_key);
  ASSERT(ocert.getTyped<Certificate>()->m_cert);
  if (PKCS7_decrypt(p7, okey.getTyped<Key>()->m_key,
                    ocert.getTyped<Certificate>()->m_cert, out,
                    PKCS7_DETACHED)) {
    ret = true;
  }

 clean_exit:
  PKCS7_free(p7);
  BIO_free(datain);
  BIO_free(in);
  BIO_free(out);

  return ret;
}

static void print_headers(BIO *outfile, CArrRef headers) {
  if (!headers.isNull()) {
    if (headers->isVectorData()) {
      for (ArrayIter iter(headers); iter; ++iter) {
        BIO_printf(outfile, "%s\n", iter.second().toString().data());
      }
    } else {
      for (ArrayIter iter(headers); iter; ++iter) {
        BIO_printf(outfile, "%s: %s\n", iter.first().toString().data(),
                   iter.second().toString().data());
      }
    }
  }
}

bool f_openssl_pkcs7_encrypt(CStrRef infilename, CStrRef outfilename,
                             CVarRef recipcerts, CArrRef headers,
                             int flags /* = 0 */,
                             int cipherid /* = k_OPENSSL_CIPHER_RC2_40 */) {
  bool ret = false;
  BIO *infile = NULL, *outfile = NULL;
  STACK_OF(X509) *precipcerts = NULL;
  PKCS7 *p7 = NULL;
  const EVP_CIPHER *cipher = NULL;

  infile = BIO_new_file(infilename.data(), "r");
  if (infile == NULL) {
    raise_warning("error opening the file, %s", infilename.data());
    goto clean_exit;
  }
  outfile = BIO_new_file(outfilename, "w");
  if (outfile == NULL) {
    raise_warning("error opening the file, %s", outfilename.data());
    goto clean_exit;
  }

  precipcerts = php_array_to_X509_sk(recipcerts);

  /* sanity check the cipher */
  switch (cipherid) {
#ifndef OPENSSL_NO_RC2
  case PHP_OPENSSL_CIPHER_RC2_40:  cipher = EVP_rc2_40_cbc();   break;
  case PHP_OPENSSL_CIPHER_RC2_64:  cipher = EVP_rc2_64_cbc();   break;
  case PHP_OPENSSL_CIPHER_RC2_128: cipher = EVP_rc2_cbc();      break;
#endif
#ifndef OPENSSL_NO_DES
  case PHP_OPENSSL_CIPHER_DES:     cipher = EVP_des_cbc();      break;
  case PHP_OPENSSL_CIPHER_3DES:    cipher = EVP_des_ede3_cbc(); break;
#endif
  default:
    raise_warning("Invalid cipher type `%ld'", cipherid);
    goto clean_exit;
  }
  if (cipher == NULL) {
    raise_warning("Failed to get cipher");
    goto clean_exit;
  }

  p7 = PKCS7_encrypt(precipcerts, infile, (EVP_CIPHER*)cipher, flags);
  if (p7 == NULL) goto clean_exit;

  print_headers(outfile, headers);
  (void)BIO_reset(infile);
  SMIME_write_PKCS7(outfile, p7, infile, flags);
  ret = true;

 clean_exit:
  PKCS7_free(p7);
  BIO_free(infile);
  BIO_free(outfile);
  sk_X509_free(precipcerts);
  return ret;
}

bool f_openssl_pkcs7_sign(CStrRef infilename, CStrRef outfilename,
                          CVarRef signcert, CVarRef privkey, CVarRef headers,
                          int flags /* = k_PKCS7_DETACHED */,
                          CStrRef extracerts /* = null_string */) {
  bool ret = false;
  STACK_OF(X509) *others = NULL;
  BIO *infile = NULL, *outfile = NULL;
  PKCS7 *p7 = NULL;
  Object okey, ocert;

  if (!extracerts.empty()) {
    others = load_all_certs_from_file(extracerts.data());
    if (others == NULL) {
      goto clean_exit;
    }
  }

  okey = Key::Get(privkey, false);
  if (okey.isNull()) {
    raise_warning("error getting private key");
    goto clean_exit;
  }
  EVP_PKEY *key;
  key = okey.getTyped<Key>()->m_key;

  ocert = Certificate::Get(signcert);
  if (ocert.isNull()) {
    raise_warning("error getting cert");
    goto clean_exit;
  }
  X509 *cert;
  cert = ocert.getTyped<Certificate>()->m_cert;

  infile = BIO_new_file(infilename.data(), "r");
  if (infile == NULL) {
    raise_warning("error opening input file %s!", infilename.data());
    goto clean_exit;
  }

  outfile = BIO_new_file(outfilename.data(), "w");
  if (outfile == NULL) {
    raise_warning("error opening output file %s!", outfilename.data());
    goto clean_exit;
  }

  p7 = PKCS7_sign(cert, key, others, infile, flags);
  if (p7 == NULL) {
    raise_warning("error creating PKCS7 structure!");
    goto clean_exit;
  }

  print_headers(outfile, headers.toArray());
  (void)BIO_reset(infile);
  SMIME_write_PKCS7(outfile, p7, infile, flags);
  ret = true;

 clean_exit:
  PKCS7_free(p7);
  BIO_free(infile);
  BIO_free(outfile);
  if (others) {
    sk_X509_pop_free(others, X509_free);
  }

  return ret;
}

Variant f_openssl_pkcs7_verify(CStrRef filename, int flags,
                               CStrRef outfilename /* = null_string */,
                               CArrRef cainfo /* = null_array */,
                               CStrRef extracerts /* = null_string */,
                               CStrRef content /* = null_string */) {
  Variant ret = -1;
  X509_STORE *store = NULL;
  BIO *in = NULL;
  PKCS7 *p7 = NULL;
  BIO *datain = NULL;
  BIO *dataout = NULL;

  STACK_OF(X509) *others = NULL;
  if (!extracerts.empty()) {
    others = load_all_certs_from_file(extracerts.data());
    if (others == NULL) {
      goto clean_exit;
    }
  }

  flags = flags & ~PKCS7_DETACHED;

  store = setup_verify(cainfo);
  if (!store) {
    goto clean_exit;
  }

  in = BIO_new_file(filename.data(), (flags & PKCS7_BINARY) ? "rb" : "r");
  if (in == NULL) {
    raise_warning("error opening the file, %s", filename.data());
    goto clean_exit;
  }

  p7 = SMIME_read_PKCS7(in, &datain);
  if (p7 == NULL) {
    goto clean_exit;
  }

  if (!content.empty()) {
    dataout = BIO_new_file(content.data(), "w");
    if (dataout == NULL) {
      raise_warning("error opening the file, %s", content.data());
      goto clean_exit;
    }
  }

  if (PKCS7_verify(p7, others, store, datain, dataout, flags)) {
    ret = true;
    if (!outfilename.empty()) {
      BIO *certout = BIO_new_file(outfilename.data(), "w");
      if (certout) {
        STACK_OF(X509) *signers = PKCS7_get0_signers(p7, NULL, flags);
        for (int i = 0; i < sk_X509_num(signers); i++) {
          PEM_write_bio_X509(certout, sk_X509_value(signers, i));
        }
        BIO_free(certout);
        sk_X509_free(signers);
      } else {
        raise_warning("signature OK, but cannot open %s for writing",
                        outfilename.data());
        ret = -1;
      }
    }
    goto clean_exit;
  } else {
    ret = false;
  }

 clean_exit:
  X509_STORE_free(store);
  BIO_free(datain);
  BIO_free(in);
  BIO_free(dataout);
  PKCS7_free(p7);
  sk_X509_free(others);

  return ret;
}

static bool openssl_pkey_export_impl(CVarRef key, BIO *bio_out,
                                     CStrRef passphrase /* = null_string */,
                                     CVarRef configargs /* = null_variant */) {
  Object okey = Key::Get(key, false, passphrase.data());
  if (okey.isNull()) {
    raise_warning("cannot get key from parameter 1");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;

  struct php_x509_request req;
  memset(&req, 0, sizeof(req));
  vector<String> strings;
  bool ret = false;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    const EVP_CIPHER *cipher;
    if (!passphrase.empty() && req.priv_key_encrypt) {
      cipher = (EVP_CIPHER *)EVP_des_ede3_cbc();
    } else {
      cipher = NULL;
    }
    ASSERT(bio_out);
    ret = PEM_write_bio_PrivateKey(bio_out, pkey, cipher,
                                   (unsigned char *)passphrase.data(),
                                   passphrase.size(), NULL, NULL);
  }
  php_openssl_dispose_config(&req);
  return ret;
}

bool f_openssl_pkey_export_to_file(CVarRef key,
                                   CStrRef outfilename,
                                   CStrRef passphrase /* = null_string */,
                                   CVarRef configargs /* = null_variant */) {
  BIO *bio_out = BIO_new_file(outfilename.data(), "w");
  if (bio_out == NULL) {
    raise_warning("error opening the file, %s", outfilename.data());
    return false;
  }
  bool ret = openssl_pkey_export_impl(key, bio_out, passphrase, configargs);
  BIO_free(bio_out);
  return ret;
}

bool f_openssl_pkey_export(CVarRef key, VRefParam out,
                           CStrRef passphrase /* = null_string */,
                           CVarRef configargs /* = null_variant */) {
  BIO *bio_out = BIO_new(BIO_s_mem());
  bool ret = openssl_pkey_export_impl(key, bio_out, passphrase, configargs);
  if (ret) {
    char *bio_mem_ptr;
    long bio_mem_len = BIO_get_mem_data(bio_out, &bio_mem_ptr);
    out = String(bio_mem_ptr, bio_mem_len, CopyString);
  }
  BIO_free(bio_out);
  return ret;
}

void f_openssl_pkey_free(CObjRef key) {
  // do nothing
}

Array f_openssl_pkey_get_details(CObjRef key) {
  EVP_PKEY *pkey = key.getTyped<Key>()->m_key;
  BIO *out = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(out, pkey);
  char *pbio;
  unsigned int pbio_len = BIO_get_mem_data(out, &pbio);

  Array ret;
  ret.set("bits", EVP_PKEY_bits(pkey));
  ret.set("key", String(pbio, pbio_len, CopyString));
  long ktype = -1;
  switch (EVP_PKEY_type(pkey->type)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:    ktype = k_OPENSSL_KEYTYPE_RSA;   break;
  case EVP_PKEY_DSA:
  case EVP_PKEY_DSA2:
  case EVP_PKEY_DSA3:
  case EVP_PKEY_DSA4:    ktype = k_OPENSSL_KEYTYPE_DSA;   break;
  case EVP_PKEY_DH:      ktype = k_OPENSSL_KEYTYPE_DH;    break;
#ifdef EVP_PKEY_EC
  case EVP_PKEY_EC:      ktype = k_OPENSSL_KEYTYPE_EC;    break;
#endif
  }
  ret.set("type", ktype);
  BIO_free(out);
  return ret;
}

Variant f_openssl_pkey_get_private(CVarRef key,
                                   CStrRef passphrase /* = null_string */) {
  Object okey = Key::Get(key, false, passphrase.data());
  if (okey.isNull()) {
    return false;
  }
  return okey;
}

Variant f_openssl_get_privatekey(CVarRef key,
                                 CStrRef passphrase /* = null_string */) {
  return f_openssl_pkey_get_private(key, passphrase);
}

Variant f_openssl_pkey_get_public(CVarRef certificate) {
  Object okey = Key::Get(certificate, true);
  if (okey.isNull()) {
    return false;
  }
  return okey;
}

Variant f_openssl_get_publickey(CVarRef certificate) {
  return f_openssl_pkey_get_public(certificate);
}

Object f_openssl_pkey_new(CVarRef configargs /* = null_variant */) {
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));

  Object ret;
  vector<String> strings;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings) &&
      req.generatePrivateKey()) {
    ret = Object(new Key(req.priv_key));
  }

  php_openssl_dispose_config(&req);
  return ret;
}

bool f_openssl_private_decrypt(CStrRef data, VRefParam decrypted, CVarRef key,
                               int padding /* = k_OPENSSL_PKCS1_PADDING */) {
  Object okey = Key::Get(key, false);
  if (okey.isNull()) {
    raise_warning("key parameter is not a valid private key");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  unsigned char *cryptedbuf = (unsigned char *)malloc(cryptedlen + 1);

  int successful = 0;
  switch (pkey->type) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    cryptedlen = RSA_private_decrypt(data.size(),
                                     (unsigned char *)data.data(),
                                     cryptedbuf,
                                     pkey->pkey.rsa,
                                     padding);
    if (cryptedlen != -1) {
      successful = 1;
    }
    break;

  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    cryptedbuf[cryptedlen] = '\0';
    decrypted = String((char*)cryptedbuf, cryptedlen, AttachString);
    return true;
  }

  free(cryptedbuf);
  return false;
}

bool f_openssl_private_encrypt(CStrRef data, VRefParam crypted, CVarRef key,
                               int padding /* = k_OPENSSL_PKCS1_PADDING */) {
  Object okey = Key::Get(key, false);
  if (okey.isNull()) {
    raise_warning("key param is not a valid private key");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  unsigned char *cryptedbuf = (unsigned char *)malloc(cryptedlen + 1);

  int successful = 0;
  switch (pkey->type) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    successful = (RSA_private_encrypt(data.size(),
                                      (unsigned char *)data.data(),
                                      cryptedbuf,
                                      pkey->pkey.rsa,
                                      padding) == cryptedlen);
    break;
  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    cryptedbuf[cryptedlen] = '\0';
    crypted = String((char*)cryptedbuf, cryptedlen, AttachString);
    return true;
  }

  free(cryptedbuf);
  return false;
}

bool f_openssl_public_decrypt(CStrRef data, VRefParam decrypted, CVarRef key,
                              int padding /* = k_OPENSSL_PKCS1_PADDING */) {
  Object okey = Key::Get(key, true);
  if (okey.isNull()) {
    raise_warning("key parameter is not a valid public key");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  unsigned char *cryptedbuf = (unsigned char *)malloc(cryptedlen + 1);

  int successful = 0;
  switch (pkey->type) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    cryptedlen = RSA_public_decrypt(data.size(),
                                    (unsigned char *)data.data(),
                                    cryptedbuf,
                                    pkey->pkey.rsa,
                                    padding);
    if (cryptedlen != -1) {
      successful = 1;
    }
    break;

  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    cryptedbuf[cryptedlen] = '\0';
    decrypted = String((char*)cryptedbuf, cryptedlen, AttachString);
    return true;
  }

  free(cryptedbuf);
  return false;
}

bool f_openssl_public_encrypt(CStrRef data, VRefParam crypted, CVarRef key,
                              int padding /* = k_OPENSSL_PKCS1_PADDING */) {
  Object okey = Key::Get(key, true);
  if (okey.isNull()) {
    raise_warning("key parameter is not a valid public key");
    return false;
  }
  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  unsigned char *cryptedbuf = (unsigned char *)malloc(cryptedlen + 1);

  int successful = 0;
  switch (pkey->type) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    successful = (RSA_public_encrypt(data.size(),
                                     (unsigned char *)data.data(),
                                     cryptedbuf,
                                     pkey->pkey.rsa,
                                     padding) == cryptedlen);
    break;
  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    cryptedbuf[cryptedlen] = '\0';
    crypted = String((char*)cryptedbuf, cryptedlen, AttachString);
    return true;
  }

  free(cryptedbuf);
  return false;
}

Variant f_openssl_seal(CStrRef data, VRefParam sealed_data, VRefParam env_keys,
                       CArrRef pub_key_ids) {
  int nkeys = pub_key_ids.size();
  if (nkeys == 0) {
    raise_warning("Fourth argument to openssl_seal() must be "
                    "a non-empty array");
    return false;
  }

  EVP_PKEY **pkeys = (EVP_PKEY**)malloc(nkeys * sizeof(*pkeys));
  int *eksl = (int*)malloc(nkeys * sizeof(*eksl));
  unsigned char **eks = (unsigned char **)malloc(nkeys * sizeof(*eks));
  memset(eks, 0, sizeof(*eks) * nkeys);
  vector<Object> holder;

  /* get the public keys we are using to seal this data */
  bool ret = true;
  int i = 0;
  for (ArrayIter iter(pub_key_ids); iter; ++iter, ++i) {
    Object okey = Key::Get(iter.second(), true);
    if (okey.isNull()) {
      raise_warning("not a public key (%dth member of pubkeys)", i + 1);
      ret = false;
      goto clean_exit;
    }
    holder.push_back(okey);
    pkeys[i] = okey.getTyped<Key>()->m_key;
    eks[i] = (unsigned char *)malloc(EVP_PKEY_size(pkeys[i]) + 1);
  }

  EVP_CIPHER_CTX ctx;
  if (!EVP_EncryptInit(&ctx, EVP_rc4(), NULL, NULL)) {
    ret = false;
    goto clean_exit;
  }

  int len1, len2;

  unsigned char *buf;
  buf = (unsigned char *)malloc(data.size() + EVP_CIPHER_CTX_block_size(&ctx));
  if (!EVP_SealInit(&ctx, EVP_rc4(), eks, eksl, NULL, pkeys, nkeys) ||
      !EVP_SealUpdate(&ctx, buf, &len1, (unsigned char *)data.data(),
                      data.size())) {
    ret = false;
    free(buf);
    goto clean_exit;
  }

  EVP_SealFinal(&ctx, buf + len1, &len2);
  if (len1 + len2 > 0) {
    buf[len1 + len2] = '\0';
    sealed_data = String((char*)buf, len1 + len2, AttachString);

    Array ekeys;
    for (int i = 0; i < nkeys; i++) {
      eks[i][eksl[i]] = '\0';
      ekeys.append(String((char*)eks[i], eksl[i], AttachString));
      eks[i] = NULL;
    }
    env_keys = ekeys;

  } else {
    free(buf);
  }

 clean_exit:
  for (int i = 0; i < nkeys; i++) {
    if (eks[i]) free(eks[i]);
  }
  free(eks);
  free(eksl);
  free(pkeys);

  if (ret) return len1 + len2;
  return false;
}

static const EVP_MD *php_openssl_get_evp_md_from_algo(long algo) {
  switch (algo) {
  case OPENSSL_ALGO_SHA1: return EVP_sha1();
  case OPENSSL_ALGO_MD5:  return EVP_md5();
  case OPENSSL_ALGO_MD4:  return EVP_md4();
#ifdef HAVE_OPENSSL_MD2_H
  case OPENSSL_ALGO_MD2:  return EVP_md2();
#endif
  case OPENSSL_ALGO_DSS1: return EVP_dss1();
  }
  return NULL;
}

bool f_openssl_sign(CStrRef data, VRefParam signature, CVarRef priv_key_id,
                    int signature_alg /* = k_OPENSSL_ALGO_SHA1 */) {
  Object okey = Key::Get(priv_key_id, false);
  if (okey.isNull()) {
    raise_warning("supplied key param cannot be coerced into a private key");
    return false;
  }

  const EVP_MD *mdtype = php_openssl_get_evp_md_from_algo(signature_alg);
  if (!mdtype) {
    raise_warning("Unknown signature algorithm.");
    return false;
  }

  EVP_PKEY *pkey = okey.getTyped<Key>()->m_key;
  int siglen = EVP_PKEY_size(pkey);
  unsigned char *sigbuf = (unsigned char *)malloc(siglen + 1);

  EVP_MD_CTX md_ctx;
  EVP_SignInit(&md_ctx, mdtype);
  EVP_SignUpdate(&md_ctx, (unsigned char *)data.data(), data.size());
  if (EVP_SignFinal(&md_ctx, sigbuf, (unsigned int *)&siglen, pkey)) {
    sigbuf[siglen] = '\0';
    signature = String((char*)sigbuf, siglen, AttachString);
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
    EVP_MD_CTX_cleanup(&md_ctx);
#endif
    return true;
  }
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
  EVP_MD_CTX_cleanup(&md_ctx);
#endif
  free(sigbuf);
  return false;
}

Variant f_openssl_verify(CStrRef data, CStrRef signature, CVarRef pub_key_id,
                         int signature_alg /* = k_OPENSSL_ALGO_SHA1 */) {
  const EVP_MD *mdtype = php_openssl_get_evp_md_from_algo(signature_alg);
  int err;

  if (!mdtype) {
    raise_warning("Unknown signature algorithm.");
    return false;
  }

  Object okey = Key::Get(pub_key_id, true);
  if (okey.isNull()) {
    raise_warning("supplied key param cannot be coerced into a public key");
    return false;
  }

  EVP_MD_CTX md_ctx;
  EVP_VerifyInit(&md_ctx, mdtype);
  EVP_VerifyUpdate(&md_ctx, (unsigned char*)data.data(), data.size());
  err = EVP_VerifyFinal(&md_ctx, (unsigned char *)signature.data(),
                         signature.size(), okey.getTyped<Key>()->m_key);
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
  EVP_MD_CTX_cleanup(&md_ctx);
#endif
  return err;
}

bool f_openssl_x509_check_private_key(CVarRef cert, CVarRef key) {
  Object ocert = Certificate::Get(cert);
  if (ocert.isNull()) {
    return false;
  }
  Object okey = Key::Get(key, false);
  if (okey.isNull()) {
    return false;
  }
  return X509_check_private_key(ocert.getTyped<Certificate>()->m_cert,
                                okey.getTyped<Key>()->m_key);
}

static int check_cert(X509_STORE *ctx, X509 *x, STACK_OF(X509) *untrustedchain,
                      int purpose) {
  X509_STORE_CTX *csc = X509_STORE_CTX_new();
  if (csc == NULL) {
    raise_warning("memory allocation failure");
    return 0;
  }
  X509_STORE_CTX_init(csc, ctx, x, untrustedchain);

  if (purpose >= 0) {
    X509_STORE_CTX_set_purpose(csc, purpose);
  }

  int ret = X509_verify_cert(csc);
  X509_STORE_CTX_free(csc);
  return ret;
}

int f_openssl_x509_checkpurpose(CVarRef x509cert, int purpose,
                                CArrRef cainfo /* = null_array */,
                                CStrRef untrustedfile /* = null_string */) {
  int ret = -1;
  STACK_OF(X509) *untrustedchain = NULL;
  X509_STORE *pcainfo = NULL;
  Object ocert;

  if (!untrustedfile.empty()) {
    untrustedchain = load_all_certs_from_file(untrustedfile.data());
    if (untrustedchain == NULL) {
      goto clean_exit;
    }
  }

  pcainfo = setup_verify(cainfo);
  if (pcainfo == NULL) {
    goto clean_exit;
  }

  ocert = Certificate::Get(x509cert);
  if (ocert.isNull()) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  X509 *cert;
  cert = ocert.getTyped<Certificate>()->m_cert;
  ASSERT(cert);

  ret = check_cert(pcainfo, cert, untrustedchain, purpose);

 clean_exit:
  if (pcainfo) {
    X509_STORE_free(pcainfo);
  }
  if (untrustedchain) {
    sk_X509_pop_free(untrustedchain, X509_free);
  }
  return ret;
}

static bool openssl_x509_export_impl(CVarRef x509, BIO *bio_out,
                                     bool notext /* = true */) {
  Object ocert = Certificate::Get(x509);
  if (ocert.isNull()) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  X509 *cert = ocert.getTyped<Certificate>()->m_cert;
  ASSERT(cert);

  ASSERT(bio_out);
  if (!notext) {
    X509_print(bio_out, cert);
  }
  return PEM_write_bio_X509(bio_out, cert);
}

bool f_openssl_x509_export_to_file(CVarRef x509, CStrRef outfilename,
                                   bool notext /* = true */) {
  BIO *bio_out = BIO_new_file((char*)outfilename.data(), "w");
  if (bio_out == NULL) {
    raise_warning("error opening file %s", outfilename.data());
    return false;
  }
  bool ret = openssl_x509_export_impl(x509, bio_out, notext);
  BIO_free(bio_out);
  return ret;
}

bool f_openssl_x509_export(CVarRef x509, VRefParam output,
                           bool notext /* = true */) {
  BIO *bio_out = BIO_new(BIO_s_mem());
  bool ret = openssl_x509_export_impl(x509, bio_out, notext);
  if (ret) {
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    output = String(bio_buf->data, bio_buf->length, CopyString);
  }
  BIO_free(bio_out);
  return ret;
}

void f_openssl_x509_free(CObjRef x509cert) {
  // do nothing
}

/**
 * This is how the time string is formatted:
 *
 * snprintf(p, sizeof(p), "%02d%02d%02d%02d%02d%02dZ",ts->tm_year%100,
 *          ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
 */
static time_t asn1_time_to_time_t(ASN1_UTCTIME *timestr) {
  if (timestr->length < 13) {
    raise_warning("extension author too lazy to parse %s correctly",
                    timestr->data);
    return (time_t)-1;
  }

  char *strbuf = strdup((char*)timestr->data);

  struct tm thetime;
  memset(&thetime, 0, sizeof(thetime));

  /* we work backwards so that we can use atoi more easily */
  char *thestr = strbuf + timestr->length - 3;
  thetime.tm_sec  = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_min  = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_hour = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_mday = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_mon  = atoi(thestr)-1; *thestr = '\0';  thestr -= 2;
  thetime.tm_year = atoi(thestr);
  if (thetime.tm_year < 68) {
    thetime.tm_year += 100;
  }

  thetime.tm_isdst = -1;
  time_t ret = mktime(&thetime);

  long gmadjust = 0;
#if HAVE_TM_GMTOFF
  gmadjust = thetime.tm_gmtoff;
#else
  /**
   * If correcting for daylight savings time, we set the adjustment to
   * the value of timezone - 3600 seconds. Otherwise, we need to overcorrect
   * and set the adjustment to the main timezone + 3600 seconds.
   */
  gmadjust = -(thetime.tm_isdst ?
               (long)timezone - 3600 : (long)timezone + 3600);
#endif
  ret += gmadjust;
  free(strbuf);
  return ret;
}

Variant f_openssl_x509_parse(CVarRef x509cert, bool shortnames /* = true */) {
  Object ocert = Certificate::Get(x509cert);
  if (ocert.isNull()) {
    return false;
  }
  X509 *cert = ocert.getTyped<Certificate>()->m_cert;
  ASSERT(cert);

  Array ret;
  if (cert->name) {
    ret.set("name", String(cert->name, CopyString));
  }
  add_assoc_name_entry(ret, "subject", X509_get_subject_name(cert),
                       shortnames);
  /* hash as used in CA directories to lookup cert by subject name */
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "%08lx", X509_subject_name_hash(cert));
    ret.set("hash", String(buf, CopyString));
  }

  add_assoc_name_entry(ret, "issuer", X509_get_issuer_name(cert), shortnames);
  ret.set("version", X509_get_version(cert));

  ret.set("serialNumber", String
          (i2s_ASN1_INTEGER(NULL, X509_get_serialNumber(cert)), AttachString));

  ASN1_STRING *str = X509_get_notBefore(cert);
  ret.set("validFrom", String((char*)str->data, str->length, CopyString));
  str = X509_get_notAfter(cert);
  ret.set("validTo", String((char*)str->data, str->length, CopyString));

  ret.set("validFrom_time_t", asn1_time_to_time_t(X509_get_notBefore(cert)));
  ret.set("validTo_time_t", asn1_time_to_time_t(X509_get_notAfter(cert)));

  char *tmpstr = (char *)X509_alias_get0(cert, NULL);
  if (tmpstr) {
    ret.set("alias", String(tmpstr, CopyString));
  }

  /* NOTE: the purposes are added as integer keys - the keys match up to
     the X509_PURPOSE_SSL_XXX defines in x509v3.h */
  {
    Array subitem;
    for (int i = 0; i < X509_PURPOSE_get_count(); i++) {
      X509_PURPOSE *purp = X509_PURPOSE_get0(i);
      int id = X509_PURPOSE_get_id(purp);

      Array subsub;
      subsub.append((bool)X509_check_purpose(cert, id, 0));
      subsub.append((bool)X509_check_purpose(cert, id, 1));

      char * pname = shortnames ? X509_PURPOSE_get0_sname(purp) :
        X509_PURPOSE_get0_name(purp);
      subsub.append(String(pname, CopyString));

      subitem.set(id, subsub);
    }
    ret.set("purposes", subitem);
  }
  {
    Array subitem;
    for (int i = 0; i < X509_get_ext_count(cert); i++) {
      X509_EXTENSION *extension = X509_get_ext(cert, i);
      char *extname;
      char buf[256];
      if (OBJ_obj2nid(X509_EXTENSION_get_object(extension)) != NID_undef) {
        extname = (char*)OBJ_nid2sn(OBJ_obj2nid
                                    (X509_EXTENSION_get_object(extension)));
      } else {
        OBJ_obj2txt(buf, sizeof(buf)-1, X509_EXTENSION_get_object(extension),
                    1);
        extname = buf;
      }
      BIO *bio_out = BIO_new(BIO_s_mem());
      if (X509V3_EXT_print(bio_out, extension, 0, 0)) {
        BUF_MEM *bio_buf;
        BIO_get_mem_ptr(bio_out, &bio_buf);
        subitem.set(String(extname, CopyString),
                    String((char*)bio_buf->data, bio_buf->length, CopyString));
      } else {
        ASN1_STRING *str = X509_EXTENSION_get_data(extension);
        subitem.set(String(extname, CopyString),
                    String((char*)str->data, str->length, CopyString));
      }
      BIO_free(bio_out);
    }
    ret.set("extensions", subitem);
  }

  return ret;
}

Variant f_openssl_x509_read(CVarRef x509certdata) {
  Object ocert = Certificate::Get(x509certdata);
  if (ocert.isNull()) {
    raise_warning("supplied parameter cannot be coerced into "
                    "an X509 certificate!");
    return false;
  }
  return ocert;
}

Variant f_openssl_random_pseudo_bytes(int length,
                                      VRefParam crypto_strong /* = false */) {
  if (length <= 0) {
    return false;
  }

  unsigned char *buffer = NULL;

  if ((buffer = (unsigned char *)malloc(length + 1)) == NULL) {
    return false;
  }

  crypto_strong = false;

  int crypto_strength = 0;

  if ((crypto_strength = RAND_pseudo_bytes(buffer, length)) < 0) {
    crypto_strong = false;
    free(buffer);
    return false;
  } else {
    crypto_strong = crypto_strength;
    buffer[length] = '\0';
    return String((char *)buffer, length, AttachString);
  }
}

Variant f_openssl_cipher_iv_length(CStrRef method) {
  if (method.empty()) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  const EVP_CIPHER *cipher_type = EVP_get_cipherbyname(method.c_str());
  if (!cipher_type) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  return EVP_CIPHER_iv_length(cipher_type);
}

static String php_openssl_validate_iv(String piv, int iv_required_len) {
  char *iv_new;

  /* Best case scenario, user behaved */
  if (piv.size() == iv_required_len) {
    return piv;
  }

  iv_new = (char*)calloc(1, iv_required_len + 1);

  if (piv.size() <= 0) {
    /* BC behavior */
    return String(iv_new, iv_required_len, AttachString);
  }

  if (piv.size() < iv_required_len) {
    raise_warning("IV passed is only %d bytes long, cipher "
                  "expects an IV of precisely %d bytes, padding with \\0",
                  piv.size(), iv_required_len);
    memcpy(iv_new, piv.data(), piv.size());
    return String(iv_new, iv_required_len, AttachString);
  }

  raise_warning("IV passed is %d bytes long which is longer than the %d "
                "expected by selected cipher, truncating", piv.size(),
                iv_required_len);
  memcpy(iv_new, piv.data(), iv_required_len);
  return String(iv_new, iv_required_len, AttachString);
}

Variant f_openssl_encrypt(CStrRef data, CStrRef method, CStrRef password,
                          int options /* = 0 */,
                          CStrRef iv /* = null_string */) {
  const EVP_CIPHER *cipher_type = EVP_get_cipherbyname(method);
  if (!cipher_type) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  int keylen = EVP_CIPHER_key_length(cipher_type);
  unsigned char *key;

  if (keylen > password.size()) {
    key = (unsigned char*)malloc(keylen);
    memset(key, 0, keylen);
    memcpy(key, password, password.size());
  } else {
    key = (unsigned char*)password.c_str();
  }

  int max_iv_len = EVP_CIPHER_iv_length(cipher_type);
  if (iv.size() <= 0 && max_iv_len > 0) {
    raise_warning("Using an empty Initialization Vector (iv) is potentially "
                  "insecure and not recommended");
  }

  int result_len = 0;

  String new_iv  = php_openssl_validate_iv(iv, max_iv_len);

  int outlen = data.size() + EVP_CIPHER_block_size(cipher_type);
  unsigned char *outbuf = (unsigned char*)malloc(outlen + 1);

  EVP_CIPHER_CTX cipher_ctx;
  EVP_EncryptInit(&cipher_ctx, cipher_type, key,
                  (unsigned char *)new_iv.data());
  if (options & k_OPENSSL_ZERO_PADDING) {
    EVP_CIPHER_CTX_set_padding(&cipher_ctx, 0);
  }
  EVP_EncryptUpdate(&cipher_ctx, outbuf, &result_len,
                    (unsigned char *)data.data(), data.size());
  outlen = result_len;

  if (EVP_EncryptFinal(&cipher_ctx, (unsigned char *)outbuf + result_len,
                       &result_len)) {
    outlen += result_len;
    outbuf[outlen] = '\0';
    String rv = String((char*)outbuf, outlen, AttachString);
    EVP_CIPHER_CTX_cleanup(&cipher_ctx);
    if (options & k_OPENSSL_RAW_DATA) {
      return rv;
    } else {
      return StringUtil::Base64Encode(rv);
    }
  }

  EVP_CIPHER_CTX_cleanup(&cipher_ctx);
  free(outbuf);
  return false;
}

Variant f_openssl_decrypt(CStrRef data, CStrRef method, CStrRef password,
                          int options /* = 0 */,
                          CStrRef iv /* = null_string */) {
  const EVP_CIPHER *cipher_type = EVP_get_cipherbyname(method);
  if (!cipher_type) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  String decoded_data = data;

  if (!(options & k_OPENSSL_RAW_DATA)) {
    decoded_data = StringUtil::Base64Decode(data);
  }

  int keylen = EVP_CIPHER_key_length(cipher_type);
  unsigned char *key;

  if (keylen > password.size()) {
    key = (unsigned char*)malloc(keylen);
    memset(key, 0, keylen);
    memcpy(key, password, password.size());
  } else {
    key = (unsigned char*)password.c_str();
  }

  int result_len = 0;

  String new_iv  = php_openssl_validate_iv(iv,
                                           EVP_CIPHER_iv_length(cipher_type));

  int outlen = decoded_data.size() + EVP_CIPHER_block_size(cipher_type);
  unsigned char *outbuf = (unsigned char*)malloc(outlen + 1);

  EVP_CIPHER_CTX cipher_ctx;
  EVP_DecryptInit(&cipher_ctx, cipher_type, key,
                  (unsigned char *)new_iv.data());
  if (options & k_OPENSSL_ZERO_PADDING) {
    EVP_CIPHER_CTX_set_padding(&cipher_ctx, 0);
  }
  EVP_DecryptUpdate(&cipher_ctx, outbuf, &result_len,
                    (unsigned char *)decoded_data.data(), decoded_data.size());
  outlen = result_len;

  if (EVP_DecryptFinal(&cipher_ctx, (unsigned char *)outbuf + result_len,
                       &result_len)) {
    outlen += result_len;
    outbuf[outlen] = '\0';
    EVP_CIPHER_CTX_cleanup(&cipher_ctx);
    return String((char*)outbuf, outlen, AttachString);
  } else {
    EVP_CIPHER_CTX_cleanup(&cipher_ctx);
    free(outbuf);
    return false;
  }
}

Variant f_openssl_digest(CStrRef data, CStrRef method,
                         bool raw_output /* = false */) {
  const EVP_MD *mdtype = EVP_get_digestbyname(method);
  
  if (!mdtype) {
    raise_warning("Unknown signature algorithm");
    return false;
  }
  int siglen = EVP_MD_size(mdtype);
  unsigned char *sigbuf = (unsigned char *)malloc(siglen + 1);
  EVP_MD_CTX md_ctx;

  EVP_DigestInit(&md_ctx, mdtype);
  EVP_DigestUpdate(&md_ctx, (unsigned char *)data.data(), data.size());
  if (EVP_DigestFinal(&md_ctx, (unsigned char *)sigbuf, (unsigned int *)&siglen)) {
    if (raw_output) {
      sigbuf[siglen] = '\0';
      return String((char*)sigbuf, siglen, AttachString);
    } else {
      String digest_str = string_bin2hex((char*)sigbuf, siglen);
      free(sigbuf);
      return String(digest_str, AttachString);
    }
  } else {
    free(sigbuf);
    return false;
  }
}

static void openssl_add_method_or_alias(const OBJ_NAME *name, void *arg)
{
  Array *ret = (Array*)arg;
  ret->append(String((char *)name->name, CopyString));
}

static void openssl_add_method(const OBJ_NAME *name, void *arg)
{
  if (name->alias == 0) {
    Array *ret = (Array*)arg;
    ret->append(String((char *)name->name, CopyString));
  }
}

Array f_openssl_get_cipher_methods(bool aliases /* = false */) {
  Array ret = Array::Create();
  OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH,
    aliases ? openssl_add_method_or_alias: openssl_add_method, 
    &ret);
  return ret;
}

Array f_openssl_get_md_methods(bool aliases /* = false */) {
  Array ret = Array::Create();
  OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_MD_METH,
    aliases ? openssl_add_method_or_alias: openssl_add_method, 
    &ret);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
