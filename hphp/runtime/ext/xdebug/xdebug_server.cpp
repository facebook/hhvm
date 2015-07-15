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

#include "hphp/runtime/ext/xdebug/xdebug_command.h"
#include "hphp/runtime/ext/xdebug/xdebug_hook_handler.h"
#include "hphp/runtime/ext/xdebug/xdebug_utils.h"

#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/util/timer.h"
#include "hphp/util/network.h"

#include <fcntl.h>
#include <poll.h>
#include <thread>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Helpers

using Error  = XDebugError;
using Status = XDebugStatus;
using Reason = XDebugReason;

// Globals
const StaticString
  s_SERVER("_SERVER"),
  s_GET("_GET"),
  s_COOKIE("_COOKIE");

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

// Properties of $_SERVER to grab the client addr from
const StaticString
  s_HTTP_X_FORWARDED_FOR("HTTP_X_FORWARDED_FOR"),
  s_REMOTE_ADDR("REMOTE_ADDR");

XDebugServer::XDebugServer(Mode mode)
    : m_pollingThread(this, &XDebugServer::pollSocketLoop)
    , m_requestThread(&TI())
    , m_mode(mode)
{
  // Attempt to open optional log file
  if (XDEBUG_GLOBAL(RemoteLog).size() > 0) {
    m_logFile = fopen(XDEBUG_GLOBAL(RemoteLog).c_str(), "a");
    if (m_logFile == nullptr) {
      raise_warning("XDebug could not open the remote debug file '%s'.",
                    XDEBUG_GLOBAL(RemoteLog).c_str());
    } else {
      log("Log opened at");
      XDebugUtils::fprintTimestamp(m_logFile);
      log("\n");
      logFlush();
    }
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

  m_pollingThread.start();
  return;

// Failure cleanup. A goto is used to prevent duplication
failure:
  destroySocket();
  closeLog();
  // Allows the guarantee that any instance of an xdebug server is valid
  throw Exception("XDebug Server construction failed");
}

XDebugServer::~XDebugServer() {
  {
    std::lock_guard<std::mutex> lock(m_pollingMtx);

    // Set flags to tell polling thread to end.
    m_pausePollingThread.store(false);
    m_stopPollingThread.store(true);
  }

  // Wait until polling thread gets the memo and exits.
  auto const result = m_pollingThread.waitForEnd();
  always_assert(result);

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

/*
 * Poll a socket seeing if there is any input waiting to be received.
 *
 * Returns true iff there is data to be read.
 */
static bool poll_socket(int socket) {
  pollfd pollfd;

  pollfd.fd = socket;
  pollfd.events = POLLIN;
  pollfd.revents = 0;

  // Has timeout set to zero because pollSocketLoop() already does a sleep(1).
  auto const result = poll(&pollfd, 1, 0);

  // poll() timed out, or errored.
  if (result == 0 || result == -1) {
    return false;
  }

  // Error or hangup.
  if ((pollfd.revents & POLLERR) || (pollfd.revents & POLLHUP)) {
    return false;
  }

  // Whether we have data to read.
  return pollfd.revents & POLLIN;
}

void XDebugServer::pollSocketLoop() {
  while (true) {
    // Take some load off the CPU when polling thread is paused.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // XXX: This can take the lock even when we want to pause the thread.
    std::lock_guard<std::mutex> lock(m_pollingMtx);

    // We're paused if the request thread is handling commands right now.
    if (m_pausePollingThread.load()) {
      continue;
    }

    // We've been told to exit.
    if (m_stopPollingThread.load()) {
      log("Polling thread stopping\n");
      break;
    }

    // At this point we know that the request thread is busy running, so we let
    // it enter the jit.
    m_requestThread->m_reqInjectionData.setDebuggerIntr(false);

    if (m_bufferAvail == 0) {
      // Check if there is any input waiting on us.
      if (!poll_socket(m_socket)) {
        continue;
      }

      // Actually get the input from the socket.
      if (!readInput()) {
        log("Polling thread had input but failed to read it\n");
        continue;
      }
    }

    try {
      log("Polling thread received: %s\n", m_bufferCur);
      auto cmd = parseCommand();
      if (cmd->getCommandStr() != "break") {
        delete cmd;
        continue;
      }

      log("Received break command\n");

      // We're going to set the 'break' flag on our XDebugServer, and pause
      // ourselves.  The request thread will unpause us when it exits its
      // command loop.
      m_pausePollingThread.store(true);

      auto const old = m_break.exchange(cmd);
      always_assert(old == nullptr);

      // Force request thread to run in the interpreter, and hit
      // XDebugHookHandler::onOpcode().
      m_requestThread->m_reqInjectionData.setDebuggerIntr(true);
      m_requestThread->m_reqInjectionData.setFlag(DebuggerSignalFlag);
    } catch (const XDebugExn&) {
      log("Polling thread got invalid command: %s\n", m_bufferCur);
      // Can drop the error.
    }
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
const StaticString
  s_SESSION_START("XDEBUG_SESSION_START"),
  s_SESSION_STOP("XDEBUG_SESSION_STOP"),
  s_SESSION("XDEBUG_SESSION");

void XDebugServer::onRequestInit() {
  if (!XDEBUG_GLOBAL(RemoteEnable)) {
    return;
  }

  // Need to turn on debugging regardless of the remote mode in order to
  // capture exceptions/errors
  if (!DebugHookHandler::attach<XDebugHookHandler>()) {
    raise_warning("Could not attach xdebug remote debugger to the current "
                  "thread. A debugger is already attached.");
    return;
  }

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
      int64_t expire = XDEBUG_GLOBAL(RemoteCookieExpireTime);
      if (expire > 0) {
        timespec ts;
        Timer::GetRealtimeTime(ts);
        expire += ts.tv_sec;
      }
      transport->setCookie(s_SESSION, sess_start, expire);
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

void XDebugServer::attach(Mode mode) {
  assert(XDEBUG_GLOBAL(Server) == nullptr);
  try {
    XDEBUG_GLOBAL(Server) = new XDebugServer(mode);
    if (!XDEBUG_GLOBAL(Server)->initDbgp()) {
      detach();
    }
  } catch (...) {
    // Fail silently, continue running the request.
  }
}

void XDebugServer::detach() {
  assert(XDEBUG_GLOBAL(Server) != nullptr);
  XDEBUG_GLOBAL(Server)->deinitDbgp();
  delete XDEBUG_GLOBAL(Server);
  XDEBUG_GLOBAL(Server) = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// Dbgp

// Header for sent messages
#define XML_MSG_HEADER "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"

const StaticString
  s_SCRIPT_FILENAME("SCRIPT_FILENAME"), // Needed $_SERVER variable
  s_DBGP_COOKIE("DBGP_COOKIE"); // Needed $_ENV variable

void XDebugServer::addXmlns(xdebug_xml_node& node) {
  xdebug_xml_add_attribute(&node, "xmlns", "urn:debugger_protocol_v1");
  xdebug_xml_add_attribute(&node, "xmlns:xdebug",
                           "http://xdebug.org/dbgp/xdebug");
}

void XDebugServer::addCommand(xdebug_xml_node& node, const XDebugCommand& cmd) {
  auto const& cmd_str = cmd.getCommandStr();
  auto const& trans_str = cmd.getTransactionId();

  // We don't assume the XDebugCommand will stick around before the node is
  // sent.
  xdebug_xml_add_attribute_dup(&node, "command", cmd_str.c_str());
  xdebug_xml_add_attribute_dup(&node, "transaction_id", trans_str.c_str());
}

void XDebugServer::addStatus(xdebug_xml_node& node) {
  auto const status = xdebug_status_str(m_status);
  auto const reason = xdebug_reason_str(m_reason);
  xdebug_xml_add_attribute(&node, "status", status);
  xdebug_xml_add_attribute(&node, "reason", reason);
}

void XDebugServer::addError(xdebug_xml_node& node, XDebugError code) {
  // Create the error node
  auto error = xdebug_xml_node_init("error");
  xdebug_xml_add_attribute(error, "code", static_cast<int>(code));
  xdebug_xml_add_child(&node, error);

  // Add the error code's error message
  auto message = xdebug_xml_node_init("message");
  xdebug_xml_add_text(message, const_cast<char*>(xdebug_error_str(code)), 0);
  xdebug_xml_add_child(error, message);
}

void XDebugServer::sendStream(const char* name, const char* bytes, int len) {
  // Casts are necessary due to xml api
  auto name_str = const_cast<char*>(name);
  auto bytes_str = const_cast<char*>(bytes);

  auto message = xdebug_xml_node_init("stream");
  addXmlns(*message);
  xdebug_xml_add_attribute(message, "type", name_str);
  xdebug_xml_add_text_ex(message, bytes_str, len, 0, 1);
  sendMessage(*message);
  xdebug_xml_node_dtor(message);
}

bool XDebugServer::initDbgp() {
  // Initialize the status and reason
  switch (m_mode) {
    case Mode::REQ:
      setStatus(Status::Starting, Reason::Ok);
      break;
    case Mode::JIT:
      setStatus(Status::Break, Reason::Error);
      break;
  }
  // Create the response
  auto response = xdebug_xml_node_init("init");
  addXmlns(*response);

  // Add the engine info
  auto child = xdebug_xml_node_init("engine");
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
  auto globals = get_global_variables()->asArrayData();
  Variant scriptname_var = globals->get(s_SERVER).toArray()[s_SCRIPT_FILENAME];
  assert(scriptname_var.isString());
  auto scriptname = scriptname_var.toString().get()->mutableData();
  auto fileuri = XDebugUtils::pathToUrl(scriptname);

  // Add attributes to the root init node
  xdebug_xml_add_attribute_ex(response, "fileuri", fileuri, 0, 1);
  xdebug_xml_add_attribute(response, "language", "PHP");
  xdebug_xml_add_attribute(response, "protocol_version", DBGP_VERSION);
  xdebug_xml_add_attribute(response, "appid", getpid());

  // Add the DBGP_COOKIE environment variable
  const String dbgp_cookie = g_context->getenv(s_DBGP_COOKIE);
  if (!dbgp_cookie.empty()) {
    xdebug_xml_add_attribute(response, "session", dbgp_cookie.data());
  }

  // Add the idekey
  if (XDEBUG_GLOBAL(IdeKey).size() > 0) {
    xdebug_xml_add_attribute(response, "idekey", XDEBUG_GLOBAL(IdeKey).c_str());
  }

  // Sent the response
  sendMessage(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a response from the client
  return doCommandLoop();
}

void XDebugServer::deinitDbgp() {
  // Unless we've already stopped, send the shutdown message
  if (m_status != Status::Stopped) {
    setStatus(Status::Stopping, Reason::Ok);

    // Send the xml shutdown response
    auto response = xdebug_xml_node_init("response");
    addXmlns(*response);
    addStatus(*response);
    if (m_lastCommand != nullptr) {
      addCommand(*response, *m_lastCommand);
    }
    sendMessage(*response);
    xdebug_xml_node_dtor(response);

    // Wait for a response from the client. Regardless of the command loop
    // result, we exit.
    doCommandLoop();
  }

  // Free the input buffer & the last command
  req::free(m_buffer);
  delete m_lastCommand;
}

void XDebugServer::sendMessage(xdebug_xml_node& xml) {
  // Convert xml to an xdebug_str.
  xdebug_str xml_message = {0, 0, nullptr};
  xdebug_xml_return_node(&xml, &xml_message);
  size_t msg_len = xml_message.l + sizeof(XML_MSG_HEADER) - 1;

  // Log the message
  log("-> %s\n\n", xml_message.d);
  logFlush();

  StringBuffer buf;
  buf.append(static_cast<int64_t>(msg_len));
  buf.append('\0');
  buf.append(XML_MSG_HEADER);
  buf.append(xml_message.d);
  buf.append('\0');

  write(m_socket, buf.data(), buf.size());
  return;
}

///////////////////////////////////////////////////////////////////////////////
// Commands

// elements in ExecutionContext::getCallerInfo
const StaticString
  s_FILE("file"),
  s_LINE("line");

bool XDebugServer::breakpoint(const Variant& filename,
                              const Variant& exception,
                              const Variant& message,
                              int line) {
  log("Hit breakpoint at %s:%d", filename.toString().data(), line);
  setStatus(Status::Break, Reason::Ok);

  // Initialize the response node
  auto response = xdebug_xml_node_init("response");
  addXmlns(*response);
  addStatus(*response);
  if (m_lastCommand != nullptr) {
    addCommand(*response, *m_lastCommand);
  }

  // Grab the c strings
  auto to_c_str = [] (const Variant& var) {
    return !var.isString() ? nullptr : var.toString().data();
  };

  auto filename_str = to_c_str(filename);
  auto exception_str = to_c_str(exception);
  auto message_str = to_c_str(message);
  auto line_str = xdebug_sprintf("%d", line);

  // Create the message node
  auto msg = xdebug_xml_node_init("xdebug:message");
  xdebug_xml_add_attribute_ex(msg, "lineno", line_str, 0, 1);
  if (filename_str != nullptr) {
    filename_str = XDebugUtils::pathToUrl(filename_str); // output file format
    xdebug_xml_add_attribute_ex(
      msg,
      "filename",
      filename_str,
      0 /* freeAttr */,
      1 /* freeVal */
    );
  }
  if (exception_str != nullptr) {
    xdebug_xml_add_attribute(msg, "exception", exception_str);
  }
  if (message_str != nullptr) {
    xdebug_xml_add_text(msg, message_str, 0);
  }

  // Add the message node then send the response
  xdebug_xml_add_child(response, msg);
  sendMessage(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a resonse from the user
  return doCommandLoop();
}

bool XDebugServer::breakpoint(const XDebugBreakpoint& bp,
                              const Variant& message) {
  // If we are detached, short circuit
  Status status; Reason reason;
  getStatus(status, reason);
  if (status == Status::Detached) {
    return true;
  }

  // Initialize the breakpoint message node
  switch (bp.type) {
    // Add the file/line # for line breakpoints
    case XDebugBreakpoint::Type::LINE:
      return breakpoint(bp.fileName, init_null(), message, bp.line);
    // Add the exception type and the current line # for exception breakpoints
    case XDebugBreakpoint::Type::EXCEPTION:
      return breakpoint(init_null(), bp.exceptionName,
                        message, g_context->getLine());
    // Grab the callsite
    case XDebugBreakpoint::Type::CALL:
    case XDebugBreakpoint::Type::RETURN: {
      Array callsite = g_context->getCallerInfo();
      return breakpoint(callsite[s_FILE], init_null(),
                        message, callsite[s_LINE].toInt32());
    }
    default:
      throw Exception("Invalid breakpoint type");
  }
}

// Initial size of the input buffer + how much to expand it
#define INPUT_BUFFER_INIT_SIZE 1024
#define INPUT_BUFFER_EXPANSION 2.0

bool XDebugServer::doCommandLoop() {
  log("Entered command loop");

  bool should_continue = false;

  // Pause the polling thread if it isn't already. (It might have paused itself
  // if it read a "break" command)
  m_pausePollingThread.store(true);

  // Unpause the polling thread when we leave the command loop.
  SCOPE_EXIT {
    m_pausePollingThread.store(false);
  };

  std::lock_guard<std::mutex> lock(m_pollingMtx);

  do {
    // If we are detached, short circuit
    if (m_status == Status::Detached) {
      return true;
    }

    // If we have no input buffered, read from socket, store into m_buffer.
    // On failure, return.
    if (m_bufferAvail == 0 && !readInput()) {
      return false;
    }

    // Initialize the response
    auto response = xdebug_xml_node_init("response");
    addXmlns(*response);

    try {
      // Parse the command and store it as the last command
      auto cmd = parseCommand();
      if (m_lastCommand != nullptr) {
        delete m_lastCommand;
      }
      m_lastCommand = cmd;

      // Try to handle the command. Possibly send a response.
      should_continue = cmd->handle(*response);
      if (cmd->shouldRespond()) {
        sendMessage(*response);
      }
    } catch (const XDebugExn& exn) {
      addError(*response, exn.error);
      sendMessage(*response);
    }

    // Free the response node.
    xdebug_xml_node_dtor(response);
  } while (!should_continue);

  return true;
}

bool XDebugServer::readInput() {
  assert(m_bufferAvail == 0);
  size_t bytes_read = 0;
  do {
    size_t bytes_left = m_bufferSize - bytes_read;
    // Expand if we need to
    if (bytes_left == 0) {
      m_bufferSize = (m_bufferSize == 0) ?
        INPUT_BUFFER_INIT_SIZE : m_bufferSize * INPUT_BUFFER_EXPANSION;
      bytes_left = m_bufferSize - bytes_read;
      m_buffer = (char*) req::realloc(m_buffer, m_bufferSize);
    }

    // Read into the buffer
    auto const res = recv(m_socket, &m_buffer[bytes_read], bytes_left, 0);
    if (res <= 0) {
      return false;
    }
    bytes_read += res;
  } while (m_buffer[bytes_read - 1] != '\0');
  m_bufferAvail = bytes_read;
  m_bufferCur = m_buffer;
  return true;
}

XDebugCommand* XDebugServer::parseCommand() {
  assert(m_bufferAvail > 0);

  // Log the passed in command
  log("<- %s\n", m_bufferCur);
  logFlush();

  // Attempt to parse the input.  parseInput will initialize cmd_str and args.
  String cmd_str;
  Array args;

  folly::StringPiece input(m_bufferCur);

  // Bump the current buffer pointer forward *before* calling parseInput, so we
  // don't get stuck in an infinite loop if parseInput throws.
  auto consumed = strlen(m_bufferCur) + 1;
  assert(consumed <= m_bufferAvail);
  m_bufferCur += consumed;
  m_bufferAvail -= consumed;

  parseInput(input, cmd_str, args);

  // Create the command from the command string & args
  return XDebugCommand::fromString(*this, cmd_str, args);
}

void XDebugServer::parseInput(folly::StringPiece in, String& cmd, Array& args) {
  // Always start with a blank array
  args = Array::Create();

  // Find the first space in the command. Everything before is assumed to be the
  // command string
  auto ptr = strchr(const_cast<char*>(in.data()), ' ');
  if (ptr != nullptr) {
    size_t size = ptr - in.data();
    cmd = String::attach(StringData::Make(in.data(), size, CopyString));
  } else if (in[0] != '\0') {
    // There are no spaces, the entire string is the command
    cmd = String::attach(StringData::Make(in.data(), CopyString));
    return;
  } else {
    throw_exn(Error::Parse);
  }

  // Loop starting after the space until the end of the string
  char opt;
  bool escaped = false;
  char* value = nullptr;
  ParseState state = ParseState::NORMAL;
  do {
    ptr++;
    switch (state) {
      // A new option which is prefixed with "-" is expected
      case ParseState::NORMAL:
        if (*ptr != '-') {
          throw_exn(Error::Parse);
        } else {
          state = ParseState::OPT_FOLLOWS;
        }
        break;
      // The option key follows
      case ParseState::OPT_FOLLOWS:
        opt = *ptr;
        state = ParseState::SEP_FOLLOWS;
        break;
      // Expect a " " separator to follow
      case ParseState::SEP_FOLLOWS:
        if (*ptr != ' ') {
          throw_exn(Error::Parse);
        } else {
          state = ParseState::VALUE_FOLLOWS_FIRST_CHAR;
          value = ptr + 1;
        }
        break;
      // Expect the option value's first character to follow. This character
      // could be either '"'or '-'
      case ParseState::VALUE_FOLLOWS_FIRST_CHAR:
        if (*ptr == '"' && opt != '-') {
          value = ptr + 1;
          state = ParseState::QUOTED;
        } else {
          state = ParseState::VALUE_FOLLOWS;
        }
        break;
      // The option's value should follow
      case ParseState::VALUE_FOLLOWS:
        if ((*ptr == ' ' && opt != '-') || *ptr == '\0') {
          if (args[opt].isNull()) {
            size_t size = ptr - value;
            StringData* val_data = StringData::Make(value, size, CopyString);
            args.set(opt, String::attach(val_data));
            state = ParseState::NORMAL;
          } else {
            throw_exn(Error::DupArg);
          }
        }
        break;
      // State when we are within a quoted string
      case ParseState::QUOTED:
        // if the quote is escaped, remain in ParseState::QUOTED.  This
        // will also handle other escaped chars, or an instance of
        // an escaped slash followed by a quote: \\"
        if (*ptr == '\\') {
          escaped = !escaped;
          break;
        } else if (*ptr != '"') {
          break;
        } else if (escaped) {
          escaped = false;
          break;
        }

        // Need to strip slashes before adding option
        if (args[opt].isNull()) {
          size_t size = ptr - value;
          StringData* val_data = StringData::Make(value, size, CopyString);
          args.set(opt, HHVM_FN(stripcslashes)(String::attach(val_data)));
          state = ParseState::SKIP_CHAR;
        } else {
          throw_exn(Error::DupArg);
        }
        break;
      // Do nothing
      case ParseState::SKIP_CHAR:
        state = ParseState::NORMAL;
        break;
    }
  } while (*ptr != '\0');
}

///////////////////////////////////////////////////////////////////////////////
}
