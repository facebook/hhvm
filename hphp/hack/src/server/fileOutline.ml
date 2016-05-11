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

let summarize_property kinds var =
  let modifiers = modifiers_of_ast_kinds kinds in
  let span, (pos, name), expr_opt = var in
  {
    kind = Property;
    name;
    pos;
    span;
    modifiers;
    children = [];
  }

let summarize_const ((pos, name), (expr_pos, _)) =
  let span = (Pos.btw pos expr_pos) in
  {
    kind = Const;
    name;
    pos;
    span;
    modifiers = [];
    children = [];
  }

let summarize_abs_const (pos, name) =
  {
    kind = Const;
    name;
    pos = pos;
    span = pos;
    modifiers = [Abstract];
    children = [];
  }

let modifier_of_fun_kind acc = function
  | Ast.FAsync | Ast.FAsyncGenerator -> Async :: acc
  | _ -> acc

let summarize_typeconst t =
  let pos, name = t.Ast.tconst_name in
  {
    kind = Typeconst;
    name;
    pos;
    span = t.Ast.tconst_span;
    modifiers = if t.Ast.tconst_abstract then [Abstract] else [];
    children = [];
  }

let summarize_method m =
  let modifiers = modifier_of_fun_kind [] m.Ast.m_fun_kind in
  let modifiers = (modifiers_of_ast_kinds m.Ast.m_kind) @ modifiers in
  {
    kind = Method;
    name = snd m.Ast.m_name;
    pos = (fst m.Ast.m_name);
    span = m.Ast.m_span;
    modifiers;
    children = [];
  }

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
  let children = if no_children then [] else List.concat
    (List.map class_.Ast.c_body ~f:begin function
    | Ast.Method m -> [summarize_method m]
    | Ast.ClassVars (kinds, _, vars) ->
        List.map vars ~f:(summarize_property kinds)
    | Ast.XhpAttr (_, var, _, _) -> [summarize_property [] var]
    | Ast.Const (_, cl) -> List.map cl ~f:summarize_const
    | Ast.AbsConst (_, id) -> [summarize_abs_const id]
    | Ast.TypeConst t -> [summarize_typeconst t]
    | _ -> []
    end)
  in
  let kind = match class_.Ast.c_kind with
    | Ast.Cinterface -> Interface
    | Ast.Ctrait -> Trait
    | Ast.Cenum -> Enum
    | _ -> Class
  in
  {
    kind;
    name = class_name;
    pos = class_name_pos;
    span = c_span;
    modifiers;
    children;
  }

let summarize_fun f =
  let modifiers = modifier_of_fun_kind [] f.Ast.f_fun_kind in
  {
    kind = Function;
    name = Utils.strip_ns (snd f.Ast.f_name);
    pos = fst f.Ast.f_name;
    span = f.Ast.f_span;
    modifiers;
    children = []
  }

let outline_ast ast =
  let outline = List.filter_map ast ~f:begin function
    | Ast.Fun f -> Some (summarize_fun f)
    | Ast.Class c -> Some (summarize_class c ~no_children:false)
    | _ -> None
  end in
  List.map outline SymbolDefinition.to_absolute

let to_json_legacy input =
  let entries = List.map input begin fun (pos, name, type_) ->
    let line, start, end_ = Pos.info_pos pos in
    Hh_json.JSON_Object [
        "name",  Hh_json.JSON_String name;
        "type", Hh_json.JSON_String type_;
        "line",  Hh_json.int_ line;
        "char_start", Hh_json.int_ start;
        "char_end", Hh_json.int_ end_;
    ]
  end in
  Hh_json.JSON_Array entries

(* Transforms the outline type to format that existing --outline command
 * expects *)
 let rec to_legacy prefix acc defs =
   List.fold_left defs ~init:acc ~f:begin fun acc def ->
     match def.kind with
     | Function -> (def.pos, def.name, "function") :: acc
     | Class | Enum | Interface | Trait ->
         let acc = (def.pos, def.name, "class") :: acc in
         to_legacy (prefix ^ def.name ^ "::") acc def.children
     | Method ->
       let desc =
         if List.mem def.modifiers Static
         then "static method" else "method"
       in
       (def.pos, prefix ^ def.name, desc) :: acc
     | Typeconst
     | LocalVar
     | Property
     | Const -> acc
   end

 let to_legacy outline = to_legacy "" [] outline

 let outline content =
   let {Parser_hack.ast; _} = Errors.ignore_ begin fun () ->
     Parser_hack.program Relative_path.default content
   end in
   outline_ast ast

 let outline_legacy content =
   to_legacy @@ outline content

 let rec to_json outline =
   Hh_json.JSON_Array begin
     List.map outline ~f:begin fun def -> Hh_json.JSON_Object [
       "kind", Hh_json.JSON_String (string_of_kind def.kind);
       "name", Hh_json.JSON_String def.name;
       "position", Pos.json def.pos;
       "span", Pos.multiline_json def.span;
       "modifiers", Hh_json.JSON_Array
         (List.map def.modifiers
           (fun x -> Hh_json.JSON_String (string_of_modifier x)));
       "children", (to_json def.children)
     ] end
   end

 let rec print indent defs  =
   List.iter defs ~f:begin fun def ->
     Printf.printf "%s%s\n" indent def.name;
     Printf.printf "%s  kind: %s\n" indent (string_of_kind def.kind);
     Printf.printf "%s  position: %s\n" indent (Pos.string def.pos);
     Printf.printf "%s  span: %s\n" indent (Pos.multiline_string def.span);
     Printf.printf "%s  modifiers: " indent;
     List.iter def.modifiers
       (fun x -> Printf.printf "%s " (string_of_modifier x));
       Printf.printf "\n\n";
     print (indent ^ "  ") def.children
   end

 let print  = print ""
