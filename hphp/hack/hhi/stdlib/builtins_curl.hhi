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
const int CURLAUTH_ANY = 0;
const int CURLAUTH_ANYSAFE = 0;
const int CURLAUTH_BASIC = 0;
const int CURLAUTH_BEARER = 0;
const int CURLAUTH_DIGEST = 0;
const int CURLAUTH_DIGEST_IE = 0;
const int CURLAUTH_GSSNEGOTIATE = 0;
const int CURLAUTH_NEGOTIATE = 0;
const int CURLAUTH_NONE = 0;
const int CURLAUTH_NTLM = 0;
const int CURLAUTH_ONLY = 0;

const int CURLCLOSEPOLICY_CALLBACK = 0;
const int CURLCLOSEPOLICY_LEAST_RECENTLY_USED = 0;
const int CURLCLOSEPOLICY_LEAST_TRAFFIC = 0;
const int CURLCLOSEPOLICY_OLDEST = 0;
const int CURLCLOSEPOLICY_SLOWEST = 0;

const int CURLINFO_APPCONNECT_TIME = 0;
const int CURLINFO_APPCONNECT_TIME_T = 0;
const int CURLINFO_CERTINFO = 0;
const int CURLINFO_CONDITION_UNMET = 0;
const int CURLINFO_CONNECT_TIME = 0;
const int CURLINFO_CONNECT_TIME_T = 0;
const int CURLINFO_CONTENT_LENGTH_DOWNLOAD = 0;
const int CURLINFO_CONTENT_LENGTH_DOWNLOAD_T = 0;
const int CURLINFO_CONTENT_LENGTH_UPLOAD = 0;
const int CURLINFO_CONTENT_LENGTH_UPLOAD_T = 0;
const int CURLINFO_CONTENT_TYPE = 0;
const int CURLINFO_COOKIELIST = 0;
const int CURLINFO_EFFECTIVE_URL = 0;
const int CURLINFO_FILETIME = 0;
const int CURLINFO_FILETIME_T = 0;
const int CURLINFO_FTP_ENTRY_PATH = 0;
const int CURLINFO_HEADER_OUT = 0;
const int CURLINFO_HEADER_SIZE = 0;
const int CURLINFO_HTTPAUTH_AVAIL = 0;
const int CURLINFO_HTTP_CONNECTCODE = 0;
const int CURLINFO_HTTP_CODE = 0;
const int CURLINFO_HTTP_VERSION = 0;
const int CURLINFO_LASTONE = 0;
const int CURLINFO_LOCAL_IP = 0;
const int CURLINFO_LOCAL_PORT = 0;
const int CURLINFO_NAMELOOKUP_TIME = 0;
const int CURLINFO_NAMELOOKUP_TIME_T = 0;
const int CURLINFO_NUM_CONNECTS = 0;
const int CURLINFO_OS_ERRNO = 0;
const int CURLINFO_PRETRANSFER_TIME = 0;
const int CURLINFO_PRETRANSFER_TIME_T = 0;
const int CURLINFO_PRIMARY_IP = 0;
const int CURLINFO_PRIMARY_PORT = 0;
const int CURLINFO_PRIVATE = 0;
const int CURLINFO_PROTOCOL = 0;
const int CURLINFO_PROXYAUTH_AVAIL = 0;
const int CURLINFO_PROXY_SSL_VERIFYRESULT = 0;
const int CURLINFO_REDIRECT_COUNT = 0;
const int CURLINFO_REDIRECT_TIME = 0;
const int CURLINFO_REDIRECT_TIME_T = 0;
const int CURLINFO_REDIRECT_URL = 0;
const int CURLINFO_REQUEST_SIZE = 0;
const int CURLINFO_RESPONSE_CODE = 0;
const int CURLINFO_RTSP_CLIENT_CSEQ = 0;
const int CURLINFO_RTSP_CSEQ_RECV = 0;
const int CURLINFO_RTSP_SERVER_CSEQ = 0;
const int CURLINFO_RTSP_SESSION_ID = 0;
const int CURLINFO_SCHEME = 0;
const int CURLINFO_SIZE_DOWNLOAD = 0;
const int CURLINFO_SIZE_UPLOAD = 0;
const int CURLINFO_SPEED_DOWNLOAD = 0;
const int CURLINFO_SIZE_DOWNLOAD_T = 0;
const int CURLINFO_SPEED_UPLOAD = 0;
const int CURLINFO_SIZE_UPLOAD_T = 0;
const int CURLINFO_SPEED_DOWNLOAD_T = 0;
const int CURLINFO_SPEED_UPLOAD_T = 0;
const int CURLINFO_SSL_ENGINES = 0;
const int CURLINFO_SSL_VERIFYRESULT = 0;
const int CURLINFO_STARTTRANSFER_TIME = 0;
const int CURLINFO_STARTTRANSFER_TIME_T = 0;
const int CURLINFO_TOTAL_TIME = 0;
const int CURLINFO_TOTAL_TIME_T = 0;

