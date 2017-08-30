(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * Transforms a full-fidelity syntax tree to generate the required constructs
 * for coroutines. The resulting full-fidelity syntax tree should contain the
 * required state machines for coroutine functions, coroutine functions
 * themselves should be rewritten into stubs that wrap calls into the coroutine
 * state machines, and so forth.
 *
 * The coroutine architecture used by Hack is inspired by the implementation in
 * Kotlin. You can read more about Kotlin's design and implementation details at
 * https://github.com/Kotlin/kotlin-coroutines/blob/master/kotlin-coroutines-informal.md.
 *)
val lower_coroutines : Syntax.t -> Syntax.t
