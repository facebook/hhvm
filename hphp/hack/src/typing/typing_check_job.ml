(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*
####

The type checking service receives a list of files and their symbols as input and
distributes the work to worker processes.

In this file you will only find functions to process a single file at a time
*)

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let handle_exn_as_error : type res. Pos.t -> (unit -> res option) -> res option
    =
 fun pos f ->
  try f () with
  | (WorkerCancel.Worker_should_exit | Deferred_decl.Defer) as exn ->
    (* Cancellation requests must be re-raised *)
    let e = Exception.wrap exn in
    Exception.reraise e
  | exn ->
    let e = Exception.wrap exn in
    Errors.exception_occurred pos e;
    None

let type_fun (ctx : Provider_context.t) ~(full_ast : Nast.fun_def) :
    Tast.def Tast_with_dynamic.t option =
  let f = full_ast.Aast.fd_fun in
  handle_exn_as_error f.Aast.f_span (fun () ->
      let fun_ = Naming.fun_def ctx full_ast in
      Nast_check.def ctx (Aast.Fun fun_);
      let def_opt =
        Typing_toplevel.fun_def ctx fun_
        |> Option.map ~f:(Tast_with_dynamic.map ~f:(fun f -> Aast.Fun f))
      in
      (* Only do TAST check on normal TAST *)
      Option.iter def_opt ~f:(fun fs ->
          Tast_check.def ctx fs.Tast_with_dynamic.under_normal_assumptions);
      def_opt)

let type_class (ctx : Provider_context.t) ~(full_ast : Nast.class_) :
    Tast.def Tast_with_dynamic.t option =
  handle_exn_as_error full_ast.Aast.c_span (fun () ->
      let class_ = Naming.class_ ctx full_ast in
      Nast_check.def ctx (Aast.Class class_);
      let def_opt =
        Typing_toplevel.class_def ctx class_
        |> Option.map ~f:(Tast_with_dynamic.map ~f:(fun c -> Aast.Class c))
      in
      (* Only do TAST check on normal TAST *)
      Option.iter def_opt ~f:(fun c ->
          Tast_check.def ctx c.Tast_with_dynamic.under_normal_assumptions);
      def_opt)

let check_typedef (ctx : Provider_context.t) ~(full_ast : Nast.typedef) :
    Tast.def option =
  handle_exn_as_error Pos.none (fun () ->
      let typedef = Naming.typedef ctx full_ast in
      Nast_check.def ctx (Aast.Typedef typedef);
      let ret = Typing_toplevel.typedef_def ctx typedef in
      let def = Aast.Typedef ret in
      Tast_check.def ctx def;
      Some def)

let check_const (ctx : Provider_context.t) ~(full_ast : Nast.gconst) :
    Tast.def option =
  handle_exn_as_error full_ast.Aast.cst_span (fun () ->
      let cst = Naming.global_const ctx full_ast in
      Nast_check.def ctx (Aast.Constant cst);
      let def = Aast.Constant (Typing_toplevel.gconst_def ctx cst) in
      Tast_check.def ctx def;
      Some def)

let check_module (ctx : Provider_context.t) ~(full_ast : Nast.module_def) :
    Tast.def option =
  handle_exn_as_error (fst full_ast.Aast.md_name) (fun () ->
      let md = Naming.module_ ctx full_ast in
      Nast_check.def ctx (Aast.Module md);
      let def = Aast.Module (Typing_toplevel.module_def ctx md) in
      Tast_check.def ctx def;
      Some def)

let calc_errors_and_tast ctx ?(drop_fixmed = true) fn ~full_ast :
    Errors.t * Tast.by_names =
  let { Nast.funs; classes; typedefs; constants; modules } =
    Nast.get_defs full_ast
  in
  let calc_tast
      (type def res)
      (typecheck : Provider_context.t -> full_ast:def -> res option)
      (defs : (FileInfo.id * def) list) : res SMap.t =
    List.fold defs ~init:SMap.empty ~f:(fun acc (id, full_ast) ->
        typecheck ctx ~full_ast
        |> Option.fold ~init:acc ~f:(fun acc tast ->
               SMap.add (FileInfo.id_name id) tast acc))
  in
  Errors.do_with_context ~drop_fixmed fn (fun () ->
      (* Some of our tests depend upon the order of [calc_tast] being exactly as follows, i.e. funs
         first, classes next, and so on. This is likely irrelevant to end user experience though,
         since user gets sorted errors. *)
      let fun_tasts = calc_tast type_fun funs in
      let class_tasts = calc_tast type_class classes in
      let typedef_tasts = calc_tast check_typedef typedefs in
      let gconst_tasts = calc_tast check_const constants in
      let module_tasts = calc_tast check_module modules in
      { Tast.fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts })

let calc_errors_and_tast_for
    (type def res)
    (ctx : Provider_context.t)
    ?(drop_fixmed = true)
    (fn : Relative_path.t)
    (typecheck : Provider_context.t -> full_ast:def -> res option)
    ~(full_ast : def)
    (id : FileInfo.id) : Errors.t * res SMap.t =
  Errors.do_with_context ~drop_fixmed fn (fun () ->
      typecheck ctx ~full_ast
      |> Option.fold ~init:SMap.empty ~f:(fun acc tast ->
             SMap.add (FileInfo.id_name id) tast acc))
