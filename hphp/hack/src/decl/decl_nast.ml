(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to convert an AST into a type signature. Produces signatures for
 * functions, global constants, typedefs, and records. Classes are handled in
 * Decl_folded_class (folded decl) or Typing_classes_heap (shallow decl). *)
(*****************************************************************************)

open Hh_prelude
open Decl_fun_utils
open Aast
open Typing_defs
open Typing_deps
module Reason = Typing_reason
module SN = Naming_special_names

(*****************************************************************************)
(* Section declaring the type of a function *)
(*****************************************************************************)

let rec fun_naming_and_decl
    ~(write_shmem : bool) (ctx : Provider_context.t) (f : Nast.fun_) :
    string * Typing_defs.fun_elt =
  let f = Errors.ignore_ (fun () -> Naming.fun_ ctx f) in
  let fe = fun_decl ctx f in
  if write_shmem then Decl_heap.Funs.add (snd f.f_name) fe;
  (snd f.f_name, fe)

and fun_decl (ctx : Provider_context.t) (f : Nast.fun_) : Typing_defs.fun_elt =
  let dep = Dep.Fun (snd f.f_name) in
  let env = { Decl_env.mode = f.f_mode; droot = Some dep; ctx } in
  fun_decl_in_env env ~is_lambda:false f

and fun_decl_in_env (env : Decl_env.env) ~(is_lambda : bool) (f : Nast.fun_) :
    Typing_defs.fun_elt =
  check_params f.f_params;
  let reactivity = fun_reactivity env f.f_user_attributes in
  let returns_mutable = fun_returns_mutable f.f_user_attributes in
  let returns_void_to_rx = fun_returns_void_to_rx f.f_user_attributes in
  let return_disposable = has_return_disposable_attribute f.f_user_attributes in
  let params = make_params env ~is_lambda f.f_params in
  let capability =
    hint_to_type
      ~is_lambda:false
      ~default:
        (Typing_make_type.default_capability (Reason.Rhint (fst f.f_name)))
      env
      (Reason.Rwitness (fst f.f_name))
      (hint_of_type_hint f.f_cap)
  in
  let ret_ty =
    ret_from_fun_kind
      ~is_lambda
      env
      (fst f.f_name)
      f.f_fun_kind
      (hint_of_type_hint f.f_ret)
  in
  let arity =
    match f.f_variadic with
    | FVvariadicArg param ->
      assert param.param_is_variadic;
      Fvariadic (make_param_ty env ~is_lambda param)
    | FVellipsis p -> Fvariadic (make_ellipsis_param_ty p)
    | FVnonVariadic -> Fstandard
  in
  let tparams = List.map f.f_tparams (type_param env) in
  let where_constraints =
    List.map f.f_where_constraints (where_constraint env)
  in
  let fe_deprecated =
    Naming_attributes_deprecated.deprecated
      ~kind:"function"
      f.f_name
      f.f_user_attributes
  in
  let fe_php_std_lib =
    Naming_attributes.mem SN.UserAttributes.uaPHPStdLib f.f_user_attributes
  in
  let fe_type =
    mk
      ( Reason.Rwitness (fst f.f_name),
        Tfun
          {
            ft_arity = arity;
            ft_tparams = tparams;
            ft_where_constraints = where_constraints;
            ft_params = params;
            ft_implicit_params = { capability };
            ft_ret = { et_type = ret_ty; et_enforced = false };
            ft_reactive = reactivity;
            ft_flags =
              make_ft_flags
                f.f_fun_kind
                (* Functions can't be mutable because they don't have "this" *)
                None
                ~returns_mutable
                ~return_disposable
                ~returns_void_to_rx;
          } )
  in
  { fe_pos = fst f.f_name; fe_type; fe_deprecated; fe_php_std_lib }

(*****************************************************************************)
(* Dealing with records *)
(*****************************************************************************)

let record_def_decl (rd : Nast.record_def) : Typing_defs.record_def_type =
  let extends =
    match rd.rd_extends with
    (* The only valid type hint for record parents is a record
       name. Records do not support generics. *)
    | Some (_, Happly (id, [])) -> Some id
    | _ -> None
  in
  let fields =
    List.map rd.rd_fields ~f:(fun (id, _, default) ->
        match default with
        | Some _ -> (id, Typing_defs.HasDefaultValue)
        | None -> (id, ValueRequired))
  in
  {
    rdt_name = rd.rd_name;
    rdt_extends = extends;
    rdt_fields = fields;
    rdt_abstract = rd.rd_abstract;
    rdt_pos = rd.rd_span;
  }

