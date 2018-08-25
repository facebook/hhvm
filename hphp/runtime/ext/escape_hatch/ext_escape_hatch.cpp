#include <folly/Chrono.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/socket-event.h"
#include "hphp/runtime/ext/extension.h"

#include "hphp/util/compatibility.h"
#include "hphp/util/logger.h"

using std::shared_ptr;
using folly::AsyncSocket;
using folly::AsyncTimeout;
using folly::EventBase;
using folly::IOBuf;
using folly::IOBufQueue;
using folly::SocketAddress;
using folly::stringPrintf;

#define EH_HEADER_SIZE 8
#define EH_MAX_MSG_SIZE 0xFFFFFFFF
#define LOGGER DLOG(INFO)
#define EH_THREAD_CHECKS 0

namespace HPHP {

/**
 * Escape Hatch: A simple asynchronous IPC mechanism intended for use with a
 * sidecar process. Supports request+response or one-way requests over pooled
 * [unix domain] sockets. Every message in either direction is framed with
 * an 8 byte header:
 * - Magic bytes 'E' then 'H'
 * - A single byte; 0x00 for oneway, or 0x01 for response requested.
 * - Another byte, ignored.
 * - An unsigned 32 bit big endian integer indicating the length of the
 *   payload to come.
 *
 * There are no timeouts, and possibly other gotchas. Careful!
 *
 * Grab some data and let's get the hell outta Dodge.
 */

static const auto optPool = String::FromCStr("pool");
static const auto optPoolSize = String::FromCStr("pool_size");
static const auto optPoolMaxage = String::FromCStr("pool_max_age");
static const auto optUnix = String::FromCStr("unix");
static const auto optOneway = String::FromCStr("one_way");
static const auto optTimeoutMs = String::FromCStr("timeout_ms");

//
// A singleton event loop for EH stuff.
//

struct EscapeHatchEventBase : EventBase {
  EscapeHatchEventBase() : EventBase(), m_thread([&] {
    loopForever();
    LOGGER << "event base terminated!";
  }) {
    waitUntilRunning();
  }

  ~EscapeHatchEventBase() {
    LOGGER << "event base destroyed!";
    terminateLoopSoon();
    m_thread.join();
  }

 private:
  std::thread m_thread;
};
folly::Singleton<EscapeHatchEventBase> eh_asio_event_base;

//
// A thin wrapper around AsyncSocket that tracks age.
//

struct EscapeHatchSocket {
  explicit EscapeHatchSocket(AsyncSocket* s) : 
    m_socket(s),
    m_birth(std::chrono::steady_clock::now()) {
  }
  ~EscapeHatchSocket() {
    m_socket->destroy(); // AsyncSocket destructor is private.
  }
  bool isExpired(double maxage){
    if (maxage <= 0) {
      return false;
    }
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> age = now - m_birth;
    return age > std::chrono::duration<double>(maxage);
  }
  AsyncSocket* m_socket;
  std::chrono::steady_clock::time_point m_birth;
};

//
// A simple thread safe pool.
//
struct EscapeHatchPool {
  explicit EscapeHatchPool() {}
  EscapeHatchSocket* get() {
    Lock lock(m_mutex);
    if (m_q.empty()) {
      return nullptr;
    }
    auto ret = m_q.front();
    m_q.pop_front();
    return ret;
  }

  void put(EscapeHatchSocket* s, int pool_size) {
    Lock lock(m_mutex);
    // If the pool is too big, clean it out.
    while (pool_size > 0 && m_q.size() >= pool_size) {
      delete m_q.front();
      m_q.pop_front();
    }
    m_q.push_back(s);
  }

