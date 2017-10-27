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

module WithToken(Token: Lexable_token_sig.LexableToken_S) = struct
  module Lexer = Full_fidelity_lexer.WithToken(Token)
  include Lexer
  let next_token = Lexer.next_token_in_type
end (* WithToken *)
