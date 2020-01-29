<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\OS;

newtype FileDescriptor = mixed;

const int O_RDONLY = 0;
const int O_WRONLY = 0;
const int O_RDWR= 0;
const int O_NONBLOCK = 0;
const int O_APPEND = 0;
const int O_CREAT/*E < you dropped this*/ = 0;
const int O_TRUNC = 0;
const int O_EXCL = 0;
const int O_SHLOCK = 0;
const int O_EXLOCK = 0;
const int O_NOFOLLOW = 0;
const int O_SYMLINK = 0;
const int O_CLOEXEC = 0;
