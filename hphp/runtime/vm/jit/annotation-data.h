/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#pragma once

#include <string>

#include <folly/json/dynamic.h>

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/runtime/base/rds.h"

namespace HPHP::jit {

struct AnnotationData {
  struct InliningDecision {
    InliningDecision(bool wasInlined,
                     Offset offset,
                     const Func* caller,
                     const Func* callee,
                     const std::string& reason)
      : wasInlined(wasInlined)
      , offset(offset)
      , caller(caller)
      , callee(callee)
      , reason(reason)
    {};

    std::string getDecision() const {
      return wasInlined ? "DoInline" : "NoInline";
    }

    std::string getAnnotationStr() const {
      auto const callerName = caller ? caller->fullName()->data() : "(unknown)";
      auto const calleeName = callee ? callee->fullName()->data() : "(unknown)";
      return folly::sformat("BC {}: {} -> {}: {}\n",
                            offset, callerName, calleeName, reason);
    }

    folly::dynamic toDynamic() const {
      auto const callerName = caller ?
        caller->fullName()->data() :
        folly::dynamic();
      auto const calleeName = callee ?
        callee->fullName()->data() :
        folly::dynamic();
      return folly::dynamic::object("wasInlined", wasInlined)
                                   ("offset", offset)
                                   ("caller", callerName)
                                   ("callee", calleeName)
                                   ("reason", reason);
    }

    const bool wasInlined;
    const Offset offset;
    const Func* caller;
    const Func* callee;
    const std::string reason;
  };

  Annotations getInliningAnnotations() const {
    Annotations inliningAnnotations;
    for (auto const& decision : inliningDecisions) {
      inliningAnnotations.emplace_back(decision.getDecision(),
                                       decision.getAnnotationStr());
    }
    return inliningAnnotations;
  };

  Annotations getAllAnnotations() const {
    auto allAnnotations = getInliningAnnotations();
    allAnnotations.insert(allAnnotations.begin(),
                          annotations.begin(),
                          annotations.end());
    return allAnnotations;
  }

  folly::dynamic getInliningDynamic() const {
    folly::dynamic annotations = folly::dynamic::array;
    for (auto const& decision : inliningDecisions) {
      annotations.push_back(decision.toDynamic());
    }
    return annotations;
  }

  void add(std::string label, std::string value) {
    annotations.emplace_back(label, value);
  }

  std::vector<InliningDecision> inliningDecisions;
  Annotations annotations;
};

} // namespace HPHP::jit

