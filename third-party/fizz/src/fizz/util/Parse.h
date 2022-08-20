/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/Range.h>

namespace fizz {

template <class T>
T parse(folly::StringPiece s);

} // namespace fizz

#include <fizz/util/Parse-inl.h>
