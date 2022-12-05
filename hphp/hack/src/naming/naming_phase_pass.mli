(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type ('env, 'err) t

type 'a transform = 'a -> ('a, 'a) Result.t

type ('env, 'err) pass = {
  on_program: ('env * (unit, unit) Aast.program * 'err) transform option;
  on_class_: ('env * (unit, unit) Aast.class_ * 'err) transform option;
  on_class_c_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_class_c_extends: ('env * Aast.hint list * 'err) transform option;
  on_class_c_uses: ('env * Aast.hint list * 'err) transform option;
  on_class_c_xhp_attrs:
    ('env * (unit, unit) Aast.xhp_attr list * 'err) transform option;
  on_class_c_xhp_attr_uses: ('env * Aast.hint list * 'err) transform option;
  on_class_c_req:
    ('env * (Aast.hint * Aast.require_kind) * 'err) transform option;
  on_class_c_reqs:
    ('env * (Aast.hint * Aast.require_kind) list * 'err) transform option;
  on_class_c_implements: ('env * Aast.hint list * 'err) transform option;
  on_class_c_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_class_c_consts:
    ('env * (unit, unit) Aast.class_const list * 'err) transform option;
  on_class_c_typeconsts:
    ('env * (unit, unit) Aast.class_typeconst_def list * 'err) transform option;
  on_class_c_vars:
    ('env * (unit, unit) Aast.class_var list * 'err) transform option;
  on_class_c_enum: ('env * Aast.enum_ option * 'err) transform option;
  on_class_c_methods:
    ('env * (unit, unit) Aast.method_ list * 'err) transform option;
  on_class_c_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_c_file_attributes:
    ('env * (unit, unit) Aast.file_attribute list * 'err) transform option;
  on_class_var: ('env * (unit, unit) Aast.class_var * 'err) transform option;
  on_class_var_cv_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_var_cv_expr:
    ('env * (unit, unit) Aast.expr option * 'err) transform option;
  on_class_var_cv_type: ('env * unit Aast.type_hint * 'err) transform option;
  on_class_const_kind:
    ('env * (unit, unit) Aast.class_const_kind * 'err) transform option;
  on_typedef: ('env * (unit, unit) Aast.typedef * 'err) transform option;
  on_gconst: ('env * (unit, unit) Aast.gconst * 'err) transform option;
  on_gconst_cst_type: ('env * Aast.hint option * 'err) transform option;
  on_gconst_cst_value: ('env * (unit, unit) Aast.expr * 'err) transform option;
  on_fun_def: ('env * (unit, unit) Aast.fun_def * 'err) transform option;
  on_module_def: ('env * (unit, unit) Aast.module_def * 'err) transform option;
  on_stmt: ('env * (unit, unit) Aast.stmt * 'err) transform option;
  on_stmt_: ('env * (unit, unit) Aast.stmt_ * 'err) transform option;
  on_block: ('env * (unit, unit) Aast.block * 'err) transform option;
  on_using_stmt: ('env * (unit, unit) Aast.using_stmt * 'err) transform option;
  on_hint: ('env * Aast.hint * 'err) transform option;
  on_hint_: ('env * Aast.hint_ * 'err) transform option;
  on_hint_fun: ('env * Aast.hint_fun * 'err) transform option;
  on_hint_fun_hf_param_tys: ('env * Aast.hint list * 'err) transform option;
  on_hint_fun_hf_variadic_ty:
    ('env * Aast.variadic_hint * 'err) transform option;
  on_hint_fun_hf_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_hint_fun_hf_return_ty: ('env * Aast.hint * 'err) transform option;
  on_nast_shape_info: ('env * Aast.nast_shape_info * 'err) transform option;
  on_shape_field_info: ('env * Aast.shape_field_info * 'err) transform option;
  on_expr: ('env * (unit, unit) Aast.expr * 'err) transform option;
  on_expr_: ('env * (unit, unit) Aast.expr_ * 'err) transform option;
  on_fun_: ('env * (unit, unit) Aast.fun_ * 'err) transform option;
  on_fun_f_ret: ('env * unit Aast.type_hint * 'err) transform option;
  on_fun_f_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_fun_f_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_fun_f_params:
    ('env * (unit, unit) Aast.fun_param list * 'err) transform option;
  on_fun_f_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_fun_f_unsafe_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_fun_f_body: ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_fun_f_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_method_: ('env * (unit, unit) Aast.method_ * 'err) transform option;
  on_method_m_ret: ('env * unit Aast.type_hint * 'err) transform option;
  on_method_m_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_method_m_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_method_m_params:
    ('env * (unit, unit) Aast.fun_param list * 'err) transform option;
  on_method_m_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_method_m_unsafe_ctxs:
    ('env * Aast.contexts option * 'err) transform option;
  on_method_m_body:
    ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_method_m_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_id: ('env * (unit, unit) Aast.class_id * 'err) transform option;
  on_class_id_: ('env * (unit, unit) Aast.class_id_ * 'err) transform option;
  on_func_body: ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_enum_: ('env * Aast.enum_ * 'err) transform option;
  on_tparam: ('env * (unit, unit) Aast.tparam * 'err) transform option;
  on_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_where_constraint_hint:
    ('env * Aast.where_constraint_hint * 'err) transform option;
  on_contexts: ('env * Aast.contexts * 'err) transform option;
  on_context: ('env * Aast.hint * 'err) transform option;
  on_targ: ('env * unit Aast.targ * 'err) transform option;
  on_as_expr: ('env * (unit, unit) Aast.as_expr * 'err) transform option;
}

val identity : ('env, 'err) pass

val top_down : ('env, 'err) pass -> ('env, 'err) t

val bottom_up : ('env, 'err) pass -> ('env, 'err) t

val mk_visitor :
  ('a, Naming_phase_error.t list) t list ->
  on_error:(Naming_phase_error.t -> unit) ->
  < on_'en : 'a -> unit -> unit
  ; on_'ex : 'a -> unit -> unit
  ; on_AFkvalue :
      'a ->
      (unit, unit) afield ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) afield
  ; on_AFvalue :
      'a ->
      (unit, unit) afield ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) afield
  ; on_Abstract : 'a -> Ast_defs.abstraction -> Ast_defs.abstraction
  ; on_Amp : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Ampamp : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Anonymous : 'a -> emit_id -> emit_id
  ; on_Array_get :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) expr_
  ; on_As :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      is_variadic ->
      (unit, unit) expr_
  ; on_As_kv :
      'a ->
      (unit, unit) as_expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
  ; on_As_v :
      'a ->
      (unit, unit) as_expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
  ; on_AssertEnv :
      'a ->
      (unit, unit) stmt_ ->
      env_annot ->
      (pos * unit) local_id_map ->
      (unit, unit) stmt_
  ; on_Await :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Await_as_kv :
      'a ->
      (unit, unit) as_expr ->
      pos ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
  ; on_Await_as_v :
      'a ->
      (unit, unit) as_expr ->
      pos ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
  ; on_Awaitall :
      'a ->
      (unit, unit) stmt_ ->
      (Aast_defs.lid option * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_Bar : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Barbar : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Binop :
      'a ->
      (unit, unit) expr_ ->
      Ast_defs.bop ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Block :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_Break : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_CCAbstract :
      'a ->
      (unit, unit) class_const_kind ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) class_const_kind
  ; on_CCConcrete :
      'a ->
      (unit, unit) class_const_kind ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_const_kind
  ; on_CGexpr :
      'a ->
      (unit, unit) class_get_expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_get_expr
  ; on_CGstring :
      'a ->
      (unit, unit) class_get_expr ->
      pstring ->
      (unit, unit) class_get_expr
  ; on_CI : 'a -> (unit, unit) class_id_ -> sid -> (unit, unit) class_id_
  ; on_CIexpr :
      'a ->
      (unit, unit) class_id_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_id_
  ; on_CIparent : 'a -> (unit, unit) class_id_ -> (unit, unit) class_id_
  ; on_CIself : 'a -> (unit, unit) class_id_ -> (unit, unit) class_id_
  ; on_CIstatic : 'a -> (unit, unit) class_id_ -> (unit, unit) class_id_
  ; on_CRexact : 'a -> ctx_refinement -> Aast_defs.hint -> ctx_refinement
  ; on_CRloose : 'a -> ctx_refinement -> ctx_refinement_bounds -> ctx_refinement
  ; on_Call :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      unit Aast_defs.targ list ->
      (Ast_defs.param_kind * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) expr_
  ; on_Case :
      'a ->
      (unit, unit) gen_case ->
      (unit, unit) Aast_defs.case ->
      (unit, unit) gen_case
  ; on_Cast :
      'a ->
      (unit, unit) expr_ ->
      Aast_defs.hint ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Cclass :
      'a ->
      Ast_defs.classish_kind ->
      Ast_defs.abstraction ->
      Ast_defs.classish_kind
  ; on_Cenum : 'a -> Ast_defs.classish_kind -> Ast_defs.classish_kind
  ; on_Cenum_class :
      'a ->
      Ast_defs.classish_kind ->
      Ast_defs.abstraction ->
      Ast_defs.classish_kind
  ; on_ChildBinary : 'a -> xhp_child -> xhp_child -> xhp_child -> xhp_child
  ; on_ChildList : 'a -> xhp_child -> xhp_child list -> xhp_child
  ; on_ChildName : 'a -> xhp_child -> sid -> xhp_child
  ; on_ChildPlus : 'a -> xhp_child_op -> xhp_child_op
  ; on_ChildQuestion : 'a -> xhp_child_op -> xhp_child_op
  ; on_ChildStar : 'a -> xhp_child_op -> xhp_child_op
  ; on_ChildUnary : 'a -> xhp_child -> xhp_child -> xhp_child_op -> xhp_child
  ; on_Cinterface : 'a -> Ast_defs.classish_kind -> Ast_defs.classish_kind
  ; on_Class : 'a -> (unit, unit) def -> (unit, unit) class_ -> (unit, unit) def
  ; on_Class_const :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) expr_
  ; on_Class_get :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.class_id ->
      (unit, unit) class_get_expr ->
      prop_or_method ->
      (unit, unit) expr_
  ; on_Clone :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Cmp : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Collection :
      'a ->
      (unit, unit) expr_ ->
      sid ->
      unit collection_targ option ->
      (unit, unit) afield list ->
      (unit, unit) expr_
  ; on_CollectionTKV :
      'a ->
      unit collection_targ ->
      unit Aast_defs.targ ->
      unit Aast_defs.targ ->
      unit collection_targ
  ; on_CollectionTV :
      'a -> unit collection_targ -> unit Aast_defs.targ -> unit collection_targ
  ; on_Concrete : 'a -> Ast_defs.abstraction -> Ast_defs.abstraction
  ; on_Constant :
      'a -> (unit, unit) def -> (unit, unit) gconst -> (unit, unit) def
  ; on_Constraint_as :
      'a -> Ast_defs.constraint_kind -> Ast_defs.constraint_kind
  ; on_Constraint_eq :
      'a -> Ast_defs.constraint_kind -> Ast_defs.constraint_kind
  ; on_Constraint_super :
      'a -> Ast_defs.constraint_kind -> Ast_defs.constraint_kind
  ; on_Continue : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_Contravariant : 'a -> Ast_defs.variance -> Ast_defs.variance
  ; on_Covariant : 'a -> Ast_defs.variance -> Ast_defs.variance
  ; on_Ctrait : 'a -> Ast_defs.classish_kind -> Ast_defs.classish_kind
  ; on_Darray :
      'a ->
      (unit, unit) expr_ ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      ((unit, unit) Aast_defs.expr * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) expr_
  ; on_Default :
      'a ->
      (unit, unit) gen_case ->
      (unit, unit) Aast_defs.default_case ->
      (unit, unit) gen_case
  ; on_Dict : 'a -> kvc_kind -> kvc_kind
  ; on_Diff : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Diff2 : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Do :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
  ; on_Dollardollar :
      'a -> (unit, unit) expr_ -> Aast_defs.lid -> (unit, unit) expr_
  ; on_Dot : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_ET_Splice :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Efun :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) fun_ ->
      Aast_defs.lid list ->
      (unit, unit) expr_
  ; on_Eif :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Emit_id : 'a -> emit_id -> int -> emit_id
  ; on_EnforcedCast : 'a -> hole_source -> Aast_defs.hint list -> hole_source
  ; on_EnumClassLabel :
      'a ->
      (unit, unit) expr_ ->
      sid option ->
      byte_string ->
      (unit, unit) expr_
  ; on_Eq : 'a -> Ast_defs.bop -> Ast_defs.bop option -> Ast_defs.bop
  ; on_Eqeq : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Eqeqeq : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Erased : 'a -> reify_kind -> reify_kind
  ; on_Expr :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
  ; on_ExpressionTree :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) expression_tree ->
      (unit, unit) expr_
  ; on_FAsync : 'a -> Ast_defs.fun_kind -> Ast_defs.fun_kind
  ; on_FAsyncGenerator : 'a -> Ast_defs.fun_kind -> Ast_defs.fun_kind
  ; on_FGenerator : 'a -> Ast_defs.fun_kind -> Ast_defs.fun_kind
  ; on_FP_class_const :
      'a ->
      (unit, unit) function_ptr_id ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) function_ptr_id
  ; on_FP_id :
      'a -> (unit, unit) function_ptr_id -> sid -> (unit, unit) function_ptr_id
  ; on_FSync : 'a -> Ast_defs.fun_kind -> Ast_defs.fun_kind
  ; on_Fallthrough : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_False : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_FileAttributes :
      'a -> (unit, unit) def -> (unit, unit) file_attribute -> (unit, unit) def
  ; on_Float : 'a -> (unit, unit) expr_ -> byte_string -> (unit, unit) expr_
  ; on_For :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_Foreach :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_Fun : 'a -> (unit, unit) def -> (unit, unit) fun_def -> (unit, unit) def
  ; on_Fun_id : 'a -> (unit, unit) expr_ -> sid -> (unit, unit) expr_
  ; on_FunctionPointer :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) function_ptr_id ->
      unit Aast_defs.targ list ->
      (unit, unit) expr_
  ; on_Gt : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Gte : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Gtgt : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Habstr : 'a -> hint_ -> byte_string -> Aast_defs.hint list -> hint_
  ; on_Haccess : 'a -> hint_ -> Aast_defs.hint -> sid list -> hint_
  ; on_Hany : 'a -> hint_ -> hint_
  ; on_Happly : 'a -> hint_ -> sid -> Aast_defs.hint list -> hint_
  ; on_Hdynamic : 'a -> hint_ -> hint_
  ; on_Herr : 'a -> hint_ -> hint_
  ; on_Hfun : 'a -> hint_ -> hint_fun -> hint_
  ; on_Hfun_context : 'a -> hint_ -> byte_string -> hint_
  ; on_Hintersection : 'a -> hint_ -> Aast_defs.hint list -> hint_
  ; on_Hlike : 'a -> hint_ -> Aast_defs.hint -> hint_
  ; on_Hmixed : 'a -> hint_ -> hint_
  ; on_Hnonnull : 'a -> hint_ -> hint_
  ; on_Hnothing : 'a -> hint_ -> hint_
  ; on_Hole :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      unit ->
      unit ->
      hole_source ->
      (unit, unit) expr_
  ; on_Hoption : 'a -> hint_ -> Aast_defs.hint -> hint_
  ; on_Hprim : 'a -> hint_ -> tprim -> hint_
  ; on_Hrefinement : 'a -> hint_ -> Aast_defs.hint -> refinement list -> hint_
  ; on_Hshape : 'a -> hint_ -> nast_shape_info -> hint_
  ; on_Hsoft : 'a -> hint_ -> Aast_defs.hint -> hint_
  ; on_Hthis : 'a -> hint_ -> hint_
  ; on_Htuple : 'a -> hint_ -> Aast_defs.hint list -> hint_
  ; on_Hunion : 'a -> hint_ -> Aast_defs.hint list -> hint_
  ; on_Hvar : 'a -> hint_ -> byte_string -> hint_
  ; on_Hvec_or_dict :
      'a -> hint_ -> Aast_defs.hint option -> Aast_defs.hint -> hint_
  ; on_Id : 'a -> (unit, unit) expr_ -> sid -> (unit, unit) expr_
  ; on_If :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_ImmMap : 'a -> kvc_kind -> kvc_kind
  ; on_ImmSet : 'a -> vc_kind -> vc_kind
  ; on_ImmVector : 'a -> vc_kind -> vc_kind
  ; on_Import :
      'a ->
      (unit, unit) expr_ ->
      import_flavor ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Include : 'a -> import_flavor -> import_flavor
  ; on_IncludeOnce : 'a -> import_flavor -> import_flavor
  ; on_Int : 'a -> (unit, unit) expr_ -> byte_string -> (unit, unit) expr_
  ; on_Invariant : 'a -> Ast_defs.variance -> Ast_defs.variance
  ; on_Is :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      (unit, unit) expr_
  ; on_Is_method : 'a -> prop_or_method -> prop_or_method
  ; on_Is_prop : 'a -> prop_or_method -> prop_or_method
  ; on_Join : 'a -> env_annot -> env_annot
  ; on_KeyValCollection :
      'a ->
      (unit, unit) expr_ ->
      kvc_kind ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      (unit, unit) Aast_defs.field list ->
      (unit, unit) expr_
  ; on_Keyset : 'a -> vc_kind -> vc_kind
  ; on_LateInit : 'a -> xhp_attr_tag -> xhp_attr_tag
  ; on_Lfun :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) fun_ ->
      Aast_defs.lid list ->
      (unit, unit) expr_
  ; on_List :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Lplaceholder : 'a -> (unit, unit) expr_ -> pos -> (unit, unit) expr_
  ; on_Lt : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Lte : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Ltlt : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Lvar : 'a -> (unit, unit) expr_ -> Aast_defs.lid -> (unit, unit) expr_
  ; on_MDNameExact : 'a -> md_name_kind -> sid -> md_name_kind
  ; on_MDNameGlobal : 'a -> md_name_kind -> pos -> md_name_kind
  ; on_MDNamePrefix : 'a -> md_name_kind -> sid -> md_name_kind
  ; on_Map : 'a -> kvc_kind -> kvc_kind
  ; on_Markup : 'a -> (unit, unit) stmt_ -> pstring -> (unit, unit) stmt_
  ; on_Method_caller :
      'a -> (unit, unit) expr_ -> sid -> pstring -> (unit, unit) expr_
  ; on_Method_id :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      pstring ->
      (unit, unit) expr_
  ; on_Minus : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Module :
      'a -> (unit, unit) def -> (unit, unit) module_def -> (unit, unit) def
  ; on_NSClass : 'a -> ns_kind -> ns_kind
  ; on_NSClassAndNamespace : 'a -> ns_kind -> ns_kind
  ; on_NSConst : 'a -> ns_kind -> ns_kind
  ; on_NSFun : 'a -> ns_kind -> ns_kind
  ; on_NSNamespace : 'a -> ns_kind -> ns_kind
  ; on_Namespace :
      'a -> (unit, unit) def -> sid -> (unit, unit) def list -> (unit, unit) def
  ; on_NamespaceUse :
      'a -> (unit, unit) def -> (ns_kind * sid * sid) list -> (unit, unit) def
  ; on_New :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.class_id ->
      unit Aast_defs.targ list ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.expr option ->
      unit ->
      (unit, unit) expr_
  ; on_Noop : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_Null : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_OG_nullsafe : 'a -> og_null_flavor -> og_null_flavor
  ; on_OG_nullthrows : 'a -> og_null_flavor -> og_null_flavor
  ; on_Obj_get :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      og_null_flavor ->
      prop_or_method ->
      (unit, unit) expr_
  ; on_Omitted : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_Opaque : 'a -> typedef_visibility -> typedef_visibility
  ; on_OpaqueModule : 'a -> typedef_visibility -> typedef_visibility
  ; on_Pair :
      'a ->
      (unit, unit) expr_ ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Percent : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Pinout : 'a -> Ast_defs.param_kind -> pos -> Ast_defs.param_kind
  ; on_Pipe :
      'a ->
      (unit, unit) expr_ ->
      Aast_defs.lid ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Plus : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Pnormal : 'a -> Ast_defs.param_kind -> Ast_defs.param_kind
  ; on_PrefixedString :
      'a ->
      (unit, unit) expr_ ->
      byte_string ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_QuestionQuestion : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Rctx : 'a -> refinement -> sid -> ctx_refinement -> refinement
  ; on_Readonly : 'a -> Ast_defs.readonly_kind -> Ast_defs.readonly_kind
  ; on_ReadonlyExpr :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Refinement : 'a -> env_annot -> env_annot
  ; on_Reified : 'a -> reify_kind -> reify_kind
  ; on_Require : 'a -> import_flavor -> import_flavor
  ; on_RequireClass : 'a -> require_kind -> require_kind
  ; on_RequireExtends : 'a -> require_kind -> require_kind
  ; on_RequireImplements : 'a -> require_kind -> require_kind
  ; on_RequireOnce : 'a -> import_flavor -> import_flavor
  ; on_Required : 'a -> xhp_attr_tag -> xhp_attr_tag
  ; on_Return :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) stmt_
  ; on_Rtype : 'a -> refinement -> sid -> type_refinement -> refinement
  ; on_SFclass_const :
      'a ->
      Ast_defs.shape_field_name ->
      sid ->
      pstring ->
      Ast_defs.shape_field_name
  ; on_SFlit_int :
      'a -> Ast_defs.shape_field_name -> pstring -> Ast_defs.shape_field_name
  ; on_SFlit_str :
      'a ->
      Ast_defs.shape_field_name ->
      positioned_byte_string ->
      Ast_defs.shape_field_name
  ; on_Set : 'a -> vc_kind -> vc_kind
  ; on_SetModule : 'a -> (unit, unit) def -> sid -> (unit, unit) def
  ; on_SetNamespaceEnv : 'a -> (unit, unit) def -> nsenv -> (unit, unit) def
  ; on_Shape :
      'a ->
      (unit, unit) expr_ ->
      (Ast_defs.shape_field_name * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) expr_
  ; on_Slash : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Smethod_id :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) expr_
  ; on_SoftReified : 'a -> reify_kind -> reify_kind
  ; on_Star : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Starstar : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Stmt :
      'a -> (unit, unit) def -> (unit, unit) Aast_defs.stmt -> (unit, unit) def
  ; on_String : 'a -> (unit, unit) expr_ -> byte_string -> (unit, unit) expr_
  ; on_String2 :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Switch :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.case list ->
      (unit, unit) Aast_defs.default_case option ->
      (unit, unit) stmt_
  ; on_TCAbstract :
      'a -> class_typeconst -> class_abstract_typeconst -> class_typeconst
  ; on_TCConcrete :
      'a -> class_typeconst -> class_concrete_typeconst -> class_typeconst
  ; on_TRexact : 'a -> type_refinement -> Aast_defs.hint -> type_refinement
  ; on_TRloose :
      'a -> type_refinement -> type_refinement_bounds -> type_refinement
  ; on_Tarraykey : 'a -> tprim -> tprim
  ; on_Tbool : 'a -> tprim -> tprim
  ; on_Tfloat : 'a -> tprim -> tprim
  ; on_This : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_Throw :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
  ; on_Tint : 'a -> tprim -> tprim
  ; on_Tnoreturn : 'a -> tprim -> tprim
  ; on_Tnull : 'a -> tprim -> tprim
  ; on_Tnum : 'a -> tprim -> tprim
  ; on_Transparent : 'a -> typedef_visibility -> typedef_visibility
  ; on_Tresource : 'a -> tprim -> tprim
  ; on_True : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_Try :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.catch list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_Tstring : 'a -> tprim -> tprim
  ; on_Tuple :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Tvoid : 'a -> tprim -> tprim
  ; on_Typedef :
      'a -> (unit, unit) def -> (unit, unit) typedef -> (unit, unit) def
  ; on_Typing : 'a -> hole_source -> hole_source
  ; on_Udecr : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Uincr : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Uminus : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Unop :
      'a ->
      (unit, unit) expr_ ->
      Ast_defs.uop ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
  ; on_Unot : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_UnsafeCast : 'a -> hole_source -> Aast_defs.hint list -> hole_source
  ; on_UnsafeNonnullCast : 'a -> hole_source -> hole_source
  ; on_Upcast :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      (unit, unit) expr_
  ; on_Updecr : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Upincr : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Uplus : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Usilence : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_Using :
      'a -> (unit, unit) stmt_ -> (unit, unit) using_stmt -> (unit, unit) stmt_
  ; on_Utild : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_ValCollection :
      'a ->
      (unit, unit) expr_ ->
      vc_kind ->
      unit Aast_defs.targ option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Varray :
      'a ->
      (unit, unit) expr_ ->
      unit Aast_defs.targ option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Vec : 'a -> vc_kind -> vc_kind
  ; on_Vector : 'a -> vc_kind -> vc_kind
  ; on_While :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
  ; on_XEV_Int : 'a -> Ast_defs.xhp_enum_value -> int -> Ast_defs.xhp_enum_value
  ; on_XEV_String :
      'a -> Ast_defs.xhp_enum_value -> byte_string -> Ast_defs.xhp_enum_value
  ; on_Xhp_simple :
      'a ->
      (unit, unit) xhp_attribute ->
      (unit, unit) xhp_simple ->
      (unit, unit) xhp_attribute
  ; on_Xhp_spread :
      'a ->
      (unit, unit) xhp_attribute ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) xhp_attribute
  ; on_Xml :
      'a ->
      (unit, unit) expr_ ->
      sid ->
      (unit, unit) xhp_attribute list ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
  ; on_Xor : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_Yield :
      'a -> (unit, unit) expr_ -> (unit, unit) afield -> (unit, unit) expr_
  ; on_Yield_break : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_abstraction : 'a -> Ast_defs.abstraction -> Ast_defs.abstraction
  ; on_afield : 'a -> (unit, unit) afield -> (unit, unit) afield
  ; on_as_expr : 'a -> (unit, unit) as_expr -> (unit, unit) as_expr
  ; on_block :
      'a -> (unit, unit) Aast_defs.block -> (unit, unit) Aast_defs.block
  ; on_bop : 'a -> Ast_defs.bop -> Ast_defs.bop
  ; on_byte_string : 'a -> byte_string -> byte_string
  ; on_case : 'a -> (unit, unit) Aast_defs.case -> (unit, unit) Aast_defs.case
  ; on_catch :
      'a -> (unit, unit) Aast_defs.catch -> (unit, unit) Aast_defs.catch
  ; on_class_ : 'a -> (unit, unit) class_ -> (unit, unit) class_
  ; on_class_abstract_typeconst :
      'a -> class_abstract_typeconst -> class_abstract_typeconst
  ; on_class_c_consts :
      'a -> (unit, unit) class_const list -> (unit, unit) class_const list
  ; on_class_c_enum : 'a -> enum_ option -> enum_ option
  ; on_class_c_extends : 'a -> Aast_defs.hint list -> xhp_attr_hint list
  ; on_class_c_file_attributes :
      'a -> (unit, unit) file_attribute list -> (unit, unit) file_attribute list
  ; on_class_c_implements : 'a -> Aast_defs.hint list -> xhp_attr_hint list
  ; on_class_c_methods :
      'a -> (unit, unit) method_ list -> (unit, unit) method_ list
  ; on_class_c_req :
      'a -> xhp_attr_hint * require_kind -> Aast_defs.hint * require_kind
  ; on_class_c_reqs :
      'a ->
      (xhp_attr_hint * require_kind) list ->
      (xhp_attr_hint * require_kind) list
  ; on_class_c_tparams :
      'a -> (unit, unit) tparam list -> (unit, unit) tparam list
  ; on_class_c_typeconsts :
      'a ->
      (unit, unit) class_typeconst_def list ->
      (unit, unit) class_typeconst_def list
  ; on_class_c_user_attributes :
      'a -> (unit, unit) user_attribute list -> (unit, unit) user_attribute list
  ; on_class_c_uses : 'a -> Aast_defs.hint list -> xhp_attr_hint list
  ; on_class_c_vars :
      'a -> (unit, unit) class_var list -> (unit, unit) class_var list
  ; on_class_c_where_constraints :
      'a -> Aast_defs.where_constraint_hint list -> where_constraint_hint list
  ; on_class_c_xhp_attr_uses : 'a -> Aast_defs.hint list -> xhp_attr_hint list
  ; on_class_c_xhp_attrs :
      'a -> (unit, unit) Aast_defs.xhp_attr list -> (unit, unit) xhp_attr list
  ; on_class_concrete_typeconst :
      'a -> class_concrete_typeconst -> class_concrete_typeconst
  ; on_class_const : 'a -> (unit, unit) class_const -> (unit, unit) class_const
  ; on_class_const_kind :
      'a -> (unit, unit) class_const_kind -> (unit, unit) class_const_kind
  ; on_class_get_expr :
      'a -> (unit, unit) class_get_expr -> (unit, unit) class_get_expr
  ; on_class_hint : 'a -> Aast_defs.hint -> Aast_defs.hint
  ; on_class_id :
      'a -> (unit, unit) Aast_defs.class_id -> (unit, unit) Aast_defs.class_id
  ; on_class_id_ : 'a -> (unit, unit) class_id_ -> (unit, unit) class_id_
  ; on_class_name : 'a -> sid -> sid
  ; on_class_typeconst : 'a -> class_typeconst -> class_typeconst
  ; on_class_typeconst_def :
      'a -> (unit, unit) class_typeconst_def -> (unit, unit) class_typeconst_def
  ; on_class_var : 'a -> (unit, unit) class_var -> (unit, unit) class_var
  ; on_class_var_cv_expr :
      'a -> (unit, unit) Aast_defs.expr option -> (unit, unit) expr option
  ; on_class_var_cv_type : 'a -> unit type_hint -> unit type_hint
  ; on_class_var_cv_user_attributes :
      'a -> (unit, unit) user_attribute list -> (unit, unit) user_attribute list
  ; on_classish_kind : 'a -> Ast_defs.classish_kind -> Ast_defs.classish_kind
  ; on_collection_targ : 'a -> unit collection_targ -> unit collection_targ
  ; on_constraint_kind :
      'a -> Ast_defs.constraint_kind -> Ast_defs.constraint_kind
  ; on_context : 'a -> Aast_defs.hint -> Aast_defs.hint
  ; on_contexts : 'a -> Aast_defs.contexts -> Aast_defs.contexts
  ; on_ctx_refinement : 'a -> ctx_refinement -> ctx_refinement
  ; on_ctx_refinement_bounds :
      'a -> ctx_refinement_bounds -> ctx_refinement_bounds
  ; on_def : 'a -> (unit, unit) def -> (unit, unit) def
  ; on_default_case :
      'a ->
      (unit, unit) Aast_defs.default_case ->
      (unit, unit) Aast_defs.default_case
  ; on_doc_comment : 'a -> pstring -> pstring
  ; on_emit_id : 'a -> emit_id -> emit_id
  ; on_enum_ : 'a -> enum_ -> enum_
  ; on_env_annot : 'a -> env_annot -> env_annot
  ; on_expr : 'a -> (unit, unit) Aast_defs.expr -> (unit, unit) Aast_defs.expr
  ; on_expr_ : 'a -> (unit, unit) expr_ -> (unit, unit) expr_
  ; on_expression_tree :
      'a -> (unit, unit) expression_tree -> (unit, unit) expression_tree
  ; on_field :
      'a -> (unit, unit) Aast_defs.field -> (unit, unit) Aast_defs.field
  ; on_file_attribute :
      'a -> (unit, unit) file_attribute -> (unit, unit) file_attribute
  ; on_file_attributes :
      'a -> (unit, unit) file_attribute list -> (unit, unit) file_attribute list
  ; on_fun_ : 'a -> (unit, unit) fun_ -> (unit, unit) fun_
  ; on_fun_def : 'a -> (unit, unit) fun_def -> (unit, unit) fun_def
  ; on_fun_f_body : 'a -> (unit, unit) func_body -> (unit, unit) func_body
  ; on_fun_f_ctxs : 'a -> Aast_defs.contexts option -> contexts option
  ; on_fun_f_params :
      'a -> (unit, unit) fun_param list -> (unit, unit) fun_param list
  ; on_fun_f_ret : 'a -> unit type_hint -> unit type_hint
  ; on_fun_f_tparams :
      'a -> (unit, unit) tparam list -> (unit, unit) tparam list
  ; on_fun_f_unsafe_ctxs : 'a -> Aast_defs.contexts option -> contexts option
  ; on_fun_f_user_attributes :
      'a -> (unit, unit) user_attribute list -> (unit, unit) user_attribute list
  ; on_fun_f_where_constraints :
      'a -> Aast_defs.where_constraint_hint list -> where_constraint_hint list
  ; on_fun_kind : 'a -> Ast_defs.fun_kind -> Ast_defs.fun_kind
  ; on_fun_param : 'a -> (unit, unit) fun_param -> (unit, unit) fun_param
  ; on_func_body : 'a -> (unit, unit) func_body -> (unit, unit) func_body
  ; on_function_ptr_id :
      'a -> (unit, unit) function_ptr_id -> (unit, unit) function_ptr_id
  ; on_gconst : 'a -> (unit, unit) gconst -> (unit, unit) gconst
  ; on_gconst_cst_type : 'a -> Aast_defs.hint option -> xhp_attr_hint option
  ; on_gconst_cst_value : 'a -> (unit, unit) expr -> (unit, unit) expr
  ; on_gen_case : 'a -> (unit, unit) gen_case -> (unit, unit) gen_case
  ; on_hf_param_info : 'a -> hf_param_info -> hf_param_info
  ; on_hint : 'a -> Aast_defs.hint -> Aast_defs.hint
  ; on_hint_ : 'a -> hint_ -> hint_
  ; on_hint_fun : 'a -> hint_fun -> hint_fun
  ; on_hint_fun_hf_ctxs : 'a -> Aast_defs.contexts option -> contexts option
  ; on_hint_fun_hf_param_tys : 'a -> Aast_defs.hint list -> xhp_attr_hint list
  ; on_hint_fun_hf_return_ty : 'a -> xhp_attr_hint -> xhp_attr_hint
  ; on_hint_fun_hf_variadic_ty : 'a -> variadic_hint -> variadic_hint
  ; on_hole_source : 'a -> hole_source -> hole_source
  ; on_id : 'a -> sid -> sid
  ; on_id_ : 'a -> byte_string -> byte_string
  ; on_import_flavor : 'a -> import_flavor -> import_flavor
  ; on_is_variadic : 'a -> is_variadic -> is_variadic
  ; on_kvc_kind : 'a -> kvc_kind -> kvc_kind
  ; on_lid : 'a -> Aast_defs.lid -> Aast_defs.lid
  ; on_local_id : 'a -> local_id -> local_id
  ; on_md_name_kind : 'a -> md_name_kind -> md_name_kind
  ; on_method_ : 'a -> (unit, unit) method_ -> (unit, unit) method_
  ; on_method_m_body : 'a -> (unit, unit) func_body -> (unit, unit) func_body
  ; on_method_m_ctxs : 'a -> Aast_defs.contexts option -> contexts option
  ; on_method_m_params :
      'a -> (unit, unit) fun_param list -> (unit, unit) fun_param list
  ; on_method_m_ret : 'a -> unit type_hint -> unit type_hint
  ; on_method_m_tparams :
      'a -> (unit, unit) tparam list -> (unit, unit) tparam list
  ; on_method_m_unsafe_ctxs : 'a -> Aast_defs.contexts option -> contexts option
  ; on_method_m_user_attributes :
      'a -> (unit, unit) user_attribute list -> (unit, unit) user_attribute list
  ; on_method_m_where_constraints :
      'a -> Aast_defs.where_constraint_hint list -> where_constraint_hint list
  ; on_module_def : 'a -> (unit, unit) module_def -> (unit, unit) module_def
  ; on_nast_shape_info : 'a -> nast_shape_info -> nast_shape_info
  ; on_ns_kind : 'a -> ns_kind -> ns_kind
  ; on_nsenv : 'a -> nsenv -> nsenv
  ; on_og_null_flavor : 'a -> og_null_flavor -> og_null_flavor
  ; on_param_kind : 'a -> Ast_defs.param_kind -> Ast_defs.param_kind
  ; on_pos : 'a -> pos -> pos
  ; on_positioned_byte_string :
      'a -> positioned_byte_string -> positioned_byte_string
  ; on_program : 'a -> (unit, unit) def list -> (unit, unit) def list
  ; on_prop_or_method : 'a -> prop_or_method -> prop_or_method
  ; on_pstring : 'a -> pstring -> pstring
  ; on_readonly_kind : 'a -> Ast_defs.readonly_kind -> Ast_defs.readonly_kind
  ; on_refinement : 'a -> refinement -> refinement
  ; on_reified : 'a -> is_variadic -> is_variadic
  ; on_reify_kind : 'a -> reify_kind -> reify_kind
  ; on_require_kind : 'a -> require_kind -> require_kind
  ; on_shape_field_info : 'a -> shape_field_info -> shape_field_info
  ; on_shape_field_name :
      'a -> Ast_defs.shape_field_name -> Ast_defs.shape_field_name
  ; on_sid : 'a -> sid -> sid
  ; on_stmt : 'a -> (unit, unit) Aast_defs.stmt -> (unit, unit) Aast_defs.stmt
  ; on_stmt_ : 'a -> (unit, unit) stmt_ -> (unit, unit) stmt_
  ; on_targ : 'a -> unit Aast_defs.targ -> unit Aast_defs.targ
  ; on_tparam : 'a -> (unit, unit) tparam -> (unit, unit) tparam
  ; on_tprim : 'a -> tprim -> tprim
  ; on_trait_hint : 'a -> Aast_defs.hint -> Aast_defs.hint
  ; on_type_hint : 'a -> unit Aast_defs.type_hint -> unit Aast_defs.type_hint
  ; on_type_hint_ : 'a -> Aast_defs.type_hint_ -> Aast_defs.type_hint_
  ; on_type_refinement : 'a -> type_refinement -> type_refinement
  ; on_type_refinement_bounds :
      'a -> type_refinement_bounds -> type_refinement_bounds
  ; on_typedef : 'a -> (unit, unit) typedef -> (unit, unit) typedef
  ; on_typedef_visibility : 'a -> typedef_visibility -> typedef_visibility
  ; on_uop : 'a -> Ast_defs.uop -> Ast_defs.uop
  ; on_user_attribute :
      'a -> (unit, unit) user_attribute -> (unit, unit) user_attribute
  ; on_user_attributes :
      'a -> (unit, unit) user_attribute list -> (unit, unit) user_attribute list
  ; on_using_stmt : 'a -> (unit, unit) using_stmt -> (unit, unit) using_stmt
  ; on_variadic_hint : 'a -> Aast_defs.variadic_hint -> Aast_defs.variadic_hint
  ; on_variance : 'a -> Ast_defs.variance -> Ast_defs.variance
  ; on_vc_kind : 'a -> vc_kind -> vc_kind
  ; on_visibility : 'a -> visibility -> visibility
  ; on_visibility_Internal : 'a -> visibility -> visibility
  ; on_visibility_Private : 'a -> visibility -> visibility
  ; on_visibility_Protected : 'a -> visibility -> visibility
  ; on_visibility_Public : 'a -> visibility -> visibility
  ; on_where_constraint_hint :
      'a -> Aast_defs.where_constraint_hint -> Aast_defs.where_constraint_hint
  ; on_xhp_attr :
      'a -> (unit, unit) Aast_defs.xhp_attr -> (unit, unit) Aast_defs.xhp_attr
  ; on_xhp_attr_hint : 'a -> Aast_defs.hint -> Aast_defs.hint
  ; on_xhp_attr_info : 'a -> xhp_attr_info -> xhp_attr_info
  ; on_xhp_attr_tag : 'a -> xhp_attr_tag -> xhp_attr_tag
  ; on_xhp_attribute :
      'a -> (unit, unit) xhp_attribute -> (unit, unit) xhp_attribute
  ; on_xhp_child : 'a -> xhp_child -> xhp_child
  ; on_xhp_child_op : 'a -> xhp_child_op -> xhp_child_op
  ; on_xhp_enum_value : 'a -> Ast_defs.xhp_enum_value -> Ast_defs.xhp_enum_value
  ; on_xhp_simple : 'a -> (unit, unit) xhp_simple -> (unit, unit) xhp_simple >