const int CURLMSG_DONE = 1;

const int CURLM_BAD_EASY_HANDLE = 0;
const int CURLM_BAD_HANDLE = 0;
const int CURLM_CALL_MULTI_PERFORM = 0;
const int CURLM_INTERNAL_ERROR = 0;
const int CURLM_OK = 0;
const int CURLM_OUT_OF_MEMORY = 0;

const int CURLOPT_AUTOREFERER = 0;
const int CURLOPT_BINARYTRANSFER = 0;
const int CURLOPT_COOKIESESSION = 0;
const int CURLOPT_CRLF = 0;
const int CURLOPT_DNS_USE_GLOBAL_CACHE = 0;
const int CURLOPT_DOH_URL = 0;
const int CURLOPT_FAILONERROR = 0;
const int CURLOPT_FILETIME = 0;
const int CURLOPT_FOLLOWLOCATION = 0;
const int CURLOPT_FORBID_REUSE = 0;
const int CURLOPT_FRESH_CONNECT = 0;
const int CURLOPT_HEADER = 0;
const int CURLOPT_HTTPGET = 0;
const int CURLOPT_MUTE = 0;
const int CURLOPT_NOBODY = 0;
const int CURLOPT_NOPROGRESS = 0;
const int CURLOPT_NOSIGNAL = 0;
const int CURLOPT_POST = 0;
const int CURLOPT_PUT = 0;
const int CURLOPT_RETURNTRANSFER = 0;
const int CURLOPT_UPLOAD = 0;
const int CURLOPT_VERBOSE = 0;
const int CURLOPT_BUFFERSIZE = 0;
const int CURLOPT_CLOSEPOLICY = 0;
const int CURLOPT_HTTP_VERSION = 0;
const int CURLOPT_HTTPAUTH = 0;
const int CURLOPT_INFILESIZE = 0;
const int CURLOPT_MAXCONNECTS = 0;
const int CURLOPT_MAXREDIRS = 0;
const int CURLOPT_PORT = 0;
const int CURLOPT_RESUME_FROM = 0;
const int CURLOPT_TIMECONDITION = 0;
const int CURLOPT_TIMEVALUE = 0;
const int CURLOPT_TIMEVALUE_LARGE = 0;
const int CURLOPT_COOKIE = 0;
const int CURLOPT_COOKIEFILE = 0;
const int CURLOPT_COOKIEJAR = 0;
const int CURLOPT_CUSTOMREQUEST = 0;
const int CURLOPT_EGDSOCKET = 0;
const int CURLOPT_ENCODING = 0;
const int CURLOPT_INTERFACE = 0;
const int CURLOPT_IPRESOLVE = 0;
const int CURLOPT_POSTFIELDS = 0;
const int CURLOPT_RANGE = 0;
const int CURLOPT_REFERER = 0;
const int CURLOPT_URL = 0;
const int CURLOPT_USERAGENT = 0;
const int CURLOPT_USERPWD = 0;
const int CURLOPT_HTTPHEADER = 0;
const int CURLOPT_FILE = 0;
const int CURLOPT_INFILE = 0;
const int CURLOPT_STDERR = 0;
const int CURLOPT_WRITEHEADER = 0;
const int CURLOPT_HEADERFUNCTION = 0;
const int CURLOPT_PASSWDFUNCTION = 0;
const int CURLOPT_READFUNCTION = 0;
const int CURLOPT_WRITEFUNCTION = 0;
const int CURLOPT_HTTPPROXYTUNNEL = 0;
const int CURLOPT_PROXYAUTH = 0;
const int CURLOPT_PROXYPORT = 0;
const int CURLOPT_PROXYTYPE = 0;
const int CURLOPT_PROXY = 0;
const int CURLOPT_PROXYUSERPWD = 0;
const int CURLOPT_CONNECTTIMEOUT = 0;
const int CURLOPT_CONNECTTIMEOUT_MS = 0;
const int CURLOPT_DNS_CACHE_TIMEOUT = 0;
const int CURLOPT_LOW_SPEED_LIMIT = 0;
const int CURLOPT_LOW_SPEED_TIME = 0;
const int CURLOPT_TIMEOUT = 0;
const int CURLOPT_TIMEOUT_MS = 0;
const int CURLOPT_SSL_VERIFYPEER = 0;
const int CURLOPT_SSL_VERIFYHOST = 0;
const int CURLOPT_SSLVERSION = 0;
const int CURLOPT_CAINFO = 0;
const int CURLOPT_CAPATH = 0;
const int CURLOPT_RANDOM_FILE = 0;
const int CURLOPT_SAFE_UPLOAD = 0;
const int CURLOPT_SSL_CIPHER_LIST = 0;
const int CURLOPT_SSLCERT = 0;
const int CURLOPT_SSLCERTPASSWD = 0;
const int CURLOPT_SSLCERTTYPE = 0;
const int CURLOPT_SSLCERT_BLOB = 0;
const int CURLOPT_SSLENGINE = 0;
const int CURLOPT_SSLENGINE_DEFAULT = 0;
const int CURLOPT_SSLKEY = 0;
const int CURLOPT_SSLKEYPASSWD = 0;
const int CURLOPT_SSLKEYTYPE = 0;
const int CURLOPT_SSLKEY_BLOB = 0;
const int CURLOPT_FTP_USE_EPRT = 0;
const int CURLOPT_FTP_USE_EPSV = 0;
const int CURLOPT_FTPAPPEND = 0;
const int CURLOPT_FTPLISTONLY = 0;
const int CURLOPT_NETRC = 0;
const int CURLOPT_TRANSFERTEXT = 0;
const int CURLOPT_UNRESTRICTED_AUTH = 0;
const int CURLOPT_FTPSSLAUTH = 0;
const int CURLOPT_FTPPORT = 0;
const int CURLOPT_POSTQUOTE = 0;
const int CURLOPT_RESOLVE = 0;
const int CURLOPT_QUOTE = 0;
const int CURLOPT_ABSTRACT_UNIX_SOCKET = 0;
const int CURLOPT_ACCEPT_ENCODING = 0;
const int CURLOPT_ACCEPTTIMEOUT_MS = 0;
const int CURLOPT_ADDRESS_SCOPE = 0;
const int CURLOPT_APPEND = 0;
const int CURLOPT_CERTINFO = 0;
const int CURLOPT_CONNECT_ONLY = 0;
const int CURLOPT_CONNECT_TO = 0;
const int CURLOPT_COOKIELIST = 0;
const int CURLOPT_CRLFILE = 0;
const int CURLOPT_DEFAULT_PROTOCOL = 0;
const int CURLOPT_DIRLISTONLY = 0;
const int CURLOPT_DISALLOW_USERNAME_IN_URL = 0;
const int CURLOPT_DNS_INTERFACE = 0;
const int CURLOPT_DNS_LOCAL_IP4 = 0;
const int CURLOPT_DNS_LOCAL_IP6 = 0;
const int CURLOPT_DNS_SERVERS = 0;
const int CURLOPT_DNS_SHUFFLE_ADDRESSES = 0;
const int CURLOPT_EXPECT_100_TIMEOUT_MS = 0;
const int CURLOPT_FNMATCH_FUNCTION = 0;
const int CURLOPT_FTP_ACCOUNT = 0;
const int CURLOPT_FTP_ALTERNATIVE_TO_USER = 0;
const int CURLOPT_FTP_CREATE_MISSING_DIRS = 0;
const int CURLOPT_FTP_FILEMETHOD = 0;
const int CURLOPT_FTP_RESPONSE_TIMEOUT = 0;
const int CURLOPT_FTP_SKIP_PASV_IP = 0;
const int CURLOPT_FTP_SSL = 0;
const int CURLOPT_FTP_SSL_CCC = 0;
const int CURLOPT_FTP_USE_PRET = 0;
const int CURLOPT_GSSAPI_DELEGATION = 0;
const int CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS = 0;
const int CURLOPT_HAPROXYPROTOCOL = 0;
const int CURLOPT_HTTP200ALIASES = 0;
const int CURLOPT_HTTP_CONTENT_DECODING = 0;
const int CURLOPT_HTTP_TRANSFER_DECODING = 0;
const int CURLOPT_IGNORE_CONTENT_LENGTH = 0;
const int CURLOPT_ISSUERCERT = 0;
const int CURLOPT_ISSUERCERT_BLOB = 0;
const int CURLOPT_KEEP_SENDING_ON_ERROR = 0;
const int CURLOPT_KEYPASSWD = 0;
const int CURLOPT_KRB4LEVEL = 0;
const int CURLOPT_KRBLEVEL = 0;
const int CURLOPT_LOCALPORT = 0;
const int CURLOPT_LOCALPORTRANGE = 0;
const int CURLOPT_LOGIN_OPTIONS = 0;
const int CURLOPT_MAIL_AUTH = 0;
const int CURLOPT_MAIL_FROM = 0;
const int CURLOPT_MAIL_RCPT = 0;
const int CURLOPT_MAXFILESIZE = 0;
const int CURLOPT_MAX_RECV_SPEED_LARGE = 0;
const int CURLOPT_MAX_SEND_SPEED_LARGE = 0;
const int CURLOPT_NETRC_FILE = 0;
const int CURLOPT_NEW_DIRECTORY_PERMS = 0;
const int CURLOPT_NEW_FILE_PERMS = 0;
const int CURLOPT_NOPROXY = 0;
const int CURLOPT_PASSWORD = 0;
const int CURLOPT_PATH_AS_IS = 0;
const int CURLOPT_PINNEDPUBLICKEY = 0;
const int CURLOPT_PIPEWAIT = 0;
const int CURLOPT_POSTREDIR = 0;
const int CURLOPT_PREQUOTE = 0;
const int CURLOPT_PRIVATE = 0;
const int CURLOPT_PROGRESSFUNCTION = 0;
const int CURLOPT_PROTOCOLS = 0;
const int CURLOPT_PROXYPASSWORD = 0;
const int CURLOPT_PROXY_CAINFO = 0;
const int CURLOPT_PROXY_CAPATH = 0;
const int CURLOPT_PROXY_CRLFILE = 0;
const int CURLOPT_PROXY_ISSUERCERT = 0;
const int CURLOPT_PROXY_ISSUERCERT_BLOB = 0;
const int CURLOPT_PROXY_KEYPASSWD = 0;
const int CURLOPT_PROXY_PINNEDPUBLICKEY = 0;
const int CURLOPT_PROXY_SERVICE_NAME = 0;
const int CURLOPT_PROXY_SSLCERT = 0;
const int CURLOPT_PROXY_SSLCERTTYPE = 0;
const int CURLOPT_PROXY_SSLCERT_BLOB = 0;
const int CURLOPT_PROXY_SSLKEY = 0;
const int CURLOPT_PROXY_SSLKEYTYPE = 0;
const int CURLOPT_PROXY_SSLKEY_BLOB = 0;
const int CURLOPT_PROXY_SSLVERSION = 0;
const int CURLOPT_PROXY_SSL_CIPHER_LIST = 0;
const int CURLOPT_PROXY_SSL_OPTIONS = 0;
const int CURLOPT_PROXY_SSL_VERIFYHOST = 0;
const int CURLOPT_PROXY_SSL_VERIFYPEER = 0;
const int CURLOPT_PROXY_TLS13_CIPHERS = 0;
const int CURLOPT_PROXY_TLSAUTH_PASSWORD = 0;
const int CURLOPT_PROXY_TLSAUTH_TYPE = 0;
const int CURLOPT_PROXY_TLSAUTH_USERNAME = 0;
const int CURLOPT_PROXY_TRANSFER_MODE = 0;
const int CURLOPT_PROXYUSERNAME = 0;
const int CURLOPT_READDATA = 0;
const int CURLOPT_REDIR_PROTOCOLS = 0;
const int CURLOPT_REQUEST_TARGET = 0;
const int CURLOPT_RTSP_CLIENT_CSEQ = 0;
const int CURLOPT_RTSP_REQUEST = 0;
const int CURLOPT_RTSP_SERVER_CSEQ = 0;
const int CURLOPT_RTSP_SESSION_ID = 0;
const int CURLOPT_RTSP_STREAM_URI = 0;
const int CURLOPT_RTSP_TRANSPORT = 0;
const int CURLOPT_SASL_IR = 0;
const int CURLOPT_SERVICE_NAME = 0;
const int CURLOPT_SHARE = 0;
const int CURLOPT_SOCKS5_AUTH = 0;
const int CURLOPT_SOCKS5_GSSAPI_NEC = 0;
const int CURLOPT_SOCKS5_GSSAPI_SERVICE = 0;
const int CURLOPT_SSH_AUTH_TYPES = 0;
const int CURLOPT_SSH_COMPRESSION = 0;
const int CURLOPT_SSH_HOST_PUBLIC_KEY_MD5 = 0;
const int CURLOPT_SSH_KNOWNHOSTS = 0;
const int CURLOPT_SSH_PRIVATE_KEYFILE = 0;
const int CURLOPT_SSH_PUBLIC_KEYFILE = 0;
const int CURLOPT_SSL_ENABLE_ALPN = 0;
const int CURLOPT_SSL_ENABLE_NPN = 0;
const int CURLOPT_SSL_FALSESTART = 0;
const int CURLOPT_SSL_OPTIONS = 0;
const int CURLOPT_SSL_SESSIONID_CACHE = 0;
const int CURLOPT_SSL_VERIFYSTATUS = 0;
const int CURLOPT_SUPPRESS_CONNECT_HEADERS = 0;
const int CURLOPT_STREAM_WEIGHT = 0;
const int CURLOPT_TCP_FASTOPEN = 0;
const int CURLOPT_TCP_KEEPALIVE = 0;
const int CURLOPT_TCP_KEEPIDLE = 0;
const int CURLOPT_TCP_KEEPINTVL = 0;
const int CURLOPT_TCP_NODELAY = 0;
const int CURLOPT_TELNETOPTIONS = 0;
const int CURLOPT_TFTP_BLKSIZE = 0;
const int CURLOPT_TFTP_NO_OPTIONS = 0;
const int CURLOPT_TLS13_CIPHERS = 0;
const int CURLOPT_TLSAUTH_PASSWORD = 0;
const int CURLOPT_TLSAUTH_TYPE = 0;
const int CURLOPT_TLSAUTH_USERNAME = 0;
const int CURLOPT_TRANSFER_ENCODING = 0;
const int CURLOPT_UPKEEP_INTERVAL_MS = 0;
const int CURLOPT_UPLOAD_BUFFERSIZE = 0;
const int CURLOPT_UNIX_SOCKET_PATH = 0;
const int CURLOPT_USERNAME = 0;
const int CURLOPT_USE_SSL = 0;
const int CURLOPT_WILDCARDMATCH = 0;
const int CURLOPT_XOAUTH2_BEARER = 0;

