#include <fizz/experimental/psp/PSP.h>
#include <fizz/util/Logging.h>
#include <fmt/format.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <memory>

namespace fizz::psp {

using Tag = uint8_t;
using Length = uint8_t;

constexpr size_t kPSPV0TagLengthSize = sizeof(uint8_t) + sizeof(uint8_t);
constexpr size_t kPSPV0KeyMaxSize = 32;
constexpr size_t kPSPV0ReservedSize = 3;
constexpr size_t kPSPV0SAMsgSize = sizeof(uint32_t) + sizeof(PSPVersion) +
    kPSPV0ReservedSize + kPSPV0KeyMaxSize;

AsyncPSPUpgradeFrame::~AsyncPSPUpgradeFrame() = default;
KernelPSP::~KernelPSP() = default;
Callback::~Callback() = default;

namespace {
enum class FizzPSPProtocol {
  V0 = 0,
};

enum class AsyncPSPV0Tag : uint8_t {
  Error = 0,
  SA = 1,
};

enum class ErrorCode : uint8_t {
  Internal = 0,
  PSPRXAssoc,
};

static std::unique_ptr<folly::IOBuf> makeErrorTLV(uint8_t errorCode) {
  auto buf = folly::IOBuf::create(kPSPV0TagLengthSize + sizeof(uint8_t));
  unsigned char* p = buf->writableData();
  p[0] = static_cast<uint8_t>(AsyncPSPV0Tag::Error);
  p[1] = sizeof(errorCode);
  p[2] = errorCode;

  buf->append(kPSPV0TagLengthSize + sizeof(uint8_t));
  return buf;
}

/**
 * Protocol notes:
 *
 * V0 is a simple upgrade protocol. Both parties send and receive exactly
 * one TLV message.
 *
 * There are two possible messages:
 *     * SA (this communicates the PSP SA to the peer and signals the
 *       intention that no further data shall be sent over the TLS
 *       connection)
 *     * Error (this terminates the upgrade protocol)
 *
 *  struct SA {
 *    uint32_t spi;
 *    uint8_t psp_version;
 *    uint8_t reserved[3];
 *    uint8_t key[32];   // Actual key length is implied by psp_version
 *  }
 *
 *
 *  A message of "Error" is a fixed size message:
 *  struct Error {
 *     uint8_t code;
 *  }
 *  */
class AsyncPSPUpgradeImpl : public ::fizz::psp::AsyncPSPUpgradeFrame,
                            private folly::AsyncReader::ReadCallback,
                            private folly::AsyncWriter::WriteCallback {
 public:
  AsyncPSPUpgradeImpl(
      folly::AsyncTransport* transport,
      FizzPSPProtocol upgradeProtocolVersion,
      PSPVersion pspVersion,
      const std::shared_ptr<KernelPSP>& ops)
      : transport_(transport), version_(pspVersion), ops_(ops) {
    FIZZ_CHECK_EQ(
        static_cast<unsigned>(upgradeProtocolVersion),
        static_cast<unsigned>(FizzPSPProtocol::V0));
    auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
    if (sock) {
      fd_ = sock->getNetworkSocket().toFd();
    }
  }

  AsyncPSPUpgradeImpl(const AsyncPSPUpgradeImpl&) = delete;
  AsyncPSPUpgradeImpl(AsyncPSPUpgradeImpl&&) noexcept = delete;
  AsyncPSPUpgradeImpl& operator=(const AsyncPSPUpgradeImpl&) = delete;
  AsyncPSPUpgradeImpl& operator=(AsyncPSPUpgradeImpl&&) noexcept = delete;

  ~AsyncPSPUpgradeImpl() override {
    if (awaiter_) {
      error("psp upgrade cancelled");
      return;
    }
  }
  void start(Callback* cb) override {
    FIZZ_CHECK_EQ(awaiter_, nullptr);
    awaiter_ = cb;

    return resume();
  }

 private:
  // AsyncState is meant to be held as a weak_ptr in future closures.
  //
  // AsyncPSPUpgradeImpl keeps a strong reference to AsyncState. Consequently
  // if attempting to upgrade the weak_ptr fails, this implies that the
  // original asynchronous operation was cancelled.
  struct AsyncState : public std::enable_shared_from_this<AsyncState> {
    explicit AsyncState(AsyncPSPUpgradeImpl* self) : this_(self) {}

    folly::Try<SA> sa;
    folly::Try<folly::Unit> txAssocResult;

    void resume() {
      return this_->resume();
    }

   private:
    AsyncPSPUpgradeImpl* this_;
  };

  void resume() {
    switch (state_) {
      case State::Init: {
        asyncState_ = std::make_shared<AsyncState>(this);

        transport_->setReadCB(nullptr);
        if (fd_ < 0) {
          return error(
              "psp upgrade failure: invalid socket", ErrorCode::Internal);
        }

        auto evb = transport_->getEventBase();
        evb->dcheckIsInEventBaseThread();

        state_ = State::RxAssocCompleted;
        std::weak_ptr<AsyncState> astate(asyncState_);

        ops_->rxAssoc(version_, fd_)
            .via(evb)
            .thenTry([astate = std::move(astate)](folly::Try<SA>&& sa) {
              if (auto aself = astate.lock()) {
                aself->sa = std::move(sa);
                aself->resume();
                return;
              }
            });
        return;
      }
      case State::RxAssocCompleted: {
        auto& sa = asyncState_->sa;
        if (sa.hasException()) {
          return error(
              fmt::format(
                  "psp upgrade failure: psp_rx_assoc failed: {}",
                  sa.exception().get_exception()->what()),
              ErrorCode::PSPRXAssoc);
        }
        auto saMsg = detail::encodeTLV(*sa);

        state_ = State::WriteSACompleted;
        return transport_->writeChain(this, std::move(saMsg));
      }
      case State::WriteSACompleted: {
        if (ioErr_) {
          return error(
              fmt::format(
                  "psp upgrade failure: write sa failed: {}", ioErr_->what()));
        }

        state_ = State::ReadData;
        return transport_->setReadCB(this);
      }
      case State::ReadData: {
        if (ioErr_) {
          return error(
              fmt::format(
                  "psp upgrade failure: read peer sa I/O error: {}",
                  ioErr_->what()));
        }

        auto tlv = detail::readTLV(readBuf_);
        if (!tlv) {
          return;
        }

        transport_->setReadCB(nullptr);

        auto [tag, msg] = std::move(tlv).value();
        switch (tag) {
          case static_cast<uint8_t>(AsyncPSPV0Tag::SA):
            break;
          case static_cast<uint8_t>(AsyncPSPV0Tag::Error): {
            folly::ByteRange errMsg = msg->coalesce();
            if (errMsg.size() != 1) {
              return error(
                  "psp upgrade failure: peer sent invalid error message");
            } else {
              return error(
                  fmt::format(
                      "psp upgrade failure: peer signaled error: {}",
                      (unsigned)errMsg[0]));
            }
          }
          default:
            return error(
                "psp upgrade failure: peer sent unknown protocol message");
        }
        const auto& sa = detail::tryDecodeSA(std::move(msg));
        if (sa.hasError()) {
          return error(
              fmt::format(
                  "psp upgrade failure: peer invalid sa message: {}",
                  sa.error().what()));
        }

        std::weak_ptr<AsyncState> astate(asyncState_);

        state_ = State::TxAssocCompleted;
        ops_->txAssoc(*sa, fd_)
            .via(transport_->getEventBase())
            .thenTry([astate = std::move(astate)](folly::Try<folly::Unit>&& t) {
              if (auto aself = astate.lock()) {
                aself->txAssocResult = std::move(t);
                aself->resume();
                return;
              }
            });
        return;
      }
      case State::TxAssocCompleted: {
        auto& result = asyncState_->txAssocResult;
        if (result.hasException()) {
          return error(
              fmt::format(
                  "psp upgrade failure: psp_tx_assoc failure: {}",
                  result.exception().get_exception()->what()));
        }
        return prepareForTerminalCallback()->pspSuccess(
            folly::NetworkSocket::fromFd(fd_));
      }
      default:
        FIZZ_LOG(FATAL) << "resumed in invalid state";
    }
  }

  bool isBufferMovable() noexcept override {
    return true;
  }
  void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> data) noexcept override {
    readBuf_.append(std::move(data));
    return resume();
  }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    auto [buf, sz] = readBuf_.preallocate(512, 512);
    *bufReturn = buf;
    *lenReturn = sz;
  }

