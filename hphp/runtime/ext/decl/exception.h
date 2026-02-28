// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <stdexcept>
#include <string>

namespace HPHP {
namespace Decl {

struct DeclExtractionExc : std::runtime_error {
  explicit DeclExtractionExc(const std::string& msg);
  DeclExtractionExc(const DeclExtractionExc&) = default;
  DeclExtractionExc(DeclExtractionExc&&) noexcept = default;
  DeclExtractionExc& operator=(const DeclExtractionExc&) = default;
  DeclExtractionExc& operator=(DeclExtractionExc&&) noexcept = default;
  ~DeclExtractionExc() override;
};

} // namespace Decl
} // namespace HPHP