const int CURLE_ABORTED_BY_CALLBACK = 0;
const int CURLE_BAD_CALLING_ORDER = 0;
const int CURLE_BAD_CONTENT_ENCODING = 0;
const int CURLE_BAD_DOWNLOAD_RESUME = 0;
const int CURLE_BAD_FUNCTION_ARGUMENT = 0;
const int CURLE_BAD_PASSWORD_ENTERED = 0;
const int CURLE_COULDNT_CONNECT = 0;
const int CURLE_COULDNT_RESOLVE_HOST = 0;
const int CURLE_COULDNT_RESOLVE_PROXY = 0;
const int CURLE_FAILED_INIT = 0;
const int CURLE_FILESIZE_EXCEEDED = 0;
const int CURLE_FILE_COULDNT_READ_FILE = 0;
const int CURLE_FTP_ACCESS_DENIED = 0;
const int CURLE_FTP_BAD_DOWNLOAD_RESUME = 0;
const int CURLE_FTP_CANT_GET_HOST = 0;
const int CURLE_FTP_CANT_RECONNECT = 0;
const int CURLE_FTP_COULDNT_GET_SIZE = 0;
const int CURLE_FTP_COULDNT_RETR_FILE = 0;
const int CURLE_FTP_COULDNT_SET_ASCII = 0;
const int CURLE_FTP_COULDNT_SET_BINARY = 0;
const int CURLE_FTP_COULDNT_STOR_FILE = 0;
const int CURLE_FTP_COULDNT_USE_REST = 0;
const int CURLE_FTP_PARTIAL_FILE = 0;
const int CURLE_FTP_PORT_FAILED = 0;
const int CURLE_FTP_QUOTE_ERROR = 0;
const int CURLE_FTP_SSL_FAILED = 0;
const int CURLE_FTP_USER_PASSWORD_INCORRECT = 0;
const int CURLE_FTP_WEIRD_227_FORMAT = 0;
const int CURLE_FTP_WEIRD_PASS_REPLY = 0;
const int CURLE_FTP_WEIRD_PASV_REPLY = 0;
const int CURLE_FTP_WEIRD_SERVER_REPLY = 0;
const int CURLE_FTP_WEIRD_USER_REPLY = 0;
const int CURLE_FTP_WRITE_ERROR = 0;
const int CURLE_FUNCTION_NOT_FOUND = 0;
const int CURLE_GOT_NOTHING = 0;
const int CURLE_HTTP_NOT_FOUND = 0;
const int CURLE_HTTP_PORT_FAILED = 0;
const int CURLE_HTTP_POST_ERROR = 0;
const int CURLE_HTTP_RANGE_ERROR = 0;
const int CURLE_HTTP_RETURNED_ERROR = 0;
const int CURLE_LDAP_CANNOT_BIND = 0;
const int CURLE_LDAP_INVALID_URL = 0;
const int CURLE_LDAP_SEARCH_FAILED = 0;
const int CURLE_LIBRARY_NOT_FOUND = 0;
const int CURLE_MALFORMAT_USER = 0;
const int CURLE_OBSOLETE = 0;
const int CURLE_OK = 0;
const int CURLE_OPERATION_TIMEOUTED = 0;
const int CURLE_OPERATION_TIMEDOUT = 0;
const int CURLE_OUT_OF_MEMORY = 0;
const int CURLE_PARTIAL_FILE = 0;
const int CURLE_READ_ERROR = 0;
const int CURLE_RECV_ERROR = 0;
const int CURLE_SEND_ERROR = 0;
const int CURLE_SHARE_IN_USE = 0;
const int CURLE_SSH = 0;
const int CURLE_SSL_CACERT = 0;
const int CURLE_SSL_CERTPROBLEM = 0;
const int CURLE_SSL_CIPHER = 0;
const int CURLE_SSL_CONNECT_ERROR = 0;
const int CURLE_SSL_ENGINE_NOTFOUND = 0;
const int CURLE_SSL_ENGINE_SETFAILED = 0;
const int CURLE_SSL_PEER_CERTIFICATE = 0;
const int CURLE_TELNET_OPTION_SYNTAX = 0;
const int CURLE_TOO_MANY_REDIRECTS = 0;
const int CURLE_UNKNOWN_TELNET_OPTION = 0;
const int CURLE_UNSUPPORTED_PROTOCOL = 0;
const int CURLE_URL_MALFORMAT = 0;
const int CURLE_URL_MALFORMAT_USER = 0;
const int CURLE_WRITE_ERROR = 0;

