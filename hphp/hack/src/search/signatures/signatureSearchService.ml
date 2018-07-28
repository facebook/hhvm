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

let indexable_type_to_string = function
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
  | Parser.TSoption type_ -> type_specifier_to_indexable_type type_
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
    | _                  -> ITapply type_ (* Class or interface type *)

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
  | Toption ty -> decl_ty_to_indexable_type ty
  (* All other types are types that we currently don't handle in the index. *)
  | _ -> None

let search_term_to_string = function
  | Arity arity ->
    Printf.sprintf "arity=%d" arity
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

    (* If the return type and all param_types are valid (that is, they are all
       types that we can index), we have a valid signature *)
    match param_types, return_type with
    | Some param_types, Some return_type ->
      Some (create_search_terms param_types return_type)
    | _ -> None

let index: Index.t = Index.make ()

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

let query_to_search_terms (query: Parser.signature_query) : search_term list =
  let Parser.{function_params = params; function_output = ret} = query in
  let arity = Arity (List.length params) in
  let param_keys = List.filter_mapi params ~f:(fun i param ->
    match param with
    | Parser.QTwildcard -> None
    | Parser.QTtype type_ ->
      let type_ = type_specifier_to_indexable_type type_ in
      Some (Parameter {position = i + 1; type_})
  ) in

  let return_type =
    match ret with
    | Parser.QTwildcard -> []
    | Parser.QTtype type_ ->
      let type_ = type_specifier_to_indexable_type type_ in
      [Return_type type_]
  in
  [arity] @ return_type @ param_keys

let search (index: Index.t) (search_terms: search_term list) : string list =
  let search_terms = List.map search_terms ~f:search_term_to_string in
  let results = List.map search_terms ~f:(Index.get index) in
  match results with
  | [] -> []
  | hd :: tl ->
    List.fold tl ~init:hd ~f:SSet.inter
    |> SSet.elements

let go tcopt query =
  Errors.ignore_ (fun () ->
    let keys = query_to_search_terms query in
    let results = search index keys in
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
