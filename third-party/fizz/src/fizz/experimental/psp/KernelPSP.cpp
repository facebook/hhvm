#include <fizz/experimental/psp/PSP.h>
#include <folly/futures/Future.h>

namespace fizz::psp {

namespace {
class Impl : public KernelPSP {
 public:
  folly::SemiFuture<SA> rxAssoc(PSPVersion, int) noexcept override {
    return folly::makeSemiFuture<SA>(
        folly::make_exception_wrapper<std::runtime_error>("not implemented"));
  };

  folly::SemiFuture<folly::Unit> txAssoc(const struct SA&, int) noexcept
      override {
    return folly::makeSemiFuture<folly::Unit>(
        folly::make_exception_wrapper<std::runtime_error>("not implemented"));
  }
};
} // namespace
#ifndef FIZZ_PSP_SUPPRESS_MAKE_IMPL
std::shared_ptr<KernelPSP> KernelPSP::make(folly::Executor::KeepAlive<>) {
  return std::make_shared<Impl>();
}
#endif
} // namespace fizz::psp
