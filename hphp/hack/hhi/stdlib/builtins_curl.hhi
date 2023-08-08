<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
namespace {
  const int CURLAUTH_ANY;
  const int CURLAUTH_ANYSAFE;
  const int CURLAUTH_BASIC;
  const int CURLAUTH_BEARER;
  const int CURLAUTH_DIGEST;
  const int CURLAUTH_DIGEST_IE;
  const int CURLAUTH_GSSNEGOTIATE;
  const int CURLAUTH_NEGOTIATE;
  const int CURLAUTH_NONE;
  const int CURLAUTH_NTLM;
  const int CURLAUTH_ONLY;

  const int CURLCLOSEPOLICY_CALLBACK;
  const int CURLCLOSEPOLICY_LEAST_RECENTLY_USED;
  const int CURLCLOSEPOLICY_LEAST_TRAFFIC;
  const int CURLCLOSEPOLICY_OLDEST;
  const int CURLCLOSEPOLICY_SLOWEST;

  const int CURLINFO_APPCONNECT_TIME;
  const int CURLINFO_APPCONNECT_TIME_T;
  const int CURLINFO_CAINFO;
  const int CURLINFO_CAPATH;
  const int CURLINFO_CERTINFO;
  const int CURLINFO_CONDITION_UNMET;
  const int CURLINFO_CONNECT_TIME;
  const int CURLINFO_CONNECT_TIME_T;
  const int CURLINFO_CONTENT_LENGTH_DOWNLOAD;
  const int CURLINFO_CONTENT_LENGTH_DOWNLOAD_T;
  const int CURLINFO_CONTENT_LENGTH_UPLOAD;
  const int CURLINFO_CONTENT_LENGTH_UPLOAD_T;
  const int CURLINFO_CONTENT_TYPE;
  const int CURLINFO_COOKIELIST;
  const int CURLINFO_EFFECTIVE_METHOD;
  const int CURLINFO_EFFECTIVE_URL;
  const int CURLINFO_FILETIME;
  const int CURLINFO_FILETIME_T;
  const int CURLINFO_FTP_ENTRY_PATH;
  const int CURLINFO_HEADER_OUT;
  const int CURLINFO_HEADER_SIZE;
  const int CURLINFO_HTTPAUTH_AVAIL;
  const int CURLINFO_HTTP_CONNECTCODE;
  const int CURLINFO_HTTP_CODE;
  const int CURLINFO_HTTP_VERSION;
  const int CURLINFO_LASTONE;
  const int CURLINFO_LOCAL_IP;
  const int CURLINFO_LOCAL_PORT;
  const int CURLINFO_NAMELOOKUP_TIME;
  const int CURLINFO_NAMELOOKUP_TIME_T;
  const int CURLINFO_NUM_CONNECTS;
  const int CURLINFO_OS_ERRNO;
  const int CURLINFO_PRETRANSFER_TIME;
  const int CURLINFO_PRETRANSFER_TIME_T;
  const int CURLINFO_PRIMARY_IP;
  const int CURLINFO_PRIMARY_PORT;
  const int CURLINFO_PRIVATE;
  const int CURLINFO_PROTOCOL;
  const int CURLINFO_PROXYAUTH_AVAIL;
  const int CURLINFO_PROXY_SSL_VERIFYRESULT;
  const int CURLINFO_REDIRECT_COUNT;
  const int CURLINFO_REDIRECT_TIME;
  const int CURLINFO_REDIRECT_TIME_T;
  const int CURLINFO_REDIRECT_URL;
  const int CURLINFO_REFERER;
  const int CURLINFO_REQUEST_SIZE;
  const int CURLINFO_RESPONSE_CODE;
  const int CURLINFO_RTSP_CLIENT_CSEQ;
  const int CURLINFO_RTSP_CSEQ_RECV;
  const int CURLINFO_RTSP_SERVER_CSEQ;
  const int CURLINFO_RTSP_SESSION_ID;
  const int CURLINFO_SCHEME;
  const int CURLINFO_SIZE_DOWNLOAD;
  const int CURLINFO_SIZE_UPLOAD;
  const int CURLINFO_SPEED_DOWNLOAD;
  const int CURLINFO_SIZE_DOWNLOAD_T;
  const int CURLINFO_SPEED_UPLOAD;
  const int CURLINFO_SIZE_UPLOAD_T;
  const int CURLINFO_SPEED_DOWNLOAD_T;
  const int CURLINFO_SPEED_UPLOAD_T;
  const int CURLINFO_SSL_ENGINES;
  const int CURLINFO_SSL_VERIFYRESULT;
  const int CURLINFO_STARTTRANSFER_TIME;
  const int CURLINFO_STARTTRANSFER_TIME_T;
  const int CURLINFO_TOTAL_TIME;
  const int CURLINFO_TOTAL_TIME_T;

