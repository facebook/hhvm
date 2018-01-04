/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/vsdebug/transport.h"

namespace HPHP {
namespace VSDEBUG {

DebugTransport::DebugTransport(Debugger* debugger) :
  m_debugger(debugger) {

  m_bufferSize = ReadBufferDefaultSize;
  m_buffer = (char*)malloc(m_bufferSize);
  if (m_buffer == nullptr) {
    // Failed to allocate buffer.
    m_bufferSize = 0;
  }
}

const std::string DebugTransport::wrapOutgoingMessage(
  folly::dynamic& message,
  const char* messageType
) const {
  // TODO: (Ericblue) output messages need to be encoded in UTF-8
  message["type"] = messageType;
  return folly::toJson(message);
}

int DebugTransport::sendToClient(
  folly::dynamic& message,
  const char* messageType
) {
  Lock lock(m_mutex);

  if (m_transportFd == nullptr) {
    return -1;
  }

  const auto wrapped = wrapOutgoingMessage(message, messageType);
  const char* output = wrapped.c_str();

  // Write out the entire string, *including* its terminating NULL character.
  if (write(fileno(m_transportFd), output, strlen(output) + 1) < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Sending message failed:\n%s\nWrite returned %d",
      output,
      errno
    );
    return errno;
  }

  return 0;
}

bool DebugTransport::sendUserMessage(
  const char* message,
  const char* level
) {
  folly::dynamic userMessage = folly::dynamic::object;

  userMessage["category"] = level;
  userMessage["output"] = message;
  return sendEventMessage(userMessage, EventTypeOutput);
}

bool DebugTransport::sendEventMessage(
  folly::dynamic& message,
  const char* eventType
) {
  message["event"] = eventType;
  return sendToClient(message, MessageTypeEvent) == 0;
}

int DebugTransport::readMessage(folly::dynamic& message) {
  Lock lock(m_mutex);

  if (m_transportFd == nullptr) {
    return -1;
  }

  while (true) {
    // If there are any complete messages in the buffer, process it first.
    if (m_bufferPosition > 0 && processMessage(message)) {
      return 0;
    }

    // Increase buffer size if we're out of space.
    if (m_bufferPosition == m_bufferSize) {
      const int result = resizeBuffer();
      if (result != 0) {
        return result;
      }
    }

    const size_t readSize = m_bufferSize - m_bufferPosition;
    const ssize_t result = recv(
      fileno(m_transportFd),
      &m_buffer[m_bufferPosition],
      readSize,
      0);

    if (result < 0) {
      return errno;
    }

    m_bufferPosition += result;
  }
}

bool DebugTransport::processMessage(folly::dynamic& message) {
  bool success = false;

  // Advance through the buffer until we locate the NULL separator between
  // client messages.
  int len = 0;
  while (len < m_bufferPosition && m_buffer[len] != '\0') {
    len++;
  }

  // If we did not reach the end of the available input, and a NULL character
  // was encountered, attempt to parse the message.
  if (len < m_bufferPosition && m_buffer[len] == '\0') {
    try {
      folly::dynamic parsed = folly::parseJson(m_buffer);
      message = parsed;
      success = true;
    } catch(const std::exception& exn) {
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelError,
        "Failed to parse debugger message: %s",
        m_buffer
      );
    }

    // If there's remaining data in the buffer after this message ended,
    // it remains for the next message.
    if (len < m_bufferSize - 1) {
      memmove(m_buffer, &m_buffer[len + 1], len);
    }
  }

  return success;
}

int DebugTransport::resizeBuffer() {
  // TODO: (Ericblue T23099534) One oddly huge message could cause this to
  //    allocate space and keep it. Shrink back down.
  const size_t newSize = m_bufferSize * 2;
  m_buffer = (char*)realloc(m_buffer, newSize);
  if (m_buffer == nullptr) {
    m_bufferSize = 0;
    return ENOMEM;
  }

  m_bufferSize = newSize;
  return 0;
}

}
}
