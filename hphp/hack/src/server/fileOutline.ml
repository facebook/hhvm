(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Reordered_argument_collections
open SymbolDefinition

type outline = string SymbolDefinition.t list

let modifiers_of_ast_kinds l =
  List.map l begin function
    | Ast.Final -> Final
    | Ast.Static -> Static
    | Ast.Abstract -> Abstract
    | Ast.Private -> Private
    | Ast.Public -> Public
    | Ast.Protected -> Protected
  end

let get_full_name class_name name =
  match class_name with
  | None -> name
  | Some class_name -> class_name ^ "::" ^ name

let summarize_property class_name kinds var =
  let modifiers = modifiers_of_ast_kinds kinds in
  let span, (pos, name), _expr_opt = var in
  let kind = Property in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    id;
    pos;
    span;
    modifiers;
    children = None;
    params = None;
    docblock = None;
  }

let maybe_summarize_property class_name ~skip kinds var =
  let _, (_, name), _ = var in
  if SSet.mem skip name then [] else [summarize_property class_name kinds var]

let summarize_const class_name ((pos, name), (expr_pos, _)) =
  let span = (Pos.btw pos expr_pos) in
  let kind = Const in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    id;
    pos;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_abs_const class_name (pos, name) =
  let kind = Const in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    id;
    pos = pos;
    span = pos;
    modifiers = [Abstract];
    children = None;
    params = None;
    docblock = None;
  }

let modifier_of_fun_kind acc = function
  | Ast.FAsync | Ast.FAsyncGenerator -> Async :: acc
  | _ -> acc

let summarize_typeconst class_name t =
  let pos, name = t.Ast.tconst_name in
  let kind = Typeconst in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    id;
    pos;
    span = t.Ast.tconst_span;
    modifiers = if t.Ast.tconst_abstract then [Abstract] else [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_param param =
  let pos, name = param.Ast.param_id in
  let param_start = Option.value_map param.Ast.param_hint ~f:fst ~default:pos in
  let param_end = Option.value_map param.Ast.param_expr ~f:fst ~default:pos in
  let modifiers =
    modifiers_of_ast_kinds (Option.to_list param.Ast.param_modifier) in
  let kind = Param in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    id;
    pos;
    span = Pos.btw param_start param_end;
    children = None;
    modifiers;
    params = None;
    docblock = None;
  }

let summarize_method class_name m =
  let modifiers = modifier_of_fun_kind [] m.Ast.m_fun_kind in
  let modifiers = (modifiers_of_ast_kinds m.Ast.m_kind) @ modifiers in
  let params = Some (List.map m.Ast.m_params summarize_param) in
  let name = snd m.Ast.m_name in
  let kind = Method in
  let id = get_symbol_id kind (Some class_name) name in
  let full_name = get_full_name (Some class_name) name in
  {
    kind;
    name;
    full_name;
    id;
    pos = (fst m.Ast.m_name);
    span = m.Ast.m_span;
    modifiers;
    children = None;
    params;
    docblock = None;
  }

