#pragma once

#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <folly/Unit.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>
#include <optional>

namespace fizz::psp {

/**
 * PSPVersion corresponds to a defined PSP version. A PSP version corresponds
 * to a specific cipher.
 */
enum class PSPVersion : uint8_t {
  // PSP Header Version 0, AES-GCM-128 (128 bit key)
  VER0 = 0,

  // PSP Header Version 0, AES-GCM-256 (256 bit key)
  VER1 = 1,

  // PSP Header Version 0, AES-GMAC-128 (128 bit key)
  VER2 = 2,

  // PSP Header Version 0, AES-GMAC-256 (256 bit key)
  VER3 = 3,
};

/**
 * PSP Security Association (SA) cryptographic parameters.
 */
struct SA {
  uint8_t psp_version{};
  uint32_t spi{};
  std::vector<uint8_t> key;
};

/**
 * KernelPSP represents an interface to the Linux kernel PSP netlink
 * configuration API.
 */
class KernelPSP {
 public:
  virtual folly::SemiFuture<SA> rxAssoc(
      PSPVersion version,
      int fd) noexcept = 0;
  virtual folly::SemiFuture<folly::Unit> txAssoc(
      const struct SA& sa,
      int fd) noexcept = 0;
  virtual ~KernelPSP();

  /**
   * Constructs an instance of `KernelPSP`.
   *
   * PSP configuration is a potentially blocking operation. `executor` will be
   * used to delegate PSP configuration netlink calls.
   */
  static std::shared_ptr<KernelPSP> make(folly::Executor::KeepAlive<> executor);
};

/**
 * fizz::psp::Callback represents the completion of an asynchronous psp
 * upgrade operation.
 */
class Callback {
 public:
  /**
   * pspSuccess indicates that the `fd` is now a fully associated PSP socket.
   */
  virtual void pspSuccess(folly::NetworkSocket fd) noexcept = 0;

  /**
   * pspError indicates that the upgrade process failed. The transport should
   * not be used.
   */
  virtual void pspError(const folly::exception_wrapper& ew) noexcept = 0;
  virtual ~Callback();
};

/**
 * AsyncPSPUpgradeFrame represents an asynchronous operation to upgrade an
 * established Fizz transport to a PSP based transport.
 *
 * The caller initiates the operation by invoking `start()`. The caller must
 * keep the `AsyncPSPUpgradeFrame` alive until a terminal callback (either
 * `pspSuccess` or `pspError` is invoked).
 *
 * This terminal callback may be invoked synchronously in `start()`.
 *
 * If the frame is destroyed before a terminal callback is invoked, this will
 * cause the `pspError` terminal callback to be invoked during the destruction
 * of the frame (which signals operation completion).
 *
 * A PSP upgrade operation takes a file descriptor backed AsyncTransport
 * (representing a secure, established connection), sends
 * a message (the PSP security association) on this transport, and configures
 * the socket to use PSP, leveraging the Linux PSP Netlink API.
 */
class AsyncPSPUpgradeFrame {
 public:
  virtual ~AsyncPSPUpgradeFrame();
  /**
   * Initiates the asynchronous operation.
   *
   * The operation is considered "in progress" and remains in such state until
   * one of two events occur:
   *  * One of the methods in `Callback` is invoked (signaling operation
   *    completion)
   *
   * This call may synchronously invoke the terminal callbacks (`pspSuccess`,
   * `pspError`)
   */
  virtual void start(Callback* cb) = 0;
};

/**
 * Constructs an asynchronous PSP upgrade operation, using the Fizz PSP Upgrade
 * Protocol V0.
 */
std::unique_ptr<AsyncPSPUpgradeFrame> pspUpgradeV0(
    folly::AsyncTransport* transport,
    PSPVersion version,
    const std::shared_ptr<KernelPSP>& ops);

// Exposed only for testing. Not part of the public API.
namespace detail {
std::optional<size_t> keyLength(uint8_t pspVersion);
std::unique_ptr<folly::IOBuf> encodeTLV(const SA& sa);
folly::Expected<SA, std::runtime_error> tryDecodeSA(
    std::unique_ptr<folly::IOBuf> buf);
std::optional<std::pair<uint8_t, std::unique_ptr<folly::IOBuf>>> readTLV(
    folly::IOBufQueue& readBuf);
} // namespace detail
} // namespace fizz::psp
