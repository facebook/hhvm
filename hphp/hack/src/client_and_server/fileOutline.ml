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
    | ProtectedInternal -> [SymbolDefinition.ProtectedInternal]
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

let pos_of_hint = function
  | Some (p, _) when not Pos.(equal none p) -> Some p
  | _ -> None

let summarize_property ~(source_text : string option) class_name var =
  let modifiers =
    modifiers_to_list
      ~is_final:var.cv_final
      ~visibility:var.cv_visibility
      ~is_abstract:var.cv_abstract
      ~is_static:var.cv_is_static
  in
  let (pos, name) = var.cv_id in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        let readonly =
          if var.cv_readonly then
            "readonly "
          else
            ""
        in
        let ty =
          match pos_of_hint (snd var.cv_type) with
          | Some p -> Pos.get_text_from_pos ~content:source_text p ^ " "
          | None -> ""
        in
        let dollar =
          if Option.is_some var.cv_xhp_attr then
            ""
          else
            "$"
        in
        let name = snd var.cv_id in
        Printf.sprintf "%s%s%s%s" readonly ty dollar name)
  in
  {
    kind = Member { member_kind = Property; class_name };
    name;
    pos;
    span = var.cv_span;
    modifiers;
    params = None;
    docblock = None;
    detail;
  }

let maybe_summarize_property ~(source_text : string option) class_name ~skip var
    =
  let (_, name) = var.cv_id in
  if SSet.mem skip name then
    []
  else
    [summarize_property ~source_text class_name var]

let summarize_class_const ~(source_text : string option) class_name cc =
  let (pos, name) = cc.cc_id in
  let (span, modifiers) =
    match cc.cc_kind with
    | CCConcrete (_, p, _) -> (Pos.btw pos p, [])
    | CCAbstract (Some (_, p_default, _)) -> (Pos.btw pos p_default, [Abstract])
    | CCAbstract None -> (pos, [Abstract])
  in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        let ty : string =
          match pos_of_hint cc.cc_type with
          | Some p -> Pos.get_text_from_pos ~content:source_text p ^ " "
          | _ -> ""
        in
        ty ^ Pos.get_text_from_pos ~content:source_text span)
  in
  {
    kind = Member { member_kind = ClassConst; class_name };
    name;
    pos;
    span;
    modifiers;
    params = None;
    docblock = None;
    detail;
  }

let modifier_of_fun_kind acc = function
  | Ast_defs.FAsync
  | Ast_defs.FAsyncGenerator ->
    Async :: acc
  | _ -> acc

let modifier_of_param_kind acc = function
  | Ast_defs.Pinout _ -> Inout :: acc
  | Ast_defs.Pnormal -> acc

let summarize_class_typeconst ~(source_text : string option) class_name t =
  let (pos, name) = t.c_tconst_name in
  let modifiers =
    match t.c_tconst_kind with
    | TCAbstract _ -> [Abstract]
    | _ -> []
  in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        Pos.get_text_from_pos ~content:source_text t.c_tconst_span)
  in
  {
    kind = Member { member_kind = TypeConst; class_name };
    name;
    pos;
    span = t.c_tconst_span;
    modifiers;
    params = None;
    docblock = None;
    detail;
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
    Option.value_map
      (Aast_utils.get_param_default param)
      ~f:(fun (_, p, _) -> p)
      ~default:pos
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
  {
    kind;
    name;
    pos;
    span = Pos.btw param_start param_end;
    modifiers;
    params = None;
    docblock = None;
    detail = None;
  }

let detail_of_fun
    ~(source_text : string)
    ~(fun_pos : Pos.t)
    (sid : sid)
    (body : (_, _) func_body) : string =
  let pos =
    let start = Pos.shrink_to_end (fst sid) in
    let end_ =
      match List.hd body.fb_ast with
      | Some (stmt_pos, _) when not Pos.(equal none stmt_pos) ->
        Pos.shrink_to_start stmt_pos
      | _ -> Pos.shrink_to_end fun_pos
    in
    Pos.btw start end_
  in
  "function" ^ Pos.get_text_from_pos ~content:source_text pos
  |> String.rstrip ~drop:(fun ch ->
         Char.is_whitespace ch || Char.equal ch '{' || Char.equal ch '}')

let summarize_method ~(source_text : string option) class_name m =
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
  let detail =
    Option.map source_text ~f:(fun source_text ->
        detail_of_fun ~source_text ~fun_pos:m.m_span m.m_name m.m_body)
  in
  {
    kind = Member { member_kind = Method; class_name };
    name;
    pos = fst m.m_name;
    span = m.m_span;
    modifiers;
    params;
    docblock = None;
    detail;
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

let summarize_class ~(source_text : string option) class_ ~no_children =
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
  let members =
    if no_children then
      []
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
              (maybe_summarize_property
                 ~source_text
                 class_name
                 ~skip:implicit_props
                 cv))
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
              (maybe_summarize_property
                 ~source_text
                 class_name
                 ~skip:implicit_props
                 var))
          class_.c_xhp_attrs
      in
      let acc =
        (* Summarized consts *)
        let source_text_for_class_consts =
          match class_.c_kind with
          | Ast_defs.Cenum_class _ ->
            (* The representation of enum classes in the AST is not conducive to us
               making a sensible `details` field unless we change our approach significantly.
               For now, skip providing `details` for enum classes.
               The `id` field for enum class members includes the class name and tag.
            *)
            None
          | _ -> source_text
        in
        List.fold_left
          ~init:acc
          ~f:(fun acc c ->
            summarize_class_const
              ~source_text:source_text_for_class_consts
              class_name
              c
            :: acc)
          class_.c_consts
      in
      let acc =
        (* Summarized type consts *)
        List.fold_left
          ~init:acc
          ~f:(fun acc tc ->
            summarize_class_typeconst ~source_text class_name tc :: acc)
          class_.c_typeconsts
      in
      let acc =
        (* Summarized methods *)
        List.fold_left
          ~init:acc
          ~f:(fun acc m -> summarize_method ~source_text class_name m :: acc)
          class_.c_methods
      in
      let sort_by_line summaries =
        let cmp x y = Int.compare (Pos.line x.pos) (Pos.line y.pos) in
        List.sort ~compare:cmp summaries
      in
      sort_by_line acc
  in
  let classish_kind =
    match class_.c_kind with
    | Ast_defs.Cinterface -> Interface
    | Ast_defs.Ctrait -> Trait
    | Ast_defs.Cenum_class _
    | Ast_defs.Cenum ->
      Enum
    | Ast_defs.(Cclass _) -> Class
  in
  let name = class_name in
  {
    kind = Classish { classish_kind; members };
    name;
    pos = class_name_pos;
    span = c_span;
    modifiers;
    params = None;
    docblock = None;
    detail = None;
  }

