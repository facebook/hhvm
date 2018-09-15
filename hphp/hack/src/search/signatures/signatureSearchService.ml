(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
module Index = SignatureSearchIndex
module Parser = SignatureSearchParser

type indexable_type =
  | ITprim of Nast.tprim
  | ITapply of string
  | IToption of indexable_type
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

let index: Index.t = Index.make ()

let add_backslash type_name =
  if type_name <> "" && type_name.[0] = '\\'
  then type_name
  else "\\" ^ type_name

let add_question_mark type_name =
  if type_name <> "" && type_name.[0] = '?'
  then type_name
  else "?" ^ type_name

let rec indexable_type_to_string = function
  | IToption it_type      -> add_question_mark (indexable_type_to_string it_type)
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

(** Convert a type specifier parsed by the signature search query parser to our
    indexable_type representation. *)
let rec type_specifier_to_indexable_type ty_spec =
  match ty_spec with
  | Parser.TSoption type_ -> IToption (type_specifier_to_indexable_type type_)
  | Parser.TSsimple type_ ->
    match type_ with
    | "darray"           -> ITdarray
    | "varray"           -> ITvarray
    | "varray_or_darray" -> ITvarray_or_darray
    | "mixed"            -> ITmixed
    | "nonnull"          -> ITnonnull
    | "dynamic"          -> ITdynamic
    | "void"             -> ITprim Nast.Tvoid
    | "int"              -> ITprim Nast.Tint
    | "bool"             -> ITprim Nast.Tbool
    | "float"            -> ITprim Nast.Tfloat
    | "string"           -> ITprim Nast.Tstring
    | "num"              -> ITprim Nast.Tnum
    | "resource"         -> ITprim Nast.Tresource
    | "arraykey"         -> ITprim Nast.Tarraykey
    | "noreturn"         -> ITprim Nast.Tnoreturn
    | _                  -> ITapply (add_backslash type_) (* Class or interface type *)

(** Convert a decl ty to the simplified representation of types used by the
    signature search service. If the type is not one that the service can index,
    return None. *)
let rec decl_ty_to_indexable_type ty =
  let open Typing_defs in
  match snd ty with
  | Tapply ((_, "\\Awaitable"), [ty]) -> decl_ty_to_indexable_type ty
  | Tapply ((_, type_name), _) -> Some (ITapply type_name)
  | Tprim prim -> Some (ITprim prim)
  | Tdarray _ -> Some ITdarray
  | Tvarray _ -> Some ITvarray
  | Tvarray_or_darray _ -> Some ITvarray_or_darray
  | Tmixed -> Some ITmixed
  | Tnonnull -> Some ITnonnull
  | Tdynamic -> Some ITdynamic
  | Toption ty ->
    begin match decl_ty_to_indexable_type ty with
    | Some ty -> Some (IToption ty)
    | None -> None
    end
  (* All other types are types that we currently don't handle in the index. *)
  | _ -> None

let expand_to_optional_types type_list =
  let opt_ty_list = List.map type_list ~f:(fun type_ -> IToption type_) in
  type_list @ opt_ty_list

let expand_to_supertypes (tcopt:TypecheckerOptions.t) type_ =
  let open Nast in
  let type_list =
    match type_ with
    | ITprim Tint -> [ITprim Tnum; ITprim Tarraykey; ITprim Tint]
    | ITprim Tfloat -> [ITprim Tnum; ITprim Tfloat]
    | ITprim Tstring -> [ITprim Tstring; ITprim Tarraykey]
    | ITapply type_ ->
      begin match Typing_lazy_heap.get_class tcopt type_ with
      | Some class_name ->
        let super_types = SMap.bindings (class_name.Typing_defs.tc_ancestors) in
        let super_types = List.map super_types ~f:(fun (key, _) -> ITapply key) in
        ITapply type_ :: super_types
      | None -> [ITapply type_]
      end
    | type_ -> [type_]
  in
  type_list @ (expand_to_optional_types type_list)

let search_term_to_string search_term =
  match search_term with
  | Arity arity -> Printf.sprintf "arity=%d" arity
  | Parameter { position; type_ } ->
    Printf.sprintf "arg%d<:%s" position (indexable_type_to_string type_)
  | Return_type return_type ->
    Printf.sprintf "ret<:%s" (indexable_type_to_string return_type)

let create_search_terms param_types return_types =
  let param_types = List.mapi param_types ~f:(fun i type_ ->
    Parameter {position = i + 1; type_}
  ) in
  let return_types = List.map return_types ~f:(fun type_ -> Return_type type_) in
  let arity = Arity (List.length param_types) in
  arity :: param_types @ return_types

let fun_to_search_terms tcopt fun_name =
  match Typing_lazy_heap.get_fun tcopt fun_name with
  | None -> None
  | Some funs_t ->
    let params = funs_t.Typing_defs.ft_params in
    let param_types = List.map params ~f:(fun fun_param ->
      decl_ty_to_indexable_type fun_param.Typing_defs.fp_type
    ) in

    (* If any param_type is not a type we can index, decl_ty_to_indexable_type will return
       None, so here Option.all will return None. *)
    let param_types = Option.all param_types in
    let return_type = decl_ty_to_indexable_type funs_t.Typing_defs.ft_ret in

    (* Transform return_types into a list, append super_types if applicable
         Append subtypes of primitives num and arraykey *)
    let return_types = Option.map return_type (expand_to_supertypes tcopt) in
    (* If the return type and all param_types are valid (that is, they are all
       types that we can index), we have a valid signature *)
    match param_types, return_types with
    | Some param_types, Some return_type ->
      Some (create_search_terms param_types return_type)
    | _ -> None

let add_function tcopt fun_name =
  Errors.ignore_ (fun () ->
    fun_to_search_terms tcopt fun_name
    |> Option.iter ~f:begin fun search_terms ->
      search_terms
      |> List.map ~f:search_term_to_string
      |> Index.update index fun_name
    end
  )

let build tcopt fileinfos =
  Hh_logger.log "Building Search Index";
  Relative_path.Map.iter fileinfos (fun _ value ->
    let {FileInfo.funs; _ } = value in
    List.iter funs ~f:(fun (pos, fun_name) ->
      let path =
        match pos with
        | FileInfo.Full pos -> Pos.filename pos
        | FileInfo.File (_, path) -> path
      in
      let prefix = Relative_path.prefix path in

      (* Functions that do not belong to the Hack Standard Library are filtered out of
         SignatureSearchService *)
      if prefix = Relative_path.Hhi
      || String_utils.string_starts_with fun_name "\\HH\\Lib\\"
      then add_function tcopt fun_name
      else ()
    )
  );
  Hh_logger.log "Search index is ready"


let query_to_search_terms (tcopt:TypecheckerOptions.t) query =
  let open Parser in
  let open Index in
  let {function_params = params; function_output = ret} = query in
  let query_list = List.filter_mapi params ~f:(fun i parameter ->
    match parameter with
    | QTtype ty_spec ->
      let type_ = type_specifier_to_indexable_type ty_spec in
      let types = expand_to_supertypes tcopt type_ in
      Some (Or (List.map types ~f:(fun type_ ->
        let term = Parameter {position = i + 1; type_} in
        Term (search_term_to_string term)
      )))
    | QTwildcard -> None
  ) in

  let query_list = match ret with
    | QTwildcard -> query_list
    | QTtype ty_spec ->
      let type_ = type_specifier_to_indexable_type ty_spec in
      Term (search_term_to_string (Return_type type_)) :: query_list
  in

  let arity = Arity (List.length params) in
  let query_list = Term (search_term_to_string arity) :: query_list in
  And query_list

let go tcopt query =
  Errors.ignore_ (fun () ->
    let keys = query_to_search_terms tcopt query in
    let results = Index.get index keys in
    List.filter_map results ~f:(fun fun_name ->
      let open Option.Monad_infix in
      Naming_heap.FunPosHeap.get fun_name
      >>= function
      | FileInfo.File (_, fn) ->
        Parser_heap.find_fun_in_file tcopt fn fun_name
        >>| fun fun_ ->
        let pos = fst fun_.Ast.f_name in
        {
          SearchUtils.name = fun_name;
          pos;
          result_type = HackSearchService.Function;
        }
      | FileInfo.Full pos ->
        Some {
          SearchUtils.name = fun_name;
          pos;
          result_type = HackSearchService.Function;
        }
    )
  )