const int CURLFTPAUTH_DEFAULT = 0;
const int CURLFTPAUTH_SSL = 0;
const int CURLFTPAUTH_TLS = 0;

const int CURLFTPMETHOD_MULTICWD = 0;
const int CURLFTPMETHOD_NOCWD = 0;
const int CURLFTPMETHOD_SINGLECWD = 0;

const int CURLFTPSSL_CCC_ACTIVE = 0;
const int CURLFTPSSL_CCC_NONE = 0;
const int CURLFTPSSL_CCC_PASSIVE = 0;

const int CURLFTPSSL_ALL = 0;
const int CURLFTPSSL_CONTROL = 0;
const int CURLFTPSSL_NONE = 0;
const int CURLFTPSSL_TRY = 0;

const int CURLGSSAPI_DELEGATION_FLAG = 0;
const int CURLGSSAPI_DELEGATION_POLICY_FLAG = 0;

const int CURLVERSION_NOW = 0;
const int CURL_VERSION_BROTLI = 0;
const int CURL_VERSION_HTTP2 = 0;
const int CURL_VERSION_IPV6 = 0;
const int CURL_VERSION_KERBEROS4 = 0;
const int CURL_VERSION_LIBZ = 0;
const int CURL_VERSION_MULTI_SSL = 0;
const int CURL_VERSION_SSL = 0;