  void readDataAvailable(size_t len) noexcept override {
    readBuf_.postallocate(len);
    return resume();
  }

  void writeSuccess() noexcept override {
    return resume();
  }

  void writeErr(size_t, const folly::AsyncSocketException& ex) noexcept
      override {
    ioErr_ = ex;
    return resume();
  }
  void readEOF() noexcept override {
    ioErr_ = folly::AsyncSocketException(
        folly::AsyncSocketException::END_OF_FILE, "readEOF()");
    return resume();
  }
  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    ioErr_ = ex;
    return resume();
  }

  void error(std::string msg, std::optional<ErrorCode> sendError = {}) {
    if (sendError && transport_) {
      transport_->writeChain(
          nullptr, makeErrorTLV(static_cast<uint8_t>(*sendError)));
    }
    auto err =
        folly::make_exception_wrapper<std::runtime_error>(std::move(msg));
    return prepareForTerminalCallback()->pspError(err);
  }
  Callback* prepareForTerminalCallback() {
    if (transport_) {
      transport_->setReadCB(nullptr);
      transport_ = nullptr;
    }
    if (asyncState_) {
      asyncState_.reset();
    }
    return std::exchange(awaiter_, nullptr);
  }
  enum class State {
    Init,
    RxAssocCompleted,
    WriteSACompleted,
    ReadData,
    TxAssocCompleted,
  };
  Callback* awaiter_{nullptr};
  folly::AsyncTransport* transport_;
  PSPVersion version_;
  std::shared_ptr<KernelPSP> ops_;
  State state_{State::Init};
  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};
  std::optional<folly::AsyncSocketException> ioErr_;
  int fd_{-1};
  std::shared_ptr<AsyncState> asyncState_;
};
} // namespace