let record_def_naming_and_decl
    ~(write_shmem : bool) (ctx : Provider_context.t) (rd : Nast.record_def) :
    string * Typing_defs.record_def_type =
  let rd = Errors.ignore_ (fun () -> Naming.record_def ctx rd) in
  let tdecl = record_def_decl rd in
  if write_shmem then Decl_heap.RecordDefs.add (snd rd.rd_name) tdecl;
  (snd rd.rd_name, tdecl)

let record_def_decl_if_missing
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (rd : Nast.record_def) :
    unit =
  let SharedMem.Uses = sh in
  let (_, rdid) = rd.rd_name in
  if not (Decl_heap.RecordDefs.mem rdid) then
    let (_ : string * Typing_defs.record_def_type) =
      record_def_naming_and_decl ~write_shmem:true ctx rd
    in
    ()

(*****************************************************************************)
(* Dealing with typedefs *)
(*****************************************************************************)

let rec typedef_decl_if_missing
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (typedef : Nast.typedef) :
    unit =
  let SharedMem.Uses = sh in
  let (_, name) = typedef.t_name in
  if not (Decl_heap.Typedefs.mem name) then
    let (_ : string * typedef_type) =
      typedef_naming_and_decl ~write_shmem:true ctx typedef
    in
    ()

and typedef_decl (ctx : Provider_context.t) (tdef : Nast.typedef) :
    Typing_defs.typedef_type =
  let {
    t_annotation = ();
    t_name = (td_pos, tid);
    t_tparams = params;
    t_constraint = tcstr;
    t_kind = concrete_type;
    t_user_attributes = _;
    t_namespace = _;
    t_mode = mode;
    t_vis = td_vis;
    t_span = _;
    t_emit_id = _;
  } =
    tdef
  in
  let dep = Typing_deps.Dep.Class tid in
  let env = { Decl_env.mode; droot = Some dep; ctx } in
  let td_tparams = List.map params (type_param env) in
  let td_type = Decl_hint.hint env concrete_type in
  let td_constraint = Option.map tcstr (Decl_hint.hint env) in
  { td_vis; td_tparams; td_constraint; td_type; td_pos }

and typedef_naming_and_decl
    ~(write_shmem : bool) (ctx : Provider_context.t) (tdef : Nast.typedef) :
    string * Typing_defs.typedef_type =
  let tdef = Errors.ignore_ (fun () -> Naming.typedef ctx tdef) in
  let tdecl = typedef_decl ctx tdef in
  if write_shmem then Decl_heap.Typedefs.add (snd tdef.t_name) tdecl;
  (snd tdef.t_name, tdecl)

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let const_decl (ctx : Provider_context.t) (cst : Nast.gconst) :
    Typing_defs.decl_ty =
  let (cst_pos, _cst_name) = cst.cst_name in
  let dep = Dep.GConst (snd cst.cst_name) in
  let env = { Decl_env.mode = cst.cst_mode; droot = Some dep; ctx } in
  match cst.cst_type with
  | Some h -> Decl_hint.hint env h
  | None ->
    (match Decl_utils.infer_const cst.cst_value with
    | Some tprim -> mk (Reason.Rwitness (fst cst.cst_value), Tprim tprim)
    (* A NAST check will take care of rejecting constants that have neither
     * an initializer nor a literal initializer *)
    | None -> mk (Reason.Rwitness cst_pos, Typing_defs.make_tany ()))

let const_naming_and_decl
    ~(write_shmem : bool) (ctx : Provider_context.t) (cst : Nast.gconst) :
    string * Typing_defs.decl_ty =
  let cst = Errors.ignore_ (fun () -> Naming.global_const ctx cst) in
  let hint_ty = const_decl ctx cst in
  if write_shmem then Decl_heap.GConsts.add (snd cst.cst_name) hint_ty;
  (snd cst.cst_name, hint_ty)
