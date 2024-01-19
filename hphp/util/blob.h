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

#include <cstddef>
#include <cstdint>

namespace HPHP {

namespace Blob {

struct Bounds {
  size_t offset = 0;
  size_t size = 0;

  template<class SerDe> void serde(SerDe& sd) {
    sd(offset)(size);
  }
};

template <typename KeyCompare>
struct HashMapIndex {
  using Bucket = uint32_t;
  constexpr static size_t BucketSize = sizeof(Bucket);

  HashMapIndex(Bounds indexBounds, Bounds dataBounds)
    : indexBounds(indexBounds), dataBounds(dataBounds) {
    if (indexBounds.size < BucketSize) {
      size = 0;
    } else {
      size = (indexBounds.size / BucketSize) - 1;
    }
  }
  HashMapIndex(): HashMapIndex<KeyCompare>(Bounds { 0, 0 }, Bounds { 0, 0 })
    {}

  size_t size;
  Bounds indexBounds;
  Bounds dataBounds;
};

struct ListIndex {
  using Bucket = uint32_t;
  constexpr static size_t BucketSize = sizeof(Bucket);

  ListIndex(Bounds indexBounds, Bounds dataBounds)
    : indexBounds(indexBounds), dataBounds(dataBounds) {
    if (indexBounds.size < BucketSize) {
      size = 0;
    } else {
      size = (indexBounds.size / BucketSize) - 1;
    }
  }
  ListIndex(): ListIndex(Bounds { 0, 0 }, Bounds { 0, 0 }) {}

  size_t size;
  Bounds indexBounds;
  Bounds dataBounds;
};

}

}
