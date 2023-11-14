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

#include "hphp/runtime/ext/openssl/ext_openssl.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/ssl-socket.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/util/logger.h"

#include <folly/ScopeGuard.h>
#include <openssl/conf.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/rand.h>
#include <vector>

namespace HPHP {

#define MIN_KEY_LENGTH          384

// bitfields
const int64_t k_OPENSSL_RAW_DATA = 1;
const int64_t k_OPENSSL_ZERO_PADDING = 2;
const int64_t k_OPENSSL_NO_PADDING = 3;
const int64_t k_OPENSSL_PKCS1_OAEP_PADDING = 4;

// exported constants
const int64_t k_OPENSSL_SSLV23_PADDING = 2;
const int64_t k_OPENSSL_PKCS1_PADDING = 1;

static char default_ssl_conf_filename[PATH_MAX];

struct OpenSSLInitializer {
  OpenSSLInitializer() {
    SSL_library_init();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_algorithms();

// CCM ciphers are not added by default, so let's add them!
#if !defined(OPENSSL_NO_AES) && defined(EVP_CIPH_CCM_MODE) && \
    OPENSSL_VERSION_NUMBER < 0x100020000
    EVP_add_cipher(EVP_aes_128_ccm());
    EVP_add_cipher(EVP_aes_192_ccm());
    EVP_add_cipher(EVP_aes_256_ccm());
#endif

    ERR_load_ERR_strings();
    ERR_load_crypto_strings();
    ERR_load_EVP_strings();

    /* Determine default SSL configuration file */
    char *config_filename = getenv("OPENSSL_CONF");
    if (config_filename == nullptr) {
      config_filename = getenv("SSLEAY_CONF");
    }

    /* default to 'openssl.cnf' if no environment variable is set */
    if (config_filename == nullptr) {
      snprintf(default_ssl_conf_filename, sizeof(default_ssl_conf_filename),
               "%s/%s", X509_get_default_cert_area(), "openssl.cnf");
    } else {
      always_assert(
        strlen(config_filename) < sizeof(default_ssl_conf_filename));
      strcpy(default_ssl_conf_filename, config_filename);
    }
  }

  ~OpenSSLInitializer() {
    EVP_cleanup();
  }
};
static OpenSSLInitializer s_openssl_initializer;

///////////////////////////////////////////////////////////////////////////////
// resource classes

struct Key : SweepableResourceData {
  EVP_PKEY *m_key;
  explicit Key(EVP_PKEY *key) : m_key(key) { assertx(m_key);}
  ~Key() override {
    if (m_key) EVP_PKEY_free(m_key);
  }

  CLASSNAME_IS("OpenSSL key");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  DECLARE_RESOURCE_ALLOCATION(Key)