/**
 * Constructs an asynchronous PSP upgrade operation, using the Fizz PSP Upgrade
 * Protocol V0.
 */
std::unique_ptr<AsyncPSPUpgradeFrame> pspUpgradeV0(
    folly::AsyncTransport* transport,
    PSPVersion version,
    const std::shared_ptr<KernelPSP>& ops) {
  return std::make_unique<AsyncPSPUpgradeImpl>(
      transport, FizzPSPProtocol::V0, version, ops);
}

namespace detail {
std::optional<size_t> keyLength(uint8_t pspVersion) {
  switch (pspVersion) {
    case static_cast<uint8_t>(PSPVersion::VER0):
    case static_cast<uint8_t>(PSPVersion::VER2):
      return 16;
    case static_cast<uint8_t>(PSPVersion::VER1):
    case static_cast<uint8_t>(PSPVersion::VER3):
      return 32;
    default:
      return std::nullopt;
  }
}

std::unique_ptr<folly::IOBuf> encodeTLV(const SA& sa) {
  auto buf = folly::IOBuf::create(kPSPV0TagLengthSize + kPSPV0SAMsgSize);
  folly::io::Appender appender(buf.get(), 0);

  appender.write<uint8_t>(static_cast<uint8_t>(AsyncPSPV0Tag::SA));
  appender.write<uint8_t>(static_cast<uint8_t>(kPSPV0SAMsgSize));

  appender.writeBE<uint32_t>(sa.spi);
  appender.write<uint8_t>(sa.psp_version);
  appender.write<uint8_t>(0);
  appender.write<uint8_t>(0);
  appender.write<uint8_t>(0);

  std::array<uint8_t, kPSPV0KeyMaxSize> key{};
  FIZZ_CHECK_LE(sa.key.size(), kPSPV0KeyMaxSize);
  memcpy(key.data(), sa.key.data(), sa.key.size());

  appender.push(key.data(), key.size());
  return buf;
}

folly::Expected<SA, std::runtime_error> tryDecodeSA(
    std::unique_ptr<folly::IOBuf> buf) {
  folly::io::Cursor cursor(buf.get());
  if (!cursor.canAdvance(kPSPV0SAMsgSize)) {
    return folly::makeUnexpected<std::runtime_error>(
        std::runtime_error("invalid message"));
  }
  SA ret;

  ret.spi = cursor.readBE<uint32_t>();
  ret.psp_version = cursor.read<uint8_t>();
  cursor.skip(3);

  // Read remaining bytes as key
  auto keylenOpt = detail::keyLength(ret.psp_version);
  if (!keylenOpt.has_value()) {
    return folly::makeUnexpected<std::runtime_error>(
        std::runtime_error("invalid or unsupported psp version"));
  }
  auto keylen = keylenOpt.value();
  FIZZ_CHECK_LE(keylen, kPSPV0KeyMaxSize);
  ret.key.resize(keylen);
  cursor.pull(ret.key.data(), keylen);

  return ret;
}

std::optional<std::pair<uint8_t, std::unique_ptr<folly::IOBuf>>> readTLV(
    folly::IOBufQueue& readBuf) {
  if (readBuf.chainLength() < kPSPV0TagLengthSize) {
    return std::nullopt;
  }
  folly::io::Cursor cursor(readBuf.front());
  auto tag = cursor.read<uint8_t>();
  auto length = cursor.read<uint8_t>();
  if (!cursor.canAdvance(length)) {
    return std::nullopt;
  }

  // readBuf contains ane entire TLV message. Split off the message.
  readBuf.trimStart(kPSPV0TagLengthSize);
  auto msg = readBuf.split(length);

  return std::make_pair(tag, std::move(msg));
}

} // namespace detail

} // namespace fizz::psp
