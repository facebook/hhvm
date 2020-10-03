(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let tracked_names : FileInfo.names option ref = ref None

let start_tracking () : unit = tracked_names := Some FileInfo.empty_names

let record_fun (s : string) : unit =
  match !tracked_names with
  | None -> ()
  | Some names ->
    tracked_names :=
      Some FileInfo.{ names with n_funs = SSet.add s names.n_funs }

let record_class (s : string) : unit =
  match !tracked_names with
  | None -> ()
  | Some names ->
    tracked_names :=
      Some FileInfo.{ names with n_classes = SSet.add s names.n_classes }

let record_record_def (s : string) : unit =
  match !tracked_names with
  | None -> ()
  | Some names ->
    tracked_names :=
      Some
        FileInfo.{ names with n_record_defs = SSet.add s names.n_record_defs }

let record_typedef (s : string) : unit =
  match !tracked_names with
  | None -> ()
  | Some names ->
    tracked_names :=
      Some FileInfo.{ names with n_types = SSet.add s names.n_types }

let record_const (s : string) : unit =
  match !tracked_names with
  | None -> ()
  | Some names ->
    tracked_names :=
      Some FileInfo.{ names with n_consts = SSet.add s names.n_consts }

let stop_tracking () : FileInfo.names =
  let res =
    match !tracked_names with
    | None ->
      Hh_logger.log
        "Warning: called Decl.stop_tracking without corresponding start_tracking";
      FileInfo.empty_names
    | Some names -> names
  in
  tracked_names := None;
  res

(*****************************************************************************)
let rec name_and_declare_types_program
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (prog : Nast.program) :
    unit =
  let open Aast in
  List.iter prog (fun def ->
      match def with
      | Namespace (_, prog) -> name_and_declare_types_program ~sh ctx prog
      | NamespaceUse _ -> ()
      | SetNamespaceEnv _ -> ()
      | FileAttributes _ -> ()
      | Fun f ->
        let (_ : string * Typing_defs.fun_elt) =
          Decl_nast.fun_naming_and_decl ~write_shmem:true ctx f
        in
        ()
      | Class c ->
        let class_env = Decl_folded_class.{ ctx; stack = SSet.empty } in
        let (_ : (string * Decl_defs.decl_class_type) option) =
          Decl_folded_class.class_decl_if_missing ~sh class_env c
        in
        ()
      | RecordDef rd -> Decl_nast.record_def_decl_if_missing ~sh ctx rd
      | Typedef typedef -> Decl_nast.typedef_decl_if_missing ~sh ctx typedef
      | Stmt _ -> ()
      | Constant cst ->
        let (_ : string * Typing_defs.decl_ty) =
          Decl_nast.const_naming_and_decl ~write_shmem:true ctx cst
        in
        ())

let make_env
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (fn : Relative_path.t) :
    unit =
  let ast = Ast_provider.get_ast ctx fn in
  name_and_declare_types_program ~sh ctx ast;
  ()

let err_not_found (file : Relative_path.t) (name : string) : 'a =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
  in
  raise (Decl_defs.Decl_not_found err_str)

let declare_class_in_file
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Decl_defs.decl_class_type option =
  match Ast_provider.find_class_in_file ctx file name with
  | Some cls ->
    let class_env = Decl_folded_class.{ ctx; stack = SSet.empty } in
    (match Decl_folded_class.class_decl_if_missing ~sh class_env cls with
    | None -> None
    | Some (name, decl) ->
      record_class name;
      Some decl)
  | None -> err_not_found file name

let declare_fun_in_file
    ~(write_shmem : bool)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Typing_defs.fun_elt =
  match Ast_provider.find_fun_in_file ctx file name with
  | Some f ->
    let (name, decl) = Decl_nast.fun_naming_and_decl ~write_shmem ctx f in
    record_fun name;
    decl
  | None -> err_not_found file name

let declare_record_def_in_file
    ~(write_shmem : bool)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Typing_defs.record_def_type =
  match Ast_provider.find_record_def_in_file ctx file name with
  | Some rd ->
    let (name, decl) =
      Decl_nast.record_def_naming_and_decl ~write_shmem ctx rd
    in
    record_record_def name;
    decl
  | None -> err_not_found file name

let declare_typedef_in_file
    ~(write_shmem : bool)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Typing_defs.typedef_type =
  match Ast_provider.find_typedef_in_file ctx file name with
  | Some t ->
    let (name, decl) = Decl_nast.typedef_naming_and_decl ~write_shmem ctx t in
    record_typedef name;
    decl
  | None -> err_not_found file name

let declare_const_in_file
    ~(write_shmem : bool)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Typing_defs.decl_ty =
  match Ast_provider.find_gconst_in_file ctx file name with
  | Some cst ->
    let (name, decl) = Decl_nast.const_naming_and_decl ~write_shmem ctx cst in
    record_const name;
    decl
  | None -> err_not_found file name