  const int CURLMSG_DONE;

  const int CURLM_BAD_EASY_HANDLE;
  const int CURLM_BAD_HANDLE;
  const int CURLM_CALL_MULTI_PERFORM;
  const int CURLM_INTERNAL_ERROR;
  const int CURLM_OK;
  const int CURLM_OUT_OF_MEMORY;

  const int CURLOPT_AUTOREFERER;
  const int CURLOPT_AWS_SIGV4;
  const int CURLOPT_BINARYTRANSFER;
  const int CURLOPT_COOKIESESSION;
  const int CURLOPT_CRLF;
  const int CURLOPT_DNS_USE_GLOBAL_CACHE;
  const int CURLOPT_DOH_URL;
  const int CURLOPT_FAILONERROR;
  const int CURLOPT_FILETIME;
  const int CURLOPT_FOLLOWLOCATION;
  const int CURLOPT_FORBID_REUSE;
  const int CURLOPT_FRESH_CONNECT;
  const int CURLOPT_HEADER;
  const int CURLOPT_HTTPGET;
  const int CURLOPT_MUTE;
  const int CURLOPT_NOBODY;
  const int CURLOPT_NOPROGRESS;
  const int CURLOPT_NOSIGNAL;
  const int CURLOPT_POST;
  const int CURLOPT_PUT;
  const int CURLOPT_RETURNTRANSFER;
  const int CURLOPT_UPLOAD;
  const int CURLOPT_VERBOSE;
  const int CURLOPT_BUFFERSIZE;
  const int CURLOPT_CLOSEPOLICY;
  const int CURLOPT_HTTP_VERSION;
  const int CURLOPT_HTTPAUTH;
  const int CURLOPT_INFILESIZE;
  const int CURLOPT_MAXCONNECTS;
  const int CURLOPT_MAXREDIRS;
  const int CURLOPT_PORT;
  const int CURLOPT_RESUME_FROM;
  const int CURLOPT_TIMECONDITION;
  const int CURLOPT_TIMEVALUE;
  const int CURLOPT_TIMEVALUE_LARGE;
  const int CURLOPT_COOKIE;
  const int CURLOPT_COOKIEFILE;
  const int CURLOPT_COOKIEJAR;
  const int CURLOPT_CUSTOMREQUEST;
  const int CURLOPT_EGDSOCKET;
  const int CURLOPT_ENCODING;
  const int CURLOPT_INTERFACE;
  const int CURLOPT_IPRESOLVE;
  const int CURLOPT_POSTFIELDS;
  const int CURLOPT_RANGE;
  const int CURLOPT_REFERER;
  const int CURLOPT_URL;
  const int CURLOPT_USERAGENT;
  const int CURLOPT_USERPWD;
  const int CURLOPT_HTTPHEADER;
  const int CURLOPT_FILE;
  const int CURLOPT_INFILE;
  const int CURLOPT_STDERR;
  const int CURLOPT_WRITEHEADER;
  const int CURLOPT_HEADERFUNCTION;
  const int CURLOPT_PASSWDFUNCTION;
  const int CURLOPT_READFUNCTION;
  const int CURLOPT_WRITEFUNCTION;
  const int CURLOPT_HTTPPROXYTUNNEL;
  const int CURLOPT_PROXYAUTH;
  const int CURLOPT_PROXYPORT;
  const int CURLOPT_PROXYTYPE;
  const int CURLOPT_PROXY;
  const int CURLOPT_PROXYUSERPWD;
  const int CURLOPT_CONNECTTIMEOUT;
  const int CURLOPT_CONNECTTIMEOUT_MS;
  const int CURLOPT_DNS_CACHE_TIMEOUT;
  const int CURLOPT_LOW_SPEED_LIMIT;
  const int CURLOPT_LOW_SPEED_TIME;
  const int CURLOPT_TIMEOUT;
  const int CURLOPT_TIMEOUT_MS;
  const int CURLOPT_SSL_VERIFYPEER;
  const int CURLOPT_SSL_VERIFYHOST;
  const int CURLOPT_SSLVERSION;
  const int CURLOPT_CAINFO;
  const int CURLOPT_CAINFO_BLOB;
  const int CURLOPT_CAPATH;
  const int CURLOPT_RANDOM_FILE;
  const int CURLOPT_SAFE_UPLOAD;
  const int CURLOPT_SSL_CIPHER_LIST;
  const int CURLOPT_SSLCERT;
  const int CURLOPT_SSLCERTPASSWD;
  const int CURLOPT_SSLCERTTYPE;
  const int CURLOPT_SSLCERT_BLOB;
  const int CURLOPT_SSLENGINE;
  const int CURLOPT_SSLENGINE_DEFAULT;
  const int CURLOPT_SSLKEY;
  const int CURLOPT_SSLKEYPASSWD;
  const int CURLOPT_SSLKEYTYPE;
  const int CURLOPT_SSLKEY_BLOB;
  const int CURLOPT_FTP_USE_EPRT;
  const int CURLOPT_FTP_USE_EPSV;
  const int CURLOPT_FTPAPPEND;
  const int CURLOPT_FTPLISTONLY;
  const int CURLOPT_NETRC;
  const int CURLOPT_TRANSFERTEXT;
  const int CURLOPT_UNRESTRICTED_AUTH;
  const int CURLOPT_FTPSSLAUTH;
  const int CURLOPT_FTPPORT;
  const int CURLOPT_POSTQUOTE;
  const int CURLOPT_RESOLVE;
  const int CURLOPT_QUOTE;
  const int CURLOPT_ABSTRACT_UNIX_SOCKET;
  const int CURLOPT_ACCEPT_ENCODING;
  const int CURLOPT_ACCEPTTIMEOUT_MS;
  const int CURLOPT_ADDRESS_SCOPE;
  const int CURLOPT_ALTSVC;
  const int CURLOPT_ALTSVC_CTRL;
  const int CURLOPT_APPEND;
  const int CURLOPT_CERTINFO;
  const int CURLOPT_CONNECT_ONLY;
  const int CURLOPT_CONNECT_TO;
  const int CURLOPT_COOKIELIST;
  const int CURLOPT_CRLFILE;
  const int CURLOPT_DEFAULT_PROTOCOL;
  const int CURLOPT_DIRLISTONLY;
  const int CURLOPT_DISALLOW_USERNAME_IN_URL;
  const int CURLOPT_DNS_INTERFACE;
  const int CURLOPT_DNS_LOCAL_IP4;
  const int CURLOPT_DNS_LOCAL_IP6;
  const int CURLOPT_DNS_SERVERS;
  const int CURLOPT_DNS_SHUFFLE_ADDRESSES;
  const int CURLOPT_DOH_SSL_VERIFYHOST;
  const int CURLOPT_DOH_SSL_VERIFYPEER;
  const int CURLOPT_DOH_SSL_VERIFYSTATUS;
  const int CURLOPT_EXPECT_100_TIMEOUT_MS;
  const int CURLOPT_FNMATCH_FUNCTION;
  const int CURLOPT_FTP_ACCOUNT;
  const int CURLOPT_FTP_ALTERNATIVE_TO_USER;
  const int CURLOPT_FTP_CREATE_MISSING_DIRS;
  const int CURLOPT_FTP_FILEMETHOD;
  const int CURLOPT_FTP_RESPONSE_TIMEOUT;
  const int CURLOPT_FTP_SKIP_PASV_IP;
  const int CURLOPT_FTP_SSL;
  const int CURLOPT_FTP_SSL_CCC;
  const int CURLOPT_FTP_USE_PRET;
  const int CURLOPT_GSSAPI_DELEGATION;
  const int CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS;
  const int CURLOPT_HAPROXYPROTOCOL;
  const int CURLOPT_HSTS;
  const int CURLOPT_HSTS_CTRL;
  const int CURLOPT_HTTP09_ALLOWED;
  const int CURLOPT_HTTP200ALIASES;
  const int CURLOPT_HTTP_CONTENT_DECODING;
  const int CURLOPT_HTTP_TRANSFER_DECODING;
  const int CURLOPT_IGNORE_CONTENT_LENGTH;
  const int CURLOPT_ISSUERCERT;
  const int CURLOPT_ISSUERCERT_BLOB;
  const int CURLOPT_KEEP_SENDING_ON_ERROR;
  const int CURLOPT_KEYPASSWD;
  const int CURLOPT_KRB4LEVEL;
  const int CURLOPT_KRBLEVEL;
  const int CURLOPT_LOCALPORT;
  const int CURLOPT_LOCALPORTRANGE;
  const int CURLOPT_LOGIN_OPTIONS;
  const int CURLOPT_MAIL_AUTH;
  const int CURLOPT_MAIL_FROM;
  const int CURLOPT_MAIL_RCPT;
  const int CURLOPT_MAXAGE_CONN;
  const int CURLOPT_MAXFILESIZE;
  const int CURLOPT_MAXLIFETIME_CONN;
  const int CURLOPT_MAX_RECV_SPEED_LARGE;
  const int CURLOPT_MAX_SEND_SPEED_LARGE;
  const int CURLOPT_MIME_OPTIONS;
  const int CURLOPT_NETRC_FILE;
  const int CURLOPT_NEW_DIRECTORY_PERMS;
  const int CURLOPT_NEW_FILE_PERMS;
  const int CURLOPT_NOPROXY;
  const int CURLOPT_PASSWORD;
  const int CURLOPT_PATH_AS_IS;
  const int CURLOPT_PINNEDPUBLICKEY;
  const int CURLOPT_PIPEWAIT;
  const int CURLOPT_POSTREDIR;
  const int CURLOPT_PREQUOTE;
  const int CURLOPT_PRIVATE;
  const int CURLOPT_PROGRESSFUNCTION;
  const int CURLOPT_PROTOCOLS;
  const int CURLOPT_PROXYPASSWORD;
  const int CURLOPT_PROXY_CAINFO;
  const int CURLOPT_PROXY_CAINFO_BLOB;
  const int CURLOPT_PROXY_CAPATH;
  const int CURLOPT_PROXY_CRLFILE;
  const int CURLOPT_PROXY_ISSUERCERT;
  const int CURLOPT_PROXY_ISSUERCERT_BLOB;
  const int CURLOPT_PROXY_KEYPASSWD;
  const int CURLOPT_PROXY_PINNEDPUBLICKEY;
  const int CURLOPT_PROXY_SERVICE_NAME;
  const int CURLOPT_PROXY_SSLCERT;
  const int CURLOPT_PROXY_SSLCERTTYPE;
  const int CURLOPT_PROXY_SSLCERT_BLOB;
  const int CURLOPT_PROXY_SSLKEY;
  const int CURLOPT_PROXY_SSLKEYTYPE;
  const int CURLOPT_PROXY_SSLKEY_BLOB;
  const int CURLOPT_PROXY_SSLVERSION;
  const int CURLOPT_PROXY_SSL_CIPHER_LIST;
  const int CURLOPT_PROXY_SSL_OPTIONS;
  const int CURLOPT_PROXY_SSL_VERIFYHOST;
  const int CURLOPT_PROXY_SSL_VERIFYPEER;
  const int CURLOPT_PROXY_TLS13_CIPHERS;
  const int CURLOPT_PROXY_TLSAUTH_PASSWORD;
  const int CURLOPT_PROXY_TLSAUTH_TYPE;
  const int CURLOPT_PROXY_TLSAUTH_USERNAME;
  const int CURLOPT_PROXY_TRANSFER_MODE;
  const int CURLOPT_PROXYUSERNAME;
  const int CURLOPT_READDATA;
  const int CURLOPT_REDIR_PROTOCOLS;
  const int CURLOPT_REQUEST_TARGET;
  const int CURLOPT_RTSP_CLIENT_CSEQ;
  const int CURLOPT_RTSP_REQUEST;
  const int CURLOPT_RTSP_SERVER_CSEQ;
  const int CURLOPT_RTSP_SESSION_ID;
  const int CURLOPT_RTSP_STREAM_URI;
  const int CURLOPT_RTSP_TRANSPORT;
  const int CURLOPT_SASL_AUTHZID;
  const int CURLOPT_SASL_IR;
  const int CURLOPT_SERVICE_NAME;
  const int CURLOPT_SHARE;
  const int CURLOPT_SOCKS5_AUTH;
  const int CURLOPT_SOCKS5_GSSAPI_NEC;
  const int CURLOPT_SOCKS5_GSSAPI_SERVICE;
  const int CURLOPT_SSH_AUTH_TYPES;
  const int CURLOPT_SSH_COMPRESSION;
  const int CURLOPT_SSH_HOST_PUBLIC_KEY_MD5;
  const int CURLOPT_SSH_HOST_PUBLIC_KEY_SHA256;
  const int CURLOPT_SSH_KNOWNHOSTS;
  const int CURLOPT_SSH_PRIVATE_KEYFILE;
  const int CURLOPT_SSH_PUBLIC_KEYFILE;
  const int CURLOPT_SSL_EC_CURVES;
  const int CURLOPT_SSL_ENABLE_ALPN;
  const int CURLOPT_SSL_ENABLE_NPN;
  const int CURLOPT_SSL_FALSESTART;
  const int CURLOPT_SSL_OPTIONS;
  const int CURLOPT_SSL_SESSIONID_CACHE;
  const int CURLOPT_SSL_VERIFYSTATUS;
  const int CURLOPT_SUPPRESS_CONNECT_HEADERS;
  const int CURLOPT_STREAM_WEIGHT;
  const int CURLOPT_TCP_FASTOPEN;
  const int CURLOPT_TCP_KEEPALIVE;
  const int CURLOPT_TCP_KEEPIDLE;
  const int CURLOPT_TCP_KEEPINTVL;
  const int CURLOPT_TCP_NODELAY;
  const int CURLOPT_TELNETOPTIONS;
  const int CURLOPT_TFTP_BLKSIZE;
  const int CURLOPT_TFTP_NO_OPTIONS;
  const int CURLOPT_TLS13_CIPHERS;
  const int CURLOPT_TLSAUTH_PASSWORD;
  const int CURLOPT_TLSAUTH_TYPE;
  const int CURLOPT_TLSAUTH_USERNAME;
  const int CURLOPT_TRANSFER_ENCODING;
  const int CURLOPT_UPKEEP_INTERVAL_MS;
  const int CURLOPT_UPLOAD_BUFFERSIZE;
  const int CURLOPT_UNIX_SOCKET_PATH;
  const int CURLOPT_USERNAME;
  const int CURLOPT_USE_SSL;
  const int CURLOPT_WILDCARDMATCH;
  const int CURLOPT_XOAUTH2_BEARER;

