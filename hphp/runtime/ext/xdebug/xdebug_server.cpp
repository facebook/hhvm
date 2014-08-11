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

#include "hphp/runtime/ext/xdebug/xdebug_server.h"
#include "hphp/runtime/ext/xdebug/xdebug_utils.h"

#include "hphp/runtime/base/thread-info.h"
#include "hphp/util/network.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

// Globals
const static StaticString
  s_SERVER("_SERVER"),
  s_GET("_GET"),
  s_COOKIE("_COOKIE");

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

// Properties of $_SERVER to grab the client addr from
const static StaticString
  s_HTTP_X_FORWARDED_FOR("HTTP_X_FORWARDED_FOR"),
  s_REMOTE_ADDR("REMOTE_ADDR");

XDebugServer::XDebugServer(Mode mode) : m_mode(mode) {
  // Attempt to open optional log file
  if (XDEBUG_GLOBAL(RemoteLog).size() > 0) {
    m_logFile = fopen(XDEBUG_GLOBAL(RemoteLog).c_str(), "a");
    if (m_logFile == nullptr) {
      raise_warning("XDebug could not open the remote debug file '%s'.",
                    XDEBUG_GLOBAL(RemoteLog).c_str());
    }

    log("Log opened at");
    XDebugUtils::fprintTimestamp(m_logFile);
    log("\n");
    logFlush();
  }

  // Grab the hostname and port to connect to
  const char* hostname = XDEBUG_GLOBAL(RemoteHost).c_str();
  int port = XDEBUG_GLOBAL(RemotePort);
  if (XDEBUG_GLOBAL(RemoteConnectBack)) {
    const ArrayData* globals = get_global_variables()->asArrayData();
    Array server = globals->get(s_SERVER).toArray();
    log("I: Checking remote connect back address.\n");

    // Grab $_SERVER[HTTP_X_FORWARDED_FOR] then $_SERVER[REMOTE_ADDR]
    bool host_found = true;
    if (server[s_HTTP_X_FORWARDED_FOR].isString()) {
      hostname = server[s_HTTP_X_FORWARDED_FOR].toString().data();
    } else if (server[s_REMOTE_ADDR].isString()) {
      hostname = server[s_REMOTE_ADDR].toString().data();
    } else {
      host_found = false;
    }

    // Did we find a host?
    if (host_found) {
      log("I: Remote address found, connecting to %s:%d.\n", hostname, port);
    } else {
      log("W: Remote address not found, connecting to configured address/port: "
          "%s:%d. :-|\n", hostname, port);
    }
  } else {
    log("I: Connecting to configured address/port: %s:%d.\n", hostname, port);
  }

  // Create the socket
  m_socket = createSocket(hostname, port);
  if (m_socket == -1) {
    log("E: Could not connect to client. :-(\n");
    goto failure;
  } else if (m_socket == -2) {
    log("E: Time-out connecting to client. :-(\n");
    goto failure;
  }

  // Get the requested handler
  log("I: Connected to client. :-)\n");
  if (XDEBUG_GLOBAL(RemoteHandler) != "dbgp") {
    log("E: The remote debug handler '%s' is not supported. :-(\n",
        XDEBUG_GLOBAL(RemoteHandler).c_str());
    goto failure;
  }

  // php5 xdebug has an error case here, but initializing the dbgp handler never
  // actually fails
  initDbgp();
  return;

// Failure cleanup. A goto is used to prevent duplication
failure:
  destroySocket();
  closeLog();
  // Allows the guarantee that any instance of an xdebug server is valid
  throw Exception("XDebug Server construction failed");
}

XDebugServer::~XDebugServer() {
  deinitDbgp();
  destroySocket();
  closeLog();
}

bool XDebugServer::lookupHostname(const char* hostname, struct in_addr& in) {
  HostEnt result;
  if (safe_gethostbyname(hostname, result)) {
    in = *((struct in_addr*) result.hostbuf.h_addr);
    return true;
  }
  return false;
}

