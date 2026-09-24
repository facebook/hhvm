/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/cont-prof-key.h"

#include <filesystem>

#include <folly/Range.h>

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/assertions.h"
#include "hphp/util/configs/server.h"

namespace HPHP::jit {

namespace {

bool isCanonicalRelativePath(const std::string& path) {
  if (path.empty() ||
      path.find('\\') != std::string::npos ||
      path.find('\0') != std::string::npos) {
    return false;
  }

  auto const parsed = std::filesystem::path{path};
  return
    parsed.is_relative() &&
    parsed.has_filename() &&
    parsed != "." &&
    *parsed.begin() != ".." &&
    parsed.lexically_normal().generic_string() == path;
}

std::optional<std::string> sourceRootRelativeUnitPath(const Unit& unit) {
  auto path = std::filesystem::path{unit.origFilepath()->toCppString()};
  if (path.is_absolute()) {
    auto const sourceRoot = std::filesystem::path{Cfg::Server::SourceRoot};
    if (!sourceRoot.is_absolute()) return std::nullopt;
    path = path.lexically_relative(sourceRoot);
  }

  auto const relativePath = path.generic_string();
  if (!isCanonicalRelativePath(relativePath)) return std::nullopt;
  return relativePath;
}

const StringData* existingStaticString(const std::string& value) {
  if (value.empty()) return nullptr;
  return lookupStaticString(folly::StringPiece{value});
}

Func* resolveCandidate(const ContProfFuncKey& key, Unit& unit) {
  auto const functionName = existingStaticString(key.functionName);
  if (!functionName) return nullptr;

  if (!key.className) {
    if (key.closureContextName) return nullptr;
    return Func::lookup(functionName);
  }

  auto const className = existingStaticString(*key.className);
  if (!className) return nullptr;

  auto const preClass = unit.lookupPreClass(className);
  if (!preClass) return nullptr;

  if (preClass->parent() != c_Closure::classof()->name()) {
    if (key.closureContextName) return nullptr;

    auto const cls = Class::lookup(className);
    return cls ? cls->lookupMethod(functionName) : nullptr;
  }

  Class* context = nullptr;
  if (key.closureContextName) {
    auto const contextName = existingStaticString(*key.closureContextName);
    if (!contextName) return nullptr;

    context = Class::lookup(contextName);
    if (!context) return nullptr;
  }

  // Closure classes and scoped clones are created lazily, so lookup alone
  // cannot reliably resolve them during cold startup.
  auto const closure = Class::defClosure(preClass, /*cache=*/true);
  if (!closure) return nullptr;

  auto const scopedClosure = closure->rescope(context);
  return scopedClosure ? scopedClosure->lookupMethod(functionName) : nullptr;
}

bool isValidSymbol(const std::string& value) {
  return !value.empty() && value.find('\0') == std::string::npos;
}

}

bool isValidContProfFuncKey(const ContProfFuncKey& key) {
  if (!isCanonicalRelativePath(key.resolutionUnitPath)) return false;

  if (key.bytecodeUnitPath &&
      (!isCanonicalRelativePath(*key.bytecodeUnitPath) ||
       *key.bytecodeUnitPath == key.resolutionUnitPath)) {
    return false;
  }

  if (key.bytecodeUnitHash == SHA1{}) return false;
  if (!isValidSymbol(key.functionName)) return false;

  if (key.className && !isValidSymbol(*key.className)) return false;
  if (key.closureContextName && !isValidSymbol(*key.closureContextName)) {
    return false;
  }

  return !key.closureContextName || key.className.has_value();
}

std::optional<ContProfFuncKey> makeContProfFuncKey(const Func& func) {
  if (func.isBuiltin()) return std::nullopt;

  auto const bytecodeUnit = func.unit();
  assertx(bytecodeUnit);

  auto const bytecodePath = sourceRootRelativeUnitPath(*bytecodeUnit);
  if (!bytecodePath) return std::nullopt;

  auto const bytecodeHash = bytecodeUnit->bcSha1();
  if (bytecodeHash == SHA1{}) return std::nullopt;

  auto resolutionUnit = bytecodeUnit;
  std::optional<std::string> className;
  std::optional<std::string> closureContextName;

  if (func.isMethod()) {
    auto const cls = func.implCls();
    if (!cls || !cls->preClass()) return std::nullopt;

    resolutionUnit = cls->preClass()->unit();
    className = cls->preClass()->name()->toCppString();

    if (func.isClosureBody()) {
      // A null cls() represents a closure with no class scope.
      if (auto const context = func.cls()) {
        closureContextName = context->name()->toCppString();
      }
    }
  }

  auto const resolutionPath = sourceRootRelativeUnitPath(*resolutionUnit);
  if (!resolutionPath) return std::nullopt;

  ContProfFuncKey key{
    *resolutionPath,
    *bytecodePath == *resolutionPath
      ? std::optional<std::string>{}
      : bytecodePath,
    bytecodeHash,
    func.name()->toCppString(),
    className,
    closureContextName,
  };
  if (!isValidContProfFuncKey(key)) return std::nullopt;
  return key;
}

Func* resolveContProfFunc(const ContProfFuncKey& key, Unit& resolutionUnit) {
  if (!isValidContProfFuncKey(key)) return nullptr;

  auto const candidate = resolveCandidate(key, resolutionUnit);
  if (!candidate) return nullptr;

  // Name lookup uses process-global tables, so verify that the result has the
  // expected unit paths and bytecode hash.
  auto const candidateKey = makeContProfFuncKey(*candidate);
  return candidateKey && *candidateKey == key ? candidate : nullptr;
}

}