  const int CURLE_ABORTED_BY_CALLBACK;
  const int CURLE_AUTH_ERROR;
  const int CURLE_BAD_CALLING_ORDER;
  const int CURLE_BAD_CONTENT_ENCODING;
  const int CURLE_BAD_DOWNLOAD_RESUME;
  const int CURLE_BAD_FUNCTION_ARGUMENT;
  const int CURLE_BAD_PASSWORD_ENTERED;
  const int CURLE_COULDNT_CONNECT;
  const int CURLE_COULDNT_RESOLVE_HOST;
  const int CURLE_COULDNT_RESOLVE_PROXY;
  const int CURLE_FAILED_INIT;
  const int CURLE_FILESIZE_EXCEEDED;
  const int CURLE_FILE_COULDNT_READ_FILE;
  const int CURLE_FTP_ACCESS_DENIED;
  const int CURLE_FTP_BAD_DOWNLOAD_RESUME;
  const int CURLE_FTP_CANT_GET_HOST;
  const int CURLE_FTP_CANT_RECONNECT;
  const int CURLE_FTP_COULDNT_GET_SIZE;
  const int CURLE_FTP_COULDNT_RETR_FILE;
  const int CURLE_FTP_COULDNT_SET_ASCII;
  const int CURLE_FTP_COULDNT_SET_BINARY;
  const int CURLE_FTP_COULDNT_STOR_FILE;
  const int CURLE_FTP_COULDNT_USE_REST;
  const int CURLE_FTP_PARTIAL_FILE;
  const int CURLE_FTP_PORT_FAILED;
  const int CURLE_FTP_QUOTE_ERROR;
  const int CURLE_FTP_SSL_FAILED;
  const int CURLE_FTP_USER_PASSWORD_INCORRECT;
  const int CURLE_FTP_WEIRD_227_FORMAT;
  const int CURLE_FTP_WEIRD_PASS_REPLY;
  const int CURLE_FTP_WEIRD_PASV_REPLY;
  const int CURLE_FTP_WEIRD_SERVER_REPLY;
  const int CURLE_FTP_WEIRD_USER_REPLY;
  const int CURLE_FTP_WRITE_ERROR;
  const int CURLE_FUNCTION_NOT_FOUND;
  const int CURLE_GOT_NOTHING;
  const int CURLE_HTTP3;
  const int CURLE_HTTP_NOT_FOUND;
  const int CURLE_HTTP_PORT_FAILED;
  const int CURLE_HTTP_POST_ERROR;
  const int CURLE_HTTP_RANGE_ERROR;
  const int CURLE_HTTP_RETURNED_ERROR;
  const int CURLE_LDAP_CANNOT_BIND;
  const int CURLE_LDAP_INVALID_URL;
  const int CURLE_LDAP_SEARCH_FAILED;
  const int CURLE_LIBRARY_NOT_FOUND;
  const int CURLE_MALFORMAT_USER;
  const int CURLE_OBSOLETE;
  const int CURLE_OK;
  const int CURLE_OPERATION_TIMEOUTED;
  const int CURLE_OPERATION_TIMEDOUT;
  const int CURLE_OUT_OF_MEMORY;
  const int CURLE_PARTIAL_FILE;
  const int CURLE_QUIC_CONNECT_ERROR;
  const int CURLE_READ_ERROR;
  const int CURLE_RECV_ERROR;
  const int CURLE_SEND_ERROR;
  const int CURLE_SHARE_IN_USE;
  const int CURLE_SSH;
  const int CURLE_SSL_CACERT;
  const int CURLE_SSL_CERTPROBLEM;
  const int CURLE_SSL_CIPHER;
  const int CURLE_SSL_CONNECT_ERROR;
  const int CURLE_SSL_ENGINE_NOTFOUND;
  const int CURLE_SSL_ENGINE_SETFAILED;
  const int CURLE_SSL_PEER_CERTIFICATE;
  const int CURLE_TELNET_OPTION_SYNTAX;
  const int CURLE_TOO_MANY_REDIRECTS;
  const int CURLE_UNKNOWN_TELNET_OPTION;
  const int CURLE_UNSUPPORTED_PROTOCOL;
  const int CURLE_URL_MALFORMAT;
  const int CURLE_URL_MALFORMAT_USER;
  const int CURLE_WRITE_ERROR;