int XDebugServer::createSocket(const char* hostname, int port) {
  // Grab the address info
  struct sockaddr sa;
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons((unsigned short) port);
  lookupHostname(hostname, address.sin_addr);

  // Create the socket
  int sockfd = socket(address.sin_family, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("create_debugger_socket(\"%s\", %d) socket: %s\n",
           hostname, port, strerror(errno));
    return -1;
  }

  // Put socket in non-blocking mode so we can use select for timeouts
  struct timeval timeout;
  double timeout_sec = XDEBUG_GLOBAL(RemoteTimeout);
  timeout.tv_sec = (int) timeout_sec;
  timeout.tv_usec = (timeout_sec - (double) timeout.tv_sec) * 1.0e6;
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  // Do the connect
  int status = connect(sockfd, (struct sockaddr*) &address, sizeof(address));
  if (status < 0) {
    if (errno != EINPROGRESS) {
      return -1;
    }

    // Loop until request has completed or there is an error
    while (true) {
      fd_set rset, wset, eset;
      FD_ZERO(&rset);
      FD_SET(sockfd, &rset);
      FD_ZERO(&wset);
      FD_SET(sockfd, &wset);
      FD_ZERO(&eset);
      FD_SET(sockfd, &eset);

      // Wait for a socket event. If we continue due to a timeout, return -2
      // If we exited due to an error, return -1. If the descriptor is ready,
      // we're done.
      if (select(sockfd + 1, &rset, &wset, &eset, &timeout) == 0) {
        return -2;
      } else if (FD_ISSET(sockfd, &eset)) {
        return -1;
      } else if (FD_ISSET(sockfd, &wset) || FD_ISSET(sockfd, &rset)) {
        break;
      }
    }

    // Ensure we're actually connected
    socklen_t sa_size = sizeof(sa);
    if (getpeername(sockfd, &sa, &sa_size) == -1) {
      return -1;
    }
  }

  // Reset the socket to non-blocking mode and setup socket options
  fcntl(sockfd, F_SETFL, 0);
  long optval = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
  return sockfd;
}

void XDebugServer::destroySocket() {
  if (m_socket >= 0) {
    close(m_socket);
    m_socket = -1;
  }
}

