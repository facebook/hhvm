(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val pp_tparam : Format.formatter -> ('a, 'b) Aast.tparam -> unit

val pp_tparams : Format.formatter -> ('a, 'b) Aast.tparam list -> unit

val pp_hint : is_ctx:bool -> Format.formatter -> Aast.hint -> unit

val pp_user_attrs :
  Format.formatter -> ('a, 'b) Aast.user_attribute list -> unit

val pp_type_hint :
  is_ret_type:bool -> Format.formatter -> 'a Aast.type_hint -> unit

val any_type_name : string
