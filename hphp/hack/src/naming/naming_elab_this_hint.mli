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
   - Raise errors on invalid hints
   - Validate use of `self`, `this`, `parent` and `static` type hints
   - Validate the arity of certain builtin collections
*)
val pass :
  (Naming_phase_error.t -> unit) -> Naming_phase_env.t Naming_phase_pass.t