  const int CURLFTPAUTH_DEFAULT;
  const int CURLFTPAUTH_SSL;
  const int CURLFTPAUTH_TLS;

  const int CURLFTPMETHOD_MULTICWD;
  const int CURLFTPMETHOD_NOCWD;
  const int CURLFTPMETHOD_SINGLECWD;

  const int CURLFTPSSL_CCC_ACTIVE;
  const int CURLFTPSSL_CCC_NONE;
  const int CURLFTPSSL_CCC_PASSIVE;

  const int CURLFTPSSL_ALL;
  const int CURLFTPSSL_CONTROL;
  const int CURLFTPSSL_NONE;
  const int CURLFTPSSL_TRY;

  const int CURLGSSAPI_DELEGATION_FLAG;
  const int CURLGSSAPI_DELEGATION_POLICY_FLAG;

  const int CURLVERSION_NOW;
  const int CURL_VERSION_ALTSVC;
  const int CURL_VERSION_BROTLI;
  const int CURL_VERSION_HSTS;
  const int CURL_VERSION_HTTP2;
  const int CURL_VERSION_HTTP3;
  const int CURL_VERSION_IPV6;
  const int CURL_VERSION_KERBEROS4;
  const int CURL_VERSION_LIBZ;
  const int CURL_VERSION_MULTI_SSL;
  const int CURL_VERSION_SSL;
  const int CURL_VERSION_UNICODE;
  const int CURL_VERSION_ZSTD;