  bool isPrivate() {
    assertx(m_key);
    switch (EVP_PKEY_id(m_key)) {
#ifndef NO_RSA
    case EVP_PKEY_RSA:
    case EVP_PKEY_RSA2:
      {
        const auto rsa = EVP_PKEY_get0_RSA(m_key);
        assertx(rsa);
        const BIGNUM *p, *q;
        RSA_get0_factors(rsa, &p, &q);
        if (!p || !q) {
          return false;
        }
        break;
      }
#endif
#ifndef NO_DSA
    case EVP_PKEY_DSA:
    case EVP_PKEY_DSA1:
    case EVP_PKEY_DSA2:
    case EVP_PKEY_DSA3:
    case EVP_PKEY_DSA4:
      {
        const auto dsa = EVP_PKEY_get0_DSA(m_key);
        assertx(dsa);
        const BIGNUM *p, *q, *g, *pub_key, *priv_key;
        DSA_get0_pqg(dsa, &p, &q, &g);
        if (!p || !q || !g) {
          return false;
        }
        DSA_get0_key(dsa, &pub_key, &priv_key);
        if (!priv_key) {
          return false;
        }
        break;
      }
#endif
#ifndef NO_DH
    case EVP_PKEY_DH:
      {
        const auto dh = EVP_PKEY_get0_DH(m_key);
        assertx(dh);
        const BIGNUM *p, *q, *g, *pub_key, *priv_key;
        DH_get0_pqg(dh, &p, &q, &g);
        if (!p) {
          return false;
        }
        DH_get0_key(dh, &pub_key, &priv_key);
        if (!priv_key) {
          return false;
        }
        break;
       }
#endif
#ifdef HAVE_EVP_PKEY_EC
    case EVP_PKEY_EC:
      {
        const auto ec_key = EVP_PKEY_get0_EC_KEY(m_key);
        assertx(ec_key);
        if (EC_KEY_get0_private_key(ec_key) == nullptr) {
          return false;
        }
        break;
      }
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
   *   passphrase, you should use an empty string rather than nullptr for the
   *   passphrase - nullptr causes a passphrase prompt to be emitted in
   *   the Apache error log!
   */
  static req::ptr<Key> Get(const Variant& var, bool public_key,
                           const char *passphrase = nullptr) {
    if (var.isArray()) {
      Array arr = var.toArray();
      if (!arr.exists(int64_t(0)) || !arr.exists(int64_t(1))) {
        raise_warning("key array must be of the form "
                        "array(0 => key, 1 => phrase)");
        return nullptr;
      }

      String zphrase = arr[1].toString();
      return GetHelper(arr[0], public_key, zphrase.data());
    }
    return GetHelper(var, public_key, passphrase);
  }

  static req::ptr<Key> GetHelper(const Variant& var, bool public_key,
                                 const char *passphrase) {
    req::ptr<Certificate> ocert;
    EVP_PKEY *key = nullptr;

    if (var.isResource()) {
      auto cert = dyn_cast_or_null<Certificate>(var);
      auto key = dyn_cast_or_null<Key>(var);
      if (!cert && !key) return nullptr;
      if (key) {
        bool is_priv = key->isPrivate();
        if (!public_key && !is_priv) {
          raise_warning("supplied key param is a public key");
          return nullptr;
        }
        if (public_key && is_priv) {
          raise_warning("Don't know how to get public key from "
                          "this private key");
          return nullptr;
        }
        return key;
      }
      ocert = cert;
    } else {
      /* it's an X509 file/cert of some kind, and we need to extract
         the data from that */
      if (public_key) {
        ocert = Certificate::Get(var);
        if (!ocert) {
          /* not a X509 certificate, try to retrieve public key */
          BIO *in = Certificate::ReadData(var);
          if (in == nullptr) return nullptr;
          key = PEM_read_bio_PUBKEY(in, nullptr,nullptr, nullptr);
          BIO_free(in);
        }
      } else {
        /* we want the private key */
        BIO *in = Certificate::ReadData(var);
        if (in == nullptr) return nullptr;
        key = PEM_read_bio_PrivateKey(in, nullptr,nullptr, (void*)passphrase);
        BIO_free(in);
      }
    }

    if (public_key && ocert && key == nullptr) {
      /* extract public key from X509 cert */
      key = (EVP_PKEY *)X509_get_pubkey(ocert->get());
    }

    if (key) {
      return req::make<Key>(key);
    }

    return nullptr;
  }
};

IMPLEMENT_RESOURCE_ALLOCATION(Key)

/**
 * Certificate Signing Request
 */
struct CSRequest : SweepableResourceData {
private:
  X509_REQ *m_csr;

public:
  explicit CSRequest(X509_REQ *csr) : m_csr(csr) {
    assertx(m_csr);
  }

  X509_REQ *csr() { return m_csr; }

  ~CSRequest() override {
    // X509_REQ_free(nullptr) is a no-op
    X509_REQ_free(m_csr);
  }

  CLASSNAME_IS("OpenSSL X.509 CSR");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  DECLARE_RESOURCE_ALLOCATION(CSRequest)

  static req::ptr<CSRequest> Get(const Variant& var) {
    auto csr = cast_or_null<CSRequest>(GetRequest(var));
    if (!csr || !csr->m_csr) {
      raise_warning("cannot get CSR");
      return nullptr;
    }
    return csr;
  }

private:
  static req::ptr<CSRequest> GetRequest(const Variant& var) {
    if (var.isResource()) {
      return dyn_cast_or_null<CSRequest>(var);
    }
    if (var.isString() || var.isObject()) {
      BIO *in = Certificate::ReadData(var);
      if (in == nullptr) return nullptr;

      X509_REQ *csr = PEM_read_bio_X509_REQ(in, nullptr,nullptr,nullptr);
      BIO_free(in);
      if (csr) {
        return req::make<CSRequest>(csr);
      }
    }
    return nullptr;
  }
};

IMPLEMENT_RESOURCE_ALLOCATION(CSRequest)

struct php_x509_request {
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
#ifdef HAVE_EVP_PKEY_EC
  int curve_name;
#endif
  EVP_PKEY *priv_key;

  static bool load_rand_file(const char *file, int *egdsocket, int *seeded) {
    char buffer[PATH_MAX];

    *egdsocket = 0;
    *seeded = 0;
    if (file == nullptr) {
      file = RAND_file_name(buffer, sizeof(buffer));
#if !defined(OPENSSL_NO_RAND_EGD) && !defined(OPENSSL_NO_EGD)
    } else if (RAND_egd(file) > 0) {
      /* if the given filename is an EGD socket, don't
       * write anything back to it */
      *egdsocket = 1;
      return true;
#endif
    }

    if (file == nullptr || !RAND_load_file(file, -1)) {
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
    if (file == nullptr) {
      file = RAND_file_name(buffer, sizeof(buffer));
    }
    if (file == nullptr || !RAND_write_file(file)) {
      raise_warning("unable to write random state");
      return false;
    }
    return true;
  }

  bool generatePrivateKey() {
    assertx(priv_key == nullptr);

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
    if ((priv_key = EVP_PKEY_new()) != nullptr) {
      switch (priv_key_type) {
      case OPENSSL_KEYTYPE_RSA:
        if (EVP_PKEY_assign_RSA
            (priv_key, RSA_generate_key(priv_key_bits, 0x10001, nullptr, nullptr))) {
          ret = true;
        }
        break;
#if !defined(NO_DSA) && defined(HAVE_DSA_DEFAULT_METHOD)
      case OPENSSL_KEYTYPE_DSA:
        {
          DSA *dsapar = DSA_generate_parameters(priv_key_bits, nullptr, 0, nullptr,
                                                nullptr, nullptr, nullptr);
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
#ifdef HAVE_EVP_PKEY_EC
      case OPENSSL_KEYTYPE_EC:
        {
          if (curve_name == NID_undef) {
            raise_warning("Missing configuration value: 'curve_name' not set");
            return false;
          }
          if (auto const eckey = EC_KEY_new_by_curve_name(curve_name)) {
            EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
            if (EC_KEY_generate_key(eckey) &&
                EVP_PKEY_assign_EC_KEY(priv_key, eckey)) {
              ret = true;
            } else {
              EC_KEY_free(eckey);
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
    ASN1_STRING *str = nullptr;
    unsigned char *to_add = nullptr;
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
      // Use the string instance we created above.
      subitem.set(String(sname, CopyString), subentries[0]);
    }
  }

  if (key) {
    ret.set(String(key, CopyString), subitem);
  }
}

static const char *read_string(const Array& args, const String& key, const char *def,
                               std::vector<String> &strings) {
  if (args.exists(key)) {
    String value = args[key].toString();
    strings.push_back(value);
    return (char*)value.data();
  }
  return def;
}

static int64_t read_integer(const Array& args, const String& key, int64_t def) {
  if (args.exists(key)) {
    return args[key].toInt64();
  }
  return def;
}

static bool add_oid_section(struct php_x509_request *req) {
  char *str = CONF_get_string(req->req_config, nullptr, "oid_section");
  if (str == nullptr) {
    return true;
  }

  STACK_OF(CONF_VALUE) *sktmp = CONF_get_section(req->req_config, str);
  if (sktmp == nullptr) {
    raise_warning("problem loading oid section %s", str);
    return false;
  }

  for (int i = 0; i < sk_CONF_VALUE_num(sktmp); i++) {
    CONF_VALUE *cnf = sk_CONF_VALUE_value(sktmp, i);
    if (OBJ_sn2nid(cnf->name) == NID_undef &&
      OBJ_ln2nid(cnf->name) == NID_undef &&
      OBJ_create(cnf->value, cnf->name, cnf->name) == NID_undef) {
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
  if (!X509V3_EXT_add_conf(config, &ctx, (char*)section, nullptr)) {
    raise_warning("Error loading %s section %s of %s",
                    section_label, section, config_filename);
    return false;
  }
  return true;
}

const StaticString
  s_config("config"),
  s_config_section_name("config_section_name"),
  s_digest_alg("digest_alg"),
  s_x509_extensions("x509_extensions"),
  s_req_extensions("req_extensions"),
  s_private_key_bits("private_key_bits"),
  s_private_key_type("private_key_type"),
  s_encrypt_key("encrypt_key"),
  s_curve_name("curve_name");

static bool php_openssl_parse_config(struct php_x509_request *req,
                                     const Array& args,
                                     std::vector<String> &strings) {
  req->config_filename =
    read_string(args, s_config, default_ssl_conf_filename, strings);
  req->section_name =
    read_string(args, s_config_section_name, "req", strings);
  req->global_config = CONF_load(nullptr, default_ssl_conf_filename, nullptr);
  req->req_config = CONF_load(nullptr, req->config_filename, nullptr);
  if (req->req_config == nullptr) {
    return false;
  }

  /* read in the oids */
  char *str = CONF_get_string(req->req_config, nullptr, "oid_file");
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
    read_string(args, s_digest_alg,
                CONF_get_string(req->req_config, req->section_name,
                                "default_md"),
                strings);

  req->extensions_section =
    read_string(args, s_x509_extensions,
                CONF_get_string(req->req_config, req->section_name,
                                "x509_extensions"),
                strings);

  req->request_extensions_section =
    read_string(args, s_req_extensions,
                CONF_get_string(req->req_config, req->section_name,
                                "req_extensions"),
                strings);

  req->priv_key_bits =
    read_integer(args, s_private_key_bits,
                 CONF_get_number(req->req_config, req->section_name,
                                 "default_bits"));

  req->priv_key_type =
    read_integer(args, s_private_key_type, OPENSSL_KEYTYPE_DEFAULT);

  if (args.exists(s_encrypt_key)) {
    bool value = args[s_encrypt_key].toBoolean();
    req->priv_key_encrypt = value ? 1 : 0;
  } else {
    str = CONF_get_string(req->req_config, req->section_name,
                          "encrypt_rsa_key");
    if (str == nullptr) {
      str = CONF_get_string(req->req_config, req->section_name, "encrypt_key");
    }
    if (str && strcmp(str, "no") == 0) {
      req->priv_key_encrypt = 0;
    } else {
      req->priv_key_encrypt = 1;
    }
  }

  /* digest alg */
  if (req->digest_name == nullptr) {
    req->digest_name = CONF_get_string(req->req_config, req->section_name,
                                       "default_md");
  }
  if (req->digest_name) {
    req->digest = req->md_alg = EVP_get_digestbyname(req->digest_name);
  }
  if (req->md_alg == nullptr) {
    req->md_alg = req->digest = EVP_sha256();
  }

#ifdef HAVE_EVP_PKEY_EC
  /* set the ec group curve name */
  req->curve_name = NID_undef;
  if (args.exists(s_curve_name)) {
    auto const curve_name = args[s_curve_name].toString();
    req->curve_name = OBJ_sn2nid(curve_name.data());
    if (req->curve_name == NID_undef) {
      raise_warning(
        "Unknown elliptic curve (short) name %s",
        curve_name.data()
      );
      return false;
    }
  }
#endif

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
    req->global_config = nullptr;
  }
  if (req->req_config) {
    CONF_free(req->req_config);
    req->req_config = nullptr;
  }
}

static STACK_OF(X509) *load_all_certs_from_file(const char *certfile) {
  STACK_OF(X509_INFO) *sk = nullptr;
  STACK_OF(X509) *stack = nullptr, *ret = nullptr;
  BIO *in = nullptr;
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
  if (!(sk = PEM_X509_INFO_read_bio(in, nullptr, nullptr, nullptr))) {
    raise_warning("error reading the file, %s", certfile);
    sk_X509_free(stack);
    goto end;
  }

  /* scan over it and pull out the certs */
  while (sk_X509_INFO_num(sk)) {
    xi = sk_X509_INFO_shift(sk);
    if (xi->x509 != nullptr) {
      sk_X509_push(stack, xi->x509);
      xi->x509 = nullptr;
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
static X509_STORE *setup_verify(const Array& calist) {
  X509_STORE *store = X509_STORE_new();
  if (store == nullptr) {
    return nullptr;
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
      if (file_lookup == nullptr ||
          !X509_LOOKUP_load_file(file_lookup, item.data(),
                                 X509_FILETYPE_PEM)) {
        raise_warning("error loading file %s", item.data());
      } else {
        nfiles++;
      }
      file_lookup = nullptr;
    } else {
      dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
      if (dir_lookup == nullptr ||
          !X509_LOOKUP_add_dir(dir_lookup, item.data(), X509_FILETYPE_PEM)) {
        raise_warning("error loading directory %s", item.data());
      } else {
        ndirs++;
      }
      dir_lookup = nullptr;
    }
  }
  if (nfiles == 0) {
    file_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
    if (file_lookup) {
      X509_LOOKUP_load_file(file_lookup, nullptr, X509_FILETYPE_DEFAULT);
    }
  }
  if (ndirs == 0) {
    dir_lookup = X509_STORE_add_lookup(store, X509_LOOKUP_hash_dir());
    if (dir_lookup) {
      X509_LOOKUP_add_dir(dir_lookup, nullptr, X509_FILETYPE_DEFAULT);
    }
  }
  return store;
}

///////////////////////////////////////////////////////////////////////////////

static bool add_entries(X509_NAME *subj, const Array& items) {
  for (ArrayIter iter(items); iter; ++iter) {
    auto const index = iter.first().toString();
    auto const item = iter.second().toString();
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
                                 const Array& dn, const Array& attribs) {
  char *dn_sect = CONF_get_string(req->req_config, req->section_name,
                                  "distinguished_name");
  if (dn_sect == nullptr) return false;

  STACK_OF(CONF_VALUE) *dn_sk = CONF_get_section(req->req_config, dn_sect);
  if (dn_sk == nullptr) return false;

  char *attr_sect = CONF_get_string(req->req_config, req->section_name,
                                    "attributes");
  STACK_OF(CONF_VALUE) *attr_sk = nullptr;
  if (attr_sect) {
    attr_sk = CONF_get_section(req->req_config, attr_sect);
    if (attr_sk == nullptr) {
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

bool HHVM_FUNCTION(openssl_csr_export_to_file, const Variant& csr,
                                               const String& outfilename,
                                               bool notext /* = true */) {
  auto pcsr = CSRequest::Get(csr);
  if (!pcsr) return false;

  BIO *bio_out = BIO_new_file((char*)outfilename.data(), "w");
  if (bio_out == nullptr) {
    raise_warning("error opening file %s", outfilename.data());
    return false;
  }

  if (!notext) {
    X509_REQ_print(bio_out, pcsr->csr());
  }
  PEM_write_bio_X509_REQ(bio_out, pcsr->csr());
  BIO_free(bio_out);
  return true;
}

bool HHVM_FUNCTION(openssl_csr_export, const Variant& csr, Variant& out,
                                       bool notext /* = true */) {
  auto pcsr = CSRequest::Get(csr);
  if (!pcsr) return false;

  BIO *bio_out = BIO_new(BIO_s_mem());
  if (!notext) {
    X509_REQ_print(bio_out, pcsr->csr());
  }

  if (PEM_write_bio_X509_REQ(bio_out, pcsr->csr())) {
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    out = String((char*)bio_buf->data, bio_buf->length, CopyString);
    BIO_free(bio_out);
    return true;
  }

  BIO_free(bio_out);
  return false;
}

Variant HHVM_FUNCTION(openssl_csr_get_public_key, const Variant& csr) {
  auto pcsr = CSRequest::Get(csr);
  if (!pcsr) return false;

  auto input_csr = pcsr->csr();

#if OPENSSL_VERSION_NUMBER >= 0x10100000
  /* Due to changes in OpenSSL 1.1 related to locking when decoding CSR,
   * the pub key is not changed after assigning. It means if we pass
   * a private key, it will be returned including the private part.
   * If we duplicate it, then we get just the public part which is
   * the same behavior as for OpenSSL 1.0 */
  input_csr = X509_REQ_dup(input_csr);
  /* We need to free the CSR as it was duplicated */
  SCOPE_EXIT { X509_REQ_free(input_csr); };
#endif
  auto pubkey = X509_REQ_get_pubkey(input_csr);
  if (!pubkey) return false;
  return Variant(req::make<Key>(pubkey));
}

Variant HHVM_FUNCTION(openssl_csr_get_subject, const Variant& csr,
                      bool use_shortnames /* = true */) {
  auto pcsr = CSRequest::Get(csr);
  if (!pcsr) return false;

  X509_NAME *subject = X509_REQ_get_subject_name(pcsr->csr());
  Array ret = Array::CreateDict();
  add_assoc_name_entry(ret, nullptr, subject, use_shortnames);
  return ret;
}

Variant HHVM_FUNCTION(openssl_csr_new,
                      const Variant& dn, Variant& privkey,
                      const Variant& configargs /* = uninit_variant */,
                      const Variant& extraattribs /* = uninit_variant */) {
  Variant ret = false;
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));

  req::ptr<Key> okey;
  X509_REQ *csr = nullptr;
  std::vector<String> strings;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    /* Generate or use a private key */
    if (!privkey.isNull()) {
      okey = Key::Get(privkey, false);
      if (okey) {
        req.priv_key = okey->m_key;
      }
    }
    if (req.priv_key == nullptr) {
      req.generatePrivateKey();
      if (req.priv_key) {
        okey = req::make<Key>(req.priv_key);
      }
    }
    if (req.priv_key == nullptr) {
      raise_warning("Unable to generate a private key");
    } else {
      csr = X509_REQ_new();
      if (csr && php_openssl_make_REQ(&req, csr, dn.toArray(),
                                      extraattribs.toArray())) {
        X509V3_CTX ext_ctx;
        X509V3_set_ctx(&ext_ctx, nullptr, nullptr, csr, nullptr, 0);
        X509V3_set_conf_lhash(&ext_ctx, req.req_config);

        /* Add extensions */
        if (req.request_extensions_section &&
            !X509V3_EXT_REQ_add_conf(req.req_config, &ext_ctx,
                                     (char*)req.request_extensions_section,
                                     csr)) {
          raise_warning("Error loading extension section %s",
                          req.request_extensions_section);
        } else {
          ret = true;
          if (X509_REQ_sign(csr, req.priv_key, req.digest)) {
            ret = req::make<CSRequest>(csr);
            csr = nullptr;
          } else {
            raise_warning("Error signing request");
          }

          privkey = Variant(okey);
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

Variant HHVM_FUNCTION(openssl_csr_sign, const Variant& csr,
                                        const Variant& cacert,
                                        const Variant& priv_key, int64_t days,
                                        const Variant& configargs /* = null */,
                                        int64_t serial /* = 0 */) {
  auto pcsr = CSRequest::Get(csr);
  if (!pcsr) return false;

  req::ptr<Certificate> ocert;
  if (!cacert.isNull()) {
    ocert = Certificate::Get(cacert);
    if (!ocert) {
      raise_warning("cannot get cert from parameter 2");
      return false;
    }
  }
  auto okey = Key::Get(priv_key, false);
  if (!okey) {
    raise_warning("cannot get private key from parameter 3");
    return false;
  }
  X509 *cert = nullptr;
  if (ocert) {
    cert = ocert->m_cert;
  }
  EVP_PKEY *pkey = okey->m_key;
  if (cert && !X509_check_private_key(cert, pkey)) {
    raise_warning("private key does not correspond to signing cert");
    return false;
  }

  req::ptr<Certificate> onewcert;
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));
  Variant ret = false;
  std::vector<String> strings;
  if (!php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    goto cleanup;
  }

  /* Check that the request matches the signature */
  EVP_PKEY *key;
  key = X509_REQ_get_pubkey(pcsr->csr());
  if (key == nullptr) {
    raise_warning("error unpacking public key");
    goto cleanup;
  }
  int i;
  i = X509_REQ_verify(pcsr->csr(), key);
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
  if (new_cert == nullptr) {
    raise_warning("No memory");
    goto cleanup;
  }
  onewcert = req::make<Certificate>(new_cert);
  /* Version 3 cert */
  if (!X509_set_version(new_cert, 2)) {
    goto cleanup;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(new_cert), serial);
  X509_set_subject_name(new_cert, X509_REQ_get_subject_name(pcsr->csr()));

  if (cert == nullptr) {
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
    X509V3_set_ctx(&ctx, cert, new_cert, pcsr->csr(), nullptr, 0);
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

Variant HHVM_FUNCTION(openssl_error_string) {
  char buf[512];
  unsigned long val = ERR_get_error();
  if (val) {
    return String(ERR_error_string(val, buf), CopyString);
  }
  return false;
}


bool HHVM_FUNCTION(openssl_open, const String& sealed_data, Variant& open_data,
                                 const String& env_key,
                                 const Variant& priv_key_id,
                                 const String& method, /* = null_string */
                                 const String& iv /* = null_string */) {
  const EVP_CIPHER *cipher_type;
  if (method.empty()) {
    cipher_type = EVP_rc4();
  } else {
    cipher_type = EVP_get_cipherbyname(method.c_str());
    if (!cipher_type) {
      raise_warning("Unknown cipher algorithm");
      return false;
    }
  }

  auto okey = Key::Get(priv_key_id, false);
  if (!okey) {
    raise_warning("unable to coerce parameter 4 into a private key");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;

  const unsigned char *iv_buf = nullptr;
  int iv_len = EVP_CIPHER_iv_length(cipher_type);
  if (iv_len > 0) {
    if (iv.empty()) {
      raise_warning(
        "Cipher algorithm requires an IV to be supplied as a sixth parameter");
      return false;
    }
    if (iv.length() != iv_len) {
      raise_warning("IV length is invalid");
      return false;
    }
    iv_buf = reinterpret_cast<const unsigned char*>(iv.c_str());
  }

  String s = String(sealed_data.size(), ReserveString);
  unsigned char *buf = (unsigned char *)s.mutableData();

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) {
    raise_warning("Failed to allocate an EVP_CIPHER_CTX object");
    return false;
  }
  SCOPE_EXIT {
    EVP_CIPHER_CTX_free(ctx);
  };
  int len1, len2;
  if (!EVP_OpenInit(
          ctx,
          cipher_type,
          (unsigned char*)env_key.data(),
          env_key.size(),
          iv_buf,
          pkey) ||
      !EVP_OpenUpdate(
          ctx,
          buf,
          &len1,
          (unsigned char*)sealed_data.data(),
          sealed_data.size()) ||
      !EVP_OpenFinal(ctx, buf + len1, &len2) || len1 + len2 == 0) {
    return false;
  }
  open_data = s.setSize(len1 + len2);
  return true;
}

static STACK_OF(X509) *php_array_to_X509_sk(const Variant& certs) {
  STACK_OF(X509) *pcerts = sk_X509_new_null();
  Array arrCerts;
  if (certs.isArray()) {
    arrCerts = certs.toArray();
  } else {
    arrCerts.append(certs);
  }
  for (ArrayIter iter(arrCerts); iter; ++iter) {
    auto ocert = Certificate::Get(iter.second());
    if (!ocert) {
      break;
    }
    sk_X509_push(pcerts, ocert->m_cert);
  }
  return pcerts;
}

const StaticString
  s_friendly_name("friendly_name"),
  s_extracerts("extracerts");

static bool
openssl_pkcs12_export_impl(const Variant& x509, BIO *bio_out,
                           const Variant& priv_key, const String& pass,
                           const Variant& args /* = uninit_variant */) {
  auto ocert = Certificate::Get(x509);
  if (!ocert) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  auto okey = Key::Get(priv_key, false);
  if (!okey) {
    raise_warning("cannot get private key from parameter 3");
    return false;
  }
  X509 *cert = ocert->m_cert;
  EVP_PKEY *key = okey->m_key;
  if (cert && !X509_check_private_key(cert, key)) {
    raise_warning("private key does not correspond to cert");
    return false;
  }

  Array arrArgs = args.toArray();

  String friendly_name;
  if (arrArgs.exists(s_friendly_name)) {
    friendly_name = arrArgs[s_friendly_name].toString();
  }

  STACK_OF(X509) *ca = nullptr;
  if (arrArgs.exists(s_extracerts)) {
    ca = php_array_to_X509_sk(arrArgs[s_extracerts]);
  }

  PKCS12 *p12 = PKCS12_create
    ((char*)pass.data(),
     (char*)(friendly_name.empty() ? nullptr : friendly_name.data()),
     key, cert, ca, 0, 0, 0, 0, 0);

  assertx(bio_out);
  bool ret = i2d_PKCS12_bio(bio_out, p12);
  PKCS12_free(p12);
  sk_X509_free(ca);
  return ret;
}

bool HHVM_FUNCTION(openssl_pkcs12_export_to_file, const Variant& x509,
                                                  const String& filename,
                                                  const Variant& priv_key,
                                                  const String& pass,
                                  const Variant& args /* = uninit_variant */) {
  BIO *bio_out = BIO_new_file(filename.data(), "w");
  if (bio_out == nullptr) {
    raise_warning("error opening file %s", filename.data());
    return false;
  }
  bool ret = openssl_pkcs12_export_impl(x509, bio_out, priv_key, pass, args);
  BIO_free(bio_out);
  return ret;
}

bool HHVM_FUNCTION(openssl_pkcs12_export, const Variant& x509, Variant& out,
                                          const Variant& priv_key,
                                          const String& pass,
                                  const Variant& args /* = uninit_variant */) {
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

const StaticString
  s_cert("cert"),
  s_pkey("pkey");

bool HHVM_FUNCTION(openssl_pkcs12_read, const String& pkcs12, Variant& certs,
                                        const String& pass) {
  bool ret = false;
  PKCS12 *p12 = nullptr;

  BIO *bio_in = BIO_new(BIO_s_mem());
  if (!BIO_write(bio_in, pkcs12.data(), pkcs12.size())) {
    goto cleanup;
  }

  if (d2i_PKCS12_bio(bio_in, &p12)) {
    EVP_PKEY *pkey = nullptr;
    X509 *cert = nullptr;
    STACK_OF(X509) *ca = nullptr;
    if (PKCS12_parse(p12, pass.data(), &pkey, &cert, &ca)) {
      Variant vcerts = Array::CreateDict();
      SCOPE_EXIT {
        certs = vcerts;
      };
      BIO *bio_out = nullptr;
      if (cert) {
        bio_out = BIO_new(BIO_s_mem());
        if (PEM_write_bio_X509(bio_out, cert)) {
          BUF_MEM *bio_buf;
          BIO_get_mem_ptr(bio_out, &bio_buf);
          vcerts.asArrRef().set(s_cert,
            String((char*)bio_buf->data, bio_buf->length, CopyString));
        }
        BIO_free(bio_out);
      }

      if (pkey) {
        bio_out = BIO_new(BIO_s_mem());
        if (PEM_write_bio_PrivateKey(bio_out, pkey, nullptr, nullptr, 0, 0, nullptr)) {
          BUF_MEM *bio_buf;
          BIO_get_mem_ptr(bio_out, &bio_buf);
          vcerts.asArrRef().set(s_pkey,
            String((char*)bio_buf->data, bio_buf->length, CopyString));
        }
        BIO_free(bio_out);
      }

      if (ca) {
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
        sk_X509_free(ca);
        vcerts.asArrRef().set(s_extracerts, extracerts);
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

bool HHVM_FUNCTION(openssl_pkcs7_decrypt, const String& infilename,
                                          const String& outfilename,
                                          const Variant& recipcert,
                              const Variant& recipkey /* = uninit_variant */) {
  bool ret = false;
  BIO *in = nullptr, *out = nullptr, *datain = nullptr;
  PKCS7 *p7 = nullptr;
  req::ptr<Key> okey;

  auto ocert = Certificate::Get(recipcert);
  if (!ocert) {
    raise_warning("unable to coerce parameter 3 to x509 cert");
    goto clean_exit;
  }

  okey = Key::Get(recipkey.isNull() ? recipcert : recipkey, false);
  if (!okey) {
    raise_warning("unable to get private key");
    goto clean_exit;
  }

  in = BIO_new_file(infilename.data(), "r");
  if (in == nullptr) {
    raise_warning("error opening the file, %s", infilename.data());
    goto clean_exit;
  }
  out = BIO_new_file(outfilename.data(), "w");
  if (out == nullptr) {
    raise_warning("error opening the file, %s", outfilename.data());
    goto clean_exit;
  }

  p7 = SMIME_read_PKCS7(in, &datain);
  if (p7 == nullptr) {
    goto clean_exit;
  }
  assertx(okey->m_key);
  assertx(ocert->m_cert);
  if (PKCS7_decrypt(p7, okey->m_key, ocert->m_cert, out, PKCS7_DETACHED)) {
    ret = true;
  }

 clean_exit:
  PKCS7_free(p7);
  BIO_free(datain);
  BIO_free(in);
  BIO_free(out);

  return ret;
}

static void print_headers(BIO *outfile, const Array& headers) {
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

bool HHVM_FUNCTION(openssl_pkcs7_encrypt, const String& infilename,
                                          const String& outfilename,
                                          const Variant& recipcerts,
                                          const Array& headers,
                                          int64_t flags /* = 0 */,
                                int64_t cipherid /* = k_OPENSSL_CIPHER_RC2_40 */) {
  bool ret = false;
  BIO *infile = nullptr, *outfile = nullptr;
  STACK_OF(X509) *precipcerts = nullptr;
  PKCS7 *p7 = nullptr;
  const EVP_CIPHER *cipher = nullptr;

  infile = BIO_new_file(infilename.data(), (flags & PKCS7_BINARY) ? "rb" : "r");
  if (infile == nullptr) {
    raise_warning("error opening the file, %s", infilename.data());
    goto clean_exit;
  }
  outfile = BIO_new_file(outfilename.data(), "w");
  if (outfile == nullptr) {
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
  if (cipher == nullptr) {
    raise_warning("Failed to get cipher");
    goto clean_exit;
  }

  p7 = PKCS7_encrypt(precipcerts, infile, (EVP_CIPHER*)cipher, flags);
  if (p7 == nullptr) goto clean_exit;

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

bool HHVM_FUNCTION(openssl_pkcs7_sign, const String& infilename,
                                       const String& outfilename,
                                       const Variant& signcert,
                                       const Variant& privkey,
                                       const Variant& headers,
                                       int64_t flags /* = k_PKCS7_DETACHED */,
                                const String& extracerts /* = null_string */) {
  bool ret = false;
  STACK_OF(X509) *others = nullptr;
  BIO *infile = nullptr, *outfile = nullptr;
  PKCS7 *p7 = nullptr;
  req::ptr<Key> okey;
  req::ptr<Certificate> ocert;

  if (!extracerts.empty()) {
    others = load_all_certs_from_file(extracerts.data());
    if (others == nullptr) {
      goto clean_exit;
    }
  }

  okey = Key::Get(privkey, false);
  if (!okey) {
    raise_warning("error getting private key");
    goto clean_exit;
  }
  EVP_PKEY *key;
  key = okey->m_key;

  ocert = Certificate::Get(signcert);
  if (!ocert) {
    raise_warning("error getting cert");
    goto clean_exit;
  }
  X509 *cert;
  cert = ocert->m_cert;

  infile = BIO_new_file(infilename.data(), (flags & PKCS7_BINARY) ? "rb" : "r");
  if (infile == nullptr) {
    raise_warning("error opening input file %s!", infilename.data());
    goto clean_exit;
  }

  outfile = BIO_new_file(outfilename.data(), "w");
  if (outfile == nullptr) {
    raise_warning("error opening output file %s!", outfilename.data());
    goto clean_exit;
  }

  p7 = PKCS7_sign(cert, key, others, infile, flags);
  if (p7 == nullptr) {
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

static int pkcs7_ignore_expiration(int ok, X509_STORE_CTX *ctx) {
  if (ok) {
    return ok;
  }
  int error = X509_STORE_CTX_get_error(ctx);
  if (error == X509_V_ERR_CERT_HAS_EXPIRED) {
    // ignore cert expirations
    Logger::Verbose("Ignoring cert expiration");
    return 1;
  }
  return ok;
}

/**
 *  NOTE: when ignore_cert_expiration is true, a custom certificate validation
 *  callback is set up. Please be aware of this if you modify the function to
 *  allow other certificate validation behaviors
 */
Variant openssl_pkcs7_verify_core(
  const String& filename,
  int flags,
  const Variant& voutfilename /* = null_string */,
  const Variant& vcainfo /* = null_array */,
  const Variant& vextracerts /* = null_string */,
  const Variant& vcontent /* = null_string */,
  bool ignore_cert_expiration
) {
  Variant ret = -1;
  X509_STORE *store = nullptr;
  BIO *in = nullptr;
  PKCS7 *p7 = nullptr;
  BIO *datain = nullptr;
  BIO *dataout = nullptr;

  auto cainfo = vcainfo.toArray();
  auto extracerts = vextracerts.toString();
  auto content = vcontent.toString();

  STACK_OF(X509) *others = nullptr;
  if (!extracerts.empty()) {
    others = load_all_certs_from_file(extracerts.data());
    if (others == nullptr) {
      goto clean_exit;
    }
  }

  flags = flags & ~PKCS7_DETACHED;

  store = setup_verify(cainfo);
  if (!store) {
    goto clean_exit;
  }
  if (ignore_cert_expiration) {
#if (OPENSSL_VERSION_NUMBER >= 0x10000000)
    // make sure no other callback is specified
  #if OPENSSL_VERSION_NUMBER >= 0x10100000L
    assertx(!X509_STORE_get_verify_cb(store));
  #else
    assertx(!store->verify_cb);
  #endif
    // ignore expired certs
    X509_STORE_set_verify_cb(store, pkcs7_ignore_expiration);
#else
    always_assert(false);
#endif
  }
  in = BIO_new_file(filename.data(), (flags & PKCS7_BINARY) ? "rb" : "r");
  if (in == nullptr) {
    raise_warning("error opening the file, %s", filename.data());
    goto clean_exit;
  }

  p7 = SMIME_read_PKCS7(in, &datain);
  if (p7 == nullptr) {
    goto clean_exit;
  }

  if (!content.empty()) {
    dataout = BIO_new_file(content.data(), "w");
    if (dataout == nullptr) {
      raise_warning("error opening the file, %s", content.data());
      goto clean_exit;
    }
  }

  if (PKCS7_verify(p7, others, store, datain, dataout, flags)) {
    ret = true;
    auto outfilename = voutfilename.toString();
    if (!outfilename.empty()) {
      BIO *certout = BIO_new_file(outfilename.data(), "w");
      if (certout) {
        STACK_OF(X509) *signers = PKCS7_get0_signers(p7, nullptr, flags);
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
  sk_X509_pop_free(others, X509_free);

  return ret;
}

Variant HHVM_FUNCTION(openssl_pkcs7_verify, const String& filename, int64_t flags,
                               const Variant& voutfilename /* = null_string */,
                               const Variant& vcainfo /* = null_array */,
                               const Variant& vextracerts /* = null_string */,
                               const Variant& vcontent /* = null_string */) {
  return openssl_pkcs7_verify_core(filename, flags, voutfilename, vcainfo,
                                   vextracerts, vcontent, false);
}

static bool
openssl_pkey_export_impl(const Variant& key, BIO *bio_out,
                         const String& passphrase /* = null_string */,
                         const Variant& configargs /* = uninit_variant */) {
  auto okey = Key::Get(key, false, passphrase.data());
  if (!okey) {
    raise_warning("cannot get key from parameter 1");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;

  struct php_x509_request req;
  memset(&req, 0, sizeof(req));
  std::vector<String> strings;
  bool ret = false;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings)) {
    const EVP_CIPHER *cipher;
    if (!passphrase.empty() && req.priv_key_encrypt) {
      cipher = (EVP_CIPHER *)EVP_des_ede3_cbc();
    } else {
      cipher = nullptr;
    }
    assertx(bio_out);

    ret = PEM_write_bio_PrivateKey(bio_out, pkey, cipher,
                                   (unsigned char *)passphrase.data(),
                                   passphrase.size(), nullptr, nullptr);
  }
  php_openssl_dispose_config(&req);
  return ret;
}

bool HHVM_FUNCTION(openssl_pkey_export_to_file, const Variant& key,
                                                const String& outfilename,
                                   const String& passphrase /* = null_string */,
                             const Variant& configargs /* = uninit_variant */) {
  BIO *bio_out = BIO_new_file(outfilename.data(), "w");
  if (bio_out == nullptr) {
    raise_warning("error opening the file, %s", outfilename.data());
    return false;
  }
  bool ret = openssl_pkey_export_impl(key, bio_out, passphrase, configargs);
  BIO_free(bio_out);
  return ret;
}

bool HHVM_FUNCTION(openssl_pkey_export, const Variant& key, Variant& out,
                                   const String& passphrase /* = null_string */,
                            const Variant& configargs /* = uninit_variant */) {
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

const StaticString
  s_bits("bits"),
  s_key("key"),
  s_type("type"),
  s_name("name"),
  s_hash("hash"),
  s_version("version"),
  s_serialNumber("serialNumber"),
  s_signatureAlgorithm("signatureAlgorithm"),
  s_validFrom("validFrom"),
  s_validTo("validTo"),
  s_validFrom_time_t("validFrom_time_t"),
  s_validTo_time_t("validTo_time_t"),
  s_alias("alias"),
  s_purposes("purposes"),
  s_extensions("extensions"),
  s_rsa("rsa"),
  s_dsa("dsa"),
  s_dh("dh"),
  s_ec("ec"),
  s_n("n"),
  s_e("e"),
  s_d("d"),
  s_p("p"),
  s_q("q"),
  s_g("g"),
  s_x("x"),
  s_y("y"),
  s_dmp1("dmp1"),
  s_dmq1("dmq1"),
  s_iqmp("iqmp"),
  s_priv_key("priv_key"),
  s_pub_key("pub_key"),
  s_curve_oid("curve_oid");

static void add_bignum_as_string(Array &arr,
                                 StaticString key,
                                 const BIGNUM *bn) {
  if (!bn) {
    return;
  }
  int num_bytes = BN_num_bytes(bn);
  String str{size_t(num_bytes), ReserveString};
  BN_bn2bin(bn, (unsigned char*)str.mutableData());
  str.setSize(num_bytes);
  arr.set(key, std::move(str));
}

Array HHVM_FUNCTION(openssl_pkey_get_details, const OptResource& key) {
  EVP_PKEY *pkey = cast<Key>(key)->m_key;
  BIO *out = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(out, pkey);
  char *pbio;
  unsigned int pbio_len = BIO_get_mem_data(out, &pbio);

  auto ret = make_dict_array(
    s_bits, EVP_PKEY_bits(pkey),
    s_key, String(pbio, pbio_len, CopyString)
  );
  long ktype = -1;

  auto details = Array::CreateDict();
  switch (EVP_PKEY_id(pkey)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    {
      ktype = OPENSSL_KEYTYPE_RSA;
      RSA *rsa = EVP_PKEY_get0_RSA(pkey);
      assertx(rsa);
      const BIGNUM *n, *e, *d, *p, *q, *dmp1, *dmq1, *iqmp;
      RSA_get0_key(rsa, &n, &e, &d);
      RSA_get0_factors(rsa, &p, &q);
      RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
      add_bignum_as_string(details, s_n, n);
      add_bignum_as_string(details, s_e, e);
      add_bignum_as_string(details, s_d, d);
      add_bignum_as_string(details, s_p, p);
      add_bignum_as_string(details, s_q, q);
      add_bignum_as_string(details, s_dmp1, dmp1);
      add_bignum_as_string(details, s_dmq1, dmq1);
      add_bignum_as_string(details, s_iqmp, iqmp);
      ret.set(s_rsa, details);
      break;
  }
  case EVP_PKEY_DSA:
  case EVP_PKEY_DSA2:
  case EVP_PKEY_DSA3:
  case EVP_PKEY_DSA4:
    {
      ktype = OPENSSL_KEYTYPE_DSA;
      DSA *dsa = EVP_PKEY_get0_DSA(pkey);
      assertx(dsa);
      const BIGNUM *p, *q, *g, *pub_key, *priv_key;
      DSA_get0_pqg(dsa, &p, &q, &g);
      DSA_get0_key(dsa, &pub_key, &priv_key);
      add_bignum_as_string(details, s_p, p);
      add_bignum_as_string(details, s_q, q);
      add_bignum_as_string(details, s_g, g);
      add_bignum_as_string(details, s_priv_key, priv_key);
      add_bignum_as_string(details, s_pub_key, pub_key);
      ret.set(s_dsa, details);
      break;
    }
  case EVP_PKEY_DH:
    {
      ktype = OPENSSL_KEYTYPE_DH;
      DH *dh = EVP_PKEY_get0_DH(pkey);
      assertx(dh);
      const BIGNUM *p, *q, *g, *pub_key, *priv_key;
      DH_get0_pqg(dh, &p, &q, &g);
      DH_get0_key(dh, &pub_key, &priv_key);
      add_bignum_as_string(details, s_p, p);
      add_bignum_as_string(details, s_g, g);
      add_bignum_as_string(details, s_priv_key, priv_key);
      add_bignum_as_string(details, s_pub_key, pub_key);
      ret.set(s_dh, details);
      break;
    }
#ifdef HAVE_EVP_PKEY_EC
  case EVP_PKEY_EC:
    {
      ktype = OPENSSL_KEYTYPE_EC;
      auto const ec = EVP_PKEY_get0_EC_KEY(pkey);
      assertx(ec);

      auto const ec_group = EC_KEY_get0_group(ec);
      auto const nid = EC_GROUP_get_curve_name(ec_group);
      if (nid == NID_undef) {
        break;
      }

      auto const crv_sn = OBJ_nid2sn(nid);
      if (crv_sn != nullptr) {
        details.set(s_curve_name, String(crv_sn, CopyString));
      }

      auto const obj = OBJ_nid2obj(nid);
      if (obj != nullptr) {
        SCOPE_EXIT {
          ASN1_OBJECT_free(obj);
        };
        char oir_buf[256];
        OBJ_obj2txt(oir_buf, sizeof(oir_buf) - 1, obj, 1);
        details.set(s_curve_oid, String(oir_buf, CopyString));
      }

      auto x = BN_new();
      auto y = BN_new();
      SCOPE_EXIT {
        BN_free(x);
        BN_free(y);
      };
      auto const pub = EC_KEY_get0_public_key(ec);
      if (EC_POINT_get_affine_coordinates_GFp(ec_group, pub, x, y, nullptr)) {
        add_bignum_as_string(details, s_x, x);
        add_bignum_as_string(details, s_y, y);
      }

      auto d = BN_dup(EC_KEY_get0_private_key(ec));
      SCOPE_EXIT {
        BN_free(d);
      };
      if (d != nullptr) {
        add_bignum_as_string(details, s_d, d);
      }

      ret.set(s_ec, details);
    }
    break;
#endif
  }
  ret.set(s_type, ktype);
  BIO_free(out);
  return ret;
}

Variant HHVM_FUNCTION(openssl_pkey_get_private, const Variant& key,
                                 const String& passphrase /* = null_string */) {
  return toVariant(Key::Get(key, false, passphrase.data()));
}

Variant HHVM_FUNCTION(openssl_pkey_get_public, const Variant& certificate) {
  return toVariant(Key::Get(certificate, true));
}

Variant HHVM_FUNCTION(openssl_pkey_new,
                       const Variant& configargs /* = uninit_variant */) {
  struct php_x509_request req;
  memset(&req, 0, sizeof(req));
  SCOPE_EXIT {
    php_openssl_dispose_config(&req);
  };

  std::vector<String> strings;
  if (php_openssl_parse_config(&req, configargs.toArray(), strings) &&
      req.generatePrivateKey()) {
    return OptResource(req::make<Key>(req.priv_key));
  } else {
    return false;
  }
}

bool HHVM_FUNCTION(openssl_private_decrypt, const String& data,
                                            Variant& decrypted,
                                            const Variant& key,
                                  int64_t padding /* = k_OPENSSL_PKCS1_PADDING */) {
  auto okey = Key::Get(key, false);
  if (!okey) {
    raise_warning("key parameter is not a valid private key");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  String s = String(cryptedlen, ReserveString);
  unsigned char *cryptedbuf = (unsigned char *)s.mutableData();

  int successful = 0;
  switch (EVP_PKEY_id(pkey)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    cryptedlen = RSA_private_decrypt(data.size(),
                                     (unsigned char *)data.data(),
                                     cryptedbuf,
                                     EVP_PKEY_get0_RSA(pkey),
                                     padding);
    if (cryptedlen != -1) {
      successful = 1;
    }
    break;

  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    decrypted = s.setSize(cryptedlen);
    return true;
  }

  return false;
}

bool HHVM_FUNCTION(openssl_private_encrypt, const String& data,
                                            Variant& crypted,
                                            const Variant& key,
                                  int64_t padding /* = k_OPENSSL_PKCS1_PADDING */) {
  auto okey = Key::Get(key, false);
  if (!okey) {
    raise_warning("key param is not a valid private key");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  String s = String(cryptedlen, ReserveString);
  unsigned char *cryptedbuf = (unsigned char *)s.mutableData();

  int successful = 0;
  switch (EVP_PKEY_id(pkey)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    successful = (RSA_private_encrypt(data.size(),
                                      (unsigned char *)data.data(),
                                      cryptedbuf,
                                      EVP_PKEY_get0_RSA(pkey),
                                      padding) == cryptedlen);
    break;
  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    crypted = s.setSize(cryptedlen);
    return true;
  }

  return false;
}

bool HHVM_FUNCTION(openssl_public_decrypt, const String& data,
                                           Variant& decrypted,
                                           const Variant& key,
                                  int64_t padding /* = k_OPENSSL_PKCS1_PADDING */) {
  auto okey = Key::Get(key, true);
  if (!okey) {
    raise_warning("key parameter is not a valid public key");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  String s = String(cryptedlen, ReserveString);
  unsigned char *cryptedbuf = (unsigned char *)s.mutableData();

  int successful = 0;
  switch (EVP_PKEY_id(pkey)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    cryptedlen = RSA_public_decrypt(data.size(),
                                    (unsigned char *)data.data(),
                                    cryptedbuf,
                                    EVP_PKEY_get0_RSA(pkey),
                                    padding);
    if (cryptedlen != -1) {
      successful = 1;
    }
    break;

  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    decrypted = s.setSize(cryptedlen);
    return true;
  }

  return false;
}

bool HHVM_FUNCTION(openssl_public_encrypt, const String& data,
                                           Variant& crypted,
                                           const Variant& key,
                                  int64_t padding /* = k_OPENSSL_PKCS1_PADDING */) {
  auto okey = Key::Get(key, true);
  if (!okey) {
    raise_warning("key parameter is not a valid public key");
    return false;
  }
  EVP_PKEY *pkey = okey->m_key;
  int cryptedlen = EVP_PKEY_size(pkey);
  String s = String(cryptedlen, ReserveString);
  unsigned char *cryptedbuf = (unsigned char *)s.mutableData();

  int successful = 0;
  switch (EVP_PKEY_id(pkey)) {
  case EVP_PKEY_RSA:
  case EVP_PKEY_RSA2:
    successful = (RSA_public_encrypt(data.size(),
                                     (unsigned char *)data.data(),
                                     cryptedbuf,
                                     EVP_PKEY_get0_RSA(pkey),
                                     padding) == cryptedlen);
    break;
  default:
    raise_warning("key type not supported");
  }

  if (successful) {
    crypted = s.setSize(cryptedlen);
    return true;
  }

  return false;
}

Variant HHVM_FUNCTION(openssl_seal, const String& data, Variant& sealed_data,
                                    Variant& env_keys,
                                    const Array& pub_key_ids,
                                    const String& method,
                                    Variant& iv) {
  int nkeys = pub_key_ids.size();
  if (nkeys == 0) {
    raise_warning("Fourth argument to openssl_seal() must be "
                    "a non-empty array");
    return false;
  }

  const EVP_CIPHER *cipher_type;
  if (method.empty()) {
    cipher_type = EVP_rc4();
  } else {
    cipher_type = EVP_get_cipherbyname(method.c_str());
    if (!cipher_type) {
      raise_warning("Unknown cipher algorithm");
      return false;
    }
  }

  int iv_len = EVP_CIPHER_iv_length(cipher_type);
  unsigned char *iv_buf = nullptr;
  String iv_s;
  if (iv_len > 0) {
    iv_s = String(iv_len, ReserveString);
    iv_buf = (unsigned char*)iv_s.mutableData();

    if (!RAND_bytes(iv_buf, iv_len)) {
      raise_warning("Could not generate an IV.");
      return false;
    }
  }

  EVP_PKEY **pkeys = (EVP_PKEY**)malloc(nkeys * sizeof(*pkeys));
  int *eksl = (int*)malloc(nkeys * sizeof(*eksl));
  unsigned char **eks = (unsigned char **)malloc(nkeys * sizeof(*eks));
  memset(eks, 0, sizeof(*eks) * nkeys);

  // holder is needed to make sure none of the Keys get deleted prematurely.
  // The pkeys array points to elements inside of Keys returned from Key::Get()
  // which may be newly allocated and have no other owners.
  std::vector<req::ptr<Key>> holder;

  /* get the public keys we are using to seal this data */
  bool ret = true;
  int i = 0;
  String s;
  unsigned char* buf = nullptr;
  EVP_CIPHER_CTX* ctx = nullptr;
  for (ArrayIter iter(pub_key_ids); iter; ++iter, ++i) {
    auto okey = Key::Get(iter.second(), true);
    if (!okey) {
      raise_warning("not a public key (%dth member of pubkeys)", i + 1);
      ret = false;
      goto clean_exit;
    }
    holder.push_back(okey);
    pkeys[i] = okey->m_key;
    eks[i] = (unsigned char *)malloc(EVP_PKEY_size(pkeys[i]) + 1);
  }

  ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) {
    raise_warning("Failed to allocate an EVP_CIPHER_CTX object");
    ret = false;
    goto clean_exit;
  }
  if (!EVP_EncryptInit_ex(ctx, cipher_type, nullptr, nullptr, nullptr)) {
    ret = false;
    goto clean_exit;
  }

  int len1, len2;

  s = String(data.size() + EVP_CIPHER_CTX_block_size(ctx), ReserveString);
  buf = (unsigned char *)s.mutableData();
  if (EVP_SealInit(ctx, cipher_type, eks, eksl, iv_buf, pkeys, nkeys) <= 0 ||
      !EVP_SealUpdate(ctx, buf, &len1, (unsigned char*)data.data(), data.size()) ||
      !EVP_SealFinal(ctx, buf + len1, &len2)) {
    ret = false;
    goto clean_exit;
  }

  if (len1 + len2 > 0) {
    sealed_data = s.setSize(len1 + len2);

    auto ekeys = Array::CreateVec();
    for (i = 0; i < nkeys; i++) {
      eks[i][eksl[i]] = '\0';
      ekeys.append(String((char*)eks[i], eksl[i], AttachString));
      eks[i] = nullptr;
    }
    env_keys = ekeys;
  }

 clean_exit:
  for (i = 0; i < nkeys; i++) {
    if (eks[i]) free(eks[i]);
  }
  free(eks);
  free(eksl);
  free(pkeys);

  if (iv_buf != nullptr) {
    if (ret) {
      iv = iv_s.setSize(iv_len);
    }
  }
  if (ctx != nullptr) {
    EVP_CIPHER_CTX_free(ctx);
  }

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
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  case OPENSSL_ALGO_DSS1: return EVP_dss1();
#endif
#if OPENSSL_VERSION_NUMBER >= 0x0090708fL
  case OPENSSL_ALGO_SHA224: return EVP_sha224();
  case OPENSSL_ALGO_SHA256: return EVP_sha256();
  case OPENSSL_ALGO_SHA384: return EVP_sha384();
  case OPENSSL_ALGO_SHA512: return EVP_sha512();
  case OPENSSL_ALGO_RMD160: return EVP_ripemd160();
#endif
  }
  return nullptr;
}

bool HHVM_FUNCTION(openssl_sign, const String& data, Variant& signature,
                                 const Variant& priv_key_id,
                     const Variant& signature_alg /* = k_OPENSSL_ALGO_SHA1 */) {
  auto okey = Key::Get(priv_key_id, false);
  if (!okey) {
    raise_warning("supplied key param cannot be coerced into a private key");
    return false;
  }

  const EVP_MD *mdtype = nullptr;
  if (signature_alg.isInteger()) {
    mdtype = php_openssl_get_evp_md_from_algo(signature_alg.toInt64Val());
  } else if (signature_alg.isString()) {
    mdtype = EVP_get_digestbyname(signature_alg.toString().data());
  }

  if (!mdtype) {
    raise_warning("Unknown signature algorithm.");
    return false;
  }

  EVP_PKEY *pkey = okey->m_key;
  int siglen = EVP_PKEY_size(pkey);
  String s = String(siglen, ReserveString);
  unsigned char *sigbuf = (unsigned char *)s.mutableData();

  EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
  SCOPE_EXIT {
    EVP_MD_CTX_free(md_ctx);
  };
  EVP_SignInit(md_ctx, mdtype);
  EVP_SignUpdate(md_ctx, (unsigned char *)data.data(), data.size());
  if (EVP_SignFinal(md_ctx, sigbuf, (unsigned int *)&siglen, pkey)) {
    signature = s.setSize(siglen);
    return true;
  }
  return false;
}

Variant HHVM_FUNCTION(openssl_verify, const String& data,
                                      const String& signature,
                                      const Variant& pub_key_id,
                     const Variant& signature_alg /* = k_OPENSSL_ALGO_SHA1 */) {
  int err;
  const EVP_MD *mdtype = nullptr;

  if (signature_alg.isInteger()) {
    mdtype = php_openssl_get_evp_md_from_algo(signature_alg.toInt64Val());
  } else if (signature_alg.isString()) {
    mdtype = EVP_get_digestbyname(signature_alg.toString().data());
  }

  if (!mdtype) {
    raise_warning("Unknown signature algorithm.");
    return false;
  }

  auto okey = Key::Get(pub_key_id, true);
  if (!okey) {
    raise_warning("supplied key param cannot be coerced into a public key");
    return false;
  }

  EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
  SCOPE_EXIT {
    EVP_MD_CTX_free(md_ctx);
  };
  EVP_VerifyInit(md_ctx, mdtype);
  EVP_VerifyUpdate(md_ctx, (unsigned char*)data.data(), data.size());
  err = EVP_VerifyFinal(md_ctx, (unsigned char *)signature.data(),
                         signature.size(), okey->m_key);
  return err;
}

bool HHVM_FUNCTION(openssl_x509_check_private_key, const Variant& cert,
                                                   const Variant& key) {
  auto ocert = Certificate::Get(cert);
  if (!ocert) {
    return false;
  }
  auto okey = Key::Get(key, false);
  if (!okey) {
    return false;
  }
  return X509_check_private_key(ocert->m_cert, okey->m_key);
}

static int check_cert(X509_STORE *ctx, X509 *x, STACK_OF(X509) *untrustedchain,
                      int purpose) {
  X509_STORE_CTX *csc = X509_STORE_CTX_new();
  if (csc == nullptr) {
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

Variant HHVM_FUNCTION(openssl_x509_checkpurpose, const Variant& x509cert,
                      int64_t purpose,
                      const Array& cainfo /* = null_array */,
                      const String& untrustedfile /* = null_string */) {
  int ret = -1;
  STACK_OF(X509) *untrustedchain = nullptr;
  X509_STORE *pcainfo = nullptr;
  req::ptr<Certificate> ocert;

  if (!untrustedfile.empty()) {
    untrustedchain = load_all_certs_from_file(untrustedfile.data());
    if (untrustedchain == nullptr) {
      goto clean_exit;
    }
  }

  pcainfo = setup_verify(cainfo);
  if (pcainfo == nullptr) {
    goto clean_exit;
  }

  ocert = Certificate::Get(x509cert);
  if (!ocert) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  X509 *cert;
  cert = ocert->m_cert;
  assertx(cert);

  ret = check_cert(pcainfo, cert, untrustedchain, purpose);

 clean_exit:
  if (pcainfo) {
    X509_STORE_free(pcainfo);
  }
  if (untrustedchain) {
    sk_X509_pop_free(untrustedchain, X509_free);
  }
  return ret == 1 ? true : ret == 0 ? false : -1;
}

static bool openssl_x509_export_impl(const Variant& x509, BIO *bio_out,
                                     bool notext /* = true */) {
  auto ocert = Certificate::Get(x509);
  if (!ocert) {
    raise_warning("cannot get cert from parameter 1");
    return false;
  }
  X509 *cert = ocert->m_cert;
  assertx(cert);

  assertx(bio_out);
  if (!notext) {
    X509_print(bio_out, cert);
  }
  return PEM_write_bio_X509(bio_out, cert);
}

bool HHVM_FUNCTION(openssl_x509_export_to_file, const Variant& x509,
                                                const String& outfilename,
                                                bool notext /* = true */) {
  BIO *bio_out = BIO_new_file((char*)outfilename.data(), "w");
  if (bio_out == nullptr) {
    raise_warning("error opening file %s", outfilename.data());
    return false;
  }
  bool ret = openssl_x509_export_impl(x509, bio_out, notext);
  BIO_free(bio_out);
  return ret;
}

bool HHVM_FUNCTION(openssl_x509_export, const Variant& x509, Variant& output,
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

/**
 * This is how the time string is formatted:
 *
 * snprintf(p, sizeof(p), "%02d%02d%02d%02d%02d%02dZ",ts->tm_year%100,
 *          ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
 */
static time_t asn1_time_to_time_t(ASN1_UTCTIME *timestr) {

  auto const timestr_type = ASN1_STRING_type(timestr);

  if (timestr_type != V_ASN1_UTCTIME && timestr_type != V_ASN1_GENERALIZEDTIME) {
    raise_warning("illegal ASN1 data type for timestamp");
    return (time_t)-1;
  }

  auto const timestr_len = (size_t)ASN1_STRING_length(timestr);

  // Binary safety
  if (timestr_len != strlen((char*)ASN1_STRING_data(timestr))) {
    raise_warning("illegal length in timestamp");
    return (time_t)-1;
  }

  if (timestr_len < 13 && timestr_len != 11) {
    raise_warning("unable to parse time string %s correctly",
                    timestr->data);
    return (time_t)-1;
  }

  if (timestr_type == V_ASN1_GENERALIZEDTIME && timestr_len < 15) {
    raise_warning("unable to parse time string %s correctly", timestr->data);
    return (time_t)-1;
  }

  char *strbuf = strdup((char*)timestr->data);

  struct tm thetime;
  memset(&thetime, 0, sizeof(thetime));

  /* we work backwards so that we can use atoi more easily */
  char *thestr = strbuf + ASN1_STRING_length(timestr) - 3;
  if (ASN1_STRING_length(timestr) == 11) {
    thetime.tm_sec = 0;
  } else {
    thetime.tm_sec  = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  }
  thetime.tm_min  = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_hour = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_mday = atoi(thestr);   *thestr = '\0';  thestr -= 2;
  thetime.tm_mon  = atoi(thestr)-1; *thestr = '\0';

  if (ASN1_STRING_type(timestr) == V_ASN1_UTCTIME) {
    thestr -= 2;
    thetime.tm_year = atoi(thestr);
    if (thetime.tm_year < 68) {
      thetime.tm_year += 100;
    }
  } else if (ASN1_STRING_type(timestr) == V_ASN1_GENERALIZEDTIME) {
    thestr -= 4;
    thetime.tm_year = atoi(thestr) - 1900;
  }

  thetime.tm_isdst = -1;
  time_t ret = mktime(&thetime);

  long gmadjust = 0;
#if HAVE_TM_GMTOFF
  gmadjust = thetime.tm_gmtoff;
#elif defined(_MSC_VER)
  TIME_ZONE_INFORMATION inf;
  GetTimeZoneInformation(&inf);
  gmadjust = thetime.tm_isdst ? inf.DaylightBias : inf.StandardBias;
#else
  /**
   * If correcting for daylight savings time, we set the adjustment to
   * the value of timezone - 3600 seconds. Otherwise, we need to overcorrect
   * and set the adjustment to the main timezone + 3600 seconds.
   */
  gmadjust = -(thetime.tm_isdst ?
               (long)timezone - 3600 : (long)timezone);
#endif
  /* no adjustment for UTC */
  if (timezone) ret += gmadjust;
  free(strbuf);
  return ret;
}

/* Special handling of subjectAltName, see CVE-2013-4073
 * Christian Heimes
 */

static int openssl_x509v3_subjectAltName(BIO *bio, X509_EXTENSION *extension)
{
  GENERAL_NAMES *names;
  const X509V3_EXT_METHOD *method = nullptr;
  long i, length, num;
  const unsigned char *p;

  method = X509V3_EXT_get(extension);
  if (method == nullptr) {
    return -1;
  }

  const auto data = X509_EXTENSION_get_data(extension);
  p = data->data;
  length = data->length;
  if (method->it) {
    names = (GENERAL_NAMES*)(ASN1_item_d2i(nullptr, &p, length,
                                          ASN1_ITEM_ptr(method->it)));
  } else {
    names = (GENERAL_NAMES*)(method->d2i(nullptr, &p, length));
  }
  if (names == nullptr) {
    return -1;
  }

  num = sk_GENERAL_NAME_num(names);
  for (i = 0; i < num; i++) {
    GENERAL_NAME *name;
    ASN1_STRING *as;
    name = sk_GENERAL_NAME_value(names, i);
    switch (name->type) {
      case GEN_EMAIL:
             BIO_puts(bio, "email:");
             as = name->d.rfc822Name;
             BIO_write(bio, ASN1_STRING_data(as),
                       ASN1_STRING_length(as));
             break;
      case GEN_DNS:
             BIO_puts(bio, "DNS:");
             as = name->d.dNSName;
             BIO_write(bio, ASN1_STRING_data(as),
                       ASN1_STRING_length(as));
             break;
      case GEN_URI:
             BIO_puts(bio, "URI:");
             as = name->d.uniformResourceIdentifier;
             BIO_write(bio, ASN1_STRING_data(as),
                       ASN1_STRING_length(as));
             break;
      default:
             /* use builtin print for GEN_OTHERNAME, GEN_X400,
              * GEN_EDIPARTY, GEN_DIRNAME, GEN_IPADD and GEN_RID
              */
             GENERAL_NAME_print(bio, name);
    }
    /* trailing ', ' except for last element */
    if (i < (num - 1)) {
           BIO_puts(bio, ", ");
    }
  }
  sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);

  return 0;
}
Variant HHVM_FUNCTION(openssl_x509_parse, const Variant& x509cert,
                                          bool shortnames /* = true */) {
  auto ocert = Certificate::Get(x509cert);
  if (!ocert) {
    return false;
  }
  X509 *cert = ocert->m_cert;
  assertx(cert);

  auto ret = Array::CreateDict();
  const auto sn = X509_get_subject_name(cert);
  if (sn) {
    ret.set(s_name, String(X509_NAME_oneline(sn, nullptr, 0), CopyString));
  }
  add_assoc_name_entry(ret, "subject", sn, shortnames);
  /* hash as used in CA directories to lookup cert by subject name */
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "%08lx", X509_subject_name_hash(cert));
    ret.set(s_hash, String(buf, CopyString));
  }

  add_assoc_name_entry(ret, "issuer", X509_get_issuer_name(cert), shortnames);
  ret.set(s_version, X509_get_version(cert));

  ret.set(s_serialNumber, String
          (i2s_ASN1_INTEGER(nullptr, X509_get_serialNumber(cert)), AttachString));
  // Adding Signature Algorithm
  BIO *bio_out = BIO_new(BIO_s_mem());
  SCOPE_EXIT { BIO_free(bio_out); };
  if (i2a_ASN1_OBJECT(bio_out, X509_get0_tbs_sigalg(cert)->algorithm) > 0) {
    BUF_MEM *bio_buf;
    BIO_get_mem_ptr(bio_out, &bio_buf);
    ret.set(s_signatureAlgorithm,
            String((char*)bio_buf->data, bio_buf->length, CopyString));
  }

  ASN1_STRING *str = X509_get_notBefore(cert);
  ret.set(s_validFrom, String((char*)str->data, str->length, CopyString));
  str = X509_get_notAfter(cert);
  ret.set(s_validTo, String((char*)str->data, str->length, CopyString));
  ret.set(s_validFrom_time_t, asn1_time_to_time_t(X509_get_notBefore(cert)));
  ret.set(s_validTo_time_t, asn1_time_to_time_t(X509_get_notAfter(cert)));

  char *tmpstr = (char *)X509_alias_get0(cert, nullptr);
  if (tmpstr) {
    ret.set(s_alias, String(tmpstr, CopyString));
  }

  /* NOTE: the purposes are added as integer keys - the keys match up to
     the X509_PURPOSE_SSL_XXX defines in x509v3.h */
  {
    Array subitem;
    for (int i = 0; i < X509_PURPOSE_get_count(); i++) {
      X509_PURPOSE *purp = X509_PURPOSE_get0(i);
      int id = X509_PURPOSE_get_id(purp);
      char * pname = shortnames ? X509_PURPOSE_get0_sname(purp) :
        X509_PURPOSE_get0_name(purp);
      auto subsub = make_vec_array(
        (bool)X509_check_purpose(cert, id, 0),
        (bool)X509_check_purpose(cert, id, 1),
        String(pname, CopyString)
      );
      subitem.set(id, std::move(subsub));
    }
    ret.set(s_purposes, subitem);
  }
  {
    auto subitem = Array::CreateDict();
    for (int i = 0; i < X509_get_ext_count(cert); i++) {
      int nid;
      X509_EXTENSION *extension = X509_get_ext(cert, i);
      char *extname;
      char buf[256];
      nid = OBJ_obj2nid(X509_EXTENSION_get_object(extension));
      if (nid != NID_undef) {
        extname = (char*)OBJ_nid2sn(OBJ_obj2nid
                                    (X509_EXTENSION_get_object(extension)));
      } else {
        OBJ_obj2txt(buf, sizeof(buf)-1, X509_EXTENSION_get_object(extension),
                    1);
        extname = buf;
      }
      BIO *bio_out = BIO_new(BIO_s_mem());
      if (nid == NID_subject_alt_name) {
        if (openssl_x509v3_subjectAltName(bio_out, extension) == 0) {
          BUF_MEM *bio_buf;
          BIO_get_mem_ptr(bio_out, &bio_buf);
          subitem.set(String(extname, CopyString),
                      String((char*)bio_buf->data,
                             bio_buf->length,
                             CopyString));
        } else {
          BIO_free(bio_out);
          return false;
        }
      } else if (X509V3_EXT_print(bio_out, extension, 0, 0)) {
        BUF_MEM *bio_buf;
        BIO_get_mem_ptr(bio_out, &bio_buf);
        subitem.set(String(extname, CopyString),
                    String((char*)bio_buf->data, bio_buf->length, CopyString));
      } else {
        str = X509_EXTENSION_get_data(extension);
        subitem.set(String(extname, CopyString),
                    String((char*)str->data, str->length, CopyString));
      }
      BIO_free(bio_out);
    }
    ret.set(s_extensions, subitem);
  }

  return ret;
}

Variant HHVM_FUNCTION(openssl_x509_read, const Variant& x509certdata) {
  auto ocert = Certificate::Get(x509certdata);
  if (!ocert) {
    raise_warning("supplied parameter cannot be coerced into "
                  "an X509 certificate!");
    return false;
  }
  return Variant(ocert);
}

Variant HHVM_FUNCTION(openssl_random_pseudo_bytes, int64_t length,
                      bool& crypto_strong) {
  if (length <= 0) {
    return false;
  }

  unsigned char *buffer = nullptr;

  String s = String(length, ReserveString);
  buffer = (unsigned char *)s.mutableData();

  if (RAND_bytes(buffer, length) <= 0) {
    crypto_strong = false;
    return false;
  } else {
    crypto_strong = true;
    s.setSize(length);
    return s;
  }
}

Variant HHVM_FUNCTION(openssl_cipher_iv_length, const String& method) {
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

/* Cipher mode info */
struct php_openssl_cipher_mode {
  /* Whether this mode uses authenticated encryption. True, for example, with
     the GCM and CCM modes */
  bool is_aead;
  /* Whether this mode is a 'single run aead', meaning that DecryptFinal doesn't
     get called. For example, CCM mode is a single run aead mode. */
  bool is_single_run_aead;
  /* The OpenSSL flag to get the computed tag, if this mode is aead. */
  int aead_get_tag_flag;
  /* The OpenSSL flag to set the computed tag, if this mode is aead. */
  int aead_set_tag_flag;
  /* The OpenSSL flag to set the IV length, if this mode is aead */
  int aead_ivlen_flag;
};

// initialize a php_openssl_cipher_mode corresponding to an EVP_CIPHER.
static php_openssl_cipher_mode php_openssl_load_cipher_mode(
    const EVP_CIPHER* cipher_type) {
  php_openssl_cipher_mode mode = {};
  switch (EVP_CIPHER_mode(cipher_type)) {
#ifdef EVP_CIPH_GCM_MODE
    case EVP_CIPH_GCM_MODE:
      mode.is_aead = true;
      mode.is_single_run_aead = false;
      mode.aead_get_tag_flag = EVP_CTRL_GCM_GET_TAG;
      mode.aead_set_tag_flag = EVP_CTRL_GCM_SET_TAG;
      mode.aead_ivlen_flag = EVP_CTRL_GCM_SET_IVLEN;
      break;
#endif
#ifdef EVP_CIPH_CCM_MODE
    case EVP_CIPH_CCM_MODE:
      mode.is_aead = true;
      mode.is_single_run_aead = true;
      mode.aead_get_tag_flag = EVP_CTRL_CCM_GET_TAG;
      mode.aead_set_tag_flag = EVP_CTRL_CCM_SET_TAG;
      mode.aead_ivlen_flag = EVP_CTRL_CCM_SET_IVLEN;
      break;
#endif
    default:
      break;
  }
  return mode;
}

static bool php_openssl_validate_iv(
    String piv,
    int iv_required_len,
    String& out,
    EVP_CIPHER_CTX* cipher_ctx,
    const php_openssl_cipher_mode* mode) {
  if (cipher_ctx == nullptr || mode == nullptr) {
    return false;
  }

  /* Best case scenario, user behaved */
  if (piv.size() == iv_required_len) {
    out = std::move(piv);
    return true;
  }

  if (mode->is_aead) {
    if (EVP_CIPHER_CTX_ctrl(
            cipher_ctx, mode->aead_ivlen_flag, piv.size(), nullptr) != 1) {
      raise_warning(
          "Setting of IV length for AEAD mode failed, the expected length is "
          "%d bytes",
          iv_required_len);
      return false;
    }
    out = std::move(piv);
    return true;
  }

  String s = String(iv_required_len, ReserveString);
  char* iv_new = s.mutableData();
  memset(iv_new, 0, iv_required_len);

  if (piv.size() <= 0) {
    /* BC behavior */
    s.setSize(iv_required_len);
    out = std::move(s);
    return true;
  }

  if (piv.size() < iv_required_len) {
    raise_warning("IV passed is only %ld bytes long, cipher "
                  "expects an IV of precisely %d bytes, padding with \\0",
                  piv.size(), iv_required_len);
    memcpy(iv_new, piv.data(), piv.size());
    s.setSize(iv_required_len);
    out = std::move(s);
    return true;
  }

  raise_warning("IV passed is %ld bytes long which is longer than the %d "
                "expected by selected cipher, truncating", piv.size(),
                iv_required_len);
  memcpy(iv_new, piv.data(), iv_required_len);
  s.setSize(iv_required_len);
  out = std::move(s);
  return true;
}

namespace {

Variant openssl_encrypt_impl(const String& data,
                                    const String& method,
                                    const String& password,
                                    int options,
                                    const String& iv,
                                    Variant* tag_out,
                                    const String& aad,
                                    int tag_length) {
  const EVP_CIPHER *cipher_type = EVP_get_cipherbyname(method.c_str());
  if (!cipher_type) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  EVP_CIPHER_CTX* cipher_ctx = EVP_CIPHER_CTX_new();
  if (!cipher_ctx) {
    raise_warning("Failed to create cipher context");
    return false;
  }

  SCOPE_EXIT {
    EVP_CIPHER_CTX_free(cipher_ctx);
  };

  php_openssl_cipher_mode mode = php_openssl_load_cipher_mode(cipher_type);

  if (mode.is_aead && !tag_out) {
    raise_warning("Must call openssl_encrypt_with_tag when using an AEAD cipher");
    return false;
  }

  int keylen = EVP_CIPHER_key_length(cipher_type);
  String key = password;

  /*
   * older openssl libraries can assert if the passed in password length is
   * less than keylen
   */
  if (keylen > password.size()) {
    String s = String(keylen, ReserveString);
    char *keybuf = s.mutableData();
    memset(keybuf, 0, keylen);
    memcpy(keybuf, password.data(), password.size());
    key = s.setSize(keylen);
  }

  int max_iv_len = EVP_CIPHER_iv_length(cipher_type);
  if (iv.size() <= 0 && max_iv_len > 0 && !mode.is_aead) {
    raise_warning("Using an empty Initialization Vector (iv) is potentially "
                  "insecure and not recommended");
  }

  int result_len = 0;
  int outlen = data.size() + EVP_CIPHER_block_size(cipher_type);
  String rv = String(outlen, ReserveString);
  unsigned char *outbuf = (unsigned char*)rv.mutableData();

  EVP_EncryptInit_ex(cipher_ctx, cipher_type, nullptr, nullptr, nullptr);

  String new_iv;
  // we do this after EncryptInit because validate_iv changes cipher_ctx for
  // aead modes (must be initialized first).
  if (!php_openssl_validate_iv(
          std::move(iv), max_iv_len, new_iv, cipher_ctx, &mode)) {
    return false;
  }

  // set the tag length for CCM mode/other modes that require tag lengths to
  // be set.
  if (mode.is_single_run_aead &&
      !EVP_CIPHER_CTX_ctrl(
          cipher_ctx, mode.aead_set_tag_flag, tag_length, nullptr)) {
    raise_warning("Setting tag length failed");
    return false;
  }
  if (password.size() > keylen) {
    EVP_CIPHER_CTX_set_key_length(cipher_ctx, password.size());
  }
  EVP_EncryptInit_ex(
      cipher_ctx,
      nullptr,
      nullptr,
      (unsigned char*)key.data(),
      (unsigned char*)new_iv.data());
  if (options & k_OPENSSL_ZERO_PADDING) {
    EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
  }

  // for single run aeads like CCM, we need to provide the length of the
  // plaintext before providing AAD or ciphertext.
  if (mode.is_single_run_aead &&
      !EVP_EncryptUpdate(
          cipher_ctx, nullptr, &result_len, nullptr, data.size())) {
    raise_warning("Setting of data length failed");
    return false;
  }

  // set up aad:
  if (mode.is_aead &&
      !EVP_EncryptUpdate(
          cipher_ctx,
          nullptr,
          &result_len,
          (unsigned char*)aad.data(),
          aad.size())) {
    raise_warning("Setting of additional application data failed");
    return false;
  }

  // OpenSSL before 0.9.8i asserts with size < 0
  if (data.size() >= 0) {
    EVP_EncryptUpdate(cipher_ctx, outbuf, &result_len,
                      (unsigned char *)data.data(), data.size());
  }

  outlen = result_len;

  if (EVP_EncryptFinal_ex(
          cipher_ctx, (unsigned char*)outbuf + result_len, &result_len)) {
    outlen += result_len;
    rv.setSize(outlen);
    // Get tag if possible
    if (mode.is_aead) {
      String tagrv = String(tag_length, ReserveString);
      if (EVP_CIPHER_CTX_ctrl(
              cipher_ctx,
              mode.aead_get_tag_flag,
              tag_length,
              tagrv.mutableData()) == 1) {
        tagrv.setSize(tag_length);
        assertx(tag_out);
        *tag_out = tagrv;
      } else {
        raise_warning("Retrieving authentication tag failed");
        return false;
      }
    } else if (tag_out) {
      raise_warning(
          "The authenticated tag cannot be provided for cipher that does not"
          " support AEAD");
    }
    // Return encrypted data
    if (options & k_OPENSSL_RAW_DATA) {
      return rv;
    } else {
      return StringUtil::Base64Encode(rv);
    }
  }
  return false;
}

} // anonymous namespace

Variant HHVM_FUNCTION(openssl_encrypt,
                      const String& data,
                      const String& method,
                      const String& password,
                      int64_t options /* = 0 */,
                      const String& iv /* = null_string */,
                      const String& aad /* = null_string */,
                      int64_t tag_length /* = 16 */) {
  return openssl_encrypt_impl(data, method, password, options, iv,
                              nullptr, aad, tag_length);
}

Variant HHVM_FUNCTION(openssl_encrypt_with_tag,
                      const String& data,
                      const String& method,
                      const String& password,
                      int64_t options,
                      const String& iv,
                      Variant& tag_out,
                      const String& aad /* = null_string */,
                      int64_t tag_length /* = 16 */) {
  return openssl_encrypt_impl(data, method, password, options, iv,
                              &tag_out, aad, tag_length);
}

Variant HHVM_FUNCTION(openssl_decrypt, const String& data, const String& method,
                                       const String& password,
                                       int64_t options /* = 0 */,
                                       const String& iv /* = null_string */,
                                       const String& tag /* = null_string */,
                                       const String& aad /* = null_string */) {
  const EVP_CIPHER *cipher_type = EVP_get_cipherbyname(method.c_str());
  if (!cipher_type) {
    raise_warning("Unknown cipher algorithm");
    return false;
  }

  EVP_CIPHER_CTX* cipher_ctx = EVP_CIPHER_CTX_new();
  if (!cipher_ctx) {
    raise_warning("Failed to create cipher context");
    return false;
  }

  SCOPE_EXIT {
    EVP_CIPHER_CTX_free(cipher_ctx);
  };

  php_openssl_cipher_mode mode = php_openssl_load_cipher_mode(cipher_type);

  String decoded_data = data;

  if (!(options & k_OPENSSL_RAW_DATA)) {
    decoded_data = StringUtil::Base64Decode(data);
  }

  int keylen = EVP_CIPHER_key_length(cipher_type);
  String key = password;

  /*
   * older openssl libraries can assert if the passed in password length is
   * less than keylen
   */
   if (keylen > password.size()) {
    String s = String(keylen, ReserveString);
    char *keybuf = s.mutableData();
    memset(keybuf, 0, keylen);
    memcpy(keybuf, password.data(), password.size());
    key = s.setSize(keylen);
  }

  int result_len = 0;
  int outlen = decoded_data.size() + EVP_CIPHER_block_size(cipher_type);
  String rv = String(outlen, ReserveString);
  unsigned char *outbuf = (unsigned char*)rv.mutableData();

  EVP_DecryptInit_ex(cipher_ctx, cipher_type, nullptr, nullptr, nullptr);

  String new_iv;
  // we do this after DecryptInit because validate_iv changes cipher_ctx for
  // aead modes (must be initialized first).
  if (!php_openssl_validate_iv(
          std::move(iv),
          EVP_CIPHER_iv_length(cipher_type),
          new_iv,
          cipher_ctx,
          &mode)) {
    return false;
  }

  // set the tag if required:
  if (tag.size() > 0) {
    if (!mode.is_aead) {
      raise_warning(
          "The tag is being ignored because the cipher method does not"
          " support AEAD");
    } else if (!EVP_CIPHER_CTX_ctrl(
                   cipher_ctx,
                   mode.aead_set_tag_flag,
                   tag.size(),
                   (unsigned char*)tag.data())) {
      raise_warning("Setting tag for AEAD cipher decryption failed");
      return false;
    }
  } else {
    if (mode.is_aead) {
      raise_warning("A tag should be provided when using AEAD mode");
      return false;
    }
  }
  if (password.size() > keylen) {
    EVP_CIPHER_CTX_set_key_length(cipher_ctx, password.size());
  }
  EVP_DecryptInit_ex(
      cipher_ctx,
      nullptr,
      nullptr,
      (unsigned char*)key.data(),
      (unsigned char*)new_iv.data());
  if (options & k_OPENSSL_ZERO_PADDING) {
    EVP_CIPHER_CTX_set_padding(cipher_ctx, 0);
  }

  // for single run aeads like CCM, we need to provide the length of the
  // ciphertext before providing AAD or ciphertext.
  if (mode.is_single_run_aead &&
      !EVP_DecryptUpdate(
          cipher_ctx, nullptr, &result_len, nullptr, decoded_data.size())) {
    raise_warning("Setting of data length failed");
    return false;
  }

  // set up aad:
  if (mode.is_aead &&
      !EVP_DecryptUpdate(
          cipher_ctx,
          nullptr,
          &result_len,
          (unsigned char*)aad.data(),
          aad.size())) {
    raise_warning("Setting of additional application data failed");
    return false;
  }

  if (!EVP_DecryptUpdate(
          cipher_ctx,
          outbuf,
          &result_len,
          (unsigned char*)decoded_data.data(),
          decoded_data.size())) {
    return false;
  }
  outlen = result_len;

  // if is_single_run_aead is enabled, DecryptFinal shouldn't be called.
  // if something went wrong in this case, we would've caught it at
  // DecryptUpdate.
  if (mode.is_single_run_aead ||
      EVP_DecryptFinal_ex(
          cipher_ctx, (unsigned char*)outbuf + result_len, &result_len)) {
    // don't want to do this if is_single_run_aead was enabled, since we didn't
    // make a call to EVP_DecryptFinal.
    if (!mode.is_single_run_aead) {
      outlen += result_len;
    }
    rv.setSize(outlen);
    return rv;
  } else {
    return false;
  }
}

Variant HHVM_FUNCTION(openssl_digest, const String& data, const String& method,
                                      bool raw_output /* = false */) {
  const EVP_MD *mdtype = EVP_get_digestbyname(method.c_str());

  if (!mdtype) {
    raise_warning("Unknown signature algorithm");
    return false;
  }
  int siglen = EVP_MD_size(mdtype);
  String rv = String(siglen, ReserveString);
  unsigned char *sigbuf = (unsigned char *)rv.mutableData();
  EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
  SCOPE_EXIT {
    EVP_MD_CTX_free(md_ctx);
  };

  EVP_DigestInit(md_ctx, mdtype);

  EVP_DigestUpdate(md_ctx, (unsigned char *)data.data(), data.size());
  if (EVP_DigestFinal(md_ctx, (unsigned char *)sigbuf, (unsigned int *)&siglen)) {
    if (raw_output) {
      rv.setSize(siglen);
      return rv;
    } else {
      char* digest_str = string_bin2hex((char*)sigbuf, siglen);
      return String(digest_str, AttachString);
    }
  } else {
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

Array HHVM_FUNCTION(openssl_get_cipher_methods, bool aliases /* = false */) {
  Array ret = Array::CreateVec();
  OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH,
    aliases ? openssl_add_method_or_alias: openssl_add_method,
    &ret);
  return ret;
}

Variant HHVM_FUNCTION(openssl_get_curve_names) {
#ifdef HAVE_EVP_PKEY_EC
  const size_t len = EC_get_builtin_curves(nullptr, 0);
  std::unique_ptr<EC_builtin_curve[]> curves(new EC_builtin_curve[len]);
  if (!EC_get_builtin_curves(curves.get(), len)) {
    return false;
  }

  VecInit ret(len);
  for (size_t i = 0; i < len; ++i) {
    auto const sname = OBJ_nid2sn(curves[i].nid);
    if (sname != nullptr) {
      ret.append(String(sname, CopyString));
    }
  }

  return ret.toArray();
#else
  return false;
#endif
}

Array HHVM_FUNCTION(openssl_get_md_methods, bool aliases /* = false */) {
  Array ret = Array::CreateVec();
  OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_MD_METH,
    aliases ? openssl_add_method_or_alias: openssl_add_method,
    &ret);
  return ret;
}

/////////////////////////////////////////////////////////////////////////////

const StaticString s_OPENSSL_VERSION_TEXT("OPENSSL_VERSION_TEXT");

struct opensslExtension final : Extension {
  opensslExtension() : Extension("openssl", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleInit() override {
    HHVM_RC_INT(OPENSSL_RAW_DATA, k_OPENSSL_RAW_DATA);
    HHVM_RC_INT(OPENSSL_ZERO_PADDING, k_OPENSSL_ZERO_PADDING);
    HHVM_RC_INT(OPENSSL_NO_PADDING, k_OPENSSL_NO_PADDING);
    HHVM_RC_INT(OPENSSL_PKCS1_OAEP_PADDING, k_OPENSSL_PKCS1_OAEP_PADDING);
    HHVM_RC_INT(OPENSSL_SSLV23_PADDING, k_OPENSSL_SSLV23_PADDING);
    HHVM_RC_INT(OPENSSL_PKCS1_PADDING, k_OPENSSL_PKCS1_PADDING);

    HHVM_RC_INT_SAME(OPENSSL_ALGO_SHA1);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_MD5);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_MD4);
#ifdef HAVE_OPENSSL_MD2_H
    HHVM_RC_INT_SAME(OPENSSL_ALGO_MD2);
#endif
    HHVM_RC_INT_SAME(OPENSSL_ALGO_DSS1);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_SHA224);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_SHA256);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_SHA384);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_SHA512);
    HHVM_RC_INT_SAME(OPENSSL_ALGO_RMD160);

    HHVM_RC_INT(OPENSSL_CIPHER_RC2_40, PHP_OPENSSL_CIPHER_RC2_40);
    HHVM_RC_INT(OPENSSL_CIPHER_RC2_128, PHP_OPENSSL_CIPHER_RC2_128);
    HHVM_RC_INT(OPENSSL_CIPHER_RC2_64, PHP_OPENSSL_CIPHER_RC2_64);
    HHVM_RC_INT(OPENSSL_CIPHER_DES, PHP_OPENSSL_CIPHER_DES);
    HHVM_RC_INT(OPENSSL_CIPHER_3DES, PHP_OPENSSL_CIPHER_3DES);

    HHVM_RC_INT_SAME(OPENSSL_KEYTYPE_RSA);
    HHVM_RC_INT_SAME(OPENSSL_KEYTYPE_DSA);
    HHVM_RC_INT_SAME(OPENSSL_KEYTYPE_DH);
#ifdef HAVE_EVP_PKEY_EC
    HHVM_RC_INT_SAME(OPENSSL_KEYTYPE_EC);
#endif

    HHVM_RC_INT_SAME(OPENSSL_VERSION_NUMBER);

    HHVM_RC_INT_SAME(PKCS7_TEXT);
    HHVM_RC_INT_SAME(PKCS7_NOCERTS);
    HHVM_RC_INT_SAME(PKCS7_NOSIGS);
    HHVM_RC_INT_SAME(PKCS7_NOCHAIN);
    HHVM_RC_INT_SAME(PKCS7_NOINTERN);
    HHVM_RC_INT_SAME(PKCS7_NOVERIFY);
    HHVM_RC_INT_SAME(PKCS7_DETACHED);
    HHVM_RC_INT_SAME(PKCS7_BINARY);
    HHVM_RC_INT_SAME(PKCS7_NOATTR);

    HHVM_RC_STR_SAME(OPENSSL_VERSION_TEXT);

    HHVM_RC_INT_SAME(X509_PURPOSE_SSL_CLIENT);
    HHVM_RC_INT_SAME(X509_PURPOSE_SSL_SERVER);
    HHVM_RC_INT_SAME(X509_PURPOSE_NS_SSL_SERVER);
    HHVM_RC_INT_SAME(X509_PURPOSE_SMIME_SIGN);
    HHVM_RC_INT_SAME(X509_PURPOSE_SMIME_ENCRYPT);
    HHVM_RC_INT_SAME(X509_PURPOSE_CRL_SIGN);
#ifdef X509_PURPOSE_ANY
    HHVM_RC_INT_SAME(X509_PURPOSE_ANY);
#endif

    HHVM_FE(openssl_csr_export_to_file);
    HHVM_FE(openssl_csr_export);
    HHVM_FE(openssl_csr_get_public_key);
    HHVM_FE(openssl_csr_get_subject);
    HHVM_FE(openssl_csr_new);
    HHVM_FE(openssl_csr_sign);
    HHVM_FE(openssl_error_string);
    HHVM_FE(openssl_open);
    HHVM_FE(openssl_pkcs12_export_to_file);
    HHVM_FE(openssl_pkcs12_export);
    HHVM_FE(openssl_pkcs12_read);
    HHVM_FE(openssl_pkcs7_decrypt);
    HHVM_FE(openssl_pkcs7_encrypt);
    HHVM_FE(openssl_pkcs7_sign);
    HHVM_FE(openssl_pkcs7_verify);
    HHVM_FE(openssl_pkey_export_to_file);
    HHVM_FE(openssl_pkey_export);
    HHVM_FE(openssl_pkey_get_details);
    HHVM_FE(openssl_pkey_get_private);
    HHVM_FE(openssl_pkey_get_public);
    HHVM_FE(openssl_pkey_new);
    HHVM_FE(openssl_private_decrypt);
    HHVM_FE(openssl_private_encrypt);
    HHVM_FE(openssl_public_decrypt);
    HHVM_FE(openssl_public_encrypt);
    HHVM_FE(openssl_seal);
    HHVM_FE(openssl_sign);
    HHVM_FE(openssl_verify);
    HHVM_FE(openssl_x509_check_private_key);
    HHVM_FE(openssl_x509_checkpurpose);
    HHVM_FE(openssl_x509_export_to_file);
    HHVM_FE(openssl_x509_export);
    HHVM_FE(openssl_x509_parse);
    HHVM_FE(openssl_x509_read);
    HHVM_FE(openssl_random_pseudo_bytes);
    HHVM_FE(openssl_cipher_iv_length);
    HHVM_FE(openssl_encrypt);
    HHVM_FE(openssl_encrypt_with_tag);
    HHVM_FE(openssl_decrypt);
    HHVM_FE(openssl_digest);
    HHVM_FE(openssl_get_cipher_methods);
    HHVM_FE(openssl_get_curve_names);
    HHVM_FE(openssl_get_md_methods);
  }
} s_openssl_extension;

///////////////////////////////////////////////////////////////////////////////
}
