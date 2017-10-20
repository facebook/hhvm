(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This lexer is exactly the same as a regular lexer, except that it tokenizes
   C<D<E>> as C < D < E > >, and not C < D < E >> *)

include Full_fidelity_minimal_lexer
let next_token = Full_fidelity_minimal_lexer.next_token_in_type
