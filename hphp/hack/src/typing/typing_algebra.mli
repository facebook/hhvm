(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

val factorize_common_types :
  locl_ty list -> locl_ty list -> locl_ty list * locl_ty list * locl_ty list
