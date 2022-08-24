<?hh
// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// TODO(T119870250)
// Used as part of typechecking `hphp/runtime/ext/` without builtins; these
// constant values are not representative of the true runtime value.

namespace HH\Lib\_Private\_OS {
const int AF_UNIX = 0;
const int AF_INET = 0;
const int AF_INET6 = 0;
}