void XDebugServer::closeLog() {
  if (m_logFile == nullptr) {
    return;
  }

  log("Log closed at ");
  XDebugUtils::fprintTimestamp(m_logFile);
  log("\n\n");
  logFlush();

  fclose(m_logFile);
  m_logFile = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Statics

// Server session properties
static const StaticString
  s_SESSION_START("XDEBUG_SESSION_START"),
  s_SESSION_STOP("XDEBUG_SESSION_STOP"),
  s_SESSION("XDEBUG_SESSION");

void XDebugServer::onRequestInit() {
  if (!XDEBUG_GLOBAL(RemoteEnable)) {
    return;
  }

  // TODO(#4489053) Enable this when debugger internals have been refactored
  // Need to turn on debugging regardless of the remote mode in order to
  // capture exceptions/errors
  // ThreadInfo *ti = ThreadInfo::s_threadInfo.getNoCheck();
  // ti->m_reqInjectionData.setDebugger(true);

  // Grab $_GET, $_COOKIE, and the transport
  const ArrayData* globals = get_global_variables()->asArrayData();
  Array get = globals->get(s_GET).toArray();
  Array cookie = globals->get(s_COOKIE).toArray();
  Transport* transport = g_context->getTransport();

  // Need to check $_GET[XDEBUG_SESSION_STOP]. If set, delete the session
  // cookie
  const Variant sess_stop_var = get[s_SESSION_STOP];
  if (!sess_stop_var.isNull()) {
    cookie.set(s_SESSION, init_null());
    if (transport != nullptr) {
      transport->setCookie(s_SESSION, empty_string());
    }
  }

  // Need to check $_GET[XDEBUG_SESSION_START]. If set, store the session
  // cookie with $_GET[XDEBUG_SESSION_START] as the value
  const Variant sess_start_var = get[s_SESSION_START];
  if (sess_start_var.isString()) {
    String sess_start = sess_start_var.toString();
    cookie.set(s_SESSION,  sess_start);
    if (transport != nullptr) {
      transport->setCookie(s_SESSION,
                           sess_start,
                           XDEBUG_GLOBAL(RemoteCookieExpireTime));
    }
  }
}

bool XDebugServer::isNeeded() {
  if (!XDEBUG_GLOBAL(RemoteEnable) ||
      XDEBUG_GLOBAL(RemoteMode) == "jit") {
    return false;
  } else if (XDEBUG_GLOBAL(RemoteAutostart)) {
    return true;
  } else {
    // Check $_COOKIE[XDEBUG_SESSION]
    const ArrayData* globals = get_global_variables()->asArrayData();
    Array cookie = globals->get(s_COOKIE).toArray();
    return !cookie[s_SESSION].isNull();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dbgp

// Header for sent messages
#define XML_MSG_HEADER "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"

// Needed $_SERVER variables
static const StaticString s_SCRIPT_FILENAME("SCRIPT_FILENAME");

void XDebugServer::addXmnls(xdebug_xml_node& node) {
  xdebug_xml_add_attribute(&node, "xmlns", "urn:debugger_protocol_v1");
  xdebug_xml_add_attribute(&node, "xmlns:xdebug",
                           "http://xdebug.org/dbgp/xdebug");
}

void XDebugServer::addCmdAndTrans(xdebug_xml_node& node) {
  // lastcmd and lasttransid are not always set (for example when the
  // connection is severed before the first command is sent)
  if (m_lastCommand == Command::NONE || m_lastTransactionId == nullptr) {
    return;
  }

  // TODO(#4489053) Change this when xml api is changed
  char* command = const_cast<char*>(getCommandStr(m_lastCommand));
  xdebug_xml_add_attribute_ex(&node, "command", command, 0, 0);
  xdebug_xml_add_attribute_ex(&node, "transaction_id",
                              m_lastTransactionId, 0, 0);
}

void XDebugServer::addStatus(xdebug_xml_node& node) {
  // TODO(#4489053) Change this when xml api is changed
  char* status = const_cast<char*>(getStatusString(m_status));
  char* reason = const_cast<char*>(getReasonString(m_reason));
  xdebug_xml_add_attribute_ex(&node, "status", status, 0, 0);
  xdebug_xml_add_attribute_ex(&node, "reason", reason, 0, 0);
}

void XDebugServer::initDbgp() {
  // Initialize the status and reason
  switch (m_mode) {
    case Mode::REQ:
      setStatus(Status::STARTING, Reason::OK);
      break;
    case Mode::JIT:
      setStatus(Status::BREAK, Reason::ERROR);
      break;
  }
  // Create the response
  xdebug_xml_node* response = xdebug_xml_node_init("init");
  addXmnls(*response);

  // Add the engine info
  xdebug_xml_node* child = xdebug_xml_node_init("engine");
  xdebug_xml_add_attribute(child, "version", XDEBUG_VERSION);
  xdebug_xml_add_text(child, XDEBUG_NAME, 0);
  xdebug_xml_add_child(response, child);

  // Add the author
  child = xdebug_xml_node_init("author");
  xdebug_xml_add_text(child, XDEBUG_AUTHOR, 0);
  xdebug_xml_add_child(response, child);

  // Add the url
  child = xdebug_xml_node_init("url");
  xdebug_xml_add_text(child, XDEBUG_URL, 0);
  xdebug_xml_add_child(response, child);

  // Add the copyright
  child = xdebug_xml_node_init("copyright");
  xdebug_xml_add_text(child, XDEBUG_COPYRIGHT, 0);
  xdebug_xml_add_child(response, child);

  // Grab the absolute path of the script filename
  const ArrayData* globals = get_global_variables()->asArrayData();
  Variant scriptname_var = globals->get(s_SERVER).toArray()[s_SCRIPT_FILENAME];
  assert(scriptname_var.isString());
  char* scriptname = scriptname_var.toString().get()->mutableData();
  char* fileuri = XDebugUtils::pathToUrl(scriptname);

  // Grab the app id (pid)
  // TODO(#4489053) Specification mentions the parent app id as well, xdebug
  //                doesn't include it.
  char* appid = xdebug_sprintf("%d", getpid());

  // Add attributes to the root init node
  xdebug_xml_add_attribute_ex(response, "fileuri", fileuri, 0, 1);
  xdebug_xml_add_attribute_ex(response, "language", "PHP", 0, 0);
  xdebug_xml_add_attribute_ex(response, "protocol_version", DBGP_VERSION, 0, 0);
  xdebug_xml_add_attribute_ex(response, "appid", appid, 0, 1);

  // Add the DBGP_COOKIE environment variable
  char* dbgp_cookie = getenv("DBGP_COOKIE");
  if (dbgp_cookie != nullptr) {
    xdebug_xml_add_attribute_ex(response, "session", dbgp_cookie, 0, 0);
  }

  // Add the idekey
  if (XDEBUG_GLOBAL(IdeKey).size() > 0) {
    // TODO(#4489053) Change this when xml api is changed
    char* idekey = const_cast<char*>(XDEBUG_GLOBAL(IdeKey).c_str());
    xdebug_xml_add_attribute_ex(response, "idekey", idekey, 0, 0);
  }

  // Sent the response
  sendMessage(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a response from the client
  doCommandLoop();
}

void XDebugServer::deinitDbgp() {
  setStatus(Status::STOPPING, Reason::OK);

  // Send the xml shutdown response
  xdebug_xml_node* response = xdebug_xml_node_init("response");
  addXmnls(*response);
  addCmdAndTrans(*response);
  sendMessage(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a response from the client
  doCommandLoop();

  // Free the input buffer
  smart_free(m_buffer);
}



void XDebugServer::sendMessage(xdebug_xml_node& xml) {
  // Convert xml to an xdebug_str
  xdebug_str xml_message = {0, 0, nullptr};
  xdebug_xml_return_node(&xml, &xml_message);
  size_t msg_len = xml_message.l + sizeof(XML_MSG_HEADER) - 1;

  // Log the message
  log("-> %s\n\n", xml_message.d);
  logFlush();

  // Format the message
  xdebug_str* message;
  xdebug_str_ptr_init(message);
  xdebug_str_add(message, xdebug_sprintf("%d", msg_len, 1), 1);
  xdebug_str_addl(message, "\0", 1, 0);
  xdebug_str_add(message, XML_MSG_HEADER, 0);
  xdebug_str_add(message, xml_message.d, 0);
  xdebug_str_addl(message, "\0", 1, 0);
  xdebug_str_dtor(xml_message);

  // Write the message
  write(m_socket, message->d, message->l);
  xdebug_str_ptr_dtor(message);
}

///////////////////////////////////////////////////////////////////////////////
// Commands

// Initial size of the input buffer + how much to expand it
#define INPUT_BUFFER_INIT_SIZE 1024
#define INPUT_BUFFER_EXPANSION 2.0

void XDebugServer::doCommandLoop() {
  while (m_status == Status::BREAK ||
         m_status == Status::STARTING ||
         m_status == Status::STOPPING) {
    // TODO(#4489053) Respond & Implement
    readInput();
    m_status = Status::RUNNING;
  }
}

void XDebugServer::readInput() {
  size_t bytes_read = 0;
  do {
    size_t bytes_left = m_bufferSize - bytes_read;

    // Expand if we need to
    if (bytes_left == 0) {
      m_bufferSize = (m_bufferSize == 0) ?
        INPUT_BUFFER_INIT_SIZE : m_bufferSize * INPUT_BUFFER_EXPANSION;
      bytes_left = m_bufferSize - bytes_read;
      m_buffer = (char*) smart_realloc(m_buffer, m_bufferSize);
    }

    // Read into the buffer
    size_t res = recv(m_socket, (void*) &m_buffer[bytes_read], bytes_left, 0);
    if (res < 0) {
      return;
    }
    bytes_read += res;
  } while (m_buffer[bytes_read - 1] != '\0');
}

////////////////////////////////////////////////////////////////////////////////
// Server Status

const char* XDebugServer::getStatusString(Status status) {
  switch (status) {
    case Status::STARTING: return "starting";
    case Status::STOPPING: return "stopping";
    case Status::STOPPED: return "stopped";
    case Status::RUNNING: return "running";
    case Status::BREAK: return "break";
    case Status::DETACHED: return nullptr;
    default: throw Exception("Invalid xdebug server status");
  }
}

const char* XDebugServer::getReasonString(Reason reason) {
  switch (reason) {
    case Reason::OK: return "ok";
    case Reason::ERROR: return "error";
    case Reason::ABORTED: return "aborted";
    case Reason::EXCEPTION: return "exception";
    default: throw Exception("Invalid xdebug server reason");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