const int CURLPROTO_ALL = 0;
const int CURLPROTO_DICT = 0;
const int CURLPROTO_FILE = 0;
const int CURLPROTO_FTP = 0;
const int CURLPROTO_FTPS = 0;
const int CURLPROTO_GOPHER = 0;
const int CURLPROTO_HTTP = 0;
const int CURLPROTO_HTTPS = 0;
const int CURLPROTO_IMAP = 0;
const int CURLPROTO_IMAPS = 0;
const int CURLPROTO_LDAP = 0;
const int CURLPROTO_LDAPS = 0;
const int CURLPROTO_POP3 = 0;
const int CURLPROTO_POP3S = 0;
const int CURLPROTO_RTMP = 0;
const int CURLPROTO_RTMPE = 0;
const int CURLPROTO_RTMPS = 0;
const int CURLPROTO_RTMPT = 0;
const int CURLPROTO_RTMPTE = 0;
const int CURLPROTO_RTMPTS = 0;
const int CURLPROTO_RTSP = 0;
const int CURLPROTO_SCP = 0;
const int CURLPROTO_SFTP = 0;
const int CURLPROTO_SMB = 0;
const int CURLPROTO_SMBS = 0;
const int CURLPROTO_SMTP = 0;
const int CURLPROTO_SMTPS = 0;
const int CURLPROTO_TELNET = 0;
const int CURLPROTO_TFTP = 0;

