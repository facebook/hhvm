/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/ssl-socket.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-error.h"
#include "folly/String.h"
#include <poll.h>
#include <sys/time.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Mutex SSLSocket::s_mutex;
int SSLSocket::s_ex_data_index = -1;
int SSLSocket::GetSSLExDataIndex() {
  if (s_ex_data_index >= 0) {
    return s_ex_data_index;
  }
  Lock lock(s_mutex);
  if (s_ex_data_index < 0) {
    s_ex_data_index = SSL_get_ex_new_index(0, (void*)"PHP stream index",
                                           nullptr, nullptr, nullptr);
    assert(s_ex_data_index >= 0);
  }
  return s_ex_data_index;
}

const StaticString
  s_allow_self_signed("allow_self_signed"),
  s_verify_depth("verify_depth");

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
  int ret = preverify_ok;

  /* determine the status for the current cert */
  X509_STORE_CTX_get_current_cert(ctx);
  int err = X509_STORE_CTX_get_error(ctx);
  int depth = X509_STORE_CTX_get_error_depth(ctx);

  /* conjure the stream & context to use */
  SSL *ssl = (SSL*)X509_STORE_CTX_get_ex_data
    (ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
  SSLSocket *stream =
    (SSLSocket*)SSL_get_ex_data(ssl, SSLSocket::GetSSLExDataIndex());

  /* if allow_self_signed is set, make sure that verification succeeds */
  if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT &&
      stream->getContext()[s_allow_self_signed].toBoolean()) {
    ret = 1;
  }

  /* check the depth */
  Variant vdepth = stream->getContext()[s_verify_depth];
  if (vdepth.toBoolean() && depth > vdepth.toInt64()) {
    ret = 0;
    X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
  }

  return ret;
}

const StaticString s_passphrase("passphrase");

static int passwd_callback(char *buf, int num, int verify, void *data) {
  /* TODO: could expand this to make a callback into PHP user-space */
  SSLSocket *stream = (SSLSocket *)data;
  String passphrase = stream->getContext()[s_passphrase].toString();
  if (!passphrase.empty() && passphrase.size() < num - 1) {
    memcpy(buf, passphrase.data(), passphrase.size() + 1);
    return passphrase.size();
  }
  return 0;
}

const StaticString
  s_verify_peer("verify_peer"),
  s_cafile("cafile"),
  s_capath("capath"),
  s_ciphers("ciphers"),
  s_local_cert("local_cert");

SSL *SSLSocket::createSSL(SSL_CTX *ctx) {
  ERR_clear_error();

  /* look at options in the stream and set appropriate verification flags */
  if (m_context[s_verify_peer].toBoolean()) {
    /* turn on verification callback */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);

    /* CA stuff */
    String cafile = m_context[s_cafile].toString();
    String capath = m_context[s_capath].toString();

    if (!cafile.empty() || !capath.empty()) {
      if (!SSL_CTX_load_verify_locations(ctx, cafile.data(), capath.data())) {
        raise_warning("Unable to set verify locations `%s' `%s'",
                      cafile.data(), capath.data());
        return nullptr;
      }
    }

    int64_t depth = m_context[s_verify_depth].toInt64();
    if (depth) {
      SSL_CTX_set_verify_depth(ctx, depth);
    }
  } else {
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
  }

  /* callback for the passphrase (for localcert) */
  if (!m_context[s_passphrase].toString().empty()) {
    SSL_CTX_set_default_passwd_cb_userdata(ctx, this);
    SSL_CTX_set_default_passwd_cb(ctx, passwd_callback);
  }

  String cipherlist = m_context[s_ciphers].toString();
  if (cipherlist.empty()) {
    cipherlist = "DEFAULT";
  }
  SSL_CTX_set_cipher_list(ctx, cipherlist.data());

  String certfile = m_context[s_local_cert].toString();
  if (!certfile.empty()) {
    String resolved_path_buff = File::TranslatePath(certfile);
    if (!resolved_path_buff.empty()) {
      /* a certificate to use for authentication */
      if (SSL_CTX_use_certificate_chain_file(ctx, resolved_path_buff.data())
          != 1) {
        raise_warning("Unable to set local cert chain file `%s'; Check "
                      "that your cafile/capath settings include details of "
                      "your certificate and its issuer", certfile.data());
        return nullptr;
      }

      if (SSL_CTX_use_PrivateKey_file(ctx, resolved_path_buff.data(),
                                      SSL_FILETYPE_PEM) != 1) {
        raise_warning("Unable to set private key file `%s'",
                      resolved_path_buff.data());
        return nullptr;
      }

      SSL *tmpssl = SSL_new(ctx);
      X509 *cert = SSL_get_certificate(tmpssl);
      if (cert) {
        EVP_PKEY *key = X509_get_pubkey(cert);
        EVP_PKEY_copy_parameters(key, SSL_get_privatekey(tmpssl));
        EVP_PKEY_free(key);
      }
      SSL_free(tmpssl);

      if (!SSL_CTX_check_private_key(ctx)) {
        raise_warning("Private key does not match certificate!");
      }
    }
  }

  SSL *ssl = SSL_new(ctx);
  if (ssl) {
    SSL_set_ex_data(ssl, GetSSLExDataIndex(), this); /* map SSL => stream */
  }
  return ssl;
}