let summarize_typedef ~(source_text : string option) (tdef : _ typedef) :
    Relative_path.t SymbolDefinition.t =
  let kind = SymbolDefinition.Typedef in
  let name = Utils.strip_ns (snd tdef.t_name) in
  let pos = fst tdef.t_name in
  let hint_for_end_pos = tdef.t_runtime_type in
  let span = Pos.btw pos (fst hint_for_end_pos) in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        Pos.get_text_from_pos ~content:source_text span)
  in
  {
    kind;
    name;
    pos;
    span;
    modifiers = [];
    params = None;
    docblock = None;
    detail;
  }

let summarize_fun ~(source_text : string option) fd =
  let f = fd.fd_fun in
  let modifiers = modifier_of_fun_kind [] f.f_fun_kind in
  let params = Some (List.map f.f_params ~f:summarize_param) in
  let kind = SymbolDefinition.Function in
  let name = Utils.strip_ns (snd fd.fd_name) in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        detail_of_fun ~source_text ~fun_pos:f.f_span fd.fd_name f.f_body)
  in
  {
    kind;
    name;
    pos = fst fd.fd_name;
    span = f.f_span;
    modifiers;
    params;
    docblock = None;
    detail;
  }

let summarize_gconst ~(source_text : string option) (cst : _ gconst) :
    Relative_path.t SymbolDefinition.t =
  let pos = fst cst.cst_name in
  let gconst_start = Option.value_map cst.cst_type ~f:fst ~default:pos in
  let (_, gconst_end, _) = cst.cst_value in
  let kind = GlobalConst in
  let name = Utils.strip_ns (snd cst.cst_name) in
  let span = Pos.btw gconst_start gconst_end in
  let detail =
    Option.map source_text ~f:(fun source_text ->
        Pos.get_text_from_pos ~content:source_text span)
  in
  {
    kind;
    name;
    pos;
    span;
    modifiers = [];
    params = None;
    docblock = None;
    detail;
  }

let summarize_local name span =
  let kind = LocalVar in
  {
    kind;
    name;
    pos = span;
    span;
    modifiers = [];
    params = None;
    docblock = None;
    detail = None;
  }

let summarize_module_def md =
  let kind = SymbolDefinition.Module in
  let name = snd md.md_name in
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
    pos = span;
    span;
    modifiers = [];
    params = None;
    docblock;
    detail = None;
  }

let outline_ast ast ~(source_text : string option) =
  let outline =
    List.filter_map ast ~f:(function
        | Fun f -> Some (summarize_fun ~source_text f)
        | Class c -> Some (summarize_class ~source_text c ~no_children:false)
        | Typedef t -> Some (summarize_typedef ~source_text t)
        | Constant c -> Some (summarize_gconst ~source_text c)
        | Namespace _
        | NamespaceUse _
        | SetNamespaceEnv _
        | FileAttributes _
        | Stmt _
        | Module _
        | SetModule _ ->
          None)
  in

  List.map outline ~f:SymbolDefinition.to_absolute

