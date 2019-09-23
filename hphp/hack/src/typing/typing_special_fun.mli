(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val transform_special_fun_ty :
  Decl_provider.fun_decl ->
  Decl_provider.fun_key ->
  int ->
  Decl_provider.fun_decl