  const int CURLPROTO_ALL;
  const int CURLPROTO_DICT;
  const int CURLPROTO_FILE;
  const int CURLPROTO_FTP;
  const int CURLPROTO_FTPS;
  const int CURLPROTO_GOPHER;
  const int CURLPROTO_HTTP;
  const int CURLPROTO_HTTPS;
  const int CURLPROTO_IMAP;
  const int CURLPROTO_IMAPS;
  const int CURLPROTO_LDAP;
  const int CURLPROTO_LDAPS;
  const int CURLPROTO_POP3;
  const int CURLPROTO_POP3S;
  const int CURLPROTO_RTMP;
  const int CURLPROTO_RTMPE;
  const int CURLPROTO_RTMPS;
  const int CURLPROTO_RTMPT;
  const int CURLPROTO_RTMPTE;
  const int CURLPROTO_RTMPTS;
  const int CURLPROTO_RTSP;
  const int CURLPROTO_SCP;
  const int CURLPROTO_SFTP;
  const int CURLPROTO_SMB;
  const int CURLPROTO_SMBS;
  const int CURLPROTO_SMTP;
  const int CURLPROTO_SMTPS;
  const int CURLPROTO_TELNET;
  const int CURLPROTO_TFTP;

  const int CURLPROXY_HTTP;
  const int CURLPROXY_HTTPS;
  const int CURLPROXY_SOCKS4;
  const int CURLPROXY_SOCKS4A;
  const int CURLPROXY_SOCKS5;
  const int CURLPROXY_SOCKS5_HOSTNAME;

