(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Transform the types of special functions whose type is not denotable in hack, e.g. idx *)
val transform_special_fun_ty :
  Typing_defs.decl_fun_type -> Aast.sid -> int -> Typing_defs.decl_fun_type
