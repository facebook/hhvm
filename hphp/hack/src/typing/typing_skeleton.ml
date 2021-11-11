(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Generate method skeletons based on the known type.*)

open Hh_prelude
open Typing_defs

(* Given a type declaration, return user-denotable syntax that
   represents it. This is lossy: we use mixed if there's no better
   syntax. *)
let rec of_decl_ty (ty : decl_ty) : string =
  let open Aast in
  match get_node ty with
  | Tprim p ->
    (match p with
    | Tbool -> "bool"
    | Tint -> "int"
    | Tfloat -> "float"
    | Tnum -> "num"
    | Tstring -> "string"
    | Tarraykey -> "arraykey"
    | Tnull -> "null"
    | Tvoid -> "void"
    | Tresource -> "resource"
    | Tnoreturn -> "noreturn")
  | Tmixed
  | Tany _
  | Terr ->
    "mixed"
  | Tnonnull -> "nonnull"
  | Tdynamic -> "dynamic"
  | Tsupportdynamic -> "supportdynamic"
  | Tthis -> "this"
  | Toption ty ->
    (match get_node ty with
    | Typing_defs.Tnonnull -> "mixed"
    | _ -> "?" ^ of_decl_ty ty)
  | Tfun f ->
    let params = List.map f.ft_params ~f:of_fun_param in
    let params =
      match f.ft_arity with
      | Fstandard -> params
      | Fvariadic p -> params @ [of_enforced_ty p.fp_type ^ "..."]
    in
    Printf.sprintf
      "(function(%s): %s)"
      (String.concat ~sep:", " params)
      (of_enforced_ty f.ft_ret)
  | Tgeneric (name, args)
  | Tapply ((_, name), args) ->
    let name = Utils.strip_all_ns name in
    (match args with
    | [] -> name
    | args ->
      let args = List.map args ~f:of_decl_ty in
      Printf.sprintf "%s<%s>" name (String.concat ~sep:", " args))
  | Ttuple args ->
    let args = List.map args ~f:of_decl_ty in
    Printf.sprintf "(%s)" (String.concat ~sep:", " args)
  | Tshape (kind, fields) ->
    let fields =
      TShapeMap.fold (fun key ty acc -> of_shape_field key ty :: acc) fields []
    in
    let fields_with_ellipsis =
      match kind with
      | Closed_shape -> fields
      | Open_shape -> fields @ ["..."]
    in
    Printf.sprintf "shape(%s)" (String.concat ~sep:", " fields_with_ellipsis)
  | Tvar _ -> "mixed"
  | Tunion [] -> "nothing"
  | Tunion _ -> "mixed"
  | Tintersection _ -> "mixed"
  | Tdarray (key_ty, val_ty) ->
    (* Prefer newer dict/vec over darray/varray names. *)
    Printf.sprintf "dict<%s, %s>" (of_decl_ty key_ty) (of_decl_ty val_ty)
  | Tvarray ty -> Printf.sprintf "vec<%s>" (of_decl_ty ty)
  | Tvarray_or_darray (key_ty, val_ty)
  | Tvec_or_dict (key_ty, val_ty) ->
    Printf.sprintf "vec_or_dict<%s, %s>" (of_decl_ty key_ty) (of_decl_ty val_ty)
  | Tlike ty -> of_decl_ty ty
  | Taccess (ty, (_, name)) -> Printf.sprintf "%s::%s" (of_decl_ty ty) name

and of_enforced_ty et : string = of_decl_ty et.et_type

and of_fun_param fp : string = of_enforced_ty fp.fp_type

and of_shape_field (name : tshape_field_name) sft : string =
  let name_s =
    match name with
    | TSFlit_int (_, s) -> s
    | TSFlit_str (_, s) -> Printf.sprintf "\"%s\"" s
    | TSFclass_const ((_, c_name), (_, const_name)) ->
      Printf.sprintf "%s::%s" c_name const_name
  in
  let name_s =
    if sft.sft_optional then
      "?" ^ name_s
    else
      name_s
  in
  Printf.sprintf "%s => %s" name_s (of_decl_ty sft.sft_ty)

let param_source (param : decl_ty fun_param) ~(variadic : bool) : string =
  let name = Option.value param.fp_name ~default:"$_" in
  let ty_s = of_decl_ty param.fp_type.et_type in
  Printf.sprintf
    "%s %s%s"
    ty_s
    (if variadic then
      "..."
    else
      "")
    name

let params_source (arity : decl_ty fun_arity) (params : decl_ty fun_params) :
    string =
  let explicit_params = List.map params ~f:(param_source ~variadic:false) in
  let varaidic_params =
    match arity with
    | Fstandard -> []
    | Fvariadic ty -> [param_source ty ~variadic:true]
  in
  String.concat ~sep:", " (explicit_params @ varaidic_params)

let of_method (name : string) (meth : class_elt) : string =
  let (_, ty_) = deref (Lazy.force meth.ce_type) in
  let (params, return_ty) =
    match ty_ with
    | Tfun ft ->
      (params_source ft.ft_arity ft.ft_params, of_decl_ty ft.ft_ret.et_type)
    | _ -> ("", "mixed")
  in

  Printf.sprintf
    "\n  %s function %s(%s): %s {}\n\n"
    (Typing_defs.string_of_visibility meth.ce_visibility)
    name
    params
    return_ty
