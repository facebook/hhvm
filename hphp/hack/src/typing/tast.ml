(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

(* This is the current notion of type in the typed AST.
 * In future we might want to reconsider this and define a new representation
 * that omits type inference artefacts such as type variables and lambda
 * identifiers.
 *)
type ty = Typing_defs.locl_ty [@@deriving hash, show]

type decl_ty = Typing_defs.decl_ty [@@deriving show]

type val_kind = Typing_defs.val_kind

(* Contains information about a specific function that we
    a) want to make available to TAST checks
    b) isn't otherwise (space-efficiently) present in the saved typing env *)
type fun_tast_info = {
  has_implicit_return: bool;
  has_readonly: bool;
      (** True if there are leaves of the function's imaginary CFG without a return statement *)
}
[@@deriving hash, show]

type check_status =
  | COnce  (** The definition is checked only once. *)
  | CUnderNormalAssumptions
      (** The definition is checked twice and this is the check under normal
          assumptions that is using the parameter and return types that are
          written in the source code (but potentially implicitly pessimised).
          *)
  | CUnderDynamicAssumptions
      (** The definition is checked twice and this is the check under dynamic
          assumptions that is using the dynamic type for parameters and return.
          *)
[@@deriving hash, show]

let is_under_dynamic_assumptions = function
  | COnce
  | CUnderNormalAssumptions ->
    false
  | CUnderDynamicAssumptions -> true

type saved_env = {
  tcopt: TypecheckerOptions.t; [@opaque] [@hash.ignore]
  inference_env: Typing_inference_env.t; [@opaque] [@hash.ignore]
  tpenv: Type_parameter_env.t;
  fun_tast_info: fun_tast_info option;
  checked: check_status;
      (** Indicates how many types the callable was checked and under what
          assumptions. *)
}
[@@deriving hash, show]

type program = (ty, saved_env) Aast.program [@@deriving show]

type def = (ty, saved_env) Aast.def [@@deriving hash, show]

type def_with_dynamic = def Tast_with_dynamic.t [@@deriving hash, show]

type expr = (ty, saved_env) Aast.expr

type expr_ = (ty, saved_env) Aast.expr_

type stmt = (ty, saved_env) Aast.stmt

type stmt_ = (ty, saved_env) Aast.stmt_

type case = (ty, saved_env) Aast.case

type block = (ty, saved_env) Aast.block

type class_ = (ty, saved_env) Aast.class_

type class_id = (ty, saved_env) Aast.class_id

type type_hint = ty Aast.type_hint

type targ = ty Aast.targ

type class_get_expr = (ty, saved_env) Aast.class_get_expr

type class_typeconst_def = (ty, saved_env) Aast.class_typeconst_def

type user_attribute = (ty, saved_env) Aast.user_attribute

type capture_lid = ty Aast.capture_lid

type fun_ = (ty, saved_env) Aast.fun_

type efun = (ty, saved_env) Aast.efun

type file_attribute = (ty, saved_env) Aast.file_attribute

type fun_def = (ty, saved_env) Aast.fun_def

type fun_param = (ty, saved_env) Aast.fun_param

type func_body = (ty, saved_env) Aast.func_body

type method_ = (ty, saved_env) Aast.method_

type class_var = (ty, saved_env) Aast.class_var

type class_const = (ty, saved_env) Aast.class_const

type tparam = (ty, saved_env) Aast.tparam

type typedef = (ty, saved_env) Aast.typedef

type gconst = (ty, saved_env) Aast.gconst

type module_def = (ty, saved_env) Aast.module_def

type call_expr = (ty, saved_env) Aast.call_expr

type by_names = {
  fun_tasts: def Tast_with_dynamic.t SMap.t;
  class_tasts: def Tast_with_dynamic.t SMap.t;
  typedef_tasts: def SMap.t;
  gconst_tasts: def SMap.t;
  module_tasts: def SMap.t;
}

let empty_by_names =
  {
    fun_tasts = SMap.empty;
    class_tasts = SMap.empty;
    typedef_tasts = SMap.empty;
    gconst_tasts = SMap.empty;
    module_tasts = SMap.empty;
  }

let map_by_names (x : by_names) ~(f : def -> def) : by_names =
  let { fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts } =
    x
  in
  {
    fun_tasts = SMap.map (Tast_with_dynamic.map ~f) fun_tasts;
    class_tasts = SMap.map (Tast_with_dynamic.map ~f) class_tasts;
    typedef_tasts = SMap.map f typedef_tasts;
    gconst_tasts = SMap.map f gconst_tasts;
    module_tasts = SMap.map f module_tasts;
  }

let program_by_names (program : program Tast_with_dynamic.t) : by_names =
  let add
      ~(under_normal_assumptions : bool)
      name
      (def : def)
      (map : def Tast_with_dynamic.t SMap.t) : def Tast_with_dynamic.t SMap.t =
    let entry =
      match SMap.find_opt name map with
      | None -> Tast_with_dynamic.mk_without_dynamic def
      | Some entry -> entry
    in
    let entry =
      if under_normal_assumptions then
        { entry with Tast_with_dynamic.under_normal_assumptions = def }
      else
        { entry with Tast_with_dynamic.under_dynamic_assumptions = Some def }
    in
    SMap.add name entry map
  in
  let program_by_names
      ~under_normal_assumptions (by_names : by_names) (program : program) :
      by_names =
    Hh_prelude.List.fold program ~init:by_names ~f:(fun acc def ->
        match def with
        | Fun f ->
          {
            acc with
            fun_tasts =
              add ~under_normal_assumptions (snd f.fd_name) def acc.fun_tasts;
          }
        | Class c ->
          {
            acc with
            class_tasts =
              add ~under_normal_assumptions (snd c.c_name) def acc.class_tasts;
          }
        | Typedef td ->
          {
            acc with
            typedef_tasts = SMap.add (snd td.t_name) def acc.typedef_tasts;
          }
        | Constant c ->
          {
            acc with
            gconst_tasts = SMap.add (snd c.cst_name) def acc.gconst_tasts;
          }
        | Module m ->
          {
            acc with
            module_tasts = SMap.add (snd m.md_name) def acc.module_tasts;
          }
        | Stmt _
        | Namespace _
        | NamespaceUse _
        | SetNamespaceEnv _
        | FileAttributes _
        | SetModule _ ->
          acc)
  in
  let { Tast_with_dynamic.under_normal_assumptions; under_dynamic_assumptions }
      =
    program
  in
  let by_names = empty_by_names in
  let by_names =
    program_by_names
      ~under_normal_assumptions:true
      by_names
      under_normal_assumptions
  in
  let by_names =
    Option.fold
      under_dynamic_assumptions
      ~init:by_names
      ~f:(program_by_names ~under_normal_assumptions:false)
  in
  by_names

let tasts_as_list
    ({ fun_tasts; class_tasts; typedef_tasts; gconst_tasts; module_tasts } :
      by_names) : def Tast_with_dynamic.t list =
  SMap.values fun_tasts
  @ SMap.values class_tasts
  @ List.map ~f:Tast_with_dynamic.mk_without_dynamic (SMap.values typedef_tasts)
  @ List.map ~f:Tast_with_dynamic.mk_without_dynamic (SMap.values gconst_tasts)
  @ List.map ~f:Tast_with_dynamic.mk_without_dynamic (SMap.values module_tasts)

let empty_saved_env tcopt : saved_env =
  {
    tcopt;
    inference_env = Typing_inference_env.empty_inference_env;
    tpenv = Type_parameter_env.empty;
    fun_tast_info = None;
    checked = COnce;
  }

(* Used when an env is needed in codegen.
 * TODO: (arkumar,wilfred,thomasjiang) T42509373 Fix when when needed
 *)
let dummy_saved_env = empty_saved_env GlobalOptions.default

let dummy_type_hint (hint : hint option) : ty * hint option =
  (Typing_defs.mk (Typing_reason.Rnone, Typing_defs.Tdynamic), hint)

(* Helper function to create an annotation for a typed and positioned expression.
 * Do not construct this tuple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_expr_annotation _p ty : ty = ty

(* Helper function to create a typed and positioned expression.
 * Do not construct this triple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_typed_expr p ty te : expr = (make_expr_annotation p ty, p, te)

(* Get the position of an expression *)
let get_position ((_, p, _) : expr) = p

(* Get the type of an expression *)
let get_type ((ty, _, _) : expr) = ty

let nast_converter =
  object
    inherit [_] Aast.map as super

    method on_'ex _ _ = ()

    method on_'en _ _ = ()

    method! on_Hole _ ex _ _ src =
      let ((_, pos, ex_) as ex) = super#on_expr () ex in
      let mk_call hints name =
        let targs =
          List.map ~f:(fun hint -> ((), super#on_hint () hint)) hints
        in
        let func = ((), pos, Aast.Id (pos, name)) in
        let args = [(Ast_defs.Pnormal, ex)] in
        Aast.Call { func; targs; args; unpacked_arg = None }
      in
      match src with
      | Aast.UnsafeCast hints ->
        mk_call hints Naming_special_names.PseudoFunctions.unsafe_cast
      | Aast.UnsafeNonnullCast ->
        mk_call [] Naming_special_names.PseudoFunctions.unsafe_nonnull_cast
      | Aast.EnforcedCast hints ->
        mk_call hints Naming_special_names.PseudoFunctions.enforced_cast
      | _ -> ex_
  end

let to_nast p = nast_converter#on_program () p

let to_nast_expr (tast : expr) : Nast.expr = nast_converter#on_expr () tast

let to_nast_class_id_ cid = nast_converter#on_class_id_ () cid

let force_lazy_values_saved_env (saved_env : saved_env) : saved_env =
  let { tcopt; inference_env; tpenv; fun_tast_info; checked } = saved_env in
  {
    tcopt;
    inference_env = Typing_inference_env.force_lazy_values inference_env;
    tpenv = Type_parameter_env.force_lazy_values tpenv;
    fun_tast_info;
    checked;
  }

(** Force any lazy value in the TAST. This is useful to serialize the TAST,
  for example prior to returning over RPC or storing in the shared heap. *)
let force_lazy_values (program : program) =
  let visitor =
    object
      inherit [_] Aast.map

      method on_'ex _env ty = Type_force_lazy_values.locl_ty ty

      method on_'en _env (saved_env : saved_env) =
        force_lazy_values_saved_env saved_env
    end
  in
  visitor#on_program () program
