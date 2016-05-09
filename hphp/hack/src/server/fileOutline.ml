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
  | Function of function_
  | Class of class_

and function_ = {
  f_extents : Pos.absolute;
}
and class_ = {
  class_members : class_member list;
  c_extents : Pos.absolute;
}

and class_member = def_common * class_member_

and class_member_ =
  | Method of method_
  | Property of property
  | Const of class_const

and method_ = {
  static : bool;
  m_extents : Pos.absolute;
}

and property = {
  p_extents : Pos.absolute;
}

and class_const = {
  cc_extents : Pos.absolute;
}

let summarize_property var =
  let p_extents, (pos, name), expr_opt = var in
  let prop = {
    p_extents = Pos.to_absolute p_extents;
  } in
  ((Pos.to_absolute pos, name), Property prop)

let summarize_const ((pos, name), (expr_pos, _)) =
  let cc_extents = Pos.to_absolute (Pos.btw pos expr_pos) in
  let const = {
    cc_extents;
  } in
  ((Pos.to_absolute pos, name), Const const)

let summarize_abs_const (pos, name) =
  let const = {cc_extents = Pos.to_absolute pos; } in
  ((Pos.to_absolute pos, name), Const const)

let summarize_class class_ acc =
  let class_name = Utils.strip_ns (snd class_.Ast.c_name) in
  let class_name_pos = Pos.to_absolute (fst class_.Ast.c_name) in
  let c_extents = Pos.to_absolute class_.Ast.c_extents in
  let class_members = List.concat
    (List.map class_.Ast.c_body ~f:begin function
    | Ast.Method m ->
        let method_ = {
          static = List.mem m.Ast.m_kind (Ast.Static);
          m_extents = (Pos.to_absolute m.Ast.m_extents);
        } in
        [(Pos.to_absolute (fst m.Ast.m_name), snd m.Ast.m_name),
         Method method_ ]
    | Ast.ClassVars (_, _, vars) -> List.map vars ~f:summarize_property
    | Ast.XhpAttr (_, var, _, _) -> [summarize_property var]
    | Ast.Const (_, cl) -> List.map cl ~f:summarize_const
    | Ast.AbsConst (_, id) -> [summarize_abs_const id]
    | _ -> []
    end)
  in
  let class_ = {
    class_members;
    c_extents;
  } in
  ((class_name_pos, class_name), Class class_) :: acc

let summarize_fun f acc =
  let fun_ = {
    f_extents = (Pos.to_absolute f.Ast.f_extents);
  } in
  ((Pos.to_absolute (fst f.Ast.f_name),
   Utils.strip_ns (snd f.Ast.f_name)), (Function fun_)) :: acc

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
  | Function _ -> (pos, name, "function")::acc
  | Class c ->
      let acc = (pos, name, "class")::acc in
      List.fold_left c.class_members ~init:acc
        ~f:begin fun acc ((pos, member_name), member) ->
          match member with
          | Method m ->
            let desc = if m.static then "static method" else "method" in
            (pos, name ^ "::" ^ member_name, desc)::acc
          | Property _
          | Const _ -> acc
        end
  end

let outline content =
  let {Parser_hack.ast; _} = Errors.ignore_ begin fun () ->
    Parser_hack.program Relative_path.default content
  end in
  outline_ast ast

let outline_legacy content =
  to_legacy @@ outline content

let print_function pos name f =
  Printf.printf "%s\n" name;
  Printf.printf "  type: function\n";
  Printf.printf "  position: %s\n" (Pos.string pos);
  Printf.printf "  extents: %s\n" (Pos.multiline_string f.f_extents);
  Printf.printf "\n"

let print_method pos name m =
  Printf.printf "  %s\n" name;
  Printf.printf "    type: method\n";
  Printf.printf "    position: %s\n" (Pos.string pos);
  Printf.printf "    static: %b\n" m.static;
  Printf.printf "    extents: %s\n" (Pos.multiline_string m.m_extents);
  Printf.printf "\n"

let print_property pos name p =
  Printf.printf "  %s\n" name;
  Printf.printf "    type: property\n";
  Printf.printf "    position: %s\n" (Pos.string pos);
  Printf.printf "    extents: %s\n" (Pos.multiline_string p.p_extents);
  Printf.printf "\n"

let print_const pos name c =
  Printf.printf "  %s\n" name;
  Printf.printf "    type: const\n";
  Printf.printf "    position: %s\n" (Pos.string pos);
  Printf.printf "    extents: %s\n" (Pos.multiline_string c.cc_extents);
  Printf.printf "\n"

let print_class_member ((pos, name), member) =
  match member with
  | Method m -> print_method pos name m
  | Property p -> print_property pos name p
  | Const c -> print_const pos name c

let print_class pos name c =
  Printf.printf "%s\n" name;
  Printf.printf "  type: class\n";
  Printf.printf "  position: %s\n" (Pos.string pos);
  Printf.printf "  extents: %s\n" (Pos.multiline_string c.c_extents);
  Printf.printf "\n";
  List.iter c.class_members print_class_member;
  Printf.printf "\n"

let print (outline : outline) =
  List.iter outline begin fun ((pos, name), def) ->
    match def with
    | Function f -> print_function pos name f
    | Class c -> print_class pos name c
  end
