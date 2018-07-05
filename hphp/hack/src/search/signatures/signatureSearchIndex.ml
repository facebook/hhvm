(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type indexable_type =
  | ITprim of Nast.tprim
  | ITapply of string
  | ITdarray
  | ITvarray
  | ITvarray_or_darray
  | ITmixed
  | ITnonnull
  | ITdynamic

type search_term =
  | Arity of int
  | Parameter of {position: int; type_: indexable_type}
  | Return_type of indexable_type

type t = (string, SSet.t) Hashtbl.t

let make () = Hashtbl.create 100

let indexable_type_to_string indexable_type = match indexable_type with
  | ITapply type_name     -> type_name
  | ITdarray              -> "darray"
  | ITvarray              -> "varray"
  | ITvarray_or_darray    -> "varray_or_darray"
  | ITmixed               -> "mixed"
  | ITnonnull             -> "nonnull"
  | ITdynamic             -> "dynamic"
  | ITprim Nast.Tvoid     -> "void"
  | ITprim Nast.Tint      -> "int"
  | ITprim Nast.Tbool     -> "bool"
  | ITprim Nast.Tfloat    -> "float"
  | ITprim Nast.Tstring   -> "string"
  | ITprim Nast.Tnum      -> "num"
  | ITprim Nast.Tresource -> "resource"
  | ITprim Nast.Tarraykey -> "arraykey"
  | ITprim Nast.Tnoreturn -> "noreturn"

let rec get_base_type ty =
  let open Typing_defs in
  match snd ty with
  | Tapply ((_, "\\Awaitable"), [ty]) -> get_base_type ty
  | Tapply ((_, type_name), _) -> Some (ITapply type_name)
  | Tprim prim -> Some (ITprim prim)
  | Tdarray _ -> Some ITdarray
  | Tvarray _ -> Some ITvarray
  | Tvarray_or_darray _ -> Some ITvarray_or_darray
  | Tmixed -> Some ITmixed
  | Tnonnull -> Some ITnonnull
  | Tdynamic -> Some ITdynamic
  | Toption ty -> get_base_type ty
  (* All other types are types that we currently don't handle in the index. *)
  | _ -> None


let search_term_to_string search_term =
  match search_term with
  | Arity arity -> Printf.sprintf "arity=%d" arity
  | Parameter { position; type_ } ->
    Printf.sprintf "arg%d<:%s" position (indexable_type_to_string type_)
  | Return_type return_type ->
    Printf.sprintf "ret<:%s" (indexable_type_to_string return_type)

let create_search_terms param_types return_type =
  let param_types = List.mapi param_types ~f:(fun i param ->
    Parameter {
      position = i + 1;
      type_ = param;
    }
  ) in
  Arity (List.length param_types) :: Return_type return_type :: param_types

let fun_to_search_terms fun_name =
  match Typing_lazy_heap.get_fun GlobalOptions.default fun_name with
  | Some funs_t ->
    let params = funs_t.Typing_defs.ft_params in
    let param_types = List.map params ~f:(fun fun_param ->
      get_base_type fun_param.Typing_defs.fp_type
    ) in

    (* If any param_type is not a type we can index, get_base_type will return
       None, so here Option.all will return None. *)
    let param_types = Option.all param_types in
    let return_type = get_base_type funs_t.Typing_defs.ft_ret in

    (* If the return type and all param_types are valid (that is, they are all
       types that we can index), we have a valid signature *)
    begin match param_types, return_type with
    | Some param_types, Some return_type ->
      let search_terms = create_search_terms param_types return_type in
      Some search_terms
    | _ -> None
    end
  | None -> None


let get index search_term =
  let search_term = search_term_to_string search_term in
  match Hashtbl.find_opt index search_term with
  | Some set -> set
  | None -> SSet.empty

let update index name search_terms =
  List.iter search_terms ~f:(fun term ->
    let functions = SSet.add name (get index term) in
    Hashtbl.replace index (search_term_to_string term) functions
  )
