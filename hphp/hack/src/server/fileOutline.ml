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

type outline = def list

and kind =
  | Function
  | Class
  | Method
  | Property
  | Const

and modifier =
  | Static

and def = {
  kind : kind;
  name : string;
  pos : Pos.absolute;
  span : Pos.absolute;
  modifiers : modifier list;
  children : def list;
}

let string_of_kind = function
  | Function -> "function"
  | Class -> "class"
  | Method -> "method"
  | Property -> "property"
  | Const -> "const"

let string_of_modifier = function
  | Static -> "static"

let summarize_property var =
  let span, (pos, name), expr_opt = var in
  {
    kind = Property;
    name;
    pos = Pos.to_absolute pos;
    span = Pos.to_absolute span;
    modifiers = [];
    children = [];
  }

let summarize_const ((pos, name), (expr_pos, _)) =
  let span = Pos.to_absolute (Pos.btw pos expr_pos) in
  {
    kind = Const;
    name;
    pos = Pos.to_absolute pos;
    span;
    modifiers = [];
    children = [];
  }

let summarize_abs_const (pos, name) =
  let pos = Pos.to_absolute pos in
  {
    kind = Const;
    name;
    pos = pos;
    span = pos;
    modifiers = [];
    children = [];
  }

let summarize_class acc class_  =
  let class_name = Utils.strip_ns (snd class_.Ast.c_name) in
  let class_name_pos = Pos.to_absolute (fst class_.Ast.c_name) in
  let c_span = Pos.to_absolute class_.Ast.c_span in
  let children = List.concat
    (List.map class_.Ast.c_body ~f:begin function
    | Ast.Method m ->
        let modifiers = [] in
        let modifiers =
          if List.mem m.Ast.m_kind (Ast.Static)
          then Static :: modifiers else modifiers
        in
        let method_ = {
          kind = Method;
          name = snd m.Ast.m_name;
          pos = Pos.to_absolute (fst m.Ast.m_name);
          span = Pos.to_absolute m.Ast.m_span;
          modifiers;
          children = [];
        } in
        [method_]
    | Ast.ClassVars (_, _, vars) -> List.map vars ~f:summarize_property
    | Ast.XhpAttr (_, var, _, _) -> [summarize_property var]
    | Ast.Const (_, cl) -> List.map cl ~f:summarize_const
    | Ast.AbsConst (_, id) -> [summarize_abs_const id]
    | _ -> []
    end)
  in
  let class_ = {
    kind = Class;
    name = class_name;
    pos = class_name_pos;
    span = c_span;
    modifiers = [];
    children;
  } in
  class_ :: acc

let summarize_fun acc f =
  let fun_ = {
    kind = Function;
    name = Utils.strip_ns (snd f.Ast.f_name);
    pos = Pos.to_absolute (fst f.Ast.f_name);
    span = (Pos.to_absolute f.Ast.f_span);
    modifiers = [];
    children = []
  } in
  fun_ :: acc

let outline_ast ast =
  List.fold_right ast ~init:[] ~f:begin fun def acc ->
    match def with
    | Ast.Fun f -> summarize_fun acc f
    | Ast.Class c -> summarize_class acc c
    | _ -> acc
  end

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
     | Class ->
         let acc = (def.pos, def.name, "class") :: acc in
         to_legacy (prefix ^ def.name ^ "::") acc def.children
     | Method ->
       let desc =
         if List.mem def.modifiers Static
         then "static method" else "method"
       in
       (def.pos, prefix ^ def.name, desc) :: acc
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
