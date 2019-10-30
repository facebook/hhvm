(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decls = {
  classes: Shallow_decl_defs.shallow_class SMap.t;
  funs: Typing_defs.fun_elt SMap.t;
  typedefs: Typing_defs.typedef_type SMap.t;
  consts: Typing_defs.decl_ty SMap.t;
}
[@@deriving show]

val parse_decls : ?contents:string -> Relative_path.t -> (decls, string) result
