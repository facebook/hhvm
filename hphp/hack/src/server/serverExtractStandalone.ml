(*
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
exception Unsupported

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
  let ast_function = value_exn FunctionNotFound @@ Ast_provider.find_fun_in_file_nast filename func in
  let open Aast in
  let pos = ast_function.f_span in
  let include_first_whsp = Pos.merge (Pos.first_char_of_line pos) pos in
  Pos.get_text_from_pos file_content include_first_whsp

let strip_ns obj_name =
  let (_, name) = String.rsplit2_exn obj_name '\\' in
  name

(* Get "relative" namespace compared to the namespace of reference. For example,
   for reference=/a/b/C and name=/a/b/c/d/f, return c/d/f *)
let strip_ns_prefix reference name =
  let split_ns name = String.lsplit2 name '\\' in
  let reference = String.lstrip ~drop:(fun c -> c = '\\') reference in
  let name = String.lstrip ~drop:(fun c -> c = '\\') name in
  let rec strip_ reference name = match split_ns reference, split_ns name with
    | (None, None) -> name
    | (Some(ref_ns, refn), Some(ns, n)) ->
      if ref_ns = ns then strip_ refn n else name
    | (_, _) -> name in
  strip_ reference name

let list_items items = String.concat items ~sep:", "

let tparam_name (tp: Typing_defs.decl Typing_defs.tparam) = snd tp.tp_name

let get_function_declaration tcopt fun_name fun_type =
  let tparams = match fun_type.ft_tparams with
  | ([], _) -> ""
  | (tparams, _) -> Printf.sprintf "<%s>" @@ list_items @@ List.map tparams tparam_name in
  let fun_type_str = (Typing_print.fun_type tcopt fun_type) in
  Printf.sprintf "function %s%s%s" (strip_ns fun_name) tparams fun_type_str

let extract_object_declaration tcopt obj =
  let open Typing_deps.Dep in
  match obj with
  | Fun f | FunName f ->
    let fun_type = value_exn DependencyNotFound @@ Decl_provider.get_fun f in
    let declaration = get_function_declaration tcopt f fun_type in
    declaration ^ "{throw new \\Exception();}"
  | _ -> to_string obj

let rec name_from_hint hint = match hint with
  | (_, Aast.Happly((_, s), params)) -> if List.is_empty params then s
    else Printf.sprintf "%s<%s>" s (list_items @@ List.map params name_from_hint)
  | _ -> raise UnexpectedDependency

let list_direct_ancestors cls =
  let cls_pos = Decl_provider.Class.pos cls in
  let cls_name = Decl_provider.Class.name cls in
  let filename = Pos.filename cls_pos in
  let ast_class = value_exn DependencyNotFound @@ Ast_provider.find_class_in_file_nast filename cls_name in
  let get_namespaced_class_name hint = strip_ns_prefix cls_name @@ name_from_hint hint in
  let list_types hints = list_items @@ List.map hints get_namespaced_class_name in
  let open Aast in
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

let get_enum_declaration tcopt enum =
  let name = Decl_provider.Class.name enum in
  let enum = value_exn UnexpectedDependency @@ Decl_provider.Class.enum_type enum in
  let cons = match enum.te_constraint with
  | Some c -> " as " ^ (Typing_print.full_decl tcopt c)
  | None -> "" in
  let base = Typing_print.full_decl tcopt enum.te_base in
  Printf.sprintf "enum %s: %s%s" (strip_ns name) base cons

let get_class_declaration (cls: Decl_provider.class_decl) =
  let open Decl_provider in
  let kind = match Class.kind cls with
  | Ast_defs.Cabstract -> "abstract class"
  | Ast_defs.Cnormal -> "class"
  | Ast_defs.Cinterface -> "interface"
  | Ast_defs.Ctrait -> "trait"
  | Ast_defs.Cenum -> "enum"
  | Ast_defs.Crecord -> "record" in
  let name = strip_ns @@ Class.name cls in
  let tparams = if List.is_empty @@ Class.tparams cls then ""
  else Printf.sprintf "<%s>" (list_items @@ List.map (Class.tparams cls) tparam_name) in
  (* TODO: traits, records *)
  kind^" "^name^tparams^" "^(list_direct_ancestors cls)

let get_method_declaration tcopt (meth: Typing_defs.class_elt) ~is_static method_name =
  let abstract = if meth.ce_abstract then "abstract " else "" in
  let visibility = Typing_utils.string_of_visibility meth.ce_visibility in
  let static = if is_static then "static " else "" in
  let method_type = match Lazy.force meth.ce_type with
  | (_, Typing_defs.Tfun f) -> Typing_print.fun_type tcopt f
  | _ -> raise UnexpectedDependency in
  let body = if meth.ce_abstract then ";" else "{throw new Exception();}" in
  Printf.sprintf "%s%s %sfunction %s%s%s" abstract visibility static method_name method_type body

(* TODO: properties have to be initialized. Therefore we need to retrieve the initializer
expression (which is not part of class_elt) or the relevant part of the constructor *)
let get_property_declaration tcopt (prop: Typing_defs.class_elt) ~is_static name =
  let visibility = Typing_utils.string_of_visibility prop.ce_visibility in
  let static = if is_static then "static " else "" in
  let prop_type = Typing_print.full_decl tcopt @@ Lazy.force prop.ce_type in
  Printf.sprintf "%s %s%s $%s;" visibility static prop_type name

let bop_to_string = function
  | Ast_defs.Plus -> "+"
  | Ast_defs.Minus -> "-"
  | Ast_defs.Star -> "*"
  | _ -> raise Unsupported

let unop_to_string = function
  | Ast_defs.Unot -> "!"
  | Ast_defs.Uplus -> "+"
  | Ast_defs.Uminus -> "-"
  | _ -> raise Unsupported

let rec expr_to_string (expr:Nast.expr) =
  let (_, expr) = expr in
  match expr with
  | Aast.Null -> "null"
  | Aast.This -> "this"
  | Aast.True -> "true"
  | Aast.False -> "false"
  | Aast.Int i -> i
  | Aast.Float f -> f
  | Aast.String s -> s
  | Aast.Unop (op, expr) -> Printf.sprintf "(%s%s)" (unop_to_string op) (expr_to_string expr)
  | Aast.Binop (op, expr1, expr2) ->
    Printf.sprintf "(%s%s%s)" (expr_to_string expr1) (bop_to_string op) (expr_to_string expr2)
  | _ -> raise Unsupported

let get_class_const_declaration tcopt (cons: Typing_defs.class_const) name =
  let abstract = if cons.cc_abstract then "abstract " else "" in
  let typ = Typing_print.full_decl tcopt cons.cc_type in
  let init = match cons.cc_expr with
  | None -> ""
  | Some expr -> " = " ^ expr_to_string expr in
  Printf.sprintf "%sconst %s %s%s;" abstract typ name init

let extract_field_declaration tcopt (cls: Decl_provider.class_decl)
                              (field: Typing_deps.Dep.variant) =
  let open Typing_deps.Dep in
  let open Decl_provider in
  match field with
  | Const(_, const_name) ->
    let cons = value_exn DependencyNotFound @@ Class.get_const cls const_name in
    get_class_const_declaration tcopt cons const_name
  | Cstr _ -> let (cstr, _) = Class.construct cls in
    (match cstr with
    | None -> ""
    | Some cstr -> get_method_declaration tcopt cstr ~is_static:false "__construct")
  | Method(_, method_name) ->
    let m = value_exn DependencyNotFound @@ Class.get_method cls method_name in
    get_method_declaration tcopt m ~is_static:false method_name
  | SMethod(_, smethod_name) ->
    let m = value_exn DependencyNotFound @@ Class.get_smethod cls smethod_name in
    get_method_declaration tcopt m ~is_static:true smethod_name
  | Prop(_, prop_name) ->
    let p = value_exn DependencyNotFound @@ Class.get_prop cls prop_name in
    get_property_declaration tcopt p ~is_static:false prop_name
  | SProp(_, sprop_name) ->
    let sp = value_exn DependencyNotFound @@ Class.get_sprop cls sprop_name in
    get_property_declaration tcopt sp ~is_static:true (String.lstrip ~drop:(fun c -> c = '$') sprop_name)
  | _ -> ""

let construct_enum tcopt enum fields =
  let string_enum_const = function
    | Typing_deps.Dep.Const(_, name) ->
      let enum_const = value_exn DependencyNotFound @@ Decl_provider.Class.get_const enum name in
      Printf.sprintf "%s = %s;" name (expr_to_string @@ value_exn DependencyNotFound enum_const.cc_expr)
    | _ -> raise UnexpectedDependency in
  let enum_decl = get_enum_declaration tcopt enum in
  let constants = HashSet.fold (fun f acc -> (string_enum_const f) :: acc) fields [] in
  Printf.sprintf "%s {\n%s\n}" enum_decl (String.concat ~sep:"\n" constants)

let construct_class_declaration tcopt cls fields =
  let decl = get_class_declaration cls in
  (* Enum declaration have a very different format: for example, no 'const' keyword
     for values, which are actually just class constants *)
  if Decl_provider.Class.kind cls = Ast_defs.Cenum then (construct_enum tcopt cls fields)
  else
  let open Typing_deps.Dep in
  let process_field f = match f with
  | AllMembers _ | Extends _ -> raise UnexpectedDependency
  | _ -> extract_field_declaration tcopt cls f in
  let body = HashSet.fold (fun f accum -> accum^"\n"^(process_field f)) fields "" in
  decl^" {"^body^"}"

let construct_typedef_declaration tcopt t =
  let td = value_exn DependencyNotFound @@ Decl_provider.get_typedef t in
  let typ = if td.td_vis = Aast_defs.Transparent then "type" else "newtype" in
  Printf.sprintf "%s %s = %s;" typ (strip_ns t) (Typing_print.full_decl tcopt td.td_type)

let construct_type_declaration tcopt t fields =
  match Decl_provider.get_class t with
  | Some cls -> construct_class_declaration tcopt cls fields
  | None -> construct_typedef_declaration tcopt t

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

let get_dependency_origin cls (dep: Typing_deps.Dep.variant) =
  let open Decl_provider in
  let open Typing_deps.Dep in
  let cls = value_exn DependencyNotFound @@ get_class cls in
  match dep with
  | Prop (_, name) ->
    let p = value_exn DependencyNotFound @@ Class.get_prop cls name in
    p.ce_origin
  | SProp (_, name) ->
    let sp = value_exn DependencyNotFound @@ Class.get_sprop cls name in
    sp.ce_origin
  | Method (_, name) ->
    let m = value_exn DependencyNotFound @@ Class.get_method cls name in
    m.ce_origin
  | SMethod (_, name) ->
    let sm = value_exn DependencyNotFound @@ Class.get_smethod cls name in
    sm.ce_origin
  | Const (_, name) ->
    let c = value_exn DependencyNotFound @@ Class.get_const cls name in
    c.cc_origin
  | Cstr _ ->
    let c = value_exn DependencyNotFound @@ fst @@ Class.construct cls in
    c.ce_origin
  | _ -> assert false

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
  let group_by_type obj =
    match obj with
    | Class cls ->
      (match Caml.Hashtbl.find_opt types cls with
      | Some _ -> ()
      | None -> let set = HashSet.create 0 in Caml.Hashtbl.add types cls set)
    | Prop (cls, _)
    | SProp (cls, _)
    | Method (cls, _)
    | SMethod (cls, _)
    | Const (cls, _)
    | Cstr cls -> (let origin = get_dependency_origin cls obj in
      (* Consider the following example:
      * class Base {
      *   public static function do(): void {}
      * }
      * class Derived {}
      * function f(): void {
      *   Derived::do();
      * }
      * We will pull both SMethod(Base, do) and SMethod(Derived, do) as dependencies, but we should
      * not generate method do() in Derived. Therefore we should ignore dependencies whose origin
      * differ from their class. *)
      if origin = cls then
      match Caml.Hashtbl.find_opt types cls with
      | Some set -> HashSet.add set obj
      | None -> let set = HashSet.create 0 in HashSet.add set obj;
                Caml.Hashtbl.add types cls set)
    | Extends _ -> raise UnexpectedDependency
    | AllMembers _ -> raise UnexpectedDependency
    | _ -> HashSet.add globals obj in
  HashSet.iter group_by_type dependencies;
  (types, globals)

let global_dep_name = function
  | Typing_deps.Dep.GConst s | Typing_deps.Dep.GConstName s
  | Typing_deps.Dep.Fun s | Typing_deps.Dep.FunName s | Typing_deps.Dep.Class s -> s
  | _ -> raise UnexpectedDependency

(* Every namespace can contain declarations of classes, functions, constants
   as well as nested namespaces *)
type hack_namespace = {
  namespaces : (string, hack_namespace) Caml.Hashtbl.t;
  decls : string HashSet.t;
}

let subnamespace index name =
  let nspaces, _ = String.rsplit2_exn ~on:'\\' name in
  if nspaces = "" then None else
  let nspaces = String.strip ~drop:(fun c -> c = '\\') nspaces in
  let nspaces = String.split ~on:'\\' nspaces in
  List.nth nspaces index

(* TODO: to be continued? *)
let is_builtin name = String.is_prefix ~prefix:"\\HH" name

(* Build the recursive hack_namespace data structure for given declarations *)
let sort_by_namespace declarations =
  let rec add_decl nspace decl index =
    (* Ignore builtins because we shouldn't generate their declarations *)
    if is_builtin decl then () else
    match subnamespace index decl with
    | Some name -> if Caml.Hashtbl.find_opt nspace.namespaces name = None then
      (let nested = Caml.Hashtbl.create 0 in
      let declarations = HashSet.create 0 in
      Caml.Hashtbl.add nspace.namespaces name { namespaces = nested; decls = declarations});
      add_decl (Caml.Hashtbl.find nspace.namespaces name) decl (index + 1)
    | None -> HashSet.add nspace.decls decl in
  let namespaces = { namespaces = Caml.Hashtbl.create 0; decls = HashSet.create 0 } in
  HashSet.iter (fun decl -> add_decl namespaces decl 0) declarations;
  namespaces

(* Takes declarations of Hack classes, functions, constants (map name -> code)
   and produces file(s) with Hack code:
    1) Groups declarations by namespaces, creating hack_namespace data structure
    2) Recursively prints the code in every namespace.
   Special case: since Hack files cannot contain both namespaces and toplevel
   declarations, we "generate" a separate file for toplevel declarations, using
   hh_single_type_check multifile syntax.
*)
let get_code (decl_names: string HashSet.t) declarations =
  let global_namespace = sort_by_namespace decl_names in
  let code_from_namespace_decls name acc =
    List.append (Caml.Hashtbl.find_all declarations name) acc in
  let hh_prefix = "<?hh" in
  (* Toplevel code has to be in a separate file *)
  let toplevel = HashSet.fold code_from_namespace_decls global_namespace.decls [] in
  let toplevel = format @@ String.concat ~sep:"\n" @@ hh_prefix :: toplevel in
  let rec code_from_namespace nspace_name nspace_content code =
    let code = "}\n"::code in
    let code = Caml.Hashtbl.fold code_from_namespace nspace_content.namespaces code in
    let code = HashSet.fold code_from_namespace_decls nspace_content.decls code in
    Printf.sprintf "namespace %s {" nspace_name :: code in
  let code = Caml.Hashtbl.fold code_from_namespace global_namespace.namespaces [] in
  let code = format @@ String.concat ~sep:"\n" @@ hh_prefix :: code in
  let file_or_ignore filename code =
    if (String.strip code) = hh_prefix then ""
    else Printf.sprintf "////%s\n%s" filename code in
  (file_or_ignore "toplevel.php" toplevel) ^ (file_or_ignore "namespaces.php" code)

let go tcopt function_name =
  try
    let (types, globals) = collect_dependencies tcopt function_name in
    let declarations = Caml.Hashtbl.create 0 in
    let decl_names = HashSet.create 0 in
    let add_global_declaration dep =
      let name = global_dep_name dep in
      HashSet.add decl_names name;
      Caml.Hashtbl.add declarations name (extract_object_declaration tcopt dep) in
    let add_class_declaration cls fields =
      HashSet.add decl_names cls;
      Caml.Hashtbl.add declarations cls (construct_type_declaration tcopt cls fields) in
    HashSet.iter add_global_declaration globals;
    Caml.Hashtbl.iter add_class_declaration types;
    let function_text = extract_function_body function_name in
    Caml.Hashtbl.add declarations function_name function_text;
    HashSet.add decl_names function_name;
    get_code decl_names declarations
  with FunctionNotFound -> "Function not found!"