///////////////////////////////////////////////////////////////////////////////
// constructors and destructor

SSLSocket::SSLSocket()
    : m_handle(nullptr), m_ssl_active(false), m_method((CryptoMethod)-1),
      m_client(false), m_connect_timeout(0), m_enable_on_connect(false),
      m_state_set(false), m_is_blocked(true) {
}

SSLSocket::SSLSocket(int sockfd, int type, const char *address /* = NULL */,
                     int port /* = 0 */)
    : Socket(sockfd, type, address, port),
      m_handle(nullptr), m_ssl_active(false), m_method((CryptoMethod)-1),
      m_client(false), m_connect_timeout(0), m_enable_on_connect(false),
      m_state_set(false), m_is_blocked(true) {
}

SSLSocket::~SSLSocket() {
  SSLSocket::closeImpl();
}

void SSLSocket::sweep() {
  SSLSocket::closeImpl();
  File::sweep();
  SSLSocket::operator delete(this);
}

bool SSLSocket::onConnect() {
  return setupCrypto() && enableCrypto();
}

bool SSLSocket::onAccept() {
  if (m_fd >= 0 && m_enable_on_connect) {
    switch (m_method) {
    case CryptoMethod::ClientSSLv23:
      m_method = CryptoMethod::ServerSSLv23;
      break;
    case CryptoMethod::ClientSSLv2:
      m_method = CryptoMethod::ServerSSLv2;
      break;
    case CryptoMethod::ClientSSLv3:
      m_method = CryptoMethod::ServerSSLv3;
      break;
    case CryptoMethod::ClientTLS:
      m_method = CryptoMethod::ServerTLS;
      break;
    default:
      assert(false);
    }

    if (setupCrypto() && enableCrypto()) {
      return true;
    }

    raise_warning("Failed to enable crypto");
    close();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool SSLSocket::handleError(int64_t nr_bytes, bool is_init) {
  char esbuf[512];
  std::string ebuf;
  unsigned long ecode;

  bool retry = true;
  int err = SSL_get_error(m_handle, nr_bytes);
  switch (err) {
  case SSL_ERROR_ZERO_RETURN:
    /* SSL terminated (but socket may still be active) */
    retry = false;
    break;
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    /* re-negotiation, or perhaps the SSL layer needs more
     * packets: retry in next iteration */
    errno = EAGAIN;
    retry = (is_init || m_is_blocked);
    break;
  case SSL_ERROR_SYSCALL:
    if (ERR_peek_error() == 0) {
      if (nr_bytes == 0) {
        if (ERR_get_error()) {
          raise_warning("SSL: fatal protocol error");
        }
        SSL_set_shutdown(m_handle, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
        m_eof = true;
        retry = false;
      } else {
        raise_warning("SSL: %s", folly::errnoStr(errno).c_str());
        retry = false;
      }
      break;
    }
    /* fall through */
  default:
    /* some other error */
    ecode = ERR_get_error();
    switch (ERR_GET_REASON(ecode)) {
    case SSL_R_NO_SHARED_CIPHER:
      raise_warning("SSL_R_NO_SHARED_CIPHER: no suitable shared cipher "
                    "could be used.  This could be because the server is "
                    "missing an SSL certificate (local_cert context "
                    "option)");
      retry = false;
      break;

    default:
      do {
        // NULL is automatically added
        ERR_error_string_n(ecode, esbuf, sizeof(esbuf));
        if (!ebuf.empty()) {
          ebuf += '\n';
        }
        ebuf += esbuf;
      } while ((ecode = ERR_get_error()) != 0);

      raise_warning("SSL operation failed with code %d. %s%s",
                    err, !ebuf.empty() ? "OpenSSL Error messages:\n" : "",
                    !ebuf.empty() ? ebuf.c_str() : "");
    }

    retry = false;
    errno = 0;
  }

  return retry;
}

///////////////////////////////////////////////////////////////////////////////

SSLSocket *SSLSocket::Create(const HostURL &hosturl, double timeout) {
  CryptoMethod method;
  const std::string scheme = hosturl.getScheme();

  if (scheme == "ssl") {
    method = CryptoMethod::ClientSSLv23;
  } else if (scheme == "sslv2") {
    method = CryptoMethod::ClientSSLv2;
  } else if (scheme == "sslv3") {
    method = CryptoMethod::ClientSSLv3;
  } else if (scheme == "tls") {
    method = CryptoMethod::ClientTLS;
  } else {
    return nullptr;
  }

  int domain = hosturl.isIPv6() ? AF_INET6 : AF_INET;
  int type = SOCK_STREAM;
  SSLSocket *sock = new SSLSocket(socket(domain, type, 0), domain,
                                  hosturl.getHost().c_str(),
                                  hosturl.getPort());
  sock->m_method = method;
  sock->m_connect_timeout = timeout;
  sock->m_enable_on_connect = true;

  return sock;
}

bool SSLSocket::close() {
  invokeFiltersOnClose();
  return closeImpl();
}

bool SSLSocket::closeImpl() {
  if (m_ssl_active) {
    SSL_shutdown(m_handle);
    m_ssl_active = false;
  }
  if (m_handle) {
    SSL_free(m_handle);
    m_handle = nullptr;
  }
  return Socket::closeImpl();
}

int64_t SSLSocket::readImpl(char *buffer, int64_t length) {
  int64_t nr_bytes = 0;
  if (m_ssl_active) {
    bool retry = true;
    do {
      if (m_is_blocked) {
        Socket::waitForData();
        if (m_timedOut) {
          break;
        }
        // could get here and we only have parts of an SSL packet
      }
      nr_bytes = SSL_read(m_handle, buffer, length);
      if (nr_bytes > 0) break; /* we got the data */
      retry = handleError(nr_bytes, false);
      m_eof = (!retry && errno != EAGAIN && !SSL_pending(m_handle));
    } while (retry);
  } else {
    nr_bytes = Socket::readImpl(buffer, length);
  }
  return nr_bytes < 0 ? 0 : nr_bytes;
}

int64_t SSLSocket::writeImpl(const char *buffer, int64_t length) {
  int didwrite;
  if (m_ssl_active) {
    bool retry = true;
    do {
      didwrite = SSL_write(m_handle, buffer, length);
      if (didwrite > 0) break;
      retry = handleError(didwrite, false);
    } while (retry);
  } else {
    didwrite = Socket::writeImpl(buffer, length);
  }
  return didwrite < 0 ? 0 : didwrite;
}

bool SSLSocket::setupCrypto(SSLSocket *session /* = NULL */) {
  if (m_handle) {
    raise_warning("SSL/TLS already set-up for this stream");
    return false;
  }

  /* need to do slightly different things, based on client/server method,
   * so lets remember which method was selected */
#if OPENSSL_VERSION_NUMBER < 0x00909000L
  SSL_METHOD *smethod;
#else
  const SSL_METHOD *smethod;
#endif
  switch (m_method) {
  case CryptoMethod::ClientSSLv23:
    m_client = true;
    smethod = SSLv23_client_method();
    break;
  case CryptoMethod::ClientSSLv3:
    m_client = true;
    smethod = SSLv3_client_method();
    break;
  case CryptoMethod::ClientTLS:
    m_client = true;
    smethod = TLSv1_client_method();
    break;
  case CryptoMethod::ServerSSLv23:
    m_client = false;
    smethod = SSLv23_server_method();
    break;
  case CryptoMethod::ServerSSLv3:
    m_client = false;
    smethod = SSLv3_server_method();
    break;

  /* SSLv2 protocol might be disabled in the OpenSSL library */
#ifndef OPENSSL_NO_SSL2
  case CryptoMethod::ClientSSLv2:
    m_client = true;
    smethod = SSLv2_client_method();
    break;
  case CryptoMethod::ServerSSLv2:
    m_client = false;
    smethod = SSLv2_server_method();
    break;
#else
  case CryptoMethod::ClientSSLv2:
  case CryptoMethod::ServerSSLv2:
    raise_warning("OpenSSL library does not support SSL2 protocol");
    return false;
  break;
#endif

  case CryptoMethod::ServerTLS:
    m_client = false;
    smethod = TLSv1_server_method();
    break;
  default:
    return false;
  }

  SSL_CTX *ctx = SSL_CTX_new(smethod);
  if (ctx == nullptr) {
    raise_warning("failed to create an SSL context");
    return false;
  }

  SSL_CTX_set_options(ctx, SSL_OP_ALL);
  m_handle = createSSL(ctx);
  if (m_handle == nullptr) {
    raise_warning("failed to create an SSL handle");
    SSL_CTX_free(ctx);
    return false;
  }

  if (!SSL_set_fd(m_handle, m_fd)) {
    handleError(0, true);
  }
  if (session) {
    SSL_copy_session_id(m_handle, session->m_handle);
  }
  return true;
}

const StaticString s_CN_match("CN_match");

bool SSLSocket::applyVerificationPolicy(X509 *peer) {
  /* verification is turned off */
  if (!m_context[s_verify_peer].toBoolean()) {
    return true;
  }

  if (peer == nullptr) {
    raise_warning("Could not get peer certificate");
    return false;
  }

  int err = SSL_get_verify_result(m_handle);
  switch (err) {
  case X509_V_OK:
    /* fine */
    break;
  case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    if (m_context[s_allow_self_signed].toBoolean()) {
      /* allowed */
      break;
    }
    /* not allowed, so fall through */
  default:
    raise_warning("Could not verify peer: code:%d %s", err,
                  X509_verify_cert_error_string(err));
    return false;
  }

  /* if the cert passed the usual checks, apply our own local policies now */

  /* Does the common name match ? (used primarily for https://) */
  String cnmatch = m_context[s_CN_match].toString();
  if (!cnmatch.empty()) {
    X509_NAME *name = X509_get_subject_name(peer);
    char buf[1024];
    int name_len = X509_NAME_get_text_by_NID(name, NID_commonName, buf,
                                             sizeof(buf));

    if (name_len < 0) {
      raise_warning("Unable to locate peer certificate CN");
      return false;
    } else if (name_len != (int)strlen(buf)) {
      raise_warning("Peer certificate CN=`%.*s' is malformed", name_len, buf);
      return false;
    }

    bool match = (strcmp(cnmatch.c_str(), buf) == 0);
    if (!match && strlen(buf) > 3 && buf[0] == '*' && buf[1] == '.') {
      /* Try wildcard */
      if (strchr(buf+2, '.')) {
        const char* cnmatch_str = cnmatch.c_str();
        const char *tmp = strstr(cnmatch_str, buf+1);
        match = tmp && strcmp(tmp, buf+2) && tmp == strchr(cnmatch_str, '.');
      }
    }

    if (!match) {
      /* didn't match */
      raise_warning("Peer certificate CN=`%.*s' did not match expected CN=`%s'",
                    name_len, buf, cnmatch.c_str());
      return false;
    }
  }

  return true;
}

const StaticString
  s_capture_peer_cert("capture_peer_cert"),
  s_peer_certificate("peer_certificate"),
  s_capture_peer_cert_chain("capture_peer_cert_chain"),
  s_peer_certificate_chain("peer_certificate_chain");

bool SSLSocket::enableCrypto(bool activate /* = true */) {
  if (activate && !m_ssl_active) {
    double timeout = m_connect_timeout;
    bool blocked = m_is_blocked;
    if (!m_state_set) {
      if (m_client) {
        SSL_set_connect_state(m_handle);
      } else {
        SSL_set_accept_state(m_handle);
      }
      m_state_set = true;
    }

    if (m_client && setBlocking(false)) {
      m_is_blocked = false;
    }

    int n;
    bool retry = true;
    do {
      if (m_client) {
        struct timeval tvs, tve;
        struct timezone tz;

        gettimeofday(&tvs, &tz);
        n = SSL_connect(m_handle);
        gettimeofday(&tve, &tz);

        timeout -= (tve.tv_sec + (double) tve.tv_usec / 1000000) -
          (tvs.tv_sec + (double) tvs.tv_usec / 1000000);
        if (timeout < 0) {
          raise_warning("SSL: connection timeout");
          return -1;
        }
      } else {
        n = SSL_accept(m_handle);
      }

      if (n <= 0) {
        retry = handleError(n, true);
      } else {
        break;
      }
    } while (retry);

    if (m_client && m_is_blocked != blocked && setBlocking(blocked)) {
      m_is_blocked = blocked;
    }

    if (n == 1) {
      X509 *peer_cert = SSL_get_peer_certificate(m_handle);
      if (!applyVerificationPolicy(peer_cert)) {
        SSL_shutdown(m_handle);
      } else {
        m_ssl_active = true;

        /* allow the script to capture the peer cert
         * and/or the certificate chain */
        if (m_context[s_capture_peer_cert].toBoolean()) {
          Resource cert(new Certificate(peer_cert));
          m_context.set(s_peer_certificate, cert);
          peer_cert = nullptr;
        }

        if (m_context[s_capture_peer_cert_chain].toBoolean()) {
          Array arr;
          STACK_OF(X509) *chain = SSL_get_peer_cert_chain(m_handle);
          if (chain) {
            for (int i = 0; i < sk_X509_num(chain); i++) {
              X509 *mycert = X509_dup(sk_X509_value(chain, i));
              arr.append(Resource(new Certificate(mycert)));
            }
          }
          m_context.set(s_peer_certificate_chain, arr);
        }
      }

      if (peer_cert) {
        X509_free(peer_cert);
      }
    } else  {
      n = errno == EAGAIN ? 0 : -1;
    }

    return n >= 0;

  } else if (!activate && m_ssl_active) {
    /* deactivate - common for server/client */
    SSL_shutdown(m_handle);
    m_ssl_active = false;
  }
  return true;
}

bool SSLSocket::checkLiveness() {
  if (m_fd == -1) {
    return false;
  }

  pollfd p;
  p.fd = m_fd;
  p.events = POLLIN | POLLERR | POLLHUP | POLLPRI;
  p.revents = 0;
  if (poll(&p, 1, 0) > 0 && p.revents > 0) {
    char buf;
    if (m_ssl_active) {
      while (true) {
        int n = SSL_peek(m_handle, &buf, sizeof(buf));
        if (n <= 0) {
          int err = SSL_get_error(m_handle, n);
          if (err == SSL_ERROR_SYSCALL) {
            return errno == EAGAIN;
          }

          if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            /* re-negotiate */
            continue;
          }

          /* any other problem is a fatal error */
          return false;
        }
        /* either peek succeeded or there was an error; we
         * have set the alive flag appropriately */
        break;
      }
    } else if (0 == recv(m_fd, &buf, sizeof(buf), MSG_PEEK) &&
               errno != EAGAIN) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Certificate

const StaticString s_file("file://");

BIO *Certificate::ReadData(const Variant& var, bool *file /* = NULL */) {
  if (var.isString() || var.isObject()) {
    String svar = var.toString();
    if (svar.substr(0, 7) == s_file) {
      if (file) *file = true;
      BIO *ret = BIO_new_file((char*)svar.substr(7).data(), "r");
      if (ret == nullptr) {
        raise_warning("error opening the file, %s", svar.data());
      }
      return ret;
    }

    if (file) *file = false;
    return BIO_new_mem_buf((char*)svar.data(), svar.size());
  }
  return nullptr;
}


Resource Certificate::Get(const Variant& var) {
  if (var.isResource()) {
    return var.toResource();
  }
  if (var.isString() || var.isObject()) {
    bool file;
    BIO *in = ReadData(var, &file);
    if (in == nullptr) return Resource();

    X509 *cert;
    /*
    if (file) {
      cert = PEM_read_bio_X509(in, NULL, NULL, NULL);
    } else {
      cert = (X509 *)PEM_ASN1_read_bio
        ((char *(*)())d2i_X509, PEM_STRING_X509, in, NULL, NULL, NULL);
    }
    */
    cert = PEM_read_bio_X509(in, nullptr, nullptr, nullptr);
    BIO_free(in);
    if (cert) {
      return Resource(new Certificate(cert));
    }
  }
  return Resource();
}

///////////////////////////////////////////////////////////////////////////////
}
