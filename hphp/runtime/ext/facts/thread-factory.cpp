/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <folly/executors/thread_factory/InitThreadFactory.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>

#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/ext/facts/thread-factory.h"

namespace HPHP {
namespace Facts {

std::shared_ptr<folly::ThreadFactory>
make_thread_factory(folly::StringPiece name) {
  return std::make_shared<folly::InitThreadFactory>(
      std::make_shared<folly::NamedThreadFactory>(name),
      []() { hphp_thread_init(); },
      []() { hphp_thread_exit(); });
}

} // namespace Facts
} // namespace HPHP