const int CURLPROXY_HTTP = 0;
const int CURLPROXY_HTTPS = 0;
const int CURLPROXY_SOCKS4 = 0;
const int CURLPROXY_SOCKS4A = 0;
const int CURLPROXY_SOCKS5 = 0;
const int CURLPROXY_SOCKS5_HOSTNAME = 0;

const int CURLSSH_AUTH_AGENT = 0;
const int CURLSSH_AUTH_ANY = 0;
const int CURLSSH_AUTH_DEFAULT = 0;
const int CURLSSH_AUTH_GSSAPI = 0;
const int CURLSSH_AUTH_HOST = 0;
const int CURLSSH_AUTH_KEYBOARD = 0;
const int CURLSSH_AUTH_NONE = 0;
const int CURLSSH_AUTH_PASSWORD = 0;
const int CURLSSH_AUTH_PUBLICKEY = 0;

const int CURLSSLOPT_ALLOW_BEAST = 0;
const int CURLSSLOPT_NO_REVOKE = 0;

const int CURLUSESSL_ALL = 0;
const int CURLUSESSL_CONTROL = 0;
const int CURLUSESSL_NONE = 0;
const int CURLUSESSL_TRY = 0;

const int CURL_HTTP_VERSION_1_0 = 0;
const int CURL_HTTP_VERSION_1_1 = 0;
const int CURL_HTTP_VERSION_NONE = 0;
const int CURL_HTTP_VERSION_2 = 0;
const int CURL_HTTP_VERSION_2_0 = 0;
const int CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE = 0;
const int CURL_HTTP_VERSION_2TLS = 0;

