(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)

open Hh_prelude
open Common
module Env = Naming_phase_env

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)
let (on_error, reset_errors, get_errors) =
  let naming_errs = ref Naming_phase_error.empty in
  let reset_errors () = naming_errs := Naming_phase_error.empty
  and get_errors () = !naming_errs
  and on_error err = naming_errs := Naming_phase_error.add !naming_errs err in
  (on_error, reset_errors, get_errors)

let invalid_expr_ = Naming_phase_error.invalid_expr_

let mk_env filename tcopt =
  let file_str = Relative_path.suffix filename in
  let is_hhi = String.is_suffix (Relative_path.suffix filename) ~suffix:".hhi"
  and is_systemlib = TypecheckerOptions.is_systemlib tcopt
  and allow_module_def =
    TypecheckerOptions.allow_all_files_for_module_declarations tcopt
    || List.exists
         ~f:(fun allowed_file ->
           let len = String.length allowed_file in
           if len > 0 then
             match allowed_file.[len - 1] with
             | '*' ->
               let allowed_dir =
                 String.sub allowed_file ~pos:0 ~len:(len - 1)
               in
               String.is_prefix file_str ~prefix:allowed_dir
             | _ -> String.equal allowed_file file_str
           else
             false)
         (TypecheckerOptions.allowed_files_for_module_declarations tcopt)
  and everything_sdt = TypecheckerOptions.everything_sdt tcopt
  and supportdynamic_type_hint_enabled =
    TypecheckerOptions.experimental_feature_enabled
      tcopt
      TypecheckerOptions.experimental_supportdynamic_type_hint
  and hkt_enabled = TypecheckerOptions.higher_kinded_types tcopt
  and like_type_hints_enabled = TypecheckerOptions.like_type_hints tcopt
  and soft_as_like = TypecheckerOptions.interpret_soft_types_as_like_types tcopt
  and consistent_ctor_level =
    TypecheckerOptions.explicit_consistent_constructors tcopt
  in
  Env.
    {
      empty with
      is_hhi;
      is_systemlib;
      consistent_ctor_level;
      allow_module_def;
      everything_sdt;
      supportdynamic_type_hint_enabled;
      hkt_enabled;
      like_type_hints_enabled;
      soft_as_like;
    }

let passes =
  [
    (* Stop on `Invalid` expressions *)
    Naming_guard_invalid.pass;
    (* Canonicalization passes -------------------------------------------- *)
    (* Remove top-level file attributes, noop and markup statements *)
    Naming_elab_defs.pass;
    (* Remove function bodies when in hhi mode *)
    Naming_elab_func_body.pass;
    (* Flatten `Block` statements *)
    Naming_elab_block.pass;
    (* Strip `Hsoft` hints or replace with `Hlike` *)
    Naming_elab_soft.pass;
    (* Elaborate `Happly` to canonical representation, if any *)
    Naming_elab_happly_hint.pass on_error;
    (* Elaborate class identifier expressions (`CIexpr`) to canonical
        representation: `CIparent`, `CIself`, `CIstatic`, `CI` _or_
       `CIexpr (_,_, Lvar _ | This )` *)
    Naming_elab_class_id.pass on_error;
    (* Strip type parameters from type parameters when HKTs are not enabled *)
    Naming_elab_hkt.pass on_error;
    (* Elaborate `Collection` to `ValCollection` or `KeyValCollection` *)
    Naming_elab_collection.pass on_error;
    (* Check that user attributes are well-formed *)
    Naming_elab_user_attributes.pass on_error;
    (* Replace import expressions with invalid expression marker *)
    Naming_elab_import.pass;
    (* Elaborate local variables to canonical representation *)
    Naming_elab_lvar.pass;
    (* Warn of explicit use of builtin enum classes; make subtyping of
       enum classes explicit*)
    Naming_elab_enum_class.pass on_error;
    (* Elaborate class members & xhp attributes  *)
    Naming_elab_class_members.pass on_error;
    (* Elaborate special function calls to canonical representation, if any *)
    Naming_elab_call.top_down_pass;
    Naming_elab_call.bottom_up_pass on_error;
    (* Elaborate invariant calls to canonical representation *)
    Naming_elab_invariant.pass on_error;
    (* -- Mark invalid hints and expressions & miscellaneous validation --- *)
    Naming_elab_retonly_hint.pass on_error;
    Naming_elab_wildcard_hint.pass on_error;
    (* Replace uses to `self` in shape field names with referenced class *)
    Naming_elab_shape_field_name.top_down_pass;
    Naming_elab_shape_field_name.bottom_up_pass on_error;
    Naming_elab_this_hint.pass on_error;
    Naming_elab_haccess_hint.pass on_error;
    (* Replace empty `Tuple`s with invalid expression marker *)
    Naming_elab_tuple.pass on_error;
    (* Validate / replace invalid uses of dynamic classes in `New` and `Class_get`
       expressions *)
    Naming_elab_dynamic_class_name.pass on_error;
    (* Replace non-constant class or global constant with invalid expression marker *)
    Naming_elab_const_expr.top_down_pass on_error;
    Naming_elab_const_expr.bottom_up_pass on_error;
    (* Replace malformed key / value bindings in as expressions with invalid
       local var markers *)
    Naming_elab_as_expr.pass on_error;
    (* Validate hints used in `Cast` expressions *)
    Naming_validate_cast_expr.pass on_error;
    (* Validate where `dynamic` can be used in a hint *)
    Naming_validate_dynamic_hint.pass on_error;
    (* Check for duplicate function parameter names *)
    Naming_validate_fun_params.pass on_error;
    (* Validate use of `require implements`, `require extends` and
       `require class` declarations for traits, interfaces and classes *)
    Naming_validate_class_req.pass on_error;
    (* Validation dealing with common xhp naming errors *)
    Naming_validate_xhp_name.pass on_error;
    (* -- Elaboration & validation under typechecker options -------------- *)
    (* Add `supportdyn` and `Like` wrappers everywhere - under `everything-sdt`
       typechecker option *)
    Naming_elab_everything_sdt.top_down_pass;
    Naming_elab_everything_sdt.bottom_up_pass;
    (* Validate use of `Hlike` hints - depends on `enable-like-type-hints` typechecker option *)
    Naming_validate_like_hint.pass on_error;
    (* Validate constructors under
       `consistent-explicit_consistent_constructors` typechecker option *)
    Naming_validate_consistent_construct.pass on_error;
    (* Validate  use of `SupportDyn` class - depends on `enable-supportdyn`
       and `everything_sdt` typechecker options *)
    Naming_validate_supportdyn.pass on_error;
    (* Validate use of module definitions - depends on:
        - `allow_all_files_for_module_declarations`
        - `allowed_files_for_module_declarations`
       typechecker options *)
    Naming_validate_module.pass on_error;
    (* Validate that there are no top-level statements *)
    Naming_validate_toplevel_statement.pass on_error;
  ]

