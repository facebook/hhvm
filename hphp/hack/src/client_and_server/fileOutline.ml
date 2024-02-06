(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections
open SymbolDefinition
open Aast
module Parser = Full_fidelity_ast

type outline = string SymbolDefinition.t list

let modifiers_to_list ~is_final ~visibility ~is_abstract ~is_static =
  let modifiers =
    match visibility with
    | Public -> [SymbolDefinition.Public]
    | Private -> [SymbolDefinition.Private]
    | Protected -> [SymbolDefinition.Protected]
    | Internal -> [SymbolDefinition.Internal]
  in
  let modifiers =
    if is_final then
      Final :: modifiers
    else
      modifiers
  in
  let modifiers =
    if is_abstract then
      Abstract :: modifiers
    else
      modifiers
  in
  let modifiers =
    if is_static then
      Static :: modifiers
    else
      modifiers
  in
  List.rev modifiers

let get_full_name class_name name =
  match class_name with
  | None -> name
  | Some class_name -> class_name ^ "::" ^ name

let summarize_property class_name var =
  let modifiers =
    modifiers_to_list
      ~is_final:var.cv_final
      ~visibility:var.cv_visibility
      ~is_abstract:var.cv_abstract
      ~is_static:var.cv_is_static
  in
  let (pos, name) = var.cv_id in
  let kind = Property in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    class_name = Some class_name;
    id;
    pos;
    span = var.cv_span;
    modifiers;
    children = None;
    params = None;
    docblock = None;
  }

let maybe_summarize_property class_name ~skip var =
  let (_, name) = var.cv_id in
  if SSet.mem skip name then
    []
  else
    [summarize_property class_name var]

let summarize_class_const class_name cc =
  let (pos, name) = cc.cc_id in
  let (span, modifiers) =
    match cc.cc_kind with
    | CCConcrete (_, p, _) -> (Pos.btw pos p, [])
    | CCAbstract (Some (_, p_default, _)) -> (Pos.btw pos p_default, [Abstract])
    | CCAbstract None -> (pos, [Abstract])
  in
  let kind = ClassConst in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    class_name = Some class_name;
    id;
    pos;
    span;
    modifiers;
    children = None;
    params = None;
    docblock = None;
  }

let modifier_of_fun_kind acc = function
  | Ast_defs.FAsync
  | Ast_defs.FAsyncGenerator ->
    Async :: acc
  | _ -> acc

let modifier_of_param_kind acc = function
  | Ast_defs.Pinout _ -> Inout :: acc
  | Ast_defs.Pnormal -> acc

let summarize_typeconst class_name t =
  let (pos, name) = t.c_tconst_name in
  let kind = Typeconst in
  let id = get_symbol_id kind (Some class_name) name in
  let modifiers =
    match t.c_tconst_kind with
    | TCAbstract _ -> [Abstract]
    | _ -> []
  in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    class_name = Some class_name;
    id;
    pos;
    span = t.c_tconst_span;
    modifiers;
    children = None;
    params = None;
    docblock = None;
  }

let summarize_param param =
  let pos = param.param_pos in
  let name = param.param_name in
  let param_start =
    Option.value_map
      (hint_of_type_hint param.param_type_hint)
      ~f:fst
      ~default:pos
  in
  let param_end =
    Option.value_map param.param_expr ~f:(fun (_, p, _) -> p) ~default:pos
  in
  let modifiers = modifier_of_param_kind [] param.param_callconv in
  let modifiers =
    match param.param_visibility with
    | Some Public -> SymbolDefinition.Public :: modifiers
    | Some Private -> SymbolDefinition.Private :: modifiers
    | Some Protected -> SymbolDefinition.Protected :: modifiers
    | _ -> modifiers
  in
  let kind = Param in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos;
    span = Pos.btw param_start param_end;
    children = None;
    modifiers;
    params = None;
    docblock = None;
  }

let summarize_method class_name m =
  let modifiers = modifier_of_fun_kind [] m.m_fun_kind in
  let modifiers =
    modifiers_to_list
      ~is_final:m.m_final
      ~visibility:m.m_visibility
      ~is_abstract:m.m_abstract
      ~is_static:m.m_static
    @ modifiers
  in
  let params = Some (List.map m.m_params ~f:summarize_param) in
  let name = snd m.m_name in
  let kind = Method in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    class_name = Some class_name;
    id;
    pos = fst m.m_name;
    span = m.m_span;
    modifiers;
    children = None;
    params;
    docblock = None;
  }

