(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

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

(* Helper function to make visitor methods. Given a list, `ts`, of
  `('env,Naming_phase_error.t) t`s, and a function to `select` a record field
  from a `('env,Naming_phase_error.t) pass`, we filter and partition the
  selected `transform`s into two lists one to be applied 'top-down' (i.e. before
  the element's children have been transformed) and one to be applied bottom up
  (i.e. after the element's children have been transformed)

  We then combine the transforms using the `super` function to perform our
  traversal. *)
let mk_handler ts ~super ~select ~on_error =
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
      let (_, elem, errs) =
        (Transform.combine
           ~traverse:(fun (env, elem, errs) ->
             let elem = super env elem in
             (env, elem, errs))
           ts)
          (env, elem, [])
      in
      List.iter ~f:on_error errs;
      elem

let mk_visitor ts ~on_error =
  object (_self)
    inherit [_] Naming_visitors.endo as super

    method! on_program =
      mk_handler
        ts
        ~super:super#on_program
        ~select:(fun t -> t.on_program)
        ~on_error

    method! on_class_ =
      mk_handler
        ts
        ~super:super#on_class_
        ~select:(fun t -> t.on_class_)
        ~on_error

    method! on_class_c_tparams =
      mk_handler
        ts
        ~super:super#on_class_c_tparams
        ~select:(fun t -> t.on_class_c_tparams)
        ~on_error

    method! on_class_c_extends =
      mk_handler
        ts
        ~super:super#on_class_c_extends
        ~select:(fun t -> t.on_class_c_extends)
        ~on_error

    method! on_class_c_uses =
      mk_handler
        ts
        ~super:super#on_class_c_uses
        ~select:(fun t -> t.on_class_c_uses)
        ~on_error

    method! on_class_c_xhp_attrs =
      mk_handler
        ts
        ~super:super#on_class_c_xhp_attrs
        ~select:(fun t -> t.on_class_c_xhp_attrs)
        ~on_error

    method! on_class_c_xhp_attr_uses =
      mk_handler
        ts
        ~super:super#on_class_c_xhp_attr_uses
        ~select:(fun t -> t.on_class_c_xhp_attr_uses)
        ~on_error

    method! on_class_c_req =
      mk_handler
        ts
        ~super:super#on_class_c_req
        ~select:(fun t -> t.on_class_c_req)
        ~on_error

    method! on_class_c_reqs =
      mk_handler
        ts
        ~super:super#on_class_c_reqs
        ~select:(fun t -> t.on_class_c_reqs)
        ~on_error

    method! on_class_c_implements =
      mk_handler
        ts
        ~super:super#on_class_c_implements
        ~select:(fun t -> t.on_class_c_implements)
        ~on_error

    method! on_class_c_where_constraints =
      mk_handler
        ts
        ~super:super#on_class_c_where_constraints
        ~select:(fun t -> t.on_class_c_where_constraints)
        ~on_error

    method! on_class_c_consts =
      mk_handler
        ts
        ~super:super#on_class_c_consts
        ~select:(fun t -> t.on_class_c_consts)
        ~on_error

    method! on_class_c_typeconsts =
      mk_handler
        ts
        ~super:super#on_class_c_typeconsts
        ~select:(fun t -> t.on_class_c_typeconsts)
        ~on_error

    method! on_class_c_vars =
      mk_handler
        ts
        ~super:super#on_class_c_vars
        ~select:(fun t -> t.on_class_c_vars)
        ~on_error

    method! on_class_c_enum =
      mk_handler
        ts
        ~super:super#on_class_c_enum
        ~select:(fun t -> t.on_class_c_enum)
        ~on_error

    method! on_class_c_methods =
      mk_handler
        ts
        ~super:super#on_class_c_methods
        ~select:(fun t -> t.on_class_c_methods)
        ~on_error

    method! on_class_c_user_attributes =
      mk_handler
        ts
        ~super:super#on_class_c_user_attributes
        ~select:(fun t -> t.on_class_c_user_attributes)
        ~on_error

    method! on_class_c_file_attributes =
      mk_handler
        ts
        ~super:super#on_class_c_file_attributes
        ~select:(fun t -> t.on_class_c_file_attributes)
        ~on_error

    method! on_class_const_kind =
      mk_handler
        ts
        ~super:super#on_class_const_kind
        ~select:(fun t -> t.on_class_const_kind)
        ~on_error

    method! on_class_var =
      mk_handler
        ts
        ~super:super#on_class_var
        ~select:(fun t -> t.on_class_var)
        ~on_error

    method! on_class_var_cv_user_attributes =
      mk_handler
        ts
        ~super:super#on_class_var_cv_user_attributes
        ~select:(fun t -> t.on_class_var_cv_user_attributes)
        ~on_error

    method! on_class_var_cv_expr =
      mk_handler
        ts
        ~super:super#on_class_var_cv_expr
        ~select:(fun t -> t.on_class_var_cv_expr)
        ~on_error

    method! on_class_var_cv_type =
      mk_handler
        ts
        ~super:super#on_class_var_cv_type
        ~select:(fun t -> t.on_class_var_cv_type)
        ~on_error

    method! on_typedef =
      mk_handler
        ts
        ~super:super#on_typedef
        ~select:(fun t -> t.on_typedef)
        ~on_error

    method! on_gconst =
      mk_handler
        ts
        ~super:super#on_gconst
        ~select:(fun t -> t.on_gconst)
        ~on_error

    method! on_gconst_cst_type =
      mk_handler
        ts
        ~super:super#on_gconst_cst_type
        ~select:(fun t -> t.on_gconst_cst_type)
        ~on_error

    method! on_gconst_cst_value =
      mk_handler
        ts
        ~super:super#on_gconst_cst_value
        ~select:(fun t -> t.on_gconst_cst_value)
        ~on_error

    method! on_fun_def =
      mk_handler
        ts
        ~super:super#on_fun_def
        ~select:(fun t -> t.on_fun_def)
        ~on_error

    method! on_module_def =
      mk_handler
        ts
        ~super:super#on_module_def
        ~select:(fun t -> t.on_module_def)
        ~on_error

    method! on_stmt =
      mk_handler ts ~super:super#on_stmt ~select:(fun t -> t.on_stmt) ~on_error

    method! on_stmt_ =
      mk_handler
        ts
        ~super:super#on_stmt_
        ~select:(fun t -> t.on_stmt_)
        ~on_error

    method! on_block =
      mk_handler
        ts
        ~super:super#on_block
        ~select:(fun t -> t.on_block)
        ~on_error

    method! on_using_stmt =
      mk_handler
        ts
        ~super:super#on_using_stmt
        ~select:(fun t -> t.on_using_stmt)
        ~on_error

    method! on_hint =
      mk_handler ts ~super:super#on_hint ~select:(fun t -> t.on_hint) ~on_error

    method! on_hint_ =
      mk_handler
        ts
        ~super:super#on_hint_
        ~select:(fun t -> t.on_hint_)
        ~on_error

    method! on_hint_fun =
      mk_handler
        ts
        ~super:super#on_hint_fun
        ~select:(fun t -> t.on_hint_fun)
        ~on_error

    method! on_hint_fun_hf_param_tys =
      mk_handler
        ts
        ~super:super#on_hint_fun_hf_param_tys
        ~select:(fun t -> t.on_hint_fun_hf_param_tys)
        ~on_error

    method! on_hint_fun_hf_variadic_ty =
      mk_handler
        ts
        ~super:super#on_hint_fun_hf_variadic_ty
        ~select:(fun t -> t.on_hint_fun_hf_variadic_ty)
        ~on_error

    method! on_hint_fun_hf_ctxs =
      mk_handler
        ts
        ~super:super#on_hint_fun_hf_ctxs
        ~select:(fun t -> t.on_hint_fun_hf_ctxs)
        ~on_error

    method! on_hint_fun_hf_return_ty =
      mk_handler
        ts
        ~super:super#on_hint_fun_hf_return_ty
        ~select:(fun t -> t.on_hint_fun_hf_return_ty)
        ~on_error

    method! on_nast_shape_info =
      mk_handler
        ts
        ~super:super#on_nast_shape_info
        ~select:(fun t -> t.on_nast_shape_info)
        ~on_error

    method! on_shape_field_info =
      mk_handler
        ts
        ~super:super#on_shape_field_info
        ~select:(fun t -> t.on_shape_field_info)
        ~on_error

    method! on_expr =
      mk_handler ts ~super:super#on_expr ~select:(fun t -> t.on_expr) ~on_error

    method! on_expr_ =
      mk_handler
        ts
        ~super:super#on_expr_
        ~select:(fun t -> t.on_expr_)
        ~on_error

    method! on_fun_ =
      mk_handler ts ~super:super#on_fun_ ~select:(fun t -> t.on_fun_) ~on_error

    method! on_fun_f_ret =
      mk_handler
        ts
        ~super:super#on_fun_f_ret
        ~select:(fun t -> t.on_fun_f_ret)
        ~on_error

    method! on_fun_f_tparams =
      mk_handler
        ts
        ~super:super#on_fun_f_tparams
        ~select:(fun t -> t.on_fun_f_tparams)
        ~on_error

    method! on_fun_f_where_constraints =
      mk_handler
        ts
        ~super:super#on_fun_f_where_constraints
        ~select:(fun t -> t.on_fun_f_where_constraints)
        ~on_error

    method! on_fun_f_params =
      mk_handler
        ts
        ~super:super#on_fun_f_params
        ~select:(fun t -> t.on_fun_f_params)
        ~on_error

    method! on_fun_f_ctxs =
      mk_handler
        ts
        ~super:super#on_fun_f_ctxs
        ~select:(fun t -> t.on_fun_f_ctxs)
        ~on_error

    method! on_fun_f_unsafe_ctxs =
      mk_handler
        ts
        ~super:super#on_fun_f_unsafe_ctxs
        ~select:(fun t -> t.on_fun_f_unsafe_ctxs)
        ~on_error

    method! on_fun_f_body =
      mk_handler
        ts
        ~super:super#on_fun_f_body
        ~select:(fun t -> t.on_fun_f_body)
        ~on_error

    method! on_fun_f_user_attributes =
      mk_handler
        ts
        ~super:super#on_fun_f_user_attributes
        ~select:(fun t -> t.on_fun_f_user_attributes)
        ~on_error

    method! on_method_ =
      mk_handler
        ts
        ~super:super#on_method_
        ~select:(fun t -> t.on_method_)
        ~on_error

    method! on_method_m_ret =
      mk_handler
        ts
        ~super:super#on_method_m_ret
        ~select:(fun t -> t.on_method_m_ret)
        ~on_error

    method! on_method_m_tparams =
      mk_handler
        ts
        ~super:super#on_method_m_tparams
        ~select:(fun t -> t.on_method_m_tparams)
        ~on_error

    method! on_method_m_where_constraints =
      mk_handler
        ts
        ~super:super#on_method_m_where_constraints
        ~select:(fun t -> t.on_method_m_where_constraints)
        ~on_error

    method! on_method_m_params =
      mk_handler
        ts
        ~super:super#on_method_m_params
        ~select:(fun t -> t.on_method_m_params)
        ~on_error

    method! on_method_m_ctxs =
      mk_handler
        ts
        ~super:super#on_method_m_ctxs
        ~select:(fun t -> t.on_method_m_ctxs)
        ~on_error

    method! on_method_m_unsafe_ctxs =
      mk_handler
        ts
        ~super:super#on_method_m_unsafe_ctxs
        ~select:(fun t -> t.on_method_m_unsafe_ctxs)
        ~on_error

    method! on_method_m_body =
      mk_handler
        ts
        ~super:super#on_method_m_body
        ~select:(fun t -> t.on_method_m_body)
        ~on_error

    method! on_method_m_user_attributes =
      mk_handler
        ts
        ~super:super#on_method_m_user_attributes
        ~select:(fun t -> t.on_method_m_user_attributes)
        ~on_error

    method! on_class_id =
      mk_handler
        ts
        ~super:super#on_class_id
        ~select:(fun t -> t.on_class_id)
        ~on_error

    method! on_class_id_ =
      mk_handler
        ts
        ~super:super#on_class_id_
        ~select:(fun t -> t.on_class_id_)
        ~on_error

    method! on_func_body =
      mk_handler
        ts
        ~super:super#on_func_body
        ~select:(fun t -> t.on_func_body)
        ~on_error

    method! on_enum_ =
      mk_handler
        ts
        ~super:super#on_enum_
        ~select:(fun t -> t.on_enum_)
        ~on_error

    method! on_tparam =
      mk_handler
        ts
        ~super:super#on_tparam
        ~select:(fun t -> t.on_tparam)
        ~on_error

    method! on_user_attributes =
      mk_handler
        ts
        ~super:super#on_user_attributes
        ~select:(fun t -> t.on_user_attributes)
        ~on_error

    method! on_where_constraint_hint =
      mk_handler
        ts
        ~super:super#on_where_constraint_hint
        ~select:(fun t -> t.on_where_constraint_hint)
        ~on_error

    method! on_contexts =
      mk_handler
        ts
        ~super:super#on_contexts
        ~select:(fun t -> t.on_contexts)
        ~on_error

    method! on_context =
      mk_handler
        ts
        ~super:super#on_context
        ~select:(fun t -> t.on_context)
        ~on_error

    method! on_targ =
      mk_handler ts ~super:super#on_targ ~select:(fun t -> t.on_targ) ~on_error

    method! on_as_expr =
      mk_handler
        ts
        ~super:super#on_as_expr
        ~select:(fun t -> t.on_as_expr)
        ~on_error
  end
