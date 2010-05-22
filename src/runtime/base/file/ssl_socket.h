/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_SSL_SOCKET_H__
#define __HPHP_SSL_SOCKET_H__

#include <runtime/base/file/socket.h>
#include <util/lock.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * TCP sockets running SSL protocol.
 */
class SSLSocket : public Socket {
public:
  enum CryptoMethod {
    ClientSSLv2,
    ClientSSLv3,
    ClientSSLv23,
    ClientTLS,

    ServerSSLv2,
    ServerSSLv3,
    ServerSSLv23,
    ServerTLS,
  };

  static int GetSSLExDataIndex();
  static SSLSocket *Create(const char *&name, int port, double timeout);

public:
  SSLSocket();
  SSLSocket(int sockfd, int type, const char *address = NULL, int port = 0);
  virtual ~SSLSocket();

  // will setup and enable crypto
  bool onConnect();
  bool onAccept();

  // overriding ResourceData
  const char *o_getClassName() const { return "SSLSocket";}

  // overriding Socket
  virtual bool close();
  virtual int readImpl(char *buffer, int length);
  virtual int writeImpl(const char *buffer, int length);
  virtual bool checkLiveness();

  Array &getContext() { return m_context;}

private:
  Array m_context;

  SSL *m_handle;
  bool m_ssl_active;
  CryptoMethod m_method;
  bool m_client;

  double m_connect_timeout;
  bool m_enable_on_connect;
  bool m_state_set;
  bool m_is_blocked;

  bool closeImpl();
  bool handleError(int nr_bytes, bool is_init);

  bool setupCrypto(SSLSocket *session = NULL);
  bool enableCrypto(bool activate = true);

  SSL *createSSL(SSL_CTX *ctx);
  bool applyVerificationPolicy(X509 *peer);

  static Mutex s_mutex;
  static int s_ex_data_index;
};

///////////////////////////////////////////////////////////////////////////////
// helper class

class Certificate : public SweepableResourceData {
public:
  X509 *m_cert;
  Certificate(X509 *cert) : m_cert(cert) { ASSERT(m_cert);}
  ~Certificate() { if (m_cert) X509_free(m_cert);}

  // overriding ResourceData
  const char *o_getClassName() const { return "OpenSSL X.509";}

  /**
   * Given a variant, coerce it into an X509 object. It can be:
   *
   *  . X509 resource created using openssl_read_x509()
   *  . if it starts with file:// then it will be interpreted as the path
   *    to that cert
   *  . it will be interpreted as the cert data
   */
  static Object Get(CVarRef var);
  static BIO *ReadData(CVarRef var, bool *file = NULL);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SSL_SOCKET_H__
