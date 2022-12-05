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

val pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t
