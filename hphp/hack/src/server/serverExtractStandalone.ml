(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax(Syntax)
module SyntaxError = Full_fidelity_syntax_error

exception FunctionNotFound
exception UnexpectedDependency
exception DependencyNotFound

let value_exn ex opt = match opt with
| Some s -> s
| None -> raise ex

let get_filename func =
  let f = value_exn FunctionNotFound @@ Decl_provider.get_fun func  in
  Pos.filename f.ft_pos

let extract_function_body func =
  let filename = get_filename func in
  let abs_filename = Relative_path.to_absolute filename in
  let file_content = In_channel.read_all abs_filename in
  let ast_function = value_exn FunctionNotFound @@ Ast_provider.find_fun_in_file filename func in
  let open Ast in
  let pos = ast_function.f_span in
  let include_first_whsp = Pos.merge (Pos.first_char_of_line pos) pos in
  Pos.get_text_from_pos file_content include_first_whsp

let strip_ns obj_name =
  let (_, name) = String.rsplit2_exn obj_name '\\' in
  name

let list_items items = String.concat items ~sep:", "

let tparam_name (tp: Typing_defs.decl Typing_defs.tparam) = snd tp.tp_name

let get_function_declaration tcopt fun_name fun_type =
  let tparams = match fun_type.ft_tparams with
  | ([], _) -> ""
  | (tparams, _) -> Printf.sprintf "<%s>" @@ list_items @@ List.map tparams tparam_name in
  let fun_type_str = (Typing_print.fun_type tcopt fun_type) in
  Printf.sprintf "function %s%s%s" (strip_ns fun_name) tparams fun_type_str

(* TODO: constants, class fields *)
let extract_object_declaration tcopt obj =
  let open Typing_deps.Dep in
  match obj with
  | Fun f | FunName f ->
    let fun_type = value_exn DependencyNotFound @@ Decl_provider.get_fun f in
    let declaration = get_function_declaration tcopt f fun_type in
    declaration ^ "{throw new Exception();}"
  | _ -> to_string obj

let rec name_from_hint hint = match hint with
  | (_, Ast.Happly((_, s), params)) -> if List.is_empty params then s
    else Printf.sprintf "%s<%s>" s (list_items @@ List.map params name_from_hint)
  | _ -> raise UnexpectedDependency

let list_direct_ancestors cls =
  let cls_pos = Decl_provider.Class.pos cls in
  let cls_name = Decl_provider.Class.name cls in
  let filename = Pos.filename cls_pos in
  let ast_class = value_exn DependencyNotFound @@ Ast_provider.find_class_in_file filename cls_name in
  let get_unqualified_class_name hint = strip_ns @@ name_from_hint hint in
  let list_types hints = list_items @@ List.map hints get_unqualified_class_name in
  let open Ast in
  let extends = list_types ast_class.c_extends in
  let implements = list_types ast_class.c_implements in
  let prefix_if_nonempty prefix s = if s = "" then "" else prefix ^ s in
  (prefix_if_nonempty "extends " extends) ^ (prefix_if_nonempty " implements " implements)

let print_error source_text error =
  let text = SyntaxError.to_positioned_string
    error (SourceText.offset_to_position source_text) in
  Hh_logger.log "%s\n" text

let tree_from_string s =
  let source_text = SourceText.make Relative_path.default s in
  let mode = Full_fidelity_parser.parse_mode ~rust:false source_text in
  let env = Full_fidelity_parser_env.make ?mode () in
  let tree = SyntaxTree.make ~env source_text in
  if List.is_empty (SyntaxTree.all_errors tree) then tree
  else
    (List.iter (SyntaxTree.all_errors tree) (print_error source_text);
    raise Hackfmt_error.InvalidSyntax)

let format text =
  try Libhackfmt.format_tree (tree_from_string text)
  with Hackfmt_error.InvalidSyntax -> text

let get_class_declaration cls =
  let open Decl_provider in
  match get_class cls with
  | None -> raise DependencyNotFound
  | Some cls ->
  let kind = match Class.kind cls with
  | Ast_defs.Cabstract -> "abstract class"
  | Ast_defs.Cnormal -> "class"
  | Ast_defs.Cinterface -> "interface"
  | Ast_defs.Ctrait -> "trait"
  | Ast_defs.Cenum -> "enum"
  | Ast_defs.Crecord -> "record" in
  let name = strip_ns (Class.name cls) in
  let tparams = if List.is_empty @@ Class.tparams cls then ""
  else Printf.sprintf "<%s>" (list_items @@ List.map (Class.tparams cls) tparam_name) in
  (* TODO: traits, enums, records *)
  kind^" "^name^tparams^" "^(list_direct_ancestors cls)

(* TODO: namespaces *)
let construct_class_declaration tcopt cls fields =
  let decl = get_class_declaration cls in
  let open Typing_deps.Dep in
  let process_field f = match f with
  | AllMembers _ | Extends _ -> raise UnexpectedDependency
  | _ -> extract_object_declaration tcopt f in
  let body = HashSet.fold (fun f accum -> accum^"\n"^(process_field f)) fields "" in
  decl^" {"^body^"}"

let construct_typedef_declaration tcopt t =
  let td = value_exn DependencyNotFound @@ Decl_provider.get_typedef t in
  let typ = if td.td_vis = Aast_defs.Transparent then "type" else "newtype" in
  Printf.sprintf "%s %s = %s;" typ (strip_ns t) (Typing_print.full_decl tcopt td.td_type)

let construct_type_declaration tcopt t fields acc =
  match Decl_provider.get_class t with
  | Some _ -> construct_class_declaration tcopt t fields :: acc
  | None -> construct_typedef_declaration tcopt t :: acc

(* TODO: Tfun? Any other cases? *)
let add_dep ty deps = match ty with
  | (_, Tapply((_, str), _)) -> HashSet.add deps (Typing_deps.Dep.Class str)
  | _ -> ()

let get_signature_dependencies obj deps =
  let open Typing_deps.Dep in
  match obj with
  | Prop (cls, _)
  | SProp (cls, _)
  | Method (cls, _)
  | SMethod (cls, _)
  | Const (cls, _)
  | Cstr cls -> ignore cls;
  | Class cls ->
      (match Decl_provider.get_class cls with
      | None ->
        let td = value_exn DependencyNotFound @@ Decl_provider.get_typedef cls in
        add_dep td.td_type deps
      | Some c ->
        Sequence.iter (Decl_provider.Class.all_ancestors c) (fun (_, ty) -> add_dep ty deps)
      )
  | Fun f | FunName f ->
    let func = value_exn DependencyNotFound @@ Decl_provider.get_fun f in
    add_dep func.ft_ret deps;
    List.iter func.ft_params (fun p -> add_dep p.fp_type deps)
  | GConst c | GConstName c ->
    (let (ty, _) = value_exn DependencyNotFound @@ Decl_provider.get_gconst c in
    add_dep ty deps)
  | AllMembers _ | Extends _ -> raise UnexpectedDependency

let collect_dependencies tcopt func =
  let dependencies = HashSet.create 0 in
  let open Typing_deps.Dep in
  let add_dependency root obj =
    match root with
    | Fun f | FunName f -> if f = func then begin
      HashSet.add dependencies obj;
      get_signature_dependencies obj dependencies;
    end
    | _ -> () in
  Typing_deps.add_dependency_callback "add_dependency" add_dependency;
  let filename = get_filename func in
  let _ : Tast.def option = Typing_check_service.type_fun tcopt filename func in
  let types = Caml.Hashtbl.create 0 in
  let globals = HashSet.create 0 in
  let group_by_type obj = match obj with
  | Class cls -> (match Caml.Hashtbl.find_opt types cls with
    | Some set -> HashSet.add set obj
    | None -> let set = HashSet.create 0 in Caml.Hashtbl.add types cls set)
  | Prop (cls, _)
  | SProp (cls, _)
  | Method (cls, _)
  | SMethod (cls, _)
  | Const (cls, _)
  | Cstr cls -> (match Caml.Hashtbl.find_opt types cls with
    | Some set -> HashSet.add set obj
    | None -> let set = HashSet.create 0 in HashSet.add set obj;
              Caml.Hashtbl.add types cls set)
  | Extends _ -> raise UnexpectedDependency
  | AllMembers _ -> raise UnexpectedDependency
  | _ -> HashSet.add globals obj in
  HashSet.iter group_by_type dependencies;
  let global_code = HashSet.fold (fun el l -> extract_object_declaration tcopt el :: l) globals [] in
  let code = Caml.Hashtbl.fold (construct_type_declaration tcopt) types global_code in
  List.map code format


let go tcopt function_name =
  try
    let header = "<?hh\n" in
    let function_text = extract_function_body function_name in
    let dependencies = collect_dependencies tcopt function_name in
    format @@ String.concat ~sep:"\n" (header :: dependencies @ [function_text])
  with FunctionNotFound -> "Function not found!"