(* Parser synthesizes AST nodes for implicit properties (defined in constructor
 * parameter lists. We don't want them to show up in outline view *)
let params_implicit_fields params =
  List.filter_map params ~f:(function
      | { param_visibility = Some _vis; param_name; _ } ->
        Some (String_utils.lstrip param_name "$")
      | _ -> None)

let class_implicit_fields class_ =
  List.concat_map class_.c_methods ~f:(fun m ->
      let (_, name) = m.m_name in
      if String.equal name "__construct" then
        params_implicit_fields m.m_params
      else
        [])

let summarize_class class_ ~no_children =
  let class_name = Utils.strip_ns (snd class_.c_name) in
  let class_name_pos = fst class_.c_name in
  let c_span = class_.c_span in
  let modifiers =
    if class_.c_final then
      [Final]
    else
      []
  in
  let modifiers =
    if Ast_defs.is_classish_abstract class_.c_kind then
      Abstract :: modifiers
    else
      modifiers
  in
  let children =
    if no_children then
      None
    else
      let implicit_props =
        List.fold (class_implicit_fields class_) ~f:SSet.add ~init:SSet.empty
      in
      let acc =
        (* Summarized class properties *)
        List.fold_left
          ~init:[]
          ~f:(fun acc cv ->
            List.fold_right
              ~init:acc
              ~f:List.cons
              (maybe_summarize_property class_name ~skip:implicit_props cv))
          class_.c_vars
      in
      let acc =
        (* Summarized xhp_attrs *)
        List.fold_left
          ~init:acc
          ~f:(fun acc (_, var, _, _) ->
            List.fold_right
              ~init:acc
              ~f:List.cons
              (maybe_summarize_property class_name ~skip:implicit_props var))
          class_.c_xhp_attrs
      in
      let acc =
        (* Summarized consts *)
        List.fold_left
          ~init:acc
          ~f:(fun acc c -> summarize_class_const class_name c :: acc)
          class_.c_consts
      in
      let acc =
        (* Summarized type consts *)
        List.fold_left
          ~init:acc
          ~f:(fun acc tc -> summarize_typeconst class_name tc :: acc)
          class_.c_typeconsts
      in
      let acc =
        (* Summarized methods *)
        List.fold_left
          ~init:acc
          ~f:(fun acc m -> summarize_method class_name m :: acc)
          class_.c_methods
      in
      let sort_by_line summaries =
        let cmp x y = Int.compare (Pos.line x.pos) (Pos.line y.pos) in
        List.sort ~compare:cmp summaries
      in
      Some (sort_by_line acc)
  in
  let kind =
    match class_.c_kind with
    | Ast_defs.Cinterface -> Interface
    | Ast_defs.Ctrait -> Trait
    | Ast_defs.Cenum_class _
    | Ast_defs.Cenum ->
      Enum
    | Ast_defs.(Cclass _) -> Class
  in
  let name = class_name in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    class_name = Some class_name;
    id;
    pos = class_name_pos;
    span = c_span;
    modifiers;
    children;
    params = None;
    docblock = None;
  }

let summarize_typedef tdef =
  let kind = SymbolDefinition.Typedef in
  let name = Utils.strip_ns (snd tdef.t_name) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  let pos = fst tdef.t_name in
  let kind_pos = fst tdef.t_kind in
  let span = Pos.btw pos kind_pos in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_fun fd =
  let f = fd.fd_fun in
  let modifiers = modifier_of_fun_kind [] f.f_fun_kind in
  let params = Some (List.map f.f_params ~f:summarize_param) in
  let kind = SymbolDefinition.Function in
  let name = Utils.strip_ns (snd fd.fd_name) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos = fst fd.fd_name;
    span = f.f_span;
    modifiers;
    children = None;
    params;
    docblock = None;
  }

let summarize_gconst cst =
  let pos = fst cst.cst_name in
  let gconst_start = Option.value_map cst.cst_type ~f:fst ~default:pos in
  let (_, gconst_end, _) = cst.cst_value in
  let kind = GlobalConst in
  let name = Utils.strip_ns (snd cst.cst_name) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos;
    span = Pos.btw gconst_start gconst_end;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_local name span =
  let kind = LocalVar in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos = span;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_module_def md =
  let kind = SymbolDefinition.Module in
  let name = snd md.md_name in
  let full_name = get_full_name None name in
  let id = get_symbol_id kind None name in
  let span = md.md_span in
  let doc_comment = md.md_doc_comment in
  let docblock =
    match doc_comment with
    | None -> None
    | Some dc -> Some (snd dc)
  in
  {
    kind;
    name;
    full_name;
    class_name = None;
    id;
    pos = span;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock;
  }

