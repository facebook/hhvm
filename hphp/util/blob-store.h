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

#pragma once

#include <folly/io/IOBuf.h>
#include <folly/Range.h>

#include <functional>
#include <string>

namespace HPHP::BlobStore {

/*
 * Storage keys include three components, the bucket and name must be non-empty
 * but the dir parameter is optional. Storage backends are responsible for
 * interpreting keys.
 */
struct Key {
  std::string bucket;
  std::string dir;
  std::string name;
};

/*
 * Write `data' to the blob-store backend. If no backend has been registered
 * no action is taken.
 */
void put(Key key, folly::StringPiece data);

/*
 * Register a storage backend for writing blobs. The backend is responsible for
 * ensuring all writes are flushed before process shutdown.
 *
 * BlobStore::registerStore() should be called once generally during process
 * initialization (e.g. during Extension::moduleInit()).
 */
using StoreFunc = std::function<void(Key, std::unique_ptr<folly::IOBuf>)>;
void registerStore(StoreFunc store);

}
