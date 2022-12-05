(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Cont = struct
  type 'a t =
    | Next of 'a
    | Finish of 'a

  let next x = Next x

  let finish x = Finish x
end

module Transform = struct
  type 'a t =
    | Top_down of ('a -> 'a Cont.t)
    | Bottom_up of ('a -> 'a Cont.t)

  let top_down k = Top_down k

  let bottom_up k = Bottom_up k

  let partition ts =
    List.fold_right ts ~init:([], []) ~f:(fun t (tds, bus) ->
        match t with
        | Top_down td -> (td :: tds, bus)
        | Bottom_up bu -> (tds, bu :: bus))

  (* Combine a sequence transforms.

     We will apply `Top_down` transforms first, then rewrite subexpressions
     before finally applying `Bottom_up` transforms

     Combining transforms gives meaning to the continuation each
     transformation returns:
     - `Next` indicates we should apply the _next_ transform to the element.
        If there are no more _top-down_ transforms, we rewrite subexpressions
        then apply _bottom-up_ transforms. If there are no more _bottom-up_
        transforms then we are done.
     - `Finish` indicates that no more transformations need be applied to this
        element or any part of it. This is useful if the transform produces
        an element that is already in its final form like our `invalid_expr_`
  *)
  let combine ts ~traverse =
    let rec aux (td, bu) elem = aux_td bu [] td elem
    and aux_td bu applied unapplied elem =
      match unapplied with
      | next :: unapplied ->
        let applied = next :: applied in
        (match next elem with
        | Cont.Next elem -> aux_td bu applied unapplied elem
        | Cont.Finish elem -> elem)
      | [] -> aux_bu (List.rev applied) [] bu @@ traverse elem
    and aux_bu td applied unapplied elem =
      match unapplied with
      | next :: unapplied ->
        let applied = next :: applied in
        (match next elem with
        | Cont.Next elem -> aux_bu td applied unapplied elem
        | Cont.Finish elem -> elem)
      | [] -> elem
    in
    aux (partition ts)
end

open Aast

(* Since we want entire passes to be top-down or bottom up, we provide
   a type so that this needn't be specified for each field in the pass
   type, below *)
type 'a transform = 'a -> 'a Cont.t

(* A pass is the collection of transforms to be applied to each element
   of the AST.

   Each transform can modify any or none of the environment, the element it is
   processing and an accumulated error. *)
type ('env, 'err) pass = {
  on_program: ('env * (unit, unit) program * 'err) transform option;
  (* -- Class and fields -------------------------------------------------- *)
  on_class_: ('env * (unit, unit) class_ * 'err) transform option;
  on_class_c_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_class_c_extends: ('env * hint list * 'err) transform option;
  on_class_c_uses: ('env * hint list * 'err) transform option;
  on_class_c_xhp_attrs:
    ('env * (unit, unit) xhp_attr list * 'err) transform option;
  on_class_c_xhp_attr_uses: ('env * hint list * 'err) transform option;
  on_class_c_req: ('env * (hint * require_kind) * 'err) transform option;
  on_class_c_reqs: ('env * (hint * require_kind) list * 'err) transform option;
  on_class_c_implements: ('env * hint list * 'err) transform option;
  on_class_c_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_class_c_consts:
    ('env * (unit, unit) class_const list * 'err) transform option;
  on_class_c_typeconsts:
    ('env * (unit, unit) class_typeconst_def list * 'err) transform option;
  on_class_c_vars: ('env * (unit, unit) class_var list * 'err) transform option;
  on_class_c_enum: ('env * enum_ option * 'err) transform option;
  on_class_c_methods:
    ('env * (unit, unit) method_ list * 'err) transform option;
  on_class_c_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_class_c_file_attributes:
    ('env * (unit, unit) file_attribute list * 'err) transform option;
  (* -- Class vars -------------------------------------------------------- *)
  on_class_var: ('env * (unit, unit) class_var * 'err) transform option;
  on_class_var_cv_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_class_var_cv_expr:
    ('env * (unit, unit) expr option * 'err) transform option;
  on_class_var_cv_type: ('env * unit type_hint * 'err) transform option;
  on_class_const_kind:
    ('env * (unit, unit) class_const_kind * 'err) transform option;
  (* -- Type defs --------------------------------------------------------- *)
  on_typedef: ('env * (unit, unit) typedef * 'err) transform option;
  (* -- Global constants -------------------------------------------------- *)
  on_gconst: ('env * (unit, unit) gconst * 'err) transform option;
  on_gconst_cst_type: ('env * hint option * 'err) transform option;
  on_gconst_cst_value: ('env * (unit, unit) expr * 'err) transform option;
  (* -- Function defs ----------------------------------------------------- *)
  on_fun_def: ('env * (unit, unit) fun_def * 'err) transform option;
  (* -- Module defs ------------------------------------------------------- *)
  on_module_def: ('env * (unit, unit) module_def * 'err) transform option;
  (* -- Statements -------------------------------------------------------- *)
  on_stmt: ('env * (unit, unit) stmt * 'err) transform option;
  on_stmt_: ('env * (unit, unit) stmt_ * 'err) transform option;
  on_block: ('env * (unit, unit) block * 'err) transform option;
  on_using_stmt: ('env * (unit, unit) using_stmt * 'err) transform option;
  (* -- Hints ------------------------------------------------------------- *)
  on_hint: ('env * hint * 'err) transform option;
  on_hint_: ('env * hint_ * 'err) transform option;
  (* -- hint_fun & fields ------------------------------------------------- *)
  on_hint_fun: ('env * hint_fun * 'err) transform option;
  on_hint_fun_hf_param_tys: ('env * hint list * 'err) transform option;
  on_hint_fun_hf_variadic_ty: ('env * variadic_hint * 'err) transform option;
  on_hint_fun_hf_ctxs: ('env * contexts option * 'err) transform option;
  on_hint_fun_hf_return_ty: ('env * hint * 'err) transform option;
  (* -- shape fields ------------------------------------------------------- *)
  on_nast_shape_info: ('env * nast_shape_info * 'err) transform option;
  on_shape_field_info: ('env * shape_field_info * 'err) transform option;
  (* -- Expressions ------------------------------------------------------- *)
  on_expr: ('env * (unit, unit) expr * 'err) transform option;
  on_expr_: ('env * (unit, unit) expr_ * 'err) transform option;
  (* -- Functions --------------------------------------------------------- *)
  on_fun_: ('env * (unit, unit) fun_ * 'err) transform option;
  on_fun_f_ret: ('env * unit type_hint * 'err) transform option;
  on_fun_f_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_fun_f_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_fun_f_params: ('env * (unit, unit) fun_param list * 'err) transform option;
  on_fun_f_ctxs: ('env * contexts option * 'err) transform option;
  on_fun_f_unsafe_ctxs: ('env * contexts option * 'err) transform option;
  on_fun_f_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_fun_f_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  (* -- Methods ----------------------------------------------------------- *)
  on_method_: ('env * (unit, unit) method_ * 'err) transform option;
  on_method_m_ret: ('env * unit type_hint * 'err) transform option;
  on_method_m_tparams:
    ('env * (unit, unit) tparam list * 'err) transform option;
  on_method_m_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_method_m_params:
    ('env * (unit, unit) fun_param list * 'err) transform option;
  on_method_m_ctxs: ('env * contexts option * 'err) transform option;
  on_method_m_unsafe_ctxs: ('env * contexts option * 'err) transform option;
  on_method_m_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_method_m_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  (* -- Class ID ---------------------------------------------------------- *)
  on_class_id: ('env * (unit, unit) class_id * 'err) transform option;
  on_class_id_: ('env * (unit, unit) class_id_ * 'err) transform option;
  (* -- Common ------------------------------------------------------------ *)
  on_func_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_enum_: ('env * enum_ * 'err) transform option;
  on_tparam: ('env * (unit, unit) tparam * 'err) transform option;
  on_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_where_constraint_hint:
    ('env * where_constraint_hint * 'err) transform option;
  on_contexts: ('env * contexts * 'err) transform option;
  on_context: ('env * hint * 'err) transform option;
  on_targ: ('env * unit targ * 'err) transform option;
  on_as_expr: ('env * (unit, unit) as_expr * 'err) transform option;
}

type ('env, 'err) t =
  | Top_down of ('env, 'err) pass
  | Bottom_up of ('env, 'err) pass

let top_down pass = Top_down pass

let bottom_up pass = Bottom_up pass

(* The identity transformation pass *)
let identity =
  {
    on_program = None;
    on_class_ = None;
    on_class_c_tparams = None;
    on_class_c_extends = None;
    on_class_c_uses = None;
    on_class_c_xhp_attrs = None;
    on_class_c_xhp_attr_uses = None;
    on_class_c_req = None;
    on_class_c_reqs = None;
    on_class_c_implements = None;
    on_class_c_where_constraints = None;
    on_class_c_consts = None;
    on_class_c_typeconsts = None;
    on_class_c_vars = None;
    on_class_c_enum = None;
    on_class_c_methods = None;
    on_class_c_user_attributes = None;
    on_class_c_file_attributes = None;
    on_class_const_kind = None;
    on_class_var = None;
    on_class_var_cv_user_attributes = None;
    on_class_var_cv_expr = None;
    on_class_var_cv_type = None;
    on_typedef = None;
    on_gconst = None;
    on_gconst_cst_type = None;
    on_gconst_cst_value = None;
    on_fun_def = None;
    on_module_def = None;
    on_stmt = None;
    on_stmt_ = None;
    on_block = None;
    on_using_stmt = None;
    on_hint = None;
    on_hint_ = None;
    on_hint_fun = None;
    on_hint_fun_hf_param_tys = None;
    on_hint_fun_hf_variadic_ty = None;
    on_hint_fun_hf_ctxs = None;
    on_hint_fun_hf_return_ty = None;
    on_nast_shape_info = None;
    on_shape_field_info = None;
    on_expr = None;
    on_expr_ = None;
    on_fun_ = None;
    on_fun_f_ret = None;
    on_fun_f_tparams = None;
    on_fun_f_where_constraints = None;
    on_fun_f_params = None;
    on_fun_f_ctxs = None;
    on_fun_f_unsafe_ctxs = None;
    on_fun_f_body = None;
    on_fun_f_user_attributes = None;
    on_method_ = None;
    on_method_m_ret = None;
    on_method_m_tparams = None;
    on_method_m_where_constraints = None;
    on_method_m_params = None;
    on_method_m_ctxs = None;
    on_method_m_unsafe_ctxs = None;
    on_method_m_body = None;
    on_method_m_user_attributes = None;
    on_class_id = None;
    on_class_id_ = None;
    on_func_body = None;
    on_enum_ = None;
    on_tparam = None;
    on_user_attributes = None;
    on_where_constraint_hint = None;
    on_contexts = None;
    on_context = None;
    on_targ = None;
    on_as_expr = None;
  }

let drop_env (_, elem, err) = (elem, err)

(* Helper function to make visitor methods. Given a list, `ts`, of
  `('env,Naming_phase_error.t) t`s, and a function to `select` a record field
  from a `('env,Naming_phase_error.t) pass`, we filter and partition the
  selected `transform`s into two lists one to be applied 'top-down' (i.e. before
  the element's children have been transformed) and one to be applied bottom up
  (i.e. after the element's children have been transformed)

  We then combine the transforms using the `super` function to perform our
  traversal. *)
let mk_handler super select ts =
  let ts =
    List.fold_right ts ~init:[] ~f:(fun t default ->
        match t with
        | Top_down t ->
          Option.value_map ~default ~f:(fun t ->
              Transform.top_down t :: default)
          @@ select t
        | Bottom_up t ->
          Option.value_map ~default ~f:(fun t ->
              Transform.bottom_up t :: default)
          @@ select t)
  in
  match ts with
  | [] -> super
  | _ ->
    fun env elem ->
      drop_env
      @@ (Transform.combine
            ~traverse:(fun (env, elem, err) ->
              let (elem, err') = super env elem in
              (env, elem, Err.Free_monoid.plus err err'))
            ts)
           (env, elem, Err.Free_monoid.zero)

let mk_visitor ts =
  object (_self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_program = mk_handler super#on_program (fun t -> t.on_program) ts

    method! on_class_ = mk_handler super#on_class_ (fun t -> t.on_class_) ts

    method! on_class_c_tparams =
      mk_handler super#on_class_c_tparams (fun t -> t.on_class_c_tparams) ts

    method! on_class_c_extends =
      mk_handler super#on_class_c_extends (fun t -> t.on_class_c_extends) ts

    method! on_class_c_uses =
      mk_handler super#on_class_c_uses (fun t -> t.on_class_c_uses) ts

    method! on_class_c_xhp_attrs =
      mk_handler super#on_class_c_xhp_attrs (fun t -> t.on_class_c_xhp_attrs) ts

    method! on_class_c_xhp_attr_uses =
      mk_handler
        super#on_class_c_xhp_attr_uses
        (fun t -> t.on_class_c_xhp_attr_uses)
        ts

    method! on_class_c_req =
      mk_handler super#on_class_c_req (fun t -> t.on_class_c_req) ts

    method! on_class_c_reqs =
      mk_handler super#on_class_c_reqs (fun t -> t.on_class_c_reqs) ts

    method! on_class_c_implements =
      mk_handler
        super#on_class_c_implements
        (fun t -> t.on_class_c_implements)
        ts

    method! on_class_c_where_constraints =
      mk_handler
        super#on_class_c_where_constraints
        (fun t -> t.on_class_c_where_constraints)
        ts

    method! on_class_c_consts =
      mk_handler super#on_class_c_consts (fun t -> t.on_class_c_consts) ts

    method! on_class_c_typeconsts =
      mk_handler
        super#on_class_c_typeconsts
        (fun t -> t.on_class_c_typeconsts)
        ts

    method! on_class_c_vars =
      mk_handler super#on_class_c_vars (fun t -> t.on_class_c_vars) ts

    method! on_class_c_enum =
      mk_handler super#on_class_c_enum (fun t -> t.on_class_c_enum) ts

    method! on_class_c_methods =
      mk_handler super#on_class_c_methods (fun t -> t.on_class_c_methods) ts

    method! on_class_c_user_attributes =
      mk_handler
        super#on_class_c_user_attributes
        (fun t -> t.on_class_c_user_attributes)
        ts

    method! on_class_c_file_attributes =
      mk_handler
        super#on_class_c_file_attributes
        (fun t -> t.on_class_c_file_attributes)
        ts

    method! on_class_const_kind =
      mk_handler super#on_class_const_kind (fun t -> t.on_class_const_kind) ts

    method! on_class_var =
      mk_handler super#on_class_var (fun t -> t.on_class_var) ts

    method! on_class_var_cv_user_attributes =
      mk_handler
        super#on_class_var_cv_user_attributes
        (fun t -> t.on_class_var_cv_user_attributes)
        ts

    method! on_class_var_cv_expr =
      mk_handler super#on_class_var_cv_expr (fun t -> t.on_class_var_cv_expr) ts

    method! on_class_var_cv_type =
      mk_handler super#on_class_var_cv_type (fun t -> t.on_class_var_cv_type) ts

    method! on_typedef = mk_handler super#on_typedef (fun t -> t.on_typedef) ts

    method! on_gconst = mk_handler super#on_gconst (fun t -> t.on_gconst) ts

    method! on_gconst_cst_type =
      mk_handler super#on_gconst_cst_type (fun t -> t.on_gconst_cst_type) ts

    method! on_gconst_cst_value =
      mk_handler super#on_gconst_cst_value (fun t -> t.on_gconst_cst_value) ts

    method! on_fun_def = mk_handler super#on_fun_def (fun t -> t.on_fun_def) ts

    method! on_module_def =
      mk_handler super#on_module_def (fun t -> t.on_module_def) ts

    method! on_stmt = mk_handler super#on_stmt (fun t -> t.on_stmt) ts

    method! on_stmt_ = mk_handler super#on_stmt_ (fun t -> t.on_stmt_) ts

    method! on_block = mk_handler super#on_block (fun t -> t.on_block) ts

    method! on_using_stmt =
      mk_handler super#on_using_stmt (fun t -> t.on_using_stmt) ts

    method! on_hint = mk_handler super#on_hint (fun t -> t.on_hint) ts

    method! on_hint_ = mk_handler super#on_hint_ (fun t -> t.on_hint_) ts

    method! on_hint_fun =
      mk_handler super#on_hint_fun (fun t -> t.on_hint_fun) ts

    method! on_hint_fun_hf_param_tys =
      mk_handler
        super#on_hint_fun_hf_param_tys
        (fun t -> t.on_hint_fun_hf_param_tys)
        ts

    method! on_hint_fun_hf_variadic_ty =
      mk_handler
        super#on_hint_fun_hf_variadic_ty
        (fun t -> t.on_hint_fun_hf_variadic_ty)
        ts

    method! on_hint_fun_hf_ctxs =
      mk_handler super#on_hint_fun_hf_ctxs (fun t -> t.on_hint_fun_hf_ctxs) ts

    method! on_hint_fun_hf_return_ty =
      mk_handler
        super#on_hint_fun_hf_return_ty
        (fun t -> t.on_hint_fun_hf_return_ty)
        ts

    method! on_nast_shape_info =
      mk_handler super#on_nast_shape_info (fun t -> t.on_nast_shape_info) ts

    method! on_shape_field_info =
      mk_handler super#on_shape_field_info (fun t -> t.on_shape_field_info) ts

    method! on_expr = mk_handler super#on_expr (fun t -> t.on_expr) ts

    method! on_expr_ = mk_handler super#on_expr_ (fun t -> t.on_expr_) ts

    method! on_fun_ = mk_handler super#on_fun_ (fun t -> t.on_fun_) ts

    method! on_fun_f_ret =
      mk_handler super#on_fun_f_ret (fun t -> t.on_fun_f_ret) ts

    method! on_fun_f_tparams =
      mk_handler super#on_fun_f_tparams (fun t -> t.on_fun_f_tparams) ts

    method! on_fun_f_where_constraints =
      mk_handler
        super#on_fun_f_where_constraints
        (fun t -> t.on_fun_f_where_constraints)
        ts

    method! on_fun_f_params =
      mk_handler super#on_fun_f_params (fun t -> t.on_fun_f_params) ts

    method! on_fun_f_ctxs =
      mk_handler super#on_fun_f_ctxs (fun t -> t.on_fun_f_ctxs) ts

    method! on_fun_f_unsafe_ctxs =
      mk_handler super#on_fun_f_unsafe_ctxs (fun t -> t.on_fun_f_unsafe_ctxs) ts

    method! on_fun_f_body =
      mk_handler super#on_fun_f_body (fun t -> t.on_fun_f_body) ts

    method! on_fun_f_user_attributes =
      mk_handler
        super#on_fun_f_user_attributes
        (fun t -> t.on_fun_f_user_attributes)
        ts

    method! on_method_ = mk_handler super#on_method_ (fun t -> t.on_method_) ts

    method! on_method_m_ret =
      mk_handler super#on_method_m_ret (fun t -> t.on_method_m_ret) ts

    method! on_method_m_tparams =
      mk_handler super#on_method_m_tparams (fun t -> t.on_method_m_tparams) ts

    method! on_method_m_where_constraints =
      mk_handler
        super#on_method_m_where_constraints
        (fun t -> t.on_method_m_where_constraints)
        ts

    method! on_method_m_params =
      mk_handler super#on_method_m_params (fun t -> t.on_method_m_params) ts

    method! on_method_m_ctxs =
      mk_handler super#on_method_m_ctxs (fun t -> t.on_method_m_ctxs) ts

    method! on_method_m_unsafe_ctxs =
      mk_handler
        super#on_method_m_unsafe_ctxs
        (fun t -> t.on_method_m_unsafe_ctxs)
        ts

    method! on_method_m_body =
      mk_handler super#on_method_m_body (fun t -> t.on_method_m_body) ts

    method! on_method_m_user_attributes =
      mk_handler
        super#on_method_m_user_attributes
        (fun t -> t.on_method_m_user_attributes)
        ts

    method! on_class_id =
      mk_handler super#on_class_id (fun t -> t.on_class_id) ts

    method! on_class_id_ =
      mk_handler super#on_class_id_ (fun t -> t.on_class_id_) ts

    method! on_func_body =
      mk_handler super#on_func_body (fun t -> t.on_func_body) ts

    method! on_enum_ = mk_handler super#on_enum_ (fun t -> t.on_enum_) ts

    method! on_tparam = mk_handler super#on_tparam (fun t -> t.on_tparam) ts

    method! on_user_attributes =
      mk_handler super#on_user_attributes (fun t -> t.on_user_attributes) ts

    method! on_where_constraint_hint =
      mk_handler
        super#on_where_constraint_hint
        (fun t -> t.on_where_constraint_hint)
        ts

    method! on_contexts =
      mk_handler super#on_contexts (fun t -> t.on_contexts) ts

    method! on_context = mk_handler super#on_context (fun t -> t.on_context) ts

    method! on_targ = mk_handler super#on_targ (fun t -> t.on_targ) ts

    method! on_as_expr = mk_handler super#on_as_expr (fun t -> t.on_as_expr) ts
  end
