#include <fizz/experimental/psp/PSP.h>

namespace fizz::psp {

namespace {
class Impl : public KernelPSP {
 public:
  folly::Expected<SA, std::error_code> rxAssoc(PSPVersion, int) noexcept
      override {
    return folly::makeUnexpected<std::error_code>(
        std::make_error_code(std::errc::function_not_supported));
  };

  std::error_code txAssoc(const struct SA&, int) noexcept override {
    return std::make_error_code(std::errc::function_not_supported);
  }
};
} // namespace
#ifndef FIZZ_PSP_SUPPRESS_MAKE_IMPL
std::shared_ptr<KernelPSP> KernelPSP::make() {
  return std::make_shared<Impl>();
}
#endif
} // namespace fizz::psp
