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

and def = def_common * def_

and def_common = (Pos.absolute * string)

and def_ =
  | Function
  | Class of class_

and class_ = {
  class_members : class_member list;
}

and class_member = def_common * class_member_

and class_member_ =
  | Method of method_

and method_ = {
  static : bool;
  extents : Pos.absolute;
}

let summarize_class class_ acc =
  let class_name = Utils.strip_ns (snd class_.Ast.c_name) in
  let class_name_pos = Pos.to_absolute (fst class_.Ast.c_name) in
  let class_members =
    List.fold_right class_.Ast.c_body ~init:[] ~f:begin fun method_ acc ->
    match method_ with
      | Ast.Method m ->
          let method_ = {
            static = List.mem m.Ast.m_kind (Ast.Static);
            extents = (Pos.to_absolute m.Ast.m_extents);
          } in
          ((Pos.to_absolute (fst m.Ast.m_name), snd m.Ast.m_name),
          Method method_) :: acc
      | _ -> acc
    end
  in
  let class_ = {
    class_members;
  } in
  ((class_name_pos, class_name), Class class_) :: acc

let summarize_fun f acc =
  ((Pos.to_absolute (fst f.Ast.f_name),
   Utils.strip_ns (snd f.Ast.f_name)), Function) :: acc

let outline_ast ast =
  List.fold_right ast ~init:[] ~f:begin fun def acc ->
    match def with
    | Ast.Fun f -> summarize_fun f acc
    | Ast.Class c -> summarize_class c acc
    | _ -> acc
  end

let to_json input =
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
let to_legacy outline =
  List.fold_left outline ~init:[] ~f:begin fun acc ((pos, name), def) ->
  match def with
  | Function -> (pos, name, "function")::acc
  | Class c ->
      let acc = (pos, name, "class")::acc in
      List.fold_left c.class_members ~init:acc
        ~f:begin fun acc ((pos, member_name), member) ->
          match member with
          | Method m ->
            let desc = if m.static then "static method" else "method" in
            (pos, name ^ "::" ^ member_name, desc)::acc
        end
  end

let outline content =
  let {Parser_hack.ast; _} = Errors.ignore_ begin fun () ->
    Parser_hack.program Relative_path.default content
  end in
  outline_ast ast

let outline_legacy content =
  to_legacy @@ outline content

let print_function pos name =
  Printf.printf "%s\n" name;
  Printf.printf "  type: function\n";
  Printf.printf "  position: %s\n" (Pos.string pos);
  Printf.printf "\n"

let print_method pos name m =
  Printf.printf "  %s\n" name;
  Printf.printf "    type: method\n";
  Printf.printf "    position: %s\n" (Pos.string pos);
  Printf.printf "    static: %b\n" m.static;
  Printf.printf "    extents: %s\n" (Pos.multiline_string m.extents);
  Printf.printf "\n"

let print_class_member ((pos, name), member) =
  match member with
  | Method m -> print_method pos name m

let print_class pos name c =
  Printf.printf "%s\n" name;
  Printf.printf "  type: class\n";
  Printf.printf "  position: %s\n" (Pos.string pos);
  Printf.printf "\n";
  List.iter c.class_members print_class_member;
  Printf.printf "\n"

let print (outline : outline) =
  List.iter outline begin fun ((pos, name), def) ->
    match def with
    | Function -> print_function pos name
    | Class c -> print_class pos name c
  end
