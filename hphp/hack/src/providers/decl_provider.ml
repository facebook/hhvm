(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type fun_key = string
type class_key = string
type typedef_key = string
type gconst_key = string

module Class = Typing_classes_heap.Api

type fun_decl = Typing_defs.decl Typing_defs.fun_type
type class_decl = Class.t
type typedef_decl = Typing_defs.typedef_type
type gconst_decl = Typing_defs.decl Typing_defs.ty * Errors.t

let get_fun = Typing_lazy_heap.get_fun
let get_class = Typing_lazy_heap.get_class
let get_typedef = Typing_lazy_heap.get_typedef
let get_gconst = Typing_lazy_heap.get_gconst