  std::deque<EscapeHatchSocket*> m_q;
  Mutex m_mutex;
};

// TODO named pools.
static EscapeHatchPool escapeHatchPool;

//
// Where the magic happens.
//
struct EscapeHatchEvent final : AsioExternalThreadEvent {
  explicit EscapeHatchEvent(const String& msg, const Array& opt) {
    LOGGER << "new escape hatch event: " << msg.size();
    m_retries = 0;
    int msi = msg.size();
    if (sizeof(int) > 4 && msi > EH_MAX_MSG_SIZE) {
      appendError(stringPrintf("message too long: %d", msg.size()));
      return;
    }

    m_oneway = opt.exists(optOneway) && opt[optOneway].toBoolean();

    m_timeout_ms = 0;
    if (opt.exists(optTimeoutMs)) {
      m_timeout_ms = opt[optTimeoutMs].toInt64();
    }

    // Allocate a buffer large enough to write the header and payload.
    int wsi = EH_HEADER_SIZE + msi;
    m_buf = IOBuf::create(wsi);
    auto b = m_buf->writableData();
    b[0] = 'E';
    b[1] = 'H';
    if (m_oneway) {
      b[2] = 0;
    } else {
      // TODO nonces.
      b[2] = 1;
    }
    b[3] = 0;
    b[4] = (msi >> 24) & 0xff;
    b[5] = (msi >> 16) & 0xff;
    b[6] = (msi >> 8) & 0xff;
    b[7] = msi & 0xff;
    std::memcpy(b+EH_HEADER_SIZE, msg.c_str(), msi);
    m_buf->append(wsi);

    // Now let's get a socket.
    m_eb = getSingleton<EscapeHatchEventBase>().get();
    m_pool_size = 0;
    m_pool = opt.exists(optPool) && opt[optPool].toBoolean();
    double maxage = 0;
    if (opt.exists(optPoolMaxage)) {
      maxage = opt[optPoolMaxage].toDouble();
    }
    if (m_pool) {
      if (opt.exists(optPoolSize)) {
        m_pool_size = opt[optPoolSize].toInt32();
      }
      do {
        m_ehs = escapeHatchPool.get();
        if (m_ehs == nullptr) {
          break;
        }
        // The pool gave us a socket, check that it isn't expired.
        if (m_ehs->isExpired(maxage)) {
          delete m_ehs;
          continue;
        }
        // Success! Skip the connect callback.
        m_eb->runInEventBaseThread([this] {
          maybeScheduleTimeout();
          connectFin();
        });
        return;
      } while(true);
    }

    if (opt.exists(optUnix)) {
      m_unix = opt[optUnix].toString().toCppString();
      m_eb->runInEventBaseThread([this] {
          maybeScheduleTimeout();
          connect();
        });
      return;
    }
    appendError("no valid connect options");
  }

  std::string err() {
    return m_err;
  }

  void maybeScheduleTimeout() {
    checkIsInEventBaseThread();
    if (m_timeout_ms > 0) {
      LOGGER << "scheduling timeout: " << m_timeout_ms;
      m_timeout_handler = std::make_shared<TimeoutHandler>(m_eb, this);
      m_timeout_handler->scheduleTimeout(m_timeout_ms);
    }
  }

  // Invoked by the constructor or connect callback.
  void connectFin() {
    LOGGER << "connect fin";
    checkIsInEventBaseThread();
    m_ehs->m_socket->write(new WriteCallback(this), m_buf->data(), m_buf->length());
  }

  // Invoked by the write callback.
  void writeFin() {
    LOGGER << "write fin";
    checkIsInEventBaseThread();
    if (m_oneway) {
      m_buf.reset();
      cleanup(true);
    } else {
      m_ehs->m_socket->setReadCB(new ReadCallback(this));
    }
  }

  // Invoked by the read callback.
  void readFin(std::unique_ptr<folly::IOBuf> buf) {
    LOGGER << "read fin";
    checkIsInEventBaseThread();
    m_buf = std::move(buf);
    cleanup(true);
  }

  // Invoked by any of the callbacks.
  void errorFin(const std::string& err) {
    LOGGER << "error fin";
    checkIsInEventBaseThread();
    appendError(stringPrintf("attempt %d on '%s'; %s", m_retries, m_unix.c_str(), err.c_str()));
    cleanup(false);
  }

