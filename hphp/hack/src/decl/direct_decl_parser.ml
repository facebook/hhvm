(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decls = {
  classes: Shallow_decl_defs.shallow_class SMap.t;
      [@printer (fun fmt -> SMap.pp Shallow_decl_defs.pp_shallow_class fmt)]
  funs: Typing_defs.fun_elt SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_fun_elt fmt)]
  typedefs: Typing_defs.typedef_type SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_typedef_type fmt)]
  consts: Typing_defs.decl_ty SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_decl_ty fmt)]
}
[@@deriving show]

external parse_decls_ffi : Relative_path.t -> string -> (decls, string) result
  = "parse_decls_ffi"

let parse_decls ?contents relative_path =
  let contents =
    match contents with
    | Some c -> Some c
    | None -> File_provider.get_contents relative_path
  in
  match contents with
  | Some contents -> parse_decls_ffi relative_path contents
  | None ->
    Error
      (Printf.sprintf
         "Could not load file contents for %s"
         (Relative_path.to_absolute relative_path))