  const int CURLSSH_AUTH_AGENT;
  const int CURLSSH_AUTH_ANY;
  const int CURLSSH_AUTH_DEFAULT;
  const int CURLSSH_AUTH_GSSAPI;
  const int CURLSSH_AUTH_HOST;
  const int CURLSSH_AUTH_KEYBOARD;
  const int CURLSSH_AUTH_NONE;
  const int CURLSSH_AUTH_PASSWORD;
  const int CURLSSH_AUTH_PUBLICKEY;

  const int CURLSSLOPT_ALLOW_BEAST;
  const int CURLSSLOPT_AUTO_CLIENT_CERT;
  const int CURLSSLOPT_NO_PARTIALCHAIN;
  const int CURLSSLOPT_NO_REVOKE;
  const int CURLSSLOPT_REVOKE_BEST_EFFORT;

  const int CURLUSESSL_ALL;
  const int CURLUSESSL_CONTROL;
  const int CURLUSESSL_NONE;
  const int CURLUSESSL_TRY;

  const int CURL_HTTP_VERSION_1_0;
  const int CURL_HTTP_VERSION_1_1;
  const int CURL_HTTP_VERSION_NONE;
  const int CURL_HTTP_VERSION_2;
  const int CURL_HTTP_VERSION_2_0;
  const int CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE;
  const int CURL_HTTP_VERSION_2TLS;
  const int CURL_HTTP_VERSION_3;