let outline_ast ast =
  let outline =
    List.filter_map ast ~f:(function
        | Fun f -> Some (summarize_fun f)
        | Class c -> Some (summarize_class c ~no_children:false)
        | _ -> None)
  in
  List.map outline ~f:SymbolDefinition.to_absolute

let should_add_docblock = function
  | Function
  | Class
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | Enum
  | Interface
  | Trait
  | Typeconst
  | Typedef
  | Module ->
    true
  | LocalVar
  | TypeVar
  | Param ->
    false

let add_def_docblock finder previous_def_line def =
  let line = Pos.line def.pos in
  let docblock =
    if should_add_docblock def.kind then
      Docblock_finder.find_docblock finder previous_def_line line
    else
      None
  in
  (line, { def with docblock })

let add_docblocks defs comments =
  let finder = Docblock_finder.make_docblock_finder comments in
  let rec map_def f (acc : int) (def : string SymbolDefinition.t) =
    let (acc, def) = f acc def in
    let (acc, children) =
      Option.value_map
        def.children
        ~f:(fun defs ->
          let (acc, defs) = map_def_list f acc defs in
          (acc, Some defs))
        ~default:(acc, None)
    in
    (acc, { def with children })
  and map_def_list f (acc : int) (defs : string SymbolDefinition.t list) =
    let (acc, defs) =
      List.fold_left
        defs
        ~f:(fun (acc, defs) def ->
          let (acc, def) = map_def f acc def in
          (acc, def :: defs))
        ~init:(acc, [])
    in
    (acc, List.rev defs)
  in
  snd (map_def_list (add_def_docblock finder) 0 defs)

let outline popt content =
  let { Parser_return.ast; comments; _ } =
    let ast =
      Errors.ignore_ (fun () ->
          if Ide_parser_cache.is_enabled () then
            Ide_parser_cache.(
              with_ide_cache @@ fun () ->
              get_ast popt Relative_path.default content)
          else
            let env =
              Parser.make_env
                ~parser_options:popt
                ~include_line_comments:true
                Relative_path.default
            in
            Parser.from_text_with_legacy env content)
    in
    ast
  in
  let result = outline_ast ast in
  add_docblocks result comments

let outline_entry_no_comments
    ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) :
    string SymbolDefinition.t list =
  Ast_provider.compute_ast ~popt ~entry |> outline_ast

let rec print_def ~short_pos indent def =
  let {
    name;
    kind;
    id;
    pos;
    span;
    modifiers;
    children;
    params;
    docblock;
    full_name = _;
    class_name = _;
  } =
    def
  in
  let (print_pos, print_span) =
    if short_pos then
      (Pos.string_no_file, Pos.multiline_string_no_file)
    else
      (Pos.string, Pos.multiline_string)
  in
  Printf.printf "%s%s\n" indent name;
  Printf.printf "%s  kind: %s\n" indent (string_of_kind kind);
  Option.iter id ~f:(fun id -> Printf.printf "%s  id: %s\n" indent id);
  Printf.printf "%s  position: %s\n" indent (print_pos pos);
  Printf.printf "%s  span: %s\n" indent (print_span span);
  Printf.printf "%s  modifiers: " indent;
  List.iter modifiers ~f:(fun x -> Printf.printf "%s " (string_of_modifier x));
  Printf.printf "\n";
  Option.iter params ~f:(fun x ->
      Printf.printf "%s  params:\n" indent;
      print ~short_pos (indent ^ "    ") x);
  Option.iter docblock ~f:(fun x ->
      Printf.printf "%s  docblock:\n" indent;
      Printf.printf "%s\n" x);
  Printf.printf "\n";
  Option.iter children ~f:(fun x -> print ~short_pos (indent ^ "  ") x)

and print ~short_pos indent defs =
  List.iter defs ~f:(print_def ~short_pos indent)

let print_def ?(short_pos = false) = print_def ~short_pos

let print ?(short_pos = false) = print ~short_pos ""
