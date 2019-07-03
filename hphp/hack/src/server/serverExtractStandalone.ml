(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

exception FunctionNotFound
exception UnexpectedDependency
exception DependencyNotFound
exception LazyClass

let value_exn opt ex = match opt with
| Some s -> s
| None -> raise ex

let get_filename func =
  let f = value_exn (Decl_provider.get_fun func) FunctionNotFound in
  Pos.filename f.ft_pos

let extract_function_body func =
  let filename = get_filename func in
  let abs_filename = Relative_path.to_absolute filename in
  let file_content = In_channel.read_all abs_filename in
  let ast_function = value_exn (Ast_provider.find_fun_in_file filename func) FunctionNotFound in
  let open Ast in
  let pos = ast_function.f_span in
  let include_first_whsp = Pos.merge (Pos.first_char_of_line pos) pos in
  Pos.get_text_from_pos file_content include_first_whsp

let strip_namespace obj_name =
  let (_, name) = String.rsplit2_exn obj_name '\\' in
  name

(* TODO: constans, class fields *)
let extract_object_declaration obj =
  let open Typing_deps.Dep in
  match obj with
  | Fun f | FunName f ->
    (* TODO: generate based on Typing_defs.fun_type *)
    let body = extract_function_body f in
    let declaration = match String.index body '{' with
    | Some index -> String.sub body 0 index
    | None -> body in
    declaration ^ "{\n  throw new Exception();\n}"
  | _ -> to_string obj

let list objects = String.concat (Sequence.to_list objects) ~sep:", "

(* TODO: generics *)
let name_from_hint hint = match hint with
  | (_, Ast.Happly((_, s), _)) -> s
  | _ -> raise UnexpectedDependency

let list_direct_ancestors cls =
  let cls_pos = Decl_provider.Class.pos cls in
  let cls_name = Decl_provider.Class.name cls in
  let filename = Pos.filename cls_pos in
  let ast_class = value_exn (Ast_provider.find_class_in_file filename cls_name) DependencyNotFound in
  let get_unqualified_class_name hint = strip_namespace @@ name_from_hint hint in
  let list_types hints = String.concat ~sep:", " @@ List.map hints get_unqualified_class_name in
  let open Ast in
  let extends = list_types ast_class.c_extends in
  let implements = list_types ast_class.c_implements in
  let prefix_if_nonempty prefix s = if s = "" then "" else prefix ^ s in
  (prefix_if_nonempty "extends " extends) ^ (prefix_if_nonempty " implements " implements)

let get_class_declaration cls =
  let open Decl_provider in
  let cls = match get_class cls with
  | None -> raise DependencyNotFound  (* TODO: type alias *)
  | Some cls -> cls in
  let kind = match Class.kind cls with
  | Ast_defs.Cabstract -> "abstract class"
  | Ast_defs.Cnormal -> "class"
  | Ast_defs.Cinterface -> "interface"
  | Ast_defs.Ctrait -> "trait"
  | Ast_defs.Cenum -> "enum"
  | Ast_defs.Crecord -> "record" in
  let name = strip_namespace (Class.name cls) in
  (* TODO: traits, enums, records *)
  (* TODO: auto-indent *)
  kind^" "^name^" "^(list_direct_ancestors cls)

(* TODO: namespaces; mind that cls might start with \ at this point, remove if needed *)
let construct_class_declaration cls fields acc =
  let decl = get_class_declaration cls in
  let open Typing_deps.Dep in
  let process_field f = match f with
  | AllMembers _ | Extends _ -> raise UnexpectedDependency
  | _ -> extract_object_declaration f in
  let body = HashSet.fold (fun f accum -> accum^"\n"^(process_field f)) fields "" in
  (decl^" {\n"^body^"\n}") :: acc

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
        let td = value_exn (Decl_provider.get_typedef cls) DependencyNotFound in
        add_dep td.td_type deps
      | Some c ->
        Sequence.iter (Decl_provider.Class.all_ancestors c) (fun (_, ty) -> add_dep ty deps)
      )
  | Fun f | FunName f ->
    let func = value_exn (Decl_provider.get_fun f) DependencyNotFound in
    add_dep func.ft_ret deps;
    List.iter func.ft_params (fun p -> add_dep p.fp_type deps)
  | GConst c | GConstName c ->
    (let (ty, _) = value_exn (Decl_provider.get_gconst c) DependencyNotFound in
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
  let classes = Caml.Hashtbl.create 0 in
  let globals = HashSet.create 0 in
  let group_by_class obj = match obj with
  | Class cls -> (match Caml.Hashtbl.find_opt classes cls with
    | Some set -> HashSet.add set obj
    | None -> let set = HashSet.create 0 in Caml.Hashtbl.add classes cls set)
  | Prop (cls, _)
  | SProp (cls, _)
  | Method (cls, _)
  | SMethod (cls, _)
  | Const (cls, _)
  | Cstr cls -> (match Caml.Hashtbl.find_opt classes cls with
    | Some set -> HashSet.add set obj
    | None -> let set = HashSet.create 0 in HashSet.add set obj; Caml.Hashtbl.add classes cls set)
  | Extends _ -> raise UnexpectedDependency
  | AllMembers _ -> raise UnexpectedDependency
  | _ -> HashSet.add globals obj in
  HashSet.iter group_by_class dependencies;
  let code = HashSet.fold (fun el l -> extract_object_declaration el :: l) globals [] in
  Caml.Hashtbl.fold construct_class_declaration classes code


let go tcopt function_name =
  try
    let function_text = extract_function_body function_name in
    let dependencies = collect_dependencies tcopt function_name in
    dependencies @ [function_text]
  with FunctionNotFound -> ["Function not found!"]
