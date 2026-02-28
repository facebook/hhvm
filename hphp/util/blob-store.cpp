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

#include "hphp/util/assertions.h"
#include "hphp/util/blob-store.h"

namespace HPHP::BlobStore {

namespace {

StoreFunc s_store;

}

void put(Key key, folly::StringPiece data) {
  if (s_store) s_store(key, folly::IOBuf::copyBuffer(data));
}

void registerStore(StoreFunc store) {
  assertx(!s_store);
  s_store = store;
}

}