const int CURL_IPRESOLVE_V4 = 0;
const int CURL_IPRESOLVE_V6 = 0;
const int CURL_IPRESOLVE_WHATEVER = 0;

const int CURL_NETRC_IGNORED = 0;
const int CURL_NETRC_OPTIONAL = 0;
const int CURL_NETRC_REQUIRED = 0;

const int CURL_REDIR_POST_303 = 0;

const int CURL_RTSPREQ_ANNOUNCE = 0;
const int CURL_RTSPREQ_DESCRIBE = 0;
const int CURL_RTSPREQ_GET_PARAMETER = 0;
const int CURL_RTSPREQ_OPTIONS = 0;
const int CURL_RTSPREQ_PAUSE = 0;
const int CURL_RTSPREQ_PLAY = 0;
const int CURL_RTSPREQ_RECEIVE = 0;
const int CURL_RTSPREQ_RECORD = 0;
const int CURL_RTSPREQ_SET_PARAMETER = 0;
const int CURL_RTSPREQ_SETUP = 0;
const int CURL_RTSPREQ_TEARDOWN = 0;

const int CURL_SSLVERSION_DEFAULT = 0;
const int CURL_SSLVERSION_SSLv2 = 0;
const int CURL_SSLVERSION_SSLv3 = 0;
const int CURL_SSLVERSION_TLSv1 = 0;
const int CURL_SSLVERSION_TLSv1_0 = 0;
const int CURL_SSLVERSION_TLSv1_1 = 0;
const int CURL_SSLVERSION_TLSv1_2 = 0;
const int CURL_SSLVERSION_TLSv1_3 = 0;
const int CURL_SSLVERSION_MAX_DEFAULT = 0;
const int CURL_SSLVERSION_MAX_TLSv1_0 = 0;
const int CURL_SSLVERSION_MAX_TLSv1_1 = 0;
const int CURL_SSLVERSION_MAX_TLSv1_2 = 0;
const int CURL_SSLVERSION_MAX_TLSv1_3 = 0;

