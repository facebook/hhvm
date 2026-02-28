// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "hphp/runtime/ext/decl/exception.h"

namespace HPHP {
namespace Decl {

DeclExtractionExc::DeclExtractionExc(const std::string& msg)
    : std::runtime_error{msg} {}

DeclExtractionExc::~DeclExtractionExc() = default;

} // namespace Decl
} // namespace HPHP
