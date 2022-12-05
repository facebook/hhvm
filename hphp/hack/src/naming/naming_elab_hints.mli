(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This combined elaboration and validation pass will:
   - Canonicalise `Happly(...)` hints coming from the lowerer
   - Canonicalise `varray`, `darray` and `varray_or_darray` type hints
   - Replace invalid hints with `Herr` and raise errors
   - Validate use of `self`, `this`, `parent` and `static` type hints
   - Elaborate missing type parameters to builtin collections to `Hany`
   - Validate the arity of certain builtin collections
*)
module Env : sig
  type t

  val empty : t

  val in_class : ('a, 'b) Aast_defs.class_ -> t

  val in_fun_def : ('a, 'b) Aast_defs.fun_def -> t

  val in_typedef : ('a, 'b) Aast_defs.typedef -> t

  val in_gconst : ('a, 'b) Aast_defs.gconst -> t
end

include Naming_phase_sigs.Elabidation with module Env := Env