const int CURL_TIMECOND_IFMODSINCE = 0;
const int CURL_TIMECOND_IFUNMODSINCE = 0;
const int CURL_TIMECOND_LASTMOD = 0;
const int CURL_TIMECOND_NONE = 0;

const int CURL_TLSAUTH_SRP = 0;

const int CURLOPT_HEADEROPT = 0;
const int CURLOPT_PROXYHEADER = 0;
const int CURLHEADER_UNIFIED = 0;
const int CURLHEADER_SEPARATE = 0;

const int CURL_LOCK_DATA_COOKIE = 0;
const int CURL_LOCK_DATA_DNS = 0;
const int CURL_LOCK_DATA_SSL_SESSION = 0;
const int CURLSHE_OK = 0;
const int CURLSHOPT_SHARE = 0;
const int CURLSHOPT_UNSHARE = 0;


<<__PHPStdLib>>
function curl_init($url = null);
<<__PHPStdLib>>
function curl_init_pooled(string $pool_name, $url = null);
<<__PHPStdLib>>
function curl_copy_handle(resource $ch);
<<__PHPStdLib>>
function curl_version(int $uversion = CURLVERSION_NOW);
<<__PHPStdLib>>
function curl_setopt(resource $ch, int $option, $value);
<<__PHPStdLib>>
function curl_setopt_array(resource $ch, $options);
<<__PHPStdLib>>
function curl_exec(resource $ch);
<<__PHPStdLib>>
function curl_getinfo(resource $ch, int $opt = 0);
<<__PHPStdLib>>
function curl_errno(resource $ch);
<<__PHPStdLib>>
function curl_error(resource $ch);
<<__PHPStdLib>>
function curl_strerror(int $code);
<<__PHPStdLib>>
function curl_close(resource $ch);
<<__PHPStdLib>>
function curl_multi_init();
<<__PHPStdLib>>
function curl_multi_strerror(int $errornum);
<<__PHPStdLib>>
function curl_multi_add_handle(resource $mh, resource $ch);
<<__PHPStdLib>>
function curl_multi_remove_handle(resource $mh, resource $ch);
<<__PHPStdLib>>
function curl_multi_exec(resource $mh, inout $still_running);
<<__PHPStdLib>>
function curl_multi_select(resource $mh, float $timeout = 1.0);
<<__PHPStdLib>>
function curl_multi_await(resource $mh, float $timeout = 1.0): Awaitable<int>;
<<__PHPStdLib>>
function curl_multi_getcontent(resource $ch);
<<__PHPStdLib>>
function curl_multi_info_read(resource $mh, inout $msgs_in_queue);
<<__PHPStdLib>>
function curl_multi_close(resource $mh);
<<__PHPStdLib>>
function curl_share_init();
<<__PHPStdLib>>
function curl_share_close(resource $sh);
<<__PHPStdLib>>
function curl_share_setopt(resource $sh, int $option, $value);

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
