(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module WithSyntax (Syntax : Syntax_sig.Syntax_S) : sig
  val dump_syntax : Syntax.t -> string
end
