(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Empty_regex_pattern

exception Missing_delimiter

exception Invalid_global_option

val type_pattern : Nast.expr -> Typing_defs.locl_ty