  const int CURL_IPRESOLVE_V4;
  const int CURL_IPRESOLVE_V6;
  const int CURL_IPRESOLVE_WHATEVER;

  const int CURL_NETRC_IGNORED;
  const int CURL_NETRC_OPTIONAL;
  const int CURL_NETRC_REQUIRED;

  const int CURL_REDIR_POST_303;

  const int CURL_RTSPREQ_ANNOUNCE;
  const int CURL_RTSPREQ_DESCRIBE;
  const int CURL_RTSPREQ_GET_PARAMETER;
  const int CURL_RTSPREQ_OPTIONS;
  const int CURL_RTSPREQ_PAUSE;
  const int CURL_RTSPREQ_PLAY;
  const int CURL_RTSPREQ_RECEIVE;
  const int CURL_RTSPREQ_RECORD;
  const int CURL_RTSPREQ_SET_PARAMETER;
  const int CURL_RTSPREQ_SETUP;
  const int CURL_RTSPREQ_TEARDOWN;

  const int CURL_SSLVERSION_DEFAULT;
  const int CURL_SSLVERSION_SSLv2;
  const int CURL_SSLVERSION_SSLv3;
  const int CURL_SSLVERSION_TLSv1;
  const int CURL_SSLVERSION_TLSv1_0;
  const int CURL_SSLVERSION_TLSv1_1;
  const int CURL_SSLVERSION_TLSv1_2;
  const int CURL_SSLVERSION_TLSv1_3;
  const int CURL_SSLVERSION_MAX_DEFAULT;
  const int CURL_SSLVERSION_MAX_TLSv1_0;
  const int CURL_SSLVERSION_MAX_TLSv1_1;
  const int CURL_SSLVERSION_MAX_TLSv1_2;
  const int CURL_SSLVERSION_MAX_TLSv1_3;

