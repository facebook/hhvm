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

#include "hphp/runtime/ext/xdebug/server.h"

#include "hphp/runtime/ext/xdebug/hook.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"
#include "hphp/runtime/ext/xdebug/util.h"
#include "hphp/runtime/ext/xdebug/xdebug_command.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/debugger/debugger_hook_handler.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/network.h"
#include "hphp/util/timer.h"

#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/portability/Sockets.h>

#include <fcntl.h>
#include <sys/types.h>
#include <thread>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

namespace {

/* Valid states of the input parsing state machine. */
enum class ParseState {
  Normal,
  Quoted,
  OptFollows,
  SepFollows,
  ValueFollowsFirstChar,
  ValueFollows,
  SkipChar,
};

using Error  = XDebugError;
using Status = XDebugStatus;
using Reason = XDebugReason;

const StaticString
  s_COOKIE("_COOKIE"),
  s_DBGP_COOKIE("DBGP_COOKIE"),
  s_FILE("file"),
  s_GET("_GET"),
  s_HTTP_X_FORWARDED_FOR("HTTP_X_FORWARDED_FOR"),
  s_LINE("line"),
  s_REMOTE_ADDR("REMOTE_ADDR"),
  s_SCRIPT_FILENAME("SCRIPT_FILENAME"),
  s_SERVER("_SERVER"),
  s_SESSION("XDEBUG_SESSION"),
  s_SESSION_START("XDEBUG_SESSION_START"),
  s_SESSION_STOP("XDEBUG_SESSION_STOP");

/*
 * Get a global value as an array (i.e. $_GET).
 */
Array get_global_array(const String& name) {
  auto const globals = ArrNR(get_global_variables()->asArrayData());
  return globals.asArray().rvalAt(name).toArray();
}

/*
 * Poll a socket seeing if there is any input waiting to be received.
 *
 * Returns true iff there is data to be read.
 */
bool poll_socket(int socket) {
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

}

////////////////////////////////////////////////////////////////////////////////

XDebugServer::XDebugServer(Mode mode)
    : m_pollingThread(this, &XDebugServer::pollSocketLoop)
    , m_requestThread(&TI())
    , m_mode(mode)
{
  // Attempt to open optional log file.
  if (XDEBUG_GLOBAL(RemoteLog).size() > 0) {
    m_logFile = fopen(XDEBUG_GLOBAL(RemoteLog).c_str(), "a");
    if (m_logFile == nullptr) {
      raise_warning("XDebug could not open the remote debug file '%s'.",
                    XDEBUG_GLOBAL(RemoteLog).c_str());
    } else {
      log("Log opened at");
      xdebug_print_timestamp(m_logFile);
      log("\n");
      logFlush();
    }
  }

  // Grab the hostname and port to connect to.
  auto hostname = XDEBUG_GLOBAL(RemoteHost).c_str();
  auto const port = XDEBUG_GLOBAL(RemotePort);
  if (XDEBUG_GLOBAL(RemoteConnectBack)) {
    auto const server = get_global_array(s_SERVER);
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

  auto const fail = [&] {
    destroySocket();
    closeLog();
    // Allows the guarantee that any instance of an xdebug server is valid.
    throw Exception("XDebug Server construction failed");
  };

  // Create the socket.
  auto const status = createSocket(hostname, port);
  if (status == -1) {
    log("E: Could not connect to client. :-(\n");
    fail();
  } else if (status == -2) {
    log("E: Time-out connecting to client. :-(\n");
    fail();
  }

  assert(status == 0);

  // Get the requested handler.
  log("I: Connected to client. :-)\n");
  if (XDEBUG_GLOBAL(RemoteHandler) != "dbgp") {
    log("E: The remote debug handler '%s' is not supported. :-(\n",
        XDEBUG_GLOBAL(RemoteHandler).c_str());
    fail();
  }

  m_pollingThread.start();
}

XDebugServer::~XDebugServer() {
  {
    std::lock_guard<std::recursive_mutex> lock(m_pollingMtx);
    m_pollingState.store(PollingState::Stop);
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
    in = *((struct in_addr*)result.hostbuf.h_addr);
    return true;
  }
  return false;
}

int XDebugServer::createSocket(const char* hostname, int port) {
  // Grab the address info.
  sockaddr sa;
  sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons((unsigned short)port);
  lookupHostname(hostname, address.sin_addr);

  // Create the socket.
  auto const sockfd = socket(address.sin_family, SOCK_STREAM, 0);
  if (sockfd < 0) {
    log("create_debugger_socket(\"%s\", %d) socket: %s\n",
        hostname, port, folly::errnoStr(errno).c_str());
    return -1;
  }

  m_socket = sockfd;

  // Put socket in non-blocking mode so we can use poll() for timeouts.
  fcntl(sockfd, F_SETFL, O_NONBLOCK);

  // Do the connect.
  auto const status = connect(sockfd, (sockaddr*)(&address), sizeof(address));
  if (status < 0) {
    if (errno != EINPROGRESS) {
      return -1;
    }

    double timeout_sec = XDEBUG_GLOBAL(RemoteTimeout);

    // Loop until request has completed or there is an error.
    while (true) {
      pollfd pollfd;
      pollfd.fd = sockfd;
      pollfd.events = POLLIN | POLLOUT;
      pollfd.revents = 0;

      // Wait for a socket event.
      auto const status = poll(&pollfd, 1, (int)(timeout_sec * 1000.0));

      // Timeout.
      if (status == 0) return -2;
      // Error.
      if (status == -1) return -1;

      // Error or hang-up.
      if ((pollfd.revents & POLLERR) || (pollfd.revents & POLLHUP)) return -1;

      // Success.
      if ((pollfd.revents & POLLIN) || (pollfd.revents & POLLOUT)) break;
    }

    // Ensure we're actually connected.
    socklen_t sa_size = sizeof(sa);
    if (getpeername(sockfd, &sa, &sa_size) == -1) {
      return -1;
    }
  }

  // Reset the socket to non-blocking mode and setup socket options
  fcntl(sockfd, F_SETFL, 0);
  long optval = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
  return 0;
}

void XDebugServer::destroySocket() {
  if (m_socket >= 0) {
    close(m_socket);
    m_socket = -1;
  }
}

static bool isAsyncCommand(folly::StringPiece cmdName) {
  return cmdName == "break" ||
    cmdName == "breakpoint_set" ||
    cmdName == "breakpoint_get" ||
    cmdName == "breakpoint_list" ||
    cmdName == "breakpoint_update" ||
    cmdName == "breakpoint_remove";
}

void XDebugServer::pollSocketLoop() {
  while (true) {
    // Take some load off the CPU when polling thread is paused.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // XXX: This can take the lock even when we want to pause the thread.
    std::lock_guard<std::recursive_mutex> lock(m_pollingMtx);

    // We're paused if the request thread is handling commands right now.
    if (m_pollingState.load() == PollingState::Pause) {
      continue;
    }

    if (m_pollingState.load() == PollingState::Stop) {
      log("Polling thread stopping\n");
      break;
    }

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

    std::shared_ptr<XDebugCommand> cmd;
    try {
      log("Polling thread received: %s\n", m_bufferCur);
      cmd = parseCommand();
      const auto& cmdName = cmd->getCommandStr();
      if (!isAsyncCommand(cmdName)) {
        // TODO: emit error response
        // for invalid commands in run state.
        continue;
      }

      log("Received %s command\n", cmdName.c_str());
      addNewCommand(cmd);

      // Force request thread to run in the interpreter, and hit
      // XDebugHook::onOpcode().
      m_requestThread->m_reqInjectionData.setDebuggerIntr(true);
      m_requestThread->m_reqInjectionData.setFlag(DebuggerSignalFlag);
    } catch (const XDebugExn& exn) {
      log("Polling thread got invalid command: %s\n", m_bufferCur);
      sendErrorMessage(cmd, exn);
    }
  }
}

/*
 * Executed by request thread to process async commands from
 * poll thread.
 */
void XDebugServer::processAsyncCommandQueue() {
  std::vector<std::shared_ptr<XDebugCommand>> commandQueue;
  // Scope to limit m_asyncCommandQueue lock time so that
  // async commands can be parsed in parallel with command handling.
  {
    std::lock_guard<std::mutex> guard(m_asyncCommandQueueMtx);
    std::swap(m_asyncCommandQueue, commandQueue);
  }

  for (auto& cmd : commandQueue) {
    // TODO: gracefully handle continuation command.
    // currently we do not allow continuation commands from
    // poll thread but we may in future and should handle this
    // situation.
    processCommand(cmd);
    // Async break command will enter break state.
    if (cmd->getCommandStr() == "break") {
      int line = 0;
      auto transpath = getCurrentFilePath(&line);
      breakpoint(transpath, init_null(), init_null(), line);
    }
  }
}

bool XDebugServer::processCommand(const std::shared_ptr<XDebugCommand>& cmd) {
  auto response = xdebug_xml_node_init("response");
  SCOPE_EXIT { xdebug_xml_node_dtor(response); };

  addXmlns(*response);

  // Try to handle the command.  Possibly send a response.
  bool should_continue = cmd->handle(*response);
  if (cmd->shouldRespond()) {
    sendResponse(*response);
  }
  return should_continue;
}

void XDebugServer::closeLog() {
  if (m_logFile == nullptr) {
    return;
  }

  log("Log closed at ");
  xdebug_print_timestamp(m_logFile);
  log("\n\n");
  logFlush();

  fclose(m_logFile);
  m_logFile = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void XDebugServer::onRequestInit() {
  if (!XDEBUG_GLOBAL(RemoteEnable)) {
    return;
  }

  // Need to turn on debugging regardless of the remote mode in order to capture
  // exceptions/errors.
  if (!DebuggerHook::attach<XDebugHook>()) {
    raise_warning("Could not attach xdebug remote debugger to the current "
                  "thread. A debugger is already attached.");
    return;
  }

  // Grab $_GET, $_COOKIE, and the transport.
  auto const get = get_global_array(s_GET);
  auto cookie = get_global_array(s_COOKIE);
  auto const transport = g_context->getTransport();

  // Need to check $_GET[XDEBUG_SESSION_STOP].  If set, delete the session
  // cookie.
  auto const sess_stop_var = get[s_SESSION_STOP];
  if (!sess_stop_var.isNull()) {
    cookie.set(s_SESSION, init_null());
    if (transport != nullptr) {
      transport->setCookie(s_SESSION, empty_string());
    }
  }

  // Need to check $_GET[XDEBUG_SESSION_START].  If set, store the session
  // cookie with $_GET[XDEBUG_SESSION_START] as the value.
  auto const sess_start_var = get[s_SESSION_START];
  if (sess_start_var.isString()) {
    auto const sess_start = sess_start_var.toString();
    cookie.set(s_SESSION, sess_start);
    if (transport != nullptr) {
      auto expire = XDEBUG_GLOBAL(RemoteCookieExpireTime);
      if (expire > 0) {
        timespec ts;
        Timer::GetRealtimeTime(ts);
        expire += ts.tv_sec;
      }
      transport->setCookie(s_SESSION, sess_start, expire);
    }
  }

  // Remove the artificial memory limit for this request since there is a
  // debugger attached to it.
  MM().setMemoryLimit(std::numeric_limits<int64_t>::max());
}

bool XDebugServer::isNeeded() {
  if (!XDEBUG_GLOBAL(RemoteEnable) || XDEBUG_GLOBAL(RemoteMode) == "jit") {
    return false;
  }
  if (XDEBUG_GLOBAL(RemoteAutostart)) {
    return true;
  }

  // Check $_COOKIE[XDEBUG_SESSION].
  auto const cookie = get_global_array(s_COOKIE);
  return !cookie[s_SESSION].isNull();
}

void XDebugServer::attach(Mode mode) {
  assert(XDEBUG_GLOBAL(Server) == nullptr);
  try {
    XDEBUG_GLOBAL(Server) = new XDebugServer(mode);
    if (!XDEBUG_GLOBAL(Server)->initDbgp()) {
      detach();
    }
  } catch (...) {
    // If we fail to attach to a debugger, then we can choose to wait for an
    // exception to be thrown so we try again, or we can just detach the
    // debugger hook completely.  Leaving the debugger hook attached encurs a
    // large performance cost.
    if (LIKELY(XDEBUG_GLOBAL(RemoteMode) == "req")) {
      DebuggerHook::detach();
    }
    // Fail silently, continue running the request.
  }
}

void XDebugServer::detach() {
  assert(XDEBUG_GLOBAL(Server) != nullptr);
  XDEBUG_GLOBAL(Server)->deinitDbgp();
  delete XDEBUG_GLOBAL(Server);
  XDEBUG_GLOBAL(Server) = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

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

void XDebugServer::addError(xdebug_xml_node& node, const XDebugExn& ex) {
  auto error = xdebug_xml_node_init("error");
  xdebug_xml_add_attribute(error, "code", static_cast<int>(ex.error));
  xdebug_xml_add_child(&node, error);

  auto message = xdebug_xml_node_init("message");
  if (ex.errorMsg.empty()) {
    xdebug_xml_add_text(message,
                        const_cast<char*>(xdebug_error_str(ex.error)), 0);

  } else {
    xdebug_xml_add_text(message, xdebug_sprintf("%s\n%s",
                                                xdebug_error_str(ex.error),
                                                ex.errorMsg.c_str()));
  }

  xdebug_xml_add_child(error, message);
}

void XDebugServer::sendStream(const char* name, const char* bytes, int len) {
  // Casts are necessary due to xml api.
  auto name_str  = const_cast<char*>(name);
  auto bytes_str = const_cast<char*>(bytes);

  auto message = xdebug_xml_node_init("stream");
  SCOPE_EXIT { xdebug_xml_node_dtor(message); };

  addXmlns(*message);
  xdebug_xml_add_attribute(message, "type", name_str);
  xdebug_xml_add_text_ex(message, bytes_str, len, 0, 1);
  sendMessage(*message);
}

bool XDebugServer::initDbgp() {
  // Initialize the status and reason.
  switch (m_mode) {
    case Mode::Req:
      setStatus(Status::Starting, Reason::Ok);
      break;
    case Mode::Jit:
      setStatus(Status::Break, Reason::Error);
      break;
  }

  // Create the response.
  auto response = xdebug_xml_node_init("init");
  addXmlns(*response);

  // Engine info.
  auto child = xdebug_xml_node_init("engine");
  xdebug_xml_add_attribute(child, "version", XDEBUG_VERSION);
  xdebug_xml_add_text(child, XDEBUG_NAME, 0);
  xdebug_xml_add_child(response, child);

  // Author.
  child = xdebug_xml_node_init("author");
  xdebug_xml_add_text(child, XDEBUG_AUTHOR, 0);
  xdebug_xml_add_child(response, child);

  // URL.
  child = xdebug_xml_node_init("url");
  xdebug_xml_add_text(child, XDEBUG_URL, 0);
  xdebug_xml_add_child(response, child);

  // Copyright.
  child = xdebug_xml_node_init("copyright");
  xdebug_xml_add_text(child, XDEBUG_COPYRIGHT, 0);
  xdebug_xml_add_child(response, child);

  // Grab the absolute path of the script filename.
  auto server = get_global_array(s_SERVER);
  auto scriptname_var = server[s_SCRIPT_FILENAME];
  assertx(scriptname_var.isString());
  auto scriptname = scriptname_var.toString().get()->mutableData();
  auto fileuri = xdebug_path_to_url(scriptname);

  // Add attributes to the root init node.
  xdebug_xml_add_attribute_ex(response, "fileuri", fileuri, 0, 1);
  xdebug_xml_add_attribute(response, "language", "PHP");
  xdebug_xml_add_attribute(response, "protocol_version", DBGP_VERSION);
  xdebug_xml_add_attribute(response, "appid", getpid());

  // Add the DBGP_COOKIE environment variable.
  auto const dbgp_cookie = g_context->getenv(s_DBGP_COOKIE);
  if (!dbgp_cookie.empty()) {
    xdebug_xml_add_attribute(response, "session", dbgp_cookie.data());
  }

  // Add the idekey.
  if (XDEBUG_GLOBAL(IdeKey).size() > 0) {
    xdebug_xml_add_attribute(response, "idekey", XDEBUG_GLOBAL(IdeKey).c_str());
  }

  // Sent the response.
  sendMessage(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a response from the client.
  return doCommandLoop();
}

void XDebugServer::deinitDbgp() {
  // Unless we've already stopped, send the shutdown message.
  if (m_status != Status::Stopped) {
    setStatus(Status::Stopping, Reason::Ok);

    // Send the xml shutdown response.
    auto response = xdebug_xml_node_init("response");
    addXmlns(*response);
    addStatus(*response);
    addLastCommandIfAvailable(*response);
    sendResponse(*response);
    xdebug_xml_node_dtor(response);

    // Wait for a response from the client.  Regardless of the command loop
    // result, we exit.
    doCommandLoop();
  }

  req::free(m_buffer);
}

void XDebugServer::addLastCommandIfAvailable(xdebug_xml_node& node) {
  if (!m_lastCommands.empty()) {
    addCommand(node, *m_lastCommands.back());
  }
}

void XDebugServer::sendErrorMessage(
  const std::shared_ptr<XDebugCommand>& cmd,
  const XDebugExn& error
) {
  auto response = xdebug_xml_node_init("response");
  SCOPE_EXIT { xdebug_xml_node_dtor(response); };
  addXmlns(*response);
  if (cmd != nullptr) {
    addCommand(*response, *cmd);
  }
  addError(*response, error);
  sendResponse(*response);
}

void XDebugServer::sendResponse(xdebug_xml_node& xml) {
  sendMessage(xml);
  if (!m_lastCommands.empty()) {
    // The response for last command has been processed.
    m_lastCommands.pop_back();
  }
}

void XDebugServer::sendMessage(xdebug_xml_node& xml) {
  constexpr folly::StringPiece header =
    "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
  constexpr folly::StringPiece delimiter("\0", 1);

  auto const message = xdebug_xml_return_node(&xml);

  log("-> %s\n\n", message.data());
  logFlush();

  // Slice off the last '>' in the closing tag, verify message ends with closing
  // tag.
  auto const slice = message.slice().subpiece(0, message.size() - 1);
  always_assert(slice.endsWith(xml.tag));

  auto const lenStr = folly::to<std::string>(message.size() + header.size());

  auto const makeIovec = [] (folly::StringPiece piece) {
    return iovec { const_cast<char*>(piece.data()), piece.size() };
  };

  std::array<iovec, 5> buffers = {
    makeIovec(lenStr),
    makeIovec(delimiter),
    makeIovec(header),
    makeIovec(message.slice()),
    makeIovec(delimiter)
  };

  folly::writevFull(m_socket, buffers.data(), buffers.size());
}

////////////////////////////////////////////////////////////////////////////////

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
  addLastCommandIfAvailable(*response);

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
    filename_str = xdebug_path_to_url(filename_str); // output file format
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
  sendResponse(*response);
  xdebug_xml_node_dtor(response);

  // Wait for a resonse from the user
  return doCommandLoop();
}

bool XDebugServer::breakpoint(const XDebugBreakpoint& bp,
                              const Variant& message) {
  // If we are detached, short circuit.
  Status status;
  Reason reason;
  getStatus(status, reason);
  if (status == Status::Detached) {
    return true;
  }

  // Initialize the breakpoint message node.
  switch (bp.type) {
    // Add the file/line # for line breakpoints.
    case XDebugBreakpoint::Type::LINE:
      return breakpoint(bp.fileName, init_null(), message, bp.line);
    // Add the exception type and the current line # for exception breakpoints.
    case XDebugBreakpoint::Type::EXCEPTION:
      return breakpoint(init_null(), bp.exceptionName,
                        message, g_context->getLine());
    // Grab the callsite.
    case XDebugBreakpoint::Type::CALL:
    case XDebugBreakpoint::Type::RETURN: {
      auto const callsite = g_context->getCallerInfo();
      return breakpoint(callsite[s_FILE], init_null(),
                        message, callsite[s_LINE].toInt32());
    }
    default:
      throw Exception("Invalid breakpoint type");
  }
}

bool XDebugServer::doCommandLoop() {
  log("Entered command loop");

  bool should_continue = false;

  // Pause the polling thread if it isn't already.  (It might have paused itself
  // if it read a "break" command)
  m_pollingState.store(PollingState::Pause);

  // Unpause the polling thread when we leave the command loop.
  SCOPE_EXIT {
    m_pollingState.store(PollingState::Run);
  };

  std::lock_guard<std::recursive_mutex> lock(m_pollingMtx);

  do {
    // If we are detached, short circuit.
    if (m_status == Status::Detached) {
      return true;
    }

    // If we have no input buffered, read from socket, store into m_buffer.
    // On failure, return.
    if (m_bufferAvail == 0 && !readInput()) {
      return false;
    }

    std::shared_ptr<XDebugCommand> cmd;
    try {
      cmd = parseCommand();
      should_continue = processCommand(cmd);
    } catch (const XDebugExn& exn) {
      sendErrorMessage(cmd, exn);
    }
  } while (!should_continue);

  return true;
}

bool XDebugServer::readInput() {
  // Initial size of the input buffer + how much to expand it.
  auto constexpr INPUT_BUFFER_INIT_SIZE = 1024;
  auto constexpr INPUT_BUFFER_EXPANSION = 2.0;

  assert(m_bufferAvail == 0);
  size_t bytes_read = 0;
  do {
    size_t bytes_left = m_bufferSize - bytes_read;
    // Expand if we need to
    if (bytes_left == 0) {
      m_bufferSize = (m_bufferSize == 0) ?
        INPUT_BUFFER_INIT_SIZE : m_bufferSize * INPUT_BUFFER_EXPANSION;
      bytes_left = m_bufferSize - bytes_read;
      m_buffer = (char*) req::realloc_noptrs(m_buffer, m_bufferSize);
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

std::shared_ptr<XDebugCommand> XDebugServer::parseCommand() {
  assert(m_bufferAvail > 0);

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

  // Create the command from the command string and args.
  auto cmd = XDebugCommand::fromString(*this, cmd_str, args);
  m_lastCommands.emplace_back(cmd);
  return cmd;
}

void XDebugServer::parseInput(folly::StringPiece in, String& cmd, Array& args) {
  // Always start with a blank array.
  args = Array::Create();

  // Find the first space in the command.  Everything before is assumed to be
  // the command string.
  auto ptr = strchr(const_cast<char*>(in.data()), ' ');
  if (ptr != nullptr) {
    size_t size = ptr - in.data();
    cmd = String::attach(StringData::Make(in.data(), size, CopyString));
  } else if (!in.empty() && in[0] != '\0') {
    // There are no spaces, the entire string is the command.
    cmd = String::attach(StringData::Make(in));
    return;
  } else {
    throw_exn(Error::Parse);
  }

  // Loop starting after the space until the end of the string.
  char opt;
  bool escaped = false;
  char* value = nullptr;
  auto state = ParseState::Normal;
  do {
    ptr++;
    switch (state) {
      // A new option which is prefixed with "-" is expected.
      case ParseState::Normal:
        if (*ptr != '-') {
          throw_exn(Error::Parse);
        } else {
          state = ParseState::OptFollows;
        }
        break;
      // The option key follows.
      case ParseState::OptFollows:
        opt = *ptr;
        state = ParseState::SepFollows;
        break;
      // Expect a " " separator to follow.
      case ParseState::SepFollows:
        if (*ptr != ' ') {
          throw_exn(Error::Parse);
        } else {
          state = ParseState::ValueFollowsFirstChar;
          value = ptr + 1;
        }
        break;
      // Expect the option value's first character to follow. This character
      // could be either '"'or '-'.
      case ParseState::ValueFollowsFirstChar:
        if (*ptr == '"' && opt != '-') {
          value = ptr + 1;
          state = ParseState::Quoted;
        } else {
          state = ParseState::ValueFollows;
        }
        break;
      // The option's value should follow.
      case ParseState::ValueFollows:
        if ((*ptr == ' ' && opt != '-') || *ptr == '\0') {
          if (args[opt].isNull()) {
            size_t size = ptr - value;
            auto val_data = StringData::Make(value, size, CopyString);
            args.set(opt, String::attach(val_data));
            state = ParseState::Normal;
          } else {
            throw_exn(Error::DupArg);
          }
        }
        break;
      // State when we are within a quoted string.
      case ParseState::Quoted:
        // if the quote is escaped, remain in ParseState::Quoted.  This will
        // also handle other escaped chars, or an instance of an escaped slash
        // followed by a quote: \\"
        if (*ptr == '\\') {
          escaped = !escaped;
          break;
        } else if (*ptr != '"') {
          break;
        } else if (escaped) {
          escaped = false;
          break;
        }

        // Need to strip slashes before adding option.
        if (args[opt].isNull()) {
          size_t size = ptr - value;
          auto val_data = StringData::Make(value, size, CopyString);
          args.set(opt, HHVM_FN(stripcslashes)(String::attach(val_data)));
          state = ParseState::SkipChar;
        } else {
          throw_exn(Error::DupArg);
        }
        break;
      // Do nothing.
      case ParseState::SkipChar:
        state = ParseState::Normal;
        break;
    }
  } while (*ptr != '\0');
}

////////////////////////////////////////////////////////////////////////////////
}
