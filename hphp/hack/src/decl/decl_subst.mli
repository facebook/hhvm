(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type decl_subst = Typing_defs.decl_ty SMap.t

val make_locl :
  'ty Typing_defs.tparam list ->
  Typing_defs.locl_ty list ->
  Typing_defs.locl_ty SMap.t

val make_decl :
  'ty Typing_defs.tparam list -> Typing_defs.decl_ty list -> decl_subst