  const int CURL_TIMECOND_IFMODSINCE;
  const int CURL_TIMECOND_IFUNMODSINCE;
  const int CURL_TIMECOND_LASTMOD;
  const int CURL_TIMECOND_NONE;

  const int CURL_TLSAUTH_SRP;

  const int CURLOPT_HEADEROPT;
  const int CURLOPT_PROXYHEADER;
  const int CURLHEADER_UNIFIED;
  const int CURLHEADER_SEPARATE;

  const int CURL_LOCK_DATA_COOKIE;
  const int CURL_LOCK_DATA_DNS;
  const int CURL_LOCK_DATA_SSL_SESSION;
  const int CURLSHE_OK;
  const int CURLSHOPT_SHARE;
  const int CURLSHOPT_UNSHARE;

  const int CURLALTSVC_READONLYFILE;
  const int CURLALTSVC_H1;
  const int CURLALTSVC_H2;
  const int CURLALTSVC_H3;

  const int CURLHSTS_ENABLE;
  const int CURLHSTS_READONLYFILE;

  const int CURLMIMEOPT_FORMESCAPE;

  <<__PHPStdLib>>
  function curl_init(
    HH\FIXME\MISSING_PARAM_TYPE $url = null,
  )[leak_safe]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_init_pooled(
    string $pool_name,
    HH\FIXME\MISSING_PARAM_TYPE $url = null,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_version(
    int $uversion = CURLVERSION_NOW,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_setopt(
    resource $ch,
    int $option,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  )[write_props]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_setopt_array(
    resource $ch,
    HH\FIXME\MISSING_PARAM_TYPE $options,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_exec(resource $ch): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_getinfo(
    resource $ch,
    int $opt = 0,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_errno(resource $ch): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_error(resource $ch)[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_strerror(int $code): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_close(resource $ch)[leak_safe]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_init(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_strerror(int $errornum): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_add_handle(
    resource $mh,
    resource $ch,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_remove_handle(
    resource $mh,
    resource $ch,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_exec(
    resource $mh,
    inout int $still_running,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_select(
    resource $mh,
    float $timeout = 1.0,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_await(resource $mh, float $timeout = 1.0): Awaitable<int>;
  <<__PHPStdLib>>
  function curl_multi_getcontent(resource $ch): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_info_read(
    resource $mh,
    inout HH\FIXME\MISSING_PARAM_TYPE $msgs_in_queue,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_multi_close(resource $mh): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_share_init(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_share_close(resource $sh): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function curl_share_setopt(
    resource $sh,
    int $option,
    HH\FIXME\MISSING_PARAM_TYPE $value,
  ): \HH\FIXME\MISSING_RETURN_TYPE;

  class CURLFile {
    public string $name = '';
    public string $mime = '';
    public string $postname = '';

    public function __construct(
      string $name,
      string $mime = '',
      string $postname = '',
    ): void;

    public function getFilename(): string;
    public function getMimeType(): string;
    public function getPostFilename(): string;
    public function setMimeType(string $mime): void;
    public function setPostFilename(string $postname): void;
  }

  <<__PHPStdLib>>
  function curl_file_create(
    string $name,
    string $mime = '',
    string $postname = '',
  ): CURLFile;
}

namespace HH\Asio {
  function curl_exec(mixed $url_or_handle): Awaitable<string>;
}
