<?hh
// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// This function is compiled down to the `WhResult` opcode
function __hhvm_internal_whresult<T>(Awaitable<T> $awaitable)[]: T;