  // Invoked by the write callback to see if we should reattempt, returns true
  // if we did so. We only want to retry for connections that came from the
  // pool. Pooled connections might have been closed by the server while they
  // were in the pool, and we won't find out until we try to write.
  bool retry() {
    LOGGER << "retry";
    if (!m_pool) {
      // Don't retry connections that were not pooled.
      return false;
    }
    if (m_retries > 0) {
      return false;
    }
    m_retries++;
    delete m_ehs;
    connect();
    return true;
  }

 protected:
  void appendError(const std::string& err) {
    if (m_err.empty()) {
      m_err = "escape_hatch: " + err;
    } else {
      m_err += "; " + err;
    }
  }

  // Invoked by the ASIO framework after we have markAsFinished(); this is
  // where we return data to PHP, or throw an exception. 
  void unserialize(Cell& result) final {
    if (m_err.empty()) {
      // No error, return the contents of our buffer.
      auto const sd = StringData::Make(
        reinterpret_cast<const char*>(m_buf->data()), m_buf->length(), CopyString);
      cellCopy(make_tv<KindOfString>(sd), result);
      return;
    }
    SystemLib::throwExceptionObject(m_err);
  }

 private:
  int  m_timeout_ms;
  int  m_retries;
  int  m_pool_size;
  bool m_oneway;
  bool m_pool;
  std::string m_err;
  std::string m_unix;
  std::unique_ptr<IOBuf> m_buf;
  EventBase * m_eb;
  EscapeHatchSocket* m_ehs;
  class TimeoutHandler;
  std::shared_ptr<TimeoutHandler> m_timeout_handler;

  inline void checkIsInEventBaseThread() {
#if EH_THREAD_CHECKS
    m_eb->checkIsInEventBaseThread();
#endif
  }

  void connect() {
    m_ehs = new EscapeHatchSocket(new AsyncSocket(m_eb));
    SocketAddress addr;
    addr.setFromPath(m_unix);
    // TODO socket options? Recv window?
    m_ehs->m_socket->setSendTimeout(m_timeout_ms);
    m_ehs->m_socket->connect(new ConnectCallback(this), addr, m_timeout_ms);
  }

  void cleanup(bool success) {
    checkIsInEventBaseThread();
    m_timeout_handler.reset();
    // Destroy the read callback, if any.
    auto rcb = m_ehs->m_socket->getReadCallback();
    if (rcb != nullptr) {
      m_ehs->m_socket->setReadCB(nullptr);
      delete rcb;
    }
    if (success && m_pool) {
      escapeHatchPool.put(m_ehs, m_pool_size);
    } else {
      delete m_ehs;
    }
    // Inform the ASIO framework that this event is finished.
    markAsFinished();
  }

  class TimeoutHandler : public AsyncTimeout {
   public:
    TimeoutHandler(EventBase* base, EscapeHatchEvent* ase) : AsyncTimeout(base), m_ase(ase) {
    }

    ~TimeoutHandler() {
      LOGGER << "timeout destroyed";
    }

    void timeoutExpired() noexcept override {
      m_ase->checkIsInEventBaseThread();
      LOGGER << "timeout expired";
      if (m_ase->m_ehs != nullptr) {
        m_ase->appendError(stringPrintf("timeout exceeded (%d ms)", m_ase->m_timeout_ms));
        m_ase->m_ehs->m_socket->closeNow();
      }
    }

    EscapeHatchEvent* m_ase;
    friend class EscapeHatchEvent;
  };

  class ConnectCallback : private AsyncSocket::ConnectCallback {
    ConnectCallback(EscapeHatchEvent* ase) : m_ase(ase) {}
    void connectSuccess() noexcept override {
      m_ase->connectFin();
      delete this;
    }

    void connectErr(const folly::AsyncSocketException& ex) noexcept override {
      m_ase->errorFin(stringPrintf("connect error; %s", ex.what()));
      delete this;
    }

    EscapeHatchEvent* m_ase;
    friend class EscapeHatchEvent;
  };