(* If we don't delegate to Rust, then all naming passes are done here in OCaml.
   *)
let ( elab_core_program,
      elab_core_class,
      elab_core_fun_def,
      elab_core_module_def,
      elab_core_gconst,
      elab_core_typedef,
      elab_core_stmt ) =
  Naming_phase_pass.mk_visitor passes

let elab_elem
    elem ~elab_ns ~elab_capture ~elab_typed_locals ~validate_await ~elab_core =
  reset_errors ();
  let elem = elab_ns elem |> elab_capture |> elab_typed_locals in
  validate_await elem;
  let elem = elab_core elem in
  Naming_phase_error.emit @@ get_errors ();
  reset_errors ();
  elem

let program_filename defs =
  let open Aast_defs in
  let rec aux = function
    | Fun fun_def :: _ -> Pos.filename fun_def.fd_fun.f_span
    | Class class_ :: _ -> Pos.filename class_.c_span
    | Stmt (pos, _) :: _ -> Pos.filename pos
    | Typedef typedef :: _ -> Pos.filename typedef.t_span
    | Constant gconst :: _ -> Pos.filename gconst.cst_span
    | Module module_def :: _ -> Pos.filename module_def.md_span
    | _ :: rest -> aux rest
    | _ -> Relative_path.default
  in
  aux defs

(**************************************************************************)
(* The entry points to CHECK the program, and transform the program *)
(**************************************************************************)

module Rust_elab_core = struct
  external elab_program :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.program ->
    Nast.program * Naming_phase_error.t list = "hh_elab_program"

  external elab_fun_def :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.fun_def ->
    Nast.fun_def * Naming_phase_error.t list = "hh_elab_fun_def"

  external elab_class_ :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.class_ ->
    Nast.class_ * Naming_phase_error.t list = "hh_elab_class_"

  external elab_module_def :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.module_def ->
    Nast.module_def * Naming_phase_error.t list = "hh_elab_module_def"

  external elab_gconst :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.gconst ->
    Nast.gconst * Naming_phase_error.t list = "hh_elab_gconst"

  external elab_typedef :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.typedef ->
    Nast.typedef * Naming_phase_error.t list = "hh_elab_typedef"

  external elab_stmt :
    TypecheckerOptions.t ->
    Relative_path.t ->
    (unit, unit) Aast.stmt ->
    Nast.stmt * Naming_phase_error.t list = "hh_elab_stmt"

  let add_errors elab_x tcopt filename x =
    let (x, errs) = elab_x tcopt filename x in
    errs
    |> List.fold ~init:Naming_phase_error.empty ~f:Naming_phase_error.add
    |> Naming_phase_error.emit;
    x

  let elab_program = add_errors elab_program

  let elab_fun_def = add_errors elab_fun_def

  let elab_class_ = add_errors elab_class_

  let elab_module_def = add_errors elab_module_def

  let elab_gconst = add_errors elab_gconst

  let elab_typedef = add_errors elab_typedef

  let elab_stmt = add_errors elab_stmt
end