(* Parser synthesizes AST nodes for implicit properties (defined in constructor
 * parameter lists. We don't want them to show up in outline view *)
let params_implicit_fields params =
  List.filter_map params ~f:begin function
    | { Ast.param_modifier = Some _vis; param_id; _ } ->
        Some (String_utils.lstrip (snd param_id) "$" )
    | _ -> None
  end

let class_implicit_fields class_ =
  List.concat_map class_.Ast.c_body ~f:begin function
    | Ast.Method { Ast.m_name = _, "__construct"; m_params; _ } ->
        params_implicit_fields m_params
    | _ -> []
  end

let summarize_class class_ ~no_children =
  let class_name = Utils.strip_ns (snd class_.Ast.c_name) in
  let class_name_pos = fst class_.Ast.c_name in
  let c_span = class_.Ast.c_span in
  let modifiers =
    if class_.Ast.c_final then [Final] else []
  in
  let modifiers = match class_.Ast.c_kind with
    | Ast.Cabstract -> Abstract :: modifiers
    | _ -> modifiers
  in
  let children = if no_children then None else begin
    let implicit_props = List.fold (class_implicit_fields class_)
      ~f:SSet.add ~init:SSet.empty
    in
    Some (List.concat_map class_.Ast.c_body ~f:begin function
      | Ast.Method m -> [summarize_method class_name m]
      | Ast.ClassVars (kinds, _, vars, _doc_comment_opt) ->
          List.concat_map vars
            ~f:(maybe_summarize_property class_name ~skip:implicit_props kinds)
      | Ast.XhpAttr (_, var, _, _) ->
          maybe_summarize_property class_name ~skip:implicit_props [] var
      | Ast.Const (_, cl) -> List.map cl ~f:(summarize_const class_name)
      | Ast.AbsConst (_, id) -> [summarize_abs_const class_name id]
      | Ast.TypeConst t -> [summarize_typeconst class_name t]
      | _ -> []
      end)
  end in
  let kind = match class_.Ast.c_kind with
    | Ast.Cinterface -> Interface
    | Ast.Ctrait -> Trait
    | Ast.Cenum -> Enum
    | _ -> Class
  in
  let name = class_name in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    id;
    pos = class_name_pos;
    span = c_span;
    modifiers;
    children;
    params = None;
    docblock = None;
  }

let typedef_kind_pos tk =
  begin match tk with
  | Ast.Alias (pos,_) -> pos
  | Ast.NewType (pos,_) -> pos
  end

let summarize_typedef tdef =
  let kind = Typedef in
  let name = Utils.strip_ns (snd tdef.Ast.t_id) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  let pos = fst tdef.Ast.t_id in
  let kind_pos = typedef_kind_pos tdef.Ast.t_kind in
  let span = (Pos.btw pos kind_pos) in
  {
    kind;
    name;
    full_name;
    id;
    pos;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let summarize_fun f =
  let modifiers = modifier_of_fun_kind [] f.Ast.f_fun_kind in
  let params = Some (List.map f.Ast.f_params summarize_param) in
  let kind = Function in
  let name = Utils.strip_ns (snd f.Ast.f_name) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
    id;
    pos = fst f.Ast.f_name;
    span = f.Ast.f_span;
    modifiers;
    children = None;
    params;
    docblock = None;
  }

let summarize_gconst cst =
  let pos = fst cst.Ast.cst_name in
  let gconst_start = Option.value_map cst.Ast.cst_type ~f:fst ~default:pos in
  let gconst_end = fst cst.Ast.cst_value in
  let kind = Const in
  let name = Utils.strip_ns (snd cst.Ast.cst_name) in
  let id = get_symbol_id kind None name in
  let full_name = get_full_name None name in
  {
    kind;
    name;
    full_name;
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
    id;
    pos = span;
    span;
    modifiers = [];
    children = None;
    params = None;
    docblock = None;
  }

let outline_ast ast =
  let outline = List.filter_map ast ~f:begin function
    | Ast.Fun f -> Some (summarize_fun f)
    | Ast.Class c -> Some (summarize_class c ~no_children:false)
    | _ -> None
  end in
  List.map outline SymbolDefinition.to_absolute

let should_add_docblock = function
  | Function| Class | Method | Property | Const | Enum
  | Interface | Trait | Typeconst | Typedef -> true
  | LocalVar | Param -> false

let add_def_docblock finder previous_def_line def =
  let line = Pos.line def.pos in
  let docblock = if should_add_docblock def.kind
    then Docblock_finder.find_docblock finder previous_def_line line
    else None
  in
  line, { def with docblock }

let add_docblocks defs comments =
  let finder = Docblock_finder.make_docblock_finder comments in

  let rec map_def f acc def =
    let acc, def = f acc def in
    let acc, children = Option.value_map def.children
      ~f:(fun defs ->
        let acc, defs = map_def_list f acc defs in
        acc, Some defs)
      ~default:(acc, None)
    in
    acc, { def with children }

  and map_def_list f acc defs =
    let acc, defs = List.fold_left defs
      ~f:(fun (acc, defs) def ->
        let acc, def = map_def f acc def in
        acc, def :: defs)
      ~init:(acc, []) in
    acc, List.rev defs

  in
  snd (map_def_list (add_def_docblock finder) 0 defs)

let outline popt content =
  let {Parser_hack.ast; comments; _} =
    Parser_hack.program
      popt
      Relative_path.default
      content
      ~include_line_comments:true
      ~keep_errors:false
  in
  let result = outline_ast ast in
  add_docblocks result comments

let rec print_def ~short_pos indent def =
  let
    {name; kind; id; pos; span; modifiers; children; params; docblock;
      full_name=_} = def
  in
  let print_pos, print_span = if short_pos
    then Pos.string_no_file, Pos.multiline_string_no_file
    else Pos.string, Pos.multiline_string
  in
  Printf.printf "%s%s\n" indent name;
  Printf.printf "%s  kind: %s\n" indent (string_of_kind kind);
  Option.iter id (fun id -> Printf.printf "%s  id: %s\n" indent id);
  Printf.printf "%s  position: %s\n" indent (print_pos pos);
  Printf.printf "%s  span: %s\n" indent (print_span span);
  Printf.printf "%s  modifiers: " indent;
  List.iter modifiers
    (fun x -> Printf.printf "%s " (string_of_modifier x));
    Printf.printf "\n";
  Option.iter params (fun x ->
    Printf.printf "%s  params:\n" indent;
    print ~short_pos (indent ^ "    ") x;
  );
  Option.iter docblock (fun x ->
    Printf.printf "%s  docblock:\n" indent;
    Printf.printf "%s\n" x;
  );
  Printf.printf "\n";
  Option.iter children (fun x ->
    print ~short_pos (indent ^ "  ") x
  );

and print ~short_pos indent defs =
  List.iter defs ~f:(print_def ~short_pos indent)

let print_def ?short_pos:(short_pos = false) = print_def ~short_pos
let print ?short_pos:(short_pos = false) = print ~short_pos ""