let should_add_docblock = function
  | Function
  | Classish _
  | Member _
  | GlobalConst
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
    let (acc, kind) =
      match def.kind with
      | Classish { classish_kind; members } ->
        let (acc, members) = map_def_list f acc members in
        (acc, Classish { classish_kind; members })
      | Function -> (acc, Function)
      | Member { member_kind; class_name } ->
        (acc, Member { member_kind; class_name })
      | GlobalConst -> (acc, GlobalConst)
      | LocalVar -> (acc, LocalVar)
      | TypeVar -> (acc, TypeVar)
      | Param -> (acc, Param)
      | Typedef -> (acc, Typedef)
      | Module -> (acc, Module)
    in
    (acc, { def with kind })
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

let outline popt source_text =
  let { Parser_return.ast; comments; _ } =
    let ast =
      Diagnostics.ignore_ (fun () ->
          if Ide_parser_cache.is_enabled () then
            Ide_parser_cache.(
              with_ide_cache @@ fun () ->
              get_ast popt Relative_path.default source_text)
          else
            let env =
              Parser.make_env
                ~parser_options:popt
                ~include_line_comments:true
                Relative_path.default
            in
            Parser.from_text_with_legacy env source_text)
    in
    ast
  in
  let result = outline_ast ast ~source_text:(Some source_text) in
  add_docblocks result comments

let outline_entry_no_comments
    ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) :
    string SymbolDefinition.t list =
  let source_text = Provider_context.get_file_contents_if_present entry in
  Ast_provider.compute_ast ~popt ~entry |> outline_ast ~source_text

let rec print_def ~short_pos indent def =
  let { name; kind; pos; span; modifiers; params; docblock; detail } = def in
  let (print_pos, print_span) =
    if short_pos then
      (Pos.string_no_file, Pos.multiline_string_no_file)
    else
      (Pos.string, Pos.multiline_string)
  in
  Printf.printf "%s%s\n" indent name;
  Printf.printf "%s  kind: %s\n" indent (string_of_kind kind);
  Option.iter (SymbolDefinition.identifier def) ~f:(fun id ->
      Printf.printf "%s  id: %s\n" indent id);
  Printf.printf "%s  position: %s\n" indent (print_pos pos);
  Printf.printf "%s  span: %s\n" indent (print_span span);
  Printf.printf "%s  modifiers: " indent;
  List.iter modifiers ~f:(fun x -> Printf.printf "%s " (string_of_modifier x));
  Printf.printf "\n";
  Option.iter detail ~f:(Printf.printf "%s  detail: %s\n" indent);
  Option.iter params ~f:(fun x ->
      Printf.printf "%s  params:\n" indent;
      print ~short_pos (indent ^ "    ") x);
  Option.iter docblock ~f:(fun x ->
      Printf.printf "%s  docblock:\n" indent;
      Printf.printf "%s\n" x);
  Printf.printf "\n";
  match kind with
  | Classish { members; _ } -> print ~short_pos (indent ^ "  ") members
  | _ -> ()

and print ~short_pos indent defs =
  List.iter defs ~f:(print_def ~short_pos indent)

let print_def ?(short_pos = false) = print_def ~short_pos

let print ?(short_pos = false) = print ~short_pos ""

let summarize_method :
    string -> ('a, 'b) Aast.method_ -> Relative_path.t SymbolDefinition.t =
 (fun s meth -> summarize_method ~source_text:None s meth)

let summarize_class :
    ('a, 'b) Aast.class_ ->
    no_children:bool ->
    Relative_path.t SymbolDefinition.t =
 fun class_ ~no_children ->
  summarize_class ~source_text:None class_ ~no_children

let summarize_fun : ('a, 'b) Aast.fun_def -> Relative_path.t SymbolDefinition.t
    =
 (fun fd -> summarize_fun ~source_text:None fd)

let summarize_gconst (cst : _ gconst) : Relative_path.t SymbolDefinition.t =
  summarize_gconst ~source_text:None cst

let summarize_typedef (tdef : _ typedef) : Relative_path.t SymbolDefinition.t =
  summarize_typedef ~source_text:None tdef

let summarize_property class_name var =
  summarize_property ~source_text:None class_name var

let summarize_class_const :
    string -> _ Aast.class_const -> Relative_path.t SymbolDefinition.t =
 fun class_name c_const ->
  summarize_class_const ~source_text:None class_name c_const

let summarize_class_typeconst :
    string -> _ Aast.class_typeconst_def -> Relative_path.t SymbolDefinition.t =
 fun class_name tconst_def ->
  summarize_class_typeconst ~source_text:None class_name tconst_def
