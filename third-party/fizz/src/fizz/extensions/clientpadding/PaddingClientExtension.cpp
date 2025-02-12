// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include <fizz/extensions/clientpadding/PaddingClientExtension.h>
#include <fizz/extensions/clientpadding/Types.h>

namespace fizz {
namespace extensions {

PaddingClientExtension::PaddingClientExtension(uint16_t paddingTotalBytes)
    : paddingTotalBytes_(paddingTotalBytes) {}

std::vector<Extension> PaddingClientExtension::getClientHelloExtensions()
    const {
  extensions::Padding padding{paddingTotalBytes_};

  std::vector<Extension> extensions;
  extensions.push_back(encodeExtension(padding));
  return extensions;
}

} // namespace extensions
} // namespace fizz
