(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val transform_special_fun_ty :
  Typing_defs.decl_fun_type -> Nast.sid -> int -> Typing_defs.decl_fun_type