let fun_def ctx fd =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = Pos.filename fd.Aast.fd_fun.Aast.f_span in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_fun_def tcopt filename fd
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_fun_def
    and elab_capture = Naming_captures.elab_fun_def
    and elab_typed_locals = Naming_typed_locals.elab_fun_def
    and validate_await = Naming_validate_await.validate_fun_def on_error
    and elab_core = elab_core_fun_def (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      fd

let class_ ctx c =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = Pos.filename c.Aast.c_span in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_class_ tcopt filename c
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_class_
    and elab_capture = Naming_captures.elab_class
    and elab_typed_locals = Naming_typed_locals.elab_class
    and validate_await = Naming_validate_await.validate_class on_error
    and elab_core = elab_core_class (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      c

let module_ ctx md =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = Pos.filename md.Aast.md_span in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_module_def tcopt filename md
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_module_def
    and elab_capture = Naming_captures.elab_module_def
    and elab_typed_locals x = x
    and validate_await _ = ()
    and elab_core = elab_core_module_def (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      md

let global_const ctx cst =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = Pos.filename cst.Aast.cst_span in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_gconst tcopt filename cst
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_gconst
    and elab_capture = Naming_captures.elab_gconst
    and elab_typed_locals x = x
    and validate_await _ = ()
    and elab_core = elab_core_gconst (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      cst

let typedef ctx td =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = Pos.filename @@ td.Aast.t_span in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_typedef tcopt filename td
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_typedef
    and elab_capture = Naming_captures.elab_typedef
    and elab_typed_locals x = x
    and validate_await _ = ()
    and elab_core = elab_core_typedef (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      td

let fun_def_of_stmts ctx stmts : Nast.fun_def option =
  let stmts =
    List.filter stmts ~f:(function
        | (_, Aast.Markup _) -> false
        | _ -> true)
  in
  match stmts with
  | [] -> None
  | (pos, _) :: tail ->
    let pos = tail |> List.map ~f:fst |> List.fold ~init:pos ~f:Pos.merge in
    let stmts =
      let tcopt = Provider_context.get_tcopt ctx in
      let filename = Pos.filename pos in
      let elab_stmt stmt : Nast.stmt =
        if TypecheckerOptions.rust_elab tcopt then
          Rust_elab_core.elab_stmt tcopt filename stmt
        else
          let elab_ns = Naming_elaborate_namespaces_endo.elaborate_stmt
          and elab_capture = Naming_captures.elab_stmt
          and elab_typed_locals = Naming_typed_locals.elab_stmt
          and validate_await = Naming_validate_await.validate_stmt on_error
          and elab_core = elab_core_stmt (mk_env filename tcopt) in
          elab_elem
            ~elab_ns
            ~elab_capture
            ~elab_typed_locals
            ~validate_await
            ~elab_core
            stmt
      in
      List.map stmts ~f:elab_stmt
    in
    Some
      Aast.
        {
          fd_name =
            (pos, "")
            (* An empty function name is appropriate, as the AST can already include empty function names due to parse errors *);
          fd_namespace = Namespace_env.empty_with_default;
          fd_file_attributes = [];
          fd_mode = FileInfo.Mstrict;
          fd_fun =
            {
              f_span = pos;
              f_readonly_this = None;
              f_annotation = ();
              f_readonly_ret = None;
              f_ret =
                ( (),
                  Some
                    ( pos,
                      Happly
                        ( (pos, Naming_special_names.Classes.cAwaitable),
                          [(pos, Hprim Tvoid)] ) ) );
              f_params = [];
              f_body = { fb_ast = stmts };
              f_ctxs = None;
              f_unsafe_ctxs = None;
              f_fun_kind = Ast_defs.FAsync (* enable top-level await *);
              f_user_attributes = [];
              f_external = false;
              f_doc_comment = None;
            };
          fd_internal = false;
          fd_module = None;
          fd_tparams = [];
          fd_where_constraints = [];
        }

(** Return an updated [Nast.program] where top-level statements
* are wrapped in a function. This is not an elaboration pass
* because it affects the entire file and whole-file elaboration passes
* are not used in `Typing_check_job.calc_errors_and_tast`.
* Shared logic with `Typing_check_job.calc_errors_and_tast`
* is in `fun_def_of_stmts`.
*)
let adjust_toplevel_stmts ctx program : Nast.program =
  let (toplevel_defs, toplevel_stmts) =
    List.partition_map program ~f:(function
        | Aast.Stmt s -> Either.Second s
        | def -> Either.First def)
  in
  match fun_def_of_stmts ctx toplevel_stmts with
  | Some def -> Aast.Fun def :: toplevel_defs
  | None -> toplevel_defs

let program ctx program =
  let tcopt = Provider_context.get_tcopt ctx in
  let filename = program_filename program in
  let program = adjust_toplevel_stmts ctx program in
  if TypecheckerOptions.rust_elab tcopt then
    Rust_elab_core.elab_program tcopt filename program
  else
    let elab_ns = Naming_elaborate_namespaces_endo.elaborate_program
    and elab_capture = Naming_captures.elab_program
    and elab_typed_locals = Naming_typed_locals.elab_program
    and validate_await = Naming_validate_await.validate_program on_error
    and elab_core = elab_core_program (mk_env filename tcopt) in
    elab_elem
      ~elab_ns
      ~elab_capture
      ~elab_typed_locals
      ~validate_await
      ~elab_core
      program