  class WriteCallback : private AsyncSocket::WriteCallback {
    WriteCallback(EscapeHatchEvent* ase) : m_ase(ase) {}
    void writeSuccess() noexcept override {
      m_ase->writeFin();
      delete this;
    }

    void writeErr(size_t written,
                  const folly::AsyncSocketException& ex)
      noexcept override {
      // If we didn't write the header, maybe retry this whole flow. Otherwise
      // call the error handler.
      if (written > EH_HEADER_SIZE || !m_ase->retry()) {
        m_ase->errorFin(stringPrintf("write error after %d; %s",
              static_cast<int>(written),
              ex.what()));
      }
      delete this;
    }

    EscapeHatchEvent* m_ase;
    friend class EscapeHatchEvent;
  };

  class ReadCallback : public AsyncSocket::ReadCallback {
   public:
    ReadCallback(EscapeHatchEvent* ase) : m_ase(ase), m_read(0), m_want(0) {}

    // Called each time the socket becomes readable; allocate memory.
    void getReadBuffer(void** bufReturn, size_t* lenReturn){
      LOGGER << "getReadBuffer want:" << m_want << " read:" << m_read;
      if (m_want == 0) {
        *bufReturn = m_header + m_read;
        *lenReturn = EH_HEADER_SIZE - m_read;
        return;
      }
      *bufReturn = m_buf->writableTail();
      *lenReturn = m_want - m_buf->length(); 
    }

    // Called each time data has been written into the buffer.
    void readDataAvailable(size_t len) noexcept override {
      LOGGER << "readDataAvailable " << len;
      m_read += len;

      // Are we trying to parse the header?
      if (m_want == 0) {
        if (m_read < EH_HEADER_SIZE) {
          return; // MOAR! READ MOAR FOR HEADER!
        }
        if (m_header[0] != 'E' || m_header[1] != 'H') {
          m_ase->errorFin("read error bad header magic");
          return;
        }
        m_want |= ((int)m_header[4]) << 24;
        m_want |= ((int)m_header[5]) << 16;
        m_want |= ((int)m_header[6]) << 8;
        m_want |= ((int)m_header[7]);
        m_buf = IOBuf::createCombined(m_want);
        if (m_want == 0) {
          m_ase->readFin(std::move(m_buf));
        }
        return; // We read the header on it's own.
      } else {
        m_buf->append(len);
      }

      if (m_read < (m_want + EH_HEADER_SIZE)) {
        return; // MOAR! READ MORE DATA!
      }
      
      // We've read the entire payload.
      m_ase->readFin(std::move(m_buf));
    }

    void readEOF() noexcept override {
       m_ase->errorFin(stringPrintf("read error read;%d want:%d; EOF",
            static_cast<int>(m_read), static_cast<int>(m_want)));
    }

    void readErr(const folly::AsyncSocketException& ex) noexcept override {
      m_ase->errorFin(stringPrintf("read error read;%d want:%d; %s", 
            static_cast<int>(m_read), static_cast<int>(m_want), ex.what()));
    }

    //IOBufQueue m_buf{IOBufQueue::cacheChainLength()};
    std::unique_ptr<IOBuf> m_buf;
    EscapeHatchEvent* m_ase;
    size_t m_read; // Total bytes read.
    size_t m_want; // Remaining bytes wanted after parsing the header.
    uint8_t m_header[EH_HEADER_SIZE];

    friend class EscapeHatchEvent;
  };
};

Object HHVM_FUNCTION(escape_hatch, const String& dest, const Array& opt) {
  auto event = new EscapeHatchEvent(dest, opt);
  auto err = event->err();
  if (!err.empty()) {
    event->abandon();
    SystemLib::throwInvalidArgumentExceptionObject(err);
  }
  return Object{event->getWaitHandle()};
}

struct EscapeHatchExtension : Extension {
  EscapeHatchExtension(): Extension("escape_hatch", "1.0.10") {}

  void moduleInit() override {
    HHVM_FE(escape_hatch);
    loadSystemlib();
  }
} s_escape_hatch_extension;

HHVM_GET_MODULE(escape_hatch);

} // namespace HPHP
