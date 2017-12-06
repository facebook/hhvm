(* @generated from ast.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2017, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<45f04c02eae2d9c16e1f7b5c62bc56625a5f959a>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2015, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
include Ast_visitors_ancestors
type program = def list[@@deriving
                         ((visitors
                             {
                               variety = "endo";
                               nude = true;
                               visit_prefix = "on_";
                               ancestors = ["endo_base"]
                             }),
                           (visitors
                              {
                                variety = "reduce";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["reduce_base"]
                              }),
                           (visitors
                              {
                                variety = "map";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["map_base"]
                              }),
                           (visitors
                              {
                                variety = "iter";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["iter_base"]
                              }))]
and def =
  | Fun of fun_ 
  | Class of class_ 
  | Stmt of stmt 
  | Typedef of typedef 
  | Constant of gconst 
  | Namespace of id * program 
  | NamespaceUse of (ns_kind * id * id) list 
  | SetNamespaceEnv of Namespace_env.env 
and typedef =
  {
  t_id: id ;
  t_tparams: tparam list ;
  t_constraint: tconstraint ;
  t_kind: typedef_kind ;
  t_user_attributes: user_attribute list ;
  t_namespace: Namespace_env.env ;
  t_mode: FileInfo.mode }
and gconst =
  {
  cst_mode: FileInfo.mode ;
  cst_kind: cst_kind ;
  cst_name: id ;
  cst_type: hint option ;
  cst_value: expr ;
  cst_namespace: Namespace_env.env }
and tparam = (variance * id * (constraint_kind * hint) list)
and tconstraint = hint option
and typedef_kind =
  | Alias of hint 
  | NewType of hint 
and class_ =
  {
  c_mode: FileInfo.mode ;
  c_user_attributes: user_attribute list ;
  c_final: bool ;
  c_kind: class_kind ;
  c_is_xhp: bool ;
  c_name: id ;
  c_tparams: tparam list ;
  c_extends: hint list ;
  c_implements: hint list ;
  c_body: class_elt list ;
  c_namespace: Namespace_env.env ;
  c_enum: enum_ option ;
  c_span: Pos.t ;
  c_doc_comment: string option }
and enum_ = {
  e_base: hint ;
  e_constraint: hint option }
and user_attribute = {
  ua_name: id ;
  ua_params: expr list }
and class_elt =
  | Const of hint option * (id * expr) list 
  | AbsConst of hint option * id 
  | Attributes of class_attr list 
  | TypeConst of typeconst 
  | ClassUse of hint 
  | ClassUseAlias of id option * pstring * id option * kind list 
  | ClassUsePrecedence of id * pstring * id list 
  | XhpAttrUse of hint 
  | ClassTraitRequire of trait_req_kind * hint 
  | ClassVars of class_vars_ 
  | XhpAttr of hint option * class_var * bool * (Pos.t * bool * expr list)
  option 
  | Method of method_ 
  | XhpCategory of pstring list 
  | XhpChild of xhp_child 
and xhp_child =
  | ChildName of id 
  | ChildList of xhp_child list 
  | ChildUnary of xhp_child * xhp_child_op 
  | ChildBinary of xhp_child * xhp_child 
and xhp_child_op =
  | ChildStar 
  | ChildPlus 
  | ChildQuestion 
and class_attr =
  | CA_name of id 
  | CA_field of ca_field 
and ca_field =
  {
  ca_type: ca_type ;
  ca_id: id ;
  ca_value: expr option ;
  ca_required: bool }
and ca_type =
  | CA_hint of hint 
  | CA_enum of string list 
and class_var = (Pos.t * id * expr option)
and class_vars_ =
  {
  cv_kinds: kind list ;
  cv_hint: hint option ;
  cv_names: class_var list ;
  cv_doc_comment: string option ;
  cv_user_attributes: user_attribute list }
and method_ =
  {
  m_kind: kind list ;
  m_tparams: tparam list ;
  m_constrs: (hint * constraint_kind * hint) list ;
  m_name: id ;
  m_params: fun_param list ;
  m_body: block ;
  m_user_attributes: user_attribute list ;
  m_ret: hint option ;
  m_ret_by_ref: bool ;
  m_fun_kind: fun_kind ;
  m_span: Pos.t ;
  m_doc_comment: string option }
and typeconst =
  {
  tconst_abstract: bool ;
  tconst_name: id ;
  tconst_tparams: tparam list ;
  tconst_constraint: hint option ;
  tconst_type: hint option ;
  tconst_span: Pos.t }
and is_reference = bool
and is_variadic = bool
and fun_param =
  {
  param_hint: hint option ;
  param_is_reference: is_reference ;
  param_is_variadic: is_variadic ;
  param_id: id ;
  param_expr: expr option ;
  param_modifier: kind option ;
  param_callconv: param_kind option ;
  param_user_attributes: user_attribute list }
and fun_ =
  {
  f_mode: FileInfo.mode ;
  f_tparams: tparam list ;
  f_constrs: (hint * constraint_kind * hint) list ;
  f_ret: hint option ;
  f_ret_by_ref: bool ;
  f_name: id ;
  f_params: fun_param list ;
  f_body: block ;
  f_user_attributes: user_attribute list ;
  f_fun_kind: fun_kind ;
  f_namespace: Namespace_env.env ;
  f_span: Pos.t ;
  f_doc_comment: string option ;
  f_static: bool }
and is_coroutine = bool
and hint = (Pos.t * hint_)
and variadic_hint =
  | Hvariadic of hint option 
  | Hnon_variadic 
and hint_ =
  | Hoption of hint 
  | Hfun of is_coroutine * hint list * param_kind option list * variadic_hint
  * hint 
  | Htuple of hint list 
  | Happly of id * hint list 
  | Hshape of shape_info 
  | Haccess of id * id * id list 
  | Hsoft of hint 
and shape_info =
  {
  si_allows_unknown_fields: bool ;
  si_shape_field_list: shape_field list }
and shape_field =
  {
  sf_optional: bool ;
  sf_name: shape_field_name ;
  sf_hint: hint }
and using_stmt =
  {
  us_is_block_scoped: bool ;
  us_has_await: bool ;
  us_expr: expr ;
  us_block: block }
and stmt =
  | Unsafe 
  | Fallthrough 
  | Expr of expr 
  | Block of block 
  | Break of Pos.t * expr option 
  | Continue of Pos.t * expr option 
  | Throw of expr 
  | Return of Pos.t * expr option 
  | GotoLabel of pstring 
  | Goto of pstring 
  | Static_var of expr list 
  | Global_var of Pos.t * expr list 
  | If of expr * block * block 
  | Do of block * expr 
  | While of expr * block 
  | For of Pos.t * expr * expr * expr * block 
  | Switch of expr * case list 
  | Foreach of expr * Pos.t option * as_expr * block 
  | Try of block * catch list * block 
  | Def_inline of def 
  | Noop 
  | Markup of pstring * expr option 
  | Using of using_stmt 
  | Declare of bool * expr * block 
and as_expr =
  | As_v of expr 
  | As_kv of expr * expr 
and xhp_attribute =
  | Xhp_simple of id * expr 
  | Xhp_spread of expr 
and block = stmt list
and expr = (Pos.t * expr_)
and expr_ =
  | Array of afield list 
  | Varray of expr list 
  | Darray of (expr * expr) list 
  | Shape of (shape_field_name * expr) list 
  | Collection of id * afield list 
  | Null 
  | True 
  | False 
  | Omitted 
  | Id of id 
  | Id_type_arguments of id * hint list 
  | Lvar of id
  [@ocaml.doc
    "\n   * PHP's Variable variable. The int is number of variable indirections\n   * (i.e. number of extra $ signs.)\n   *\n   * Example:\n     * $$$sample has int = 2.\n   * "]
  | Lvarvar of int * id 
  | Clone of expr 
  | Obj_get of expr * expr * og_null_flavor 
  | Array_get of expr * expr option 
  | Class_get of expr * expr 
  | Class_const of expr * pstring 
  | Call of expr * hint list * expr list * expr list 
  | Int of pstring 
  | Float of pstring 
  | String of pstring 
  | String2 of expr list 
  | Yield of afield 
  | Yield_break 
  | Yield_from of expr 
  | Await of expr 
  | Suspend of expr 
  | List of expr list 
  | Expr_list of expr list 
  | Cast of hint * expr 
  | Unop of uop * expr 
  | Binop of bop * expr * expr 
  | Pipe of expr * expr 
  | Eif of expr * expr option * expr 
  | NullCoalesce of expr * expr 
  | InstanceOf of expr * expr 
  | Is of expr * hint 
  | BracedExpr of expr 
  | ParenthesizedExpr of expr 
  | New of expr * expr list * expr list 
  | NewAnonClass of expr list * expr list * class_ 
  | Efun of fun_ * (id * bool) list 
  | Lfun of fun_ 
  | Xml of id * xhp_attribute list * expr list 
  | Unsafeexpr of expr 
  | Import of import_flavor * expr 
  | Callconv of param_kind * expr 
  | Execution_operator of expr list 
and import_flavor =
  | Include 
  | Require 
  | IncludeOnce 
  | RequireOnce 
and afield =
  | AFvalue of expr 
  | AFkvalue of expr * expr [@@ocaml.doc
                              " \"array\" field. Fields of array, map, dict, and shape literals. "]
and case =
  | Default of block 
  | Case of expr * block 
and catch = (id * id * block)
and field = (expr * expr)
and attr = (id * expr)
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] endo =
      object (self : 'self)
        inherit  [_] endo_base
        method on_program env = self#on_list self#on_def env
        method on_Fun env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Fun _visitors_r0
        method on_Class env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_class_ env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Class _visitors_r0
        method on_Stmt env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_stmt env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Stmt _visitors_r0
        method on_Typedef env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_typedef env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Typedef _visitors_r0
        method on_Constant env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_gconst env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Constant _visitors_r0
        method on_Namespace env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_program env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Namespace (_visitors_r0, _visitors_r1)
        method on_NamespaceUse env _visitors_this _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun
                   ((_visitors_c0,_visitors_c1,_visitors_c2) as
                      _visitors_this)
                    ->
                   let _visitors_r0 = self#on_ns_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_id env _visitors_c1  in
                   let _visitors_r2 = self#on_id env _visitors_c2  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_c1 _visitors_r1)
                          (Pervasives.(==) _visitors_c2 _visitors_r2))
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else NamespaceUse _visitors_r0
        method on_SetNamespaceEnv env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_env env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else SetNamespaceEnv _visitors_r0
        method on_def env _visitors_this =
          match _visitors_this with
          | Fun _visitors_c0 as _visitors_this ->
              self#on_Fun env _visitors_this _visitors_c0
          | Class _visitors_c0 as _visitors_this ->
              self#on_Class env _visitors_this _visitors_c0
          | Stmt _visitors_c0 as _visitors_this ->
              self#on_Stmt env _visitors_this _visitors_c0
          | Typedef _visitors_c0 as _visitors_this ->
              self#on_Typedef env _visitors_this _visitors_c0
          | Constant _visitors_c0 as _visitors_this ->
              self#on_Constant env _visitors_this _visitors_c0
          | Namespace (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Namespace env _visitors_this _visitors_c0 _visitors_c1
          | NamespaceUse _visitors_c0 as _visitors_this ->
              self#on_NamespaceUse env _visitors_this _visitors_c0
          | SetNamespaceEnv _visitors_c0 as _visitors_this ->
              self#on_SetNamespaceEnv env _visitors_this _visitors_c0
        method on_typedef env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.t_id  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.t_tparams  in
          let _visitors_r2 =
            self#on_tconstraint env _visitors_this.t_constraint  in
          let _visitors_r3 = self#on_typedef_kind env _visitors_this.t_kind
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.t_user_attributes
             in
          let _visitors_r5 = self#on_env env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_mode env _visitors_this.t_mode  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.t_id _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.t_tparams _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.t_constraint _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.t_kind _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.t_user_attributes
                             _visitors_r4)
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.t_namespace
                                _visitors_r5)
                             (Pervasives.(==) _visitors_this.t_mode
                                _visitors_r6))))))
          then _visitors_this
          else
            {
              t_id = _visitors_r0;
              t_tparams = _visitors_r1;
              t_constraint = _visitors_r2;
              t_kind = _visitors_r3;
              t_user_attributes = _visitors_r4;
              t_namespace = _visitors_r5;
              t_mode = _visitors_r6
            }
        method on_gconst env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_env env _visitors_this.cst_namespace  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.cst_mode _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.cst_kind _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.cst_name _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.cst_type _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.cst_value
                             _visitors_r4)
                          (Pervasives.(==) _visitors_this.cst_namespace
                             _visitors_r5)))))
          then _visitors_this
          else
            {
              cst_mode = _visitors_r0;
              cst_kind = _visitors_r1;
              cst_name = _visitors_r2;
              cst_type = _visitors_r3;
              cst_value = _visitors_r4;
              cst_namespace = _visitors_r5
            }
        method on_tparam env
          ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun ((_visitors_c0,_visitors_c1) as _visitors_this)  ->
                   let _visitors_r0 =
                     self#on_constraint_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_hint env _visitors_c1  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(==) _visitors_c1 _visitors_r1)
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1)) env _visitors_c2
             in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_tconstraint env = self#on_option self#on_hint env
        method on_Alias env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Alias _visitors_r0
        method on_NewType env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else NewType _visitors_r0
        method on_typedef_kind env _visitors_this =
          match _visitors_this with
          | Alias _visitors_c0 as _visitors_this ->
              self#on_Alias env _visitors_this _visitors_c0
          | NewType _visitors_c0 as _visitors_this ->
              self#on_NewType env _visitors_this _visitors_c0
        method on_class_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.c_mode  in
          let _visitors_r1 =
            self#on_list self#on_user_attribute env
              _visitors_this.c_user_attributes
             in
          let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
          let _visitors_r3 = self#on_class_kind env _visitors_this.c_kind  in
          let _visitors_r4 = self#on_bool env _visitors_this.c_is_xhp  in
          let _visitors_r5 = self#on_id env _visitors_this.c_name  in
          let _visitors_r6 =
            self#on_list self#on_tparam env _visitors_this.c_tparams  in
          let _visitors_r7 =
            self#on_list self#on_hint env _visitors_this.c_extends  in
          let _visitors_r8 =
            self#on_list self#on_hint env _visitors_this.c_implements  in
          let _visitors_r9 =
            self#on_list self#on_class_elt env _visitors_this.c_body  in
          let _visitors_r10 = self#on_env env _visitors_this.c_namespace  in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_t env _visitors_this.c_span  in
          let _visitors_r13 =
            self#on_option self#on_string env _visitors_this.c_doc_comment
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.c_mode _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.c_user_attributes
                    _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.c_final _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.c_kind _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.c_is_xhp
                             _visitors_r4)
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.c_name
                                _visitors_r5)
                             (Pervasives.(&&)
                                (Pervasives.(==) _visitors_this.c_tparams
                                   _visitors_r6)
                                (Pervasives.(&&)
                                   (Pervasives.(==) _visitors_this.c_extends
                                      _visitors_r7)
                                   (Pervasives.(&&)
                                      (Pervasives.(==)
                                         _visitors_this.c_implements
                                         _visitors_r8)
                                      (Pervasives.(&&)
                                         (Pervasives.(==)
                                            _visitors_this.c_body
                                            _visitors_r9)
                                         (Pervasives.(&&)
                                            (Pervasives.(==)
                                               _visitors_this.c_namespace
                                               _visitors_r10)
                                            (Pervasives.(&&)
                                               (Pervasives.(==)
                                                  _visitors_this.c_enum
                                                  _visitors_r11)
                                               (Pervasives.(&&)
                                                  (Pervasives.(==)
                                                     _visitors_this.c_span
                                                     _visitors_r12)
                                                  (Pervasives.(==)
                                                     _visitors_this.c_doc_comment
                                                     _visitors_r13)))))))))))))
          then _visitors_this
          else
            {
              c_mode = _visitors_r0;
              c_user_attributes = _visitors_r1;
              c_final = _visitors_r2;
              c_kind = _visitors_r3;
              c_is_xhp = _visitors_r4;
              c_name = _visitors_r5;
              c_tparams = _visitors_r6;
              c_extends = _visitors_r7;
              c_implements = _visitors_r8;
              c_body = _visitors_r9;
              c_namespace = _visitors_r10;
              c_enum = _visitors_r11;
              c_span = _visitors_r12;
              c_doc_comment = _visitors_r13
            }
        method on_enum_ env _visitors_this =
          let _visitors_r0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.e_base _visitors_r0)
              (Pervasives.(==) _visitors_this.e_constraint _visitors_r1)
          then _visitors_this
          else { e_base = _visitors_r0; e_constraint = _visitors_r1 }
        method on_user_attribute env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.ua_name  in
          let _visitors_r1 =
            self#on_list self#on_expr env _visitors_this.ua_params  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.ua_name _visitors_r0)
              (Pervasives.(==) _visitors_this.ua_params _visitors_r1)
          then _visitors_this
          else { ua_name = _visitors_r0; ua_params = _visitors_r1 }
        method on_Const env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun ((_visitors_c0,_visitors_c1) as _visitors_this)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(==) _visitors_c1 _visitors_r1)
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1)) env _visitors_c1
             in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Const (_visitors_r0, _visitors_r1)
        method on_AbsConst env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else AbsConst (_visitors_r0, _visitors_r1)
        method on_Attributes env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_class_attr env _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Attributes _visitors_r0
        method on_TypeConst env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_typeconst env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else TypeConst _visitors_r0
        method on_ClassUse env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else ClassUse _visitors_r0
        method on_ClassUseAlias env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_id env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_kind env _visitors_c3  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(==) _visitors_c3 _visitors_r3)))
          then _visitors_this
          else
            ClassUseAlias
              (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_ClassUsePrecedence env _visitors_this _visitors_c0
          _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else ClassUsePrecedence (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_XhpAttrUse env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else XhpAttrUse _visitors_r0
        method on_ClassTraitRequire env _visitors_this _visitors_c0
          _visitors_c1 =
          let _visitors_r0 = self#on_trait_req_kind env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else ClassTraitRequire (_visitors_r0, _visitors_r1)
        method on_ClassVars env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_class_vars_ env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else ClassVars _visitors_r0
        method on_XhpAttr env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_class_var env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in
          let _visitors_r3 =
            self#on_option
              (fun env  ->
                 fun
                   ((_visitors_c0,_visitors_c1,_visitors_c2) as
                      _visitors_this)
                    ->
                   let _visitors_r0 = self#on_t env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   let _visitors_r2 =
                     self#on_list self#on_expr env _visitors_c2  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_c1 _visitors_r1)
                          (Pervasives.(==) _visitors_c2 _visitors_r2))
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_c3
             in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(==) _visitors_c3 _visitors_r3)))
          then _visitors_this
          else
            XhpAttr (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Method env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_method_ env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Method _visitors_r0
        method on_XhpCategory env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_pstring env _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else XhpCategory _visitors_r0
        method on_XhpChild env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else XhpChild _visitors_r0
        method on_class_elt env _visitors_this =
          match _visitors_this with
          | Const (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Const env _visitors_this _visitors_c0 _visitors_c1
          | AbsConst (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_AbsConst env _visitors_this _visitors_c0 _visitors_c1
          | Attributes _visitors_c0 as _visitors_this ->
              self#on_Attributes env _visitors_this _visitors_c0
          | TypeConst _visitors_c0 as _visitors_this ->
              self#on_TypeConst env _visitors_this _visitors_c0
          | ClassUse _visitors_c0 as _visitors_this ->
              self#on_ClassUse env _visitors_this _visitors_c0
          | ClassUseAlias
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
              _visitors_this ->
              self#on_ClassUseAlias env _visitors_this _visitors_c0
                _visitors_c1 _visitors_c2 _visitors_c3
          | ClassUsePrecedence (_visitors_c0,_visitors_c1,_visitors_c2) as
              _visitors_this ->
              self#on_ClassUsePrecedence env _visitors_this _visitors_c0
                _visitors_c1 _visitors_c2
          | XhpAttrUse _visitors_c0 as _visitors_this ->
              self#on_XhpAttrUse env _visitors_this _visitors_c0
          | ClassTraitRequire (_visitors_c0,_visitors_c1) as _visitors_this
              ->
              self#on_ClassTraitRequire env _visitors_this _visitors_c0
                _visitors_c1
          | ClassVars _visitors_c0 as _visitors_this ->
              self#on_ClassVars env _visitors_this _visitors_c0
          | XhpAttr (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
              _visitors_this ->
              self#on_XhpAttr env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | Method _visitors_c0 as _visitors_this ->
              self#on_Method env _visitors_this _visitors_c0
          | XhpCategory _visitors_c0 as _visitors_this ->
              self#on_XhpCategory env _visitors_this _visitors_c0
          | XhpChild _visitors_c0 as _visitors_this ->
              self#on_XhpChild env _visitors_this _visitors_c0
        method on_ChildName env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else ChildName _visitors_r0
        method on_ChildList env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_xhp_child env _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else ChildList _visitors_r0
        method on_ChildUnary env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child_op env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else ChildUnary (_visitors_r0, _visitors_r1)
        method on_ChildBinary env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else ChildBinary (_visitors_r0, _visitors_r1)
        method on_xhp_child env _visitors_this =
          match _visitors_this with
          | ChildName _visitors_c0 as _visitors_this ->
              self#on_ChildName env _visitors_this _visitors_c0
          | ChildList _visitors_c0 as _visitors_this ->
              self#on_ChildList env _visitors_this _visitors_c0
          | ChildUnary (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_ChildUnary env _visitors_this _visitors_c0 _visitors_c1
          | ChildBinary (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_ChildBinary env _visitors_this _visitors_c0
                _visitors_c1
        method on_ChildStar env _visitors_this =
          if true then _visitors_this else ChildStar
        method on_ChildPlus env _visitors_this =
          if true then _visitors_this else ChildPlus
        method on_ChildQuestion env _visitors_this =
          if true then _visitors_this else ChildQuestion
        method on_xhp_child_op env _visitors_this =
          match _visitors_this with
          | ChildStar  as _visitors_this ->
              self#on_ChildStar env _visitors_this
          | ChildPlus  as _visitors_this ->
              self#on_ChildPlus env _visitors_this
          | ChildQuestion  as _visitors_this ->
              self#on_ChildQuestion env _visitors_this
        method on_CA_name env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else CA_name _visitors_r0
        method on_CA_field env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_ca_field env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else CA_field _visitors_r0
        method on_class_attr env _visitors_this =
          match _visitors_this with
          | CA_name _visitors_c0 as _visitors_this ->
              self#on_CA_name env _visitors_this _visitors_c0
          | CA_field _visitors_c0 as _visitors_this ->
              self#on_CA_field env _visitors_this _visitors_c0
        method on_ca_field env _visitors_this =
          let _visitors_r0 = self#on_ca_type env _visitors_this.ca_type  in
          let _visitors_r1 = self#on_id env _visitors_this.ca_id  in
          let _visitors_r2 =
            self#on_option self#on_expr env _visitors_this.ca_value  in
          let _visitors_r3 = self#on_bool env _visitors_this.ca_required  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.ca_type _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.ca_id _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.ca_value _visitors_r2)
                    (Pervasives.(==) _visitors_this.ca_required _visitors_r3)))
          then _visitors_this
          else
            {
              ca_type = _visitors_r0;
              ca_id = _visitors_r1;
              ca_value = _visitors_r2;
              ca_required = _visitors_r3
            }
        method on_CA_hint env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else CA_hint _visitors_r0
        method on_CA_enum env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_string env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else CA_enum _visitors_r0
        method on_ca_type env _visitors_this =
          match _visitors_this with
          | CA_hint _visitors_c0 as _visitors_this ->
              self#on_CA_hint env _visitors_this _visitors_c0
          | CA_enum _visitors_c0 as _visitors_this ->
              self#on_CA_enum env _visitors_this _visitors_c0
        method on_class_var env
          ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_expr env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_class_vars_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_r2 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r3 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.cv_kinds _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.cv_hint _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.cv_names _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.cv_doc_comment
                          _visitors_r3)
                       (Pervasives.(==) _visitors_this.cv_user_attributes
                          _visitors_r4))))
          then _visitors_this
          else
            {
              cv_kinds = _visitors_r0;
              cv_hint = _visitors_r1;
              cv_names = _visitors_r2;
              cv_doc_comment = _visitors_r3;
              cv_user_attributes = _visitors_r4
            }
        method on_method_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.m_kind  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.m_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun
                   ((_visitors_c0,_visitors_c1,_visitors_c2) as
                      _visitors_this)
                    ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_c1 _visitors_r1)
                          (Pervasives.(==) _visitors_c2 _visitors_r2))
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_this.m_constrs
             in
          let _visitors_r3 = self#on_id env _visitors_this.m_name  in
          let _visitors_r4 =
            self#on_list self#on_fun_param env _visitors_this.m_params  in
          let _visitors_r5 = self#on_block env _visitors_this.m_body  in
          let _visitors_r6 =
            self#on_list self#on_user_attribute env
              _visitors_this.m_user_attributes
             in
          let _visitors_r7 =
            self#on_option self#on_hint env _visitors_this.m_ret  in
          let _visitors_r8 = self#on_bool env _visitors_this.m_ret_by_ref  in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.m_fun_kind
             in
          let _visitors_r10 = self#on_t env _visitors_this.m_span  in
          let _visitors_r11 =
            self#on_option self#on_string env _visitors_this.m_doc_comment
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.m_kind _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.m_tparams _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.m_constrs _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.m_name _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.m_params
                             _visitors_r4)
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.m_body
                                _visitors_r5)
                             (Pervasives.(&&)
                                (Pervasives.(==)
                                   _visitors_this.m_user_attributes
                                   _visitors_r6)
                                (Pervasives.(&&)
                                   (Pervasives.(==) _visitors_this.m_ret
                                      _visitors_r7)
                                   (Pervasives.(&&)
                                      (Pervasives.(==)
                                         _visitors_this.m_ret_by_ref
                                         _visitors_r8)
                                      (Pervasives.(&&)
                                         (Pervasives.(==)
                                            _visitors_this.m_fun_kind
                                            _visitors_r9)
                                         (Pervasives.(&&)
                                            (Pervasives.(==)
                                               _visitors_this.m_span
                                               _visitors_r10)
                                            (Pervasives.(==)
                                               _visitors_this.m_doc_comment
                                               _visitors_r11)))))))))))
          then _visitors_this
          else
            {
              m_kind = _visitors_r0;
              m_tparams = _visitors_r1;
              m_constrs = _visitors_r2;
              m_name = _visitors_r3;
              m_params = _visitors_r4;
              m_body = _visitors_r5;
              m_user_attributes = _visitors_r6;
              m_ret = _visitors_r7;
              m_ret_by_ref = _visitors_r8;
              m_fun_kind = _visitors_r9;
              m_span = _visitors_r10;
              m_doc_comment = _visitors_r11
            }
        method on_typeconst env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.tconst_abstract
             in
          let _visitors_r1 = self#on_id env _visitors_this.tconst_name  in
          let _visitors_r2 =
            self#on_list self#on_tparam env _visitors_this.tconst_tparams  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.tconst_constraint
             in
          let _visitors_r4 =
            self#on_option self#on_hint env _visitors_this.tconst_type  in
          let _visitors_r5 = self#on_t env _visitors_this.tconst_span  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.tconst_abstract _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.tconst_name _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.tconst_tparams
                       _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.tconst_constraint
                          _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.tconst_type
                             _visitors_r4)
                          (Pervasives.(==) _visitors_this.tconst_span
                             _visitors_r5)))))
          then _visitors_this
          else
            {
              tconst_abstract = _visitors_r0;
              tconst_name = _visitors_r1;
              tconst_tparams = _visitors_r2;
              tconst_constraint = _visitors_r3;
              tconst_type = _visitors_r4;
              tconst_span = _visitors_r5
            }
        method on_is_reference env = self#on_bool env
        method on_is_variadic env = self#on_bool env
        method on_fun_param env _visitors_this =
          let _visitors_r0 =
            self#on_option self#on_hint env _visitors_this.param_hint  in
          let _visitors_r1 =
            self#on_is_reference env _visitors_this.param_is_reference  in
          let _visitors_r2 =
            self#on_is_variadic env _visitors_this.param_is_variadic  in
          let _visitors_r3 = self#on_id env _visitors_this.param_id  in
          let _visitors_r4 =
            self#on_option self#on_expr env _visitors_this.param_expr  in
          let _visitors_r5 =
            self#on_option self#on_kind env _visitors_this.param_modifier  in
          let _visitors_r6 =
            self#on_option self#on_param_kind env
              _visitors_this.param_callconv
             in
          let _visitors_r7 =
            self#on_list self#on_user_attribute env
              _visitors_this.param_user_attributes
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.param_hint _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.param_is_reference
                    _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.param_is_variadic
                       _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.param_id _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.param_expr
                             _visitors_r4)
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.param_modifier
                                _visitors_r5)
                             (Pervasives.(&&)
                                (Pervasives.(==)
                                   _visitors_this.param_callconv _visitors_r6)
                                (Pervasives.(==)
                                   _visitors_this.param_user_attributes
                                   _visitors_r7)))))))
          then _visitors_this
          else
            {
              param_hint = _visitors_r0;
              param_is_reference = _visitors_r1;
              param_is_variadic = _visitors_r2;
              param_id = _visitors_r3;
              param_expr = _visitors_r4;
              param_modifier = _visitors_r5;
              param_callconv = _visitors_r6;
              param_user_attributes = _visitors_r7
            }
        method on_fun_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.f_mode  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.f_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun
                   ((_visitors_c0,_visitors_c1,_visitors_c2) as
                      _visitors_this)
                    ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_c1 _visitors_r1)
                          (Pervasives.(==) _visitors_c2 _visitors_r2))
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_this.f_constrs
             in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.f_ret  in
          let _visitors_r4 = self#on_bool env _visitors_this.f_ret_by_ref  in
          let _visitors_r5 = self#on_id env _visitors_this.f_name  in
          let _visitors_r6 =
            self#on_list self#on_fun_param env _visitors_this.f_params  in
          let _visitors_r7 = self#on_block env _visitors_this.f_body  in
          let _visitors_r8 =
            self#on_list self#on_user_attribute env
              _visitors_this.f_user_attributes
             in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.f_fun_kind
             in
          let _visitors_r10 = self#on_env env _visitors_this.f_namespace  in
          let _visitors_r11 = self#on_t env _visitors_this.f_span  in
          let _visitors_r12 =
            self#on_option self#on_string env _visitors_this.f_doc_comment
             in
          let _visitors_r13 = self#on_bool env _visitors_this.f_static  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.f_mode _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.f_tparams _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.f_constrs _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.f_ret _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.f_ret_by_ref
                             _visitors_r4)
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.f_name
                                _visitors_r5)
                             (Pervasives.(&&)
                                (Pervasives.(==) _visitors_this.f_params
                                   _visitors_r6)
                                (Pervasives.(&&)
                                   (Pervasives.(==) _visitors_this.f_body
                                      _visitors_r7)
                                   (Pervasives.(&&)
                                      (Pervasives.(==)
                                         _visitors_this.f_user_attributes
                                         _visitors_r8)
                                      (Pervasives.(&&)
                                         (Pervasives.(==)
                                            _visitors_this.f_fun_kind
                                            _visitors_r9)
                                         (Pervasives.(&&)
                                            (Pervasives.(==)
                                               _visitors_this.f_namespace
                                               _visitors_r10)
                                            (Pervasives.(&&)
                                               (Pervasives.(==)
                                                  _visitors_this.f_span
                                                  _visitors_r11)
                                               (Pervasives.(&&)
                                                  (Pervasives.(==)
                                                     _visitors_this.f_doc_comment
                                                     _visitors_r12)
                                                  (Pervasives.(==)
                                                     _visitors_this.f_static
                                                     _visitors_r13)))))))))))))
          then _visitors_this
          else
            {
              f_mode = _visitors_r0;
              f_tparams = _visitors_r1;
              f_constrs = _visitors_r2;
              f_ret = _visitors_r3;
              f_ret_by_ref = _visitors_r4;
              f_name = _visitors_r5;
              f_params = _visitors_r6;
              f_body = _visitors_r7;
              f_user_attributes = _visitors_r8;
              f_fun_kind = _visitors_r9;
              f_namespace = _visitors_r10;
              f_span = _visitors_r11;
              f_doc_comment = _visitors_r12;
              f_static = _visitors_r13
            }
        method on_is_coroutine env = self#on_bool env
        method on_hint env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_hint_ env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_Hvariadic env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hvariadic _visitors_r0
        method on_Hnon_variadic env _visitors_this =
          if true then _visitors_this else Hnon_variadic
        method on_variadic_hint env _visitors_this =
          match _visitors_this with
          | Hvariadic _visitors_c0 as _visitors_this ->
              self#on_Hvariadic env _visitors_this _visitors_c0
          | Hnon_variadic  as _visitors_this ->
              self#on_Hnon_variadic env _visitors_this
        method on_Hoption env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hoption _visitors_r0
        method on_Hfun env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 _visitors_c4 =
          let _visitors_r0 = self#on_is_coroutine env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c2
             in
          let _visitors_r3 = self#on_variadic_hint env _visitors_c3  in
          let _visitors_r4 = self#on_hint env _visitors_c4  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_c3 _visitors_r3)
                       (Pervasives.(==) _visitors_c4 _visitors_r4))))
          then _visitors_this
          else
            Hfun
              (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
                _visitors_r4)
        method on_Htuple env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Htuple _visitors_r0
        method on_Happly env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Happly (_visitors_r0, _visitors_r1)
        method on_Hshape env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_shape_info env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hshape _visitors_r0
        method on_Haccess env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Haccess (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Hsoft env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hsoft _visitors_r0
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 as _visitors_this ->
              self#on_Hoption env _visitors_this _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              as _visitors_this ->
              self#on_Hfun env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3 _visitors_c4
          | Htuple _visitors_c0 as _visitors_this ->
              self#on_Htuple env _visitors_this _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Happly env _visitors_this _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 as _visitors_this ->
              self#on_Hshape env _visitors_this _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1,_visitors_c2) as
              _visitors_this ->
              self#on_Haccess env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Hsoft _visitors_c0 as _visitors_this ->
              self#on_Hsoft env _visitors_this _visitors_c0
        method on_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.si_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_list self#on_shape_field env
              _visitors_this.si_shape_field_list
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.si_allows_unknown_fields
                 _visitors_r0)
              (Pervasives.(==) _visitors_this.si_shape_field_list
                 _visitors_r1)
          then _visitors_this
          else
            {
              si_allows_unknown_fields = _visitors_r0;
              si_shape_field_list = _visitors_r1
            }
        method on_shape_field env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sf_optional  in
          let _visitors_r1 =
            self#on_shape_field_name env _visitors_this.sf_name  in
          let _visitors_r2 = self#on_hint env _visitors_this.sf_hint  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.sf_optional _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.sf_name _visitors_r1)
                 (Pervasives.(==) _visitors_this.sf_hint _visitors_r2))
          then _visitors_this
          else
            {
              sf_optional = _visitors_r0;
              sf_name = _visitors_r1;
              sf_hint = _visitors_r2
            }
        method on_using_stmt env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.us_is_block_scoped  in
          let _visitors_r1 = self#on_bool env _visitors_this.us_has_await  in
          let _visitors_r2 = self#on_expr env _visitors_this.us_expr  in
          let _visitors_r3 = self#on_block env _visitors_this.us_block  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.us_is_block_scoped _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.us_has_await _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.us_expr _visitors_r2)
                    (Pervasives.(==) _visitors_this.us_block _visitors_r3)))
          then _visitors_this
          else
            {
              us_is_block_scoped = _visitors_r0;
              us_has_await = _visitors_r1;
              us_expr = _visitors_r2;
              us_block = _visitors_r3
            }
        method on_Unsafe env _visitors_this =
          if true then _visitors_this else Unsafe
        method on_Fallthrough env _visitors_this =
          if true then _visitors_this else Fallthrough
        method on_Expr env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Expr _visitors_r0
        method on_Block env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Block _visitors_r0
        method on_Break env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Break (_visitors_r0, _visitors_r1)
        method on_Continue env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Continue (_visitors_r0, _visitors_r1)
        method on_Throw env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Throw _visitors_r0
        method on_Return env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Return (_visitors_r0, _visitors_r1)
        method on_GotoLabel env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else GotoLabel _visitors_r0
        method on_Goto env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Goto _visitors_r0
        method on_Static_var env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Static_var _visitors_r0
        method on_Global_var env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Global_var (_visitors_r0, _visitors_r1)
        method on_If env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else If (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Do env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Do (_visitors_r0, _visitors_r1)
        method on_While env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else While (_visitors_r0, _visitors_r1)
        method on_For env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 _visitors_c4 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_expr env _visitors_c3  in
          let _visitors_r4 = self#on_block env _visitors_c4  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_c3 _visitors_r3)
                       (Pervasives.(==) _visitors_c4 _visitors_r4))))
          then _visitors_this
          else
            For
              (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
                _visitors_r4)
        method on_Switch env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_case env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Switch (_visitors_r0, _visitors_r1)
        method on_Foreach env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_t env _visitors_c1  in
          let _visitors_r2 = self#on_as_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(==) _visitors_c3 _visitors_r3)))
          then _visitors_this
          else
            Foreach (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Try env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_catch env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Try (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Def_inline env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_def env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Def_inline _visitors_r0
        method on_Noop env _visitors_this =
          if true then _visitors_this else Noop
        method on_Markup env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Markup (_visitors_r0, _visitors_r1)
        method on_Using env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_using_stmt env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Using _visitors_r0
        method on_Declare env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_bool env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Declare (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_stmt env _visitors_this =
          match _visitors_this with
          | Unsafe  as _visitors_this -> self#on_Unsafe env _visitors_this
          | Fallthrough  as _visitors_this ->
              self#on_Fallthrough env _visitors_this
          | Expr _visitors_c0 as _visitors_this ->
              self#on_Expr env _visitors_this _visitors_c0
          | Block _visitors_c0 as _visitors_this ->
              self#on_Block env _visitors_this _visitors_c0
          | Break (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Break env _visitors_this _visitors_c0 _visitors_c1
          | Continue (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Continue env _visitors_this _visitors_c0 _visitors_c1
          | Throw _visitors_c0 as _visitors_this ->
              self#on_Throw env _visitors_this _visitors_c0
          | Return (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Return env _visitors_this _visitors_c0 _visitors_c1
          | GotoLabel _visitors_c0 as _visitors_this ->
              self#on_GotoLabel env _visitors_this _visitors_c0
          | Goto _visitors_c0 as _visitors_this ->
              self#on_Goto env _visitors_this _visitors_c0
          | Static_var _visitors_c0 as _visitors_this ->
              self#on_Static_var env _visitors_this _visitors_c0
          | Global_var (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Global_var env _visitors_this _visitors_c0 _visitors_c1
          | If (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_If env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Do (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Do env _visitors_this _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_While env _visitors_this _visitors_c0 _visitors_c1
          | For
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              as _visitors_this ->
              self#on_For env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3 _visitors_c4
          | Switch (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Switch env _visitors_this _visitors_c0 _visitors_c1
          | Foreach (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
              _visitors_this ->
              self#on_Foreach env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | Try (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_Try env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Def_inline _visitors_c0 as _visitors_this ->
              self#on_Def_inline env _visitors_this _visitors_c0
          | Noop  as _visitors_this -> self#on_Noop env _visitors_this
          | Markup (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Markup env _visitors_this _visitors_c0 _visitors_c1
          | Using _visitors_c0 as _visitors_this ->
              self#on_Using env _visitors_this _visitors_c0
          | Declare (_visitors_c0,_visitors_c1,_visitors_c2) as
              _visitors_this ->
              self#on_Declare env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
        method on_As_v env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else As_v _visitors_r0
        method on_As_kv env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else As_kv (_visitors_r0, _visitors_r1)
        method on_as_expr env _visitors_this =
          match _visitors_this with
          | As_v _visitors_c0 as _visitors_this ->
              self#on_As_v env _visitors_this _visitors_c0
          | As_kv (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_As_kv env _visitors_this _visitors_c0 _visitors_c1
        method on_Xhp_simple env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Xhp_simple (_visitors_r0, _visitors_r1)
        method on_Xhp_spread env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Xhp_spread _visitors_r0
        method on_xhp_attribute env _visitors_this =
          match _visitors_this with
          | Xhp_simple (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Xhp_simple env _visitors_this _visitors_c0 _visitors_c1
          | Xhp_spread _visitors_c0 as _visitors_this ->
              self#on_Xhp_spread env _visitors_this _visitors_c0
        method on_block env = self#on_list self#on_stmt env
        method on_expr env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr_ env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_Array env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_afield env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Array _visitors_r0
        method on_Varray env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Varray _visitors_r0
        method on_Darray env _visitors_this _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun ((_visitors_c0,_visitors_c1) as _visitors_this)  ->
                   let _visitors_r0 = self#on_expr env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(==) _visitors_c1 _visitors_r1)
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1)) env _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Darray _visitors_r0
        method on_Shape env _visitors_this _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun ((_visitors_c0,_visitors_c1) as _visitors_this)  ->
                   let _visitors_r0 =
                     self#on_shape_field_name env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(==) _visitors_c1 _visitors_r1)
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1)) env _visitors_c0
             in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Shape _visitors_r0
        method on_Collection env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_afield env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Collection (_visitors_r0, _visitors_r1)
        method on_Null env _visitors_this =
          if true then _visitors_this else Null
        method on_True env _visitors_this =
          if true then _visitors_this else True
        method on_False env _visitors_this =
          if true then _visitors_this else False
        method on_Omitted env _visitors_this =
          if true then _visitors_this else Omitted
        method on_Id env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Id _visitors_r0
        method on_Id_type_arguments env _visitors_this _visitors_c0
          _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Id_type_arguments (_visitors_r0, _visitors_r1)
        method on_Lvar env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Lvar _visitors_r0
        method on_Lvarvar env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_int env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Lvarvar (_visitors_r0, _visitors_r1)
        method on_Clone env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Clone _visitors_r0
        method on_Obj_get env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_og_null_flavor env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Obj_get (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Array_get env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Array_get (_visitors_r0, _visitors_r1)
        method on_Class_get env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Class_get (_visitors_r0, _visitors_r1)
        method on_Class_const env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Class_const (_visitors_r0, _visitors_r1)
        method on_Call env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_expr env _visitors_c3  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(==) _visitors_c3 _visitors_r3)))
          then _visitors_this
          else Call (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Int env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Int _visitors_r0
        method on_Float env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Float _visitors_r0
        method on_String env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else String _visitors_r0
        method on_String2 env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else String2 _visitors_r0
        method on_Yield env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_afield env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Yield _visitors_r0
        method on_Yield_break env _visitors_this =
          if true then _visitors_this else Yield_break
        method on_Yield_from env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Yield_from _visitors_r0
        method on_Await env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Await _visitors_r0
        method on_Suspend env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Suspend _visitors_r0
        method on_List env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else List _visitors_r0
        method on_Expr_list env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Expr_list _visitors_r0
        method on_Cast env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Cast (_visitors_r0, _visitors_r1)
        method on_Unop env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_uop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Unop (_visitors_r0, _visitors_r1)
        method on_Binop env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_bop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Binop (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Pipe env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Pipe (_visitors_r0, _visitors_r1)
        method on_Eif env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Eif (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_NullCoalesce env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else NullCoalesce (_visitors_r0, _visitors_r1)
        method on_InstanceOf env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else InstanceOf (_visitors_r0, _visitors_r1)
        method on_Is env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Is (_visitors_r0, _visitors_r1)
        method on_BracedExpr env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else BracedExpr _visitors_r0
        method on_ParenthesizedExpr env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else ParenthesizedExpr _visitors_r0
        method on_New env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else New (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_NewAnonClass env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_class_ env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else NewAnonClass (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Efun env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun ((_visitors_c0,_visitors_c1) as _visitors_this)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   if
                     Pervasives.(&&)
                       (Pervasives.(==) _visitors_c0 _visitors_r0)
                       (Pervasives.(==) _visitors_c1 _visitors_r1)
                   then _visitors_this
                   else (_visitors_r0, _visitors_r1)) env _visitors_c1
             in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Efun (_visitors_r0, _visitors_r1)
        method on_Lfun env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Lfun _visitors_r0
        method on_Xml env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 =
            self#on_list self#on_xhp_attribute env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else Xml (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Unsafeexpr env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Unsafeexpr _visitors_r0
        method on_Import env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_import_flavor env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Import (_visitors_r0, _visitors_r1)
        method on_Callconv env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_param_kind env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Callconv (_visitors_r0, _visitors_r1)
        method on_Execution_operator env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Execution_operator _visitors_r0
        method on_expr_ env _visitors_this =
          match _visitors_this with
          | Array _visitors_c0 as _visitors_this ->
              self#on_Array env _visitors_this _visitors_c0
          | Varray _visitors_c0 as _visitors_this ->
              self#on_Varray env _visitors_this _visitors_c0
          | Darray _visitors_c0 as _visitors_this ->
              self#on_Darray env _visitors_this _visitors_c0
          | Shape _visitors_c0 as _visitors_this ->
              self#on_Shape env _visitors_this _visitors_c0
          | Collection (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Collection env _visitors_this _visitors_c0 _visitors_c1
          | Null  as _visitors_this -> self#on_Null env _visitors_this
          | True  as _visitors_this -> self#on_True env _visitors_this
          | False  as _visitors_this -> self#on_False env _visitors_this
          | Omitted  as _visitors_this -> self#on_Omitted env _visitors_this
          | Id _visitors_c0 as _visitors_this ->
              self#on_Id env _visitors_this _visitors_c0
          | Id_type_arguments (_visitors_c0,_visitors_c1) as _visitors_this
              ->
              self#on_Id_type_arguments env _visitors_this _visitors_c0
                _visitors_c1
          | Lvar _visitors_c0 as _visitors_this ->
              self#on_Lvar env _visitors_this _visitors_c0
          | Lvarvar (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Lvarvar env _visitors_this _visitors_c0 _visitors_c1
          | Clone _visitors_c0 as _visitors_this ->
              self#on_Clone env _visitors_this _visitors_c0
          | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) as
              _visitors_this ->
              self#on_Obj_get env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Array_get (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Array_get env _visitors_this _visitors_c0 _visitors_c1
          | Class_get (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Class_get env _visitors_this _visitors_c0 _visitors_c1
          | Class_const (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Class_const env _visitors_this _visitors_c0
                _visitors_c1
          | Call (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
              _visitors_this ->
              self#on_Call env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | Int _visitors_c0 as _visitors_this ->
              self#on_Int env _visitors_this _visitors_c0
          | Float _visitors_c0 as _visitors_this ->
              self#on_Float env _visitors_this _visitors_c0
          | String _visitors_c0 as _visitors_this ->
              self#on_String env _visitors_this _visitors_c0
          | String2 _visitors_c0 as _visitors_this ->
              self#on_String2 env _visitors_this _visitors_c0
          | Yield _visitors_c0 as _visitors_this ->
              self#on_Yield env _visitors_this _visitors_c0
          | Yield_break  as _visitors_this ->
              self#on_Yield_break env _visitors_this
          | Yield_from _visitors_c0 as _visitors_this ->
              self#on_Yield_from env _visitors_this _visitors_c0
          | Await _visitors_c0 as _visitors_this ->
              self#on_Await env _visitors_this _visitors_c0
          | Suspend _visitors_c0 as _visitors_this ->
              self#on_Suspend env _visitors_this _visitors_c0
          | List _visitors_c0 as _visitors_this ->
              self#on_List env _visitors_this _visitors_c0
          | Expr_list _visitors_c0 as _visitors_this ->
              self#on_Expr_list env _visitors_this _visitors_c0
          | Cast (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Cast env _visitors_this _visitors_c0 _visitors_c1
          | Unop (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Unop env _visitors_this _visitors_c0 _visitors_c1
          | Binop (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this
              ->
              self#on_Binop env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Pipe (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Pipe env _visitors_this _visitors_c0 _visitors_c1
          | Eif (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_Eif env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | NullCoalesce (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_NullCoalesce env _visitors_this _visitors_c0
                _visitors_c1
          | InstanceOf (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_InstanceOf env _visitors_this _visitors_c0 _visitors_c1
          | Is (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Is env _visitors_this _visitors_c0 _visitors_c1
          | BracedExpr _visitors_c0 as _visitors_this ->
              self#on_BracedExpr env _visitors_this _visitors_c0
          | ParenthesizedExpr _visitors_c0 as _visitors_this ->
              self#on_ParenthesizedExpr env _visitors_this _visitors_c0
          | New (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_New env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | NewAnonClass (_visitors_c0,_visitors_c1,_visitors_c2) as
              _visitors_this ->
              self#on_NewAnonClass env _visitors_this _visitors_c0
                _visitors_c1 _visitors_c2
          | Efun (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Efun env _visitors_this _visitors_c0 _visitors_c1
          | Lfun _visitors_c0 as _visitors_this ->
              self#on_Lfun env _visitors_this _visitors_c0
          | Xml (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_Xml env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Unsafeexpr _visitors_c0 as _visitors_this ->
              self#on_Unsafeexpr env _visitors_this _visitors_c0
          | Import (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Import env _visitors_this _visitors_c0 _visitors_c1
          | Callconv (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Callconv env _visitors_this _visitors_c0 _visitors_c1
          | Execution_operator _visitors_c0 as _visitors_this ->
              self#on_Execution_operator env _visitors_this _visitors_c0
        method on_Include env _visitors_this =
          if true then _visitors_this else Include
        method on_Require env _visitors_this =
          if true then _visitors_this else Require
        method on_IncludeOnce env _visitors_this =
          if true then _visitors_this else IncludeOnce
        method on_RequireOnce env _visitors_this =
          if true then _visitors_this else RequireOnce
        method on_import_flavor env _visitors_this =
          match _visitors_this with
          | Include  as _visitors_this -> self#on_Include env _visitors_this
          | Require  as _visitors_this -> self#on_Require env _visitors_this
          | IncludeOnce  as _visitors_this ->
              self#on_IncludeOnce env _visitors_this
          | RequireOnce  as _visitors_this ->
              self#on_RequireOnce env _visitors_this
        method on_AFvalue env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else AFvalue _visitors_r0
        method on_AFkvalue env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else AFkvalue (_visitors_r0, _visitors_r1)
        method on_afield env _visitors_this =
          match _visitors_this with
          | AFvalue _visitors_c0 as _visitors_this ->
              self#on_AFvalue env _visitors_this _visitors_c0
          | AFkvalue (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_AFkvalue env _visitors_this _visitors_c0 _visitors_c1
        method on_Default env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Default _visitors_r0
        method on_Case env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Case (_visitors_r0, _visitors_r1)
        method on_case env _visitors_this =
          match _visitors_this with
          | Default _visitors_c0 as _visitors_this ->
              self#on_Default env _visitors_this _visitors_c0
          | Case (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Case env _visitors_this _visitors_c0 _visitors_c1
        method on_catch env
          ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_field env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_attr env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] reduce =
      object (self : 'self)
        inherit  [_] reduce_base
        method on_program env = self#on_list self#on_def env
        method on_Fun env _visitors_c0 =
          let _visitors_s0 = self#on_fun_ env _visitors_c0  in _visitors_s0
        method on_Class env _visitors_c0 =
          let _visitors_s0 = self#on_class_ env _visitors_c0  in _visitors_s0
        method on_Stmt env _visitors_c0 =
          let _visitors_s0 = self#on_stmt env _visitors_c0  in _visitors_s0
        method on_Typedef env _visitors_c0 =
          let _visitors_s0 = self#on_typedef env _visitors_c0  in
          _visitors_s0
        method on_Constant env _visitors_c0 =
          let _visitors_s0 = self#on_gconst env _visitors_c0  in _visitors_s0
        method on_Namespace env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_program env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_NamespaceUse env _visitors_c0 =
          let _visitors_s0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_s0 = self#on_ns_kind env _visitors_c0  in
                   let _visitors_s1 = self#on_id env _visitors_c1  in
                   let _visitors_s2 = self#on_id env _visitors_c2  in
                   self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) env _visitors_c0
             in
          _visitors_s0
        method on_SetNamespaceEnv env _visitors_c0 =
          let _visitors_s0 = self#on_env env _visitors_c0  in _visitors_s0
        method on_def env _visitors_this =
          match _visitors_this with
          | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
          | Class _visitors_c0 -> self#on_Class env _visitors_c0
          | Stmt _visitors_c0 -> self#on_Stmt env _visitors_c0
          | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
          | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
          | Namespace (_visitors_c0,_visitors_c1) ->
              self#on_Namespace env _visitors_c0 _visitors_c1
          | NamespaceUse _visitors_c0 ->
              self#on_NamespaceUse env _visitors_c0
          | SetNamespaceEnv _visitors_c0 ->
              self#on_SetNamespaceEnv env _visitors_c0
        method on_typedef env _visitors_this =
          let _visitors_s0 = self#on_id env _visitors_this.t_id  in
          let _visitors_s1 =
            self#on_list self#on_tparam env _visitors_this.t_tparams  in
          let _visitors_s2 =
            self#on_tconstraint env _visitors_this.t_constraint  in
          let _visitors_s3 = self#on_typedef_kind env _visitors_this.t_kind
             in
          let _visitors_s4 =
            self#on_list self#on_user_attribute env
              _visitors_this.t_user_attributes
             in
          let _visitors_s5 = self#on_env env _visitors_this.t_namespace  in
          let _visitors_s6 = self#on_mode env _visitors_this.t_mode  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus (self#plus _visitors_s0 _visitors_s1)
                        _visitors_s2) _visitors_s3) _visitors_s4)
               _visitors_s5) _visitors_s6
        method on_gconst env _visitors_this =
          let _visitors_s0 = self#on_mode env _visitors_this.cst_mode  in
          let _visitors_s1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_s2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_s3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_s4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_s5 = self#on_env env _visitors_this.cst_namespace  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) _visitors_s3) _visitors_s4) _visitors_s5
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_s0 = self#on_variance env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          let _visitors_s2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_s0 =
                     self#on_constraint_kind env _visitors_c0  in
                   let _visitors_s1 = self#on_hint env _visitors_c1  in
                   self#plus _visitors_s0 _visitors_s1) env _visitors_c2
             in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_tconstraint env = self#on_option self#on_hint env
        method on_Alias env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_NewType env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_typedef_kind env _visitors_this =
          match _visitors_this with
          | Alias _visitors_c0 -> self#on_Alias env _visitors_c0
          | NewType _visitors_c0 -> self#on_NewType env _visitors_c0
        method on_class_ env _visitors_this =
          let _visitors_s0 = self#on_mode env _visitors_this.c_mode  in
          let _visitors_s1 =
            self#on_list self#on_user_attribute env
              _visitors_this.c_user_attributes
             in
          let _visitors_s2 = self#on_bool env _visitors_this.c_final  in
          let _visitors_s3 = self#on_class_kind env _visitors_this.c_kind  in
          let _visitors_s4 = self#on_bool env _visitors_this.c_is_xhp  in
          let _visitors_s5 = self#on_id env _visitors_this.c_name  in
          let _visitors_s6 =
            self#on_list self#on_tparam env _visitors_this.c_tparams  in
          let _visitors_s7 =
            self#on_list self#on_hint env _visitors_this.c_extends  in
          let _visitors_s8 =
            self#on_list self#on_hint env _visitors_this.c_implements  in
          let _visitors_s9 =
            self#on_list self#on_class_elt env _visitors_this.c_body  in
          let _visitors_s10 = self#on_env env _visitors_this.c_namespace  in
          let _visitors_s11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_s12 = self#on_t env _visitors_this.c_span  in
          let _visitors_s13 =
            self#on_option self#on_string env _visitors_this.c_doc_comment
             in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus
                        (self#plus
                           (self#plus
                              (self#plus
                                 (self#plus
                                    (self#plus
                                       (self#plus
                                          (self#plus
                                             (self#plus _visitors_s0
                                                _visitors_s1) _visitors_s2)
                                          _visitors_s3) _visitors_s4)
                                    _visitors_s5) _visitors_s6) _visitors_s7)
                           _visitors_s8) _visitors_s9) _visitors_s10)
                  _visitors_s11) _visitors_s12) _visitors_s13
        method on_enum_ env _visitors_this =
          let _visitors_s0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_s1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          self#plus _visitors_s0 _visitors_s1
        method on_user_attribute env _visitors_this =
          let _visitors_s0 = self#on_id env _visitors_this.ua_name  in
          let _visitors_s1 =
            self#on_list self#on_expr env _visitors_this.ua_params  in
          self#plus _visitors_s0 _visitors_s1
        method on_Const env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_s1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_s0 = self#on_id env _visitors_c0  in
                   let _visitors_s1 = self#on_expr env _visitors_c1  in
                   self#plus _visitors_s0 _visitors_s1) env _visitors_c1
             in
          self#plus _visitors_s0 _visitors_s1
        method on_AbsConst env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Attributes env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_class_attr env _visitors_c0
             in
          _visitors_s0
        method on_TypeConst env _visitors_c0 =
          let _visitors_s0 = self#on_typeconst env _visitors_c0  in
          _visitors_s0
        method on_ClassUse env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_ClassUseAlias env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_s0 = self#on_option self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_pstring env _visitors_c1  in
          let _visitors_s2 = self#on_option self#on_id env _visitors_c2  in
          let _visitors_s3 = self#on_list self#on_kind env _visitors_c3  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_ClassUsePrecedence env _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_pstring env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_id env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_XhpAttrUse env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_ClassTraitRequire env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_trait_req_kind env _visitors_c0  in
          let _visitors_s1 = self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_ClassVars env _visitors_c0 =
          let _visitors_s0 = self#on_class_vars_ env _visitors_c0  in
          _visitors_s0
        method on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_s0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_class_var env _visitors_c1  in
          let _visitors_s2 = self#on_bool env _visitors_c2  in
          let _visitors_s3 =
            self#on_option
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_s0 = self#on_t env _visitors_c0  in
                   let _visitors_s1 = self#on_bool env _visitors_c1  in
                   let _visitors_s2 =
                     self#on_list self#on_expr env _visitors_c2  in
                   self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) env _visitors_c3
             in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_Method env _visitors_c0 =
          let _visitors_s0 = self#on_method_ env _visitors_c0  in
          _visitors_s0
        method on_XhpCategory env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_pstring env _visitors_c0
             in
          _visitors_s0
        method on_XhpChild env _visitors_c0 =
          let _visitors_s0 = self#on_xhp_child env _visitors_c0  in
          _visitors_s0
        method on_class_elt env _visitors_this =
          match _visitors_this with
          | Const (_visitors_c0,_visitors_c1) ->
              self#on_Const env _visitors_c0 _visitors_c1
          | AbsConst (_visitors_c0,_visitors_c1) ->
              self#on_AbsConst env _visitors_c0 _visitors_c1
          | Attributes _visitors_c0 -> self#on_Attributes env _visitors_c0
          | TypeConst _visitors_c0 -> self#on_TypeConst env _visitors_c0
          | ClassUse _visitors_c0 -> self#on_ClassUse env _visitors_c0
          | ClassUseAlias
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_ClassUseAlias env _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | ClassUsePrecedence (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_ClassUsePrecedence env _visitors_c0 _visitors_c1
                _visitors_c2
          | XhpAttrUse _visitors_c0 -> self#on_XhpAttrUse env _visitors_c0
          | ClassTraitRequire (_visitors_c0,_visitors_c1) ->
              self#on_ClassTraitRequire env _visitors_c0 _visitors_c1
          | ClassVars _visitors_c0 -> self#on_ClassVars env _visitors_c0
          | XhpAttr (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Method _visitors_c0 -> self#on_Method env _visitors_c0
          | XhpCategory _visitors_c0 -> self#on_XhpCategory env _visitors_c0
          | XhpChild _visitors_c0 -> self#on_XhpChild env _visitors_c0
        method on_ChildName env _visitors_c0 =
          let _visitors_s0 = self#on_id env _visitors_c0  in _visitors_s0
        method on_ChildList env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_xhp_child env _visitors_c0
             in
          _visitors_s0
        method on_ChildUnary env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_s1 = self#on_xhp_child_op env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_ChildBinary env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_s1 = self#on_xhp_child env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_xhp_child env _visitors_this =
          match _visitors_this with
          | ChildName _visitors_c0 -> self#on_ChildName env _visitors_c0
          | ChildList _visitors_c0 -> self#on_ChildList env _visitors_c0
          | ChildUnary (_visitors_c0,_visitors_c1) ->
              self#on_ChildUnary env _visitors_c0 _visitors_c1
          | ChildBinary (_visitors_c0,_visitors_c1) ->
              self#on_ChildBinary env _visitors_c0 _visitors_c1
        method on_ChildStar env = self#zero
        method on_ChildPlus env = self#zero
        method on_ChildQuestion env = self#zero
        method on_xhp_child_op env _visitors_this =
          match _visitors_this with
          | ChildStar  -> self#on_ChildStar env
          | ChildPlus  -> self#on_ChildPlus env
          | ChildQuestion  -> self#on_ChildQuestion env
        method on_CA_name env _visitors_c0 =
          let _visitors_s0 = self#on_id env _visitors_c0  in _visitors_s0
        method on_CA_field env _visitors_c0 =
          let _visitors_s0 = self#on_ca_field env _visitors_c0  in
          _visitors_s0
        method on_class_attr env _visitors_this =
          match _visitors_this with
          | CA_name _visitors_c0 -> self#on_CA_name env _visitors_c0
          | CA_field _visitors_c0 -> self#on_CA_field env _visitors_c0
        method on_ca_field env _visitors_this =
          let _visitors_s0 = self#on_ca_type env _visitors_this.ca_type  in
          let _visitors_s1 = self#on_id env _visitors_this.ca_id  in
          let _visitors_s2 =
            self#on_option self#on_expr env _visitors_this.ca_value  in
          let _visitors_s3 = self#on_bool env _visitors_this.ca_required  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_CA_hint env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_CA_enum env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_string env _visitors_c0  in
          _visitors_s0
        method on_ca_type env _visitors_this =
          match _visitors_this with
          | CA_hint _visitors_c0 -> self#on_CA_hint env _visitors_c0
          | CA_enum _visitors_c0 -> self#on_CA_enum env _visitors_c0
        method on_class_var env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          let _visitors_s2 = self#on_option self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_class_vars_ env _visitors_this =
          let _visitors_s0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_s1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_s2 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_s3 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_s4 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          self#plus
            (self#plus
               (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
               _visitors_s3) _visitors_s4
        method on_method_ env _visitors_this =
          let _visitors_s0 =
            self#on_list self#on_kind env _visitors_this.m_kind  in
          let _visitors_s1 =
            self#on_list self#on_tparam env _visitors_this.m_tparams  in
          let _visitors_s2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_s0 = self#on_hint env _visitors_c0  in
                   let _visitors_s1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_s2 = self#on_hint env _visitors_c2  in
                   self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) env _visitors_this.m_constrs
             in
          let _visitors_s3 = self#on_id env _visitors_this.m_name  in
          let _visitors_s4 =
            self#on_list self#on_fun_param env _visitors_this.m_params  in
          let _visitors_s5 = self#on_block env _visitors_this.m_body  in
          let _visitors_s6 =
            self#on_list self#on_user_attribute env
              _visitors_this.m_user_attributes
             in
          let _visitors_s7 =
            self#on_option self#on_hint env _visitors_this.m_ret  in
          let _visitors_s8 = self#on_bool env _visitors_this.m_ret_by_ref  in
          let _visitors_s9 = self#on_fun_kind env _visitors_this.m_fun_kind
             in
          let _visitors_s10 = self#on_t env _visitors_this.m_span  in
          let _visitors_s11 =
            self#on_option self#on_string env _visitors_this.m_doc_comment
             in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus
                        (self#plus
                           (self#plus
                              (self#plus
                                 (self#plus
                                    (self#plus
                                       (self#plus _visitors_s0 _visitors_s1)
                                       _visitors_s2) _visitors_s3)
                                 _visitors_s4) _visitors_s5) _visitors_s6)
                        _visitors_s7) _visitors_s8) _visitors_s9)
               _visitors_s10) _visitors_s11
        method on_typeconst env _visitors_this =
          let _visitors_s0 = self#on_bool env _visitors_this.tconst_abstract
             in
          let _visitors_s1 = self#on_id env _visitors_this.tconst_name  in
          let _visitors_s2 =
            self#on_list self#on_tparam env _visitors_this.tconst_tparams  in
          let _visitors_s3 =
            self#on_option self#on_hint env _visitors_this.tconst_constraint
             in
          let _visitors_s4 =
            self#on_option self#on_hint env _visitors_this.tconst_type  in
          let _visitors_s5 = self#on_t env _visitors_this.tconst_span  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) _visitors_s3) _visitors_s4) _visitors_s5
        method on_is_reference env = self#on_bool env
        method on_is_variadic env = self#on_bool env
        method on_fun_param env _visitors_this =
          let _visitors_s0 =
            self#on_option self#on_hint env _visitors_this.param_hint  in
          let _visitors_s1 =
            self#on_is_reference env _visitors_this.param_is_reference  in
          let _visitors_s2 =
            self#on_is_variadic env _visitors_this.param_is_variadic  in
          let _visitors_s3 = self#on_id env _visitors_this.param_id  in
          let _visitors_s4 =
            self#on_option self#on_expr env _visitors_this.param_expr  in
          let _visitors_s5 =
            self#on_option self#on_kind env _visitors_this.param_modifier  in
          let _visitors_s6 =
            self#on_option self#on_param_kind env
              _visitors_this.param_callconv
             in
          let _visitors_s7 =
            self#on_list self#on_user_attribute env
              _visitors_this.param_user_attributes
             in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus
                        (self#plus (self#plus _visitors_s0 _visitors_s1)
                           _visitors_s2) _visitors_s3) _visitors_s4)
                  _visitors_s5) _visitors_s6) _visitors_s7
        method on_fun_ env _visitors_this =
          let _visitors_s0 = self#on_mode env _visitors_this.f_mode  in
          let _visitors_s1 =
            self#on_list self#on_tparam env _visitors_this.f_tparams  in
          let _visitors_s2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_s0 = self#on_hint env _visitors_c0  in
                   let _visitors_s1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_s2 = self#on_hint env _visitors_c2  in
                   self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) env _visitors_this.f_constrs
             in
          let _visitors_s3 =
            self#on_option self#on_hint env _visitors_this.f_ret  in
          let _visitors_s4 = self#on_bool env _visitors_this.f_ret_by_ref  in
          let _visitors_s5 = self#on_id env _visitors_this.f_name  in
          let _visitors_s6 =
            self#on_list self#on_fun_param env _visitors_this.f_params  in
          let _visitors_s7 = self#on_block env _visitors_this.f_body  in
          let _visitors_s8 =
            self#on_list self#on_user_attribute env
              _visitors_this.f_user_attributes
             in
          let _visitors_s9 = self#on_fun_kind env _visitors_this.f_fun_kind
             in
          let _visitors_s10 = self#on_env env _visitors_this.f_namespace  in
          let _visitors_s11 = self#on_t env _visitors_this.f_span  in
          let _visitors_s12 =
            self#on_option self#on_string env _visitors_this.f_doc_comment
             in
          let _visitors_s13 = self#on_bool env _visitors_this.f_static  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus
                        (self#plus
                           (self#plus
                              (self#plus
                                 (self#plus
                                    (self#plus
                                       (self#plus
                                          (self#plus
                                             (self#plus _visitors_s0
                                                _visitors_s1) _visitors_s2)
                                          _visitors_s3) _visitors_s4)
                                    _visitors_s5) _visitors_s6) _visitors_s7)
                           _visitors_s8) _visitors_s9) _visitors_s10)
                  _visitors_s11) _visitors_s12) _visitors_s13
        method on_is_coroutine env = self#on_bool env
        method on_hint env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_hint_ env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hvariadic env _visitors_c0 =
          let _visitors_s0 = self#on_option self#on_hint env _visitors_c0  in
          _visitors_s0
        method on_Hnon_variadic env = self#zero
        method on_variadic_hint env _visitors_this =
          match _visitors_this with
          | Hvariadic _visitors_c0 -> self#on_Hvariadic env _visitors_c0
          | Hnon_variadic  -> self#on_Hnon_variadic env
        method on_Hoption env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 _visitors_c4 =
          let _visitors_s0 = self#on_is_coroutine env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_s2 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c2
             in
          let _visitors_s3 = self#on_variadic_hint env _visitors_c3  in
          let _visitors_s4 = self#on_hint env _visitors_c4  in
          self#plus
            (self#plus
               (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
               _visitors_s3) _visitors_s4
        method on_Htuple env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_hint env _visitors_c0  in
          _visitors_s0
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hshape env _visitors_c0 =
          let _visitors_s0 = self#on_shape_info env _visitors_c0  in
          _visitors_s0
        method on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_id env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Hsoft env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2
          | Hsoft _visitors_c0 -> self#on_Hsoft env _visitors_c0
        method on_shape_info env _visitors_this =
          let _visitors_s0 =
            self#on_bool env _visitors_this.si_allows_unknown_fields  in
          let _visitors_s1 =
            self#on_list self#on_shape_field env
              _visitors_this.si_shape_field_list
             in
          self#plus _visitors_s0 _visitors_s1
        method on_shape_field env _visitors_this =
          let _visitors_s0 = self#on_bool env _visitors_this.sf_optional  in
          let _visitors_s1 =
            self#on_shape_field_name env _visitors_this.sf_name  in
          let _visitors_s2 = self#on_hint env _visitors_this.sf_hint  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_using_stmt env _visitors_this =
          let _visitors_s0 =
            self#on_bool env _visitors_this.us_is_block_scoped  in
          let _visitors_s1 = self#on_bool env _visitors_this.us_has_await  in
          let _visitors_s2 = self#on_expr env _visitors_this.us_expr  in
          let _visitors_s3 = self#on_block env _visitors_this.us_block  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_Unsafe env = self#zero
        method on_Fallthrough env = self#zero
        method on_Expr env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Block env _visitors_c0 =
          let _visitors_s0 = self#on_block env _visitors_c0  in _visitors_s0
        method on_Break env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Continue env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Throw env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Return env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_GotoLabel env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_Goto env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_Static_var env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Global_var env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_If env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_block env _visitors_c1  in
          let _visitors_s2 = self#on_block env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Do env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_block env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_While env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_block env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_For env _visitors_c0 _visitors_c1 _visitors_c2 _visitors_c3
          _visitors_c4 =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_expr env _visitors_c2  in
          let _visitors_s3 = self#on_expr env _visitors_c3  in
          let _visitors_s4 = self#on_block env _visitors_c4  in
          self#plus
            (self#plus
               (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
               _visitors_s3) _visitors_s4
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_case env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_t env _visitors_c1  in
          let _visitors_s2 = self#on_as_expr env _visitors_c2  in
          let _visitors_s3 = self#on_block env _visitors_c3  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_block env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_catch env _visitors_c1  in
          let _visitors_s2 = self#on_block env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Def_inline env _visitors_c0 =
          let _visitors_s0 = self#on_def env _visitors_c0  in _visitors_s0
        method on_Noop env = self#zero
        method on_Markup env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Using env _visitors_c0 =
          let _visitors_s0 = self#on_using_stmt env _visitors_c0  in
          _visitors_s0
        method on_Declare env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_bool env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_block env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_stmt env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break (_visitors_c0,_visitors_c1) ->
              self#on_Break env _visitors_c0 _visitors_c1
          | Continue (_visitors_c0,_visitors_c1) ->
              self#on_Continue env _visitors_c0 _visitors_c1
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return (_visitors_c0,_visitors_c1) ->
              self#on_Return env _visitors_c0 _visitors_c1
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var (_visitors_c0,_visitors_c1) ->
              self#on_Global_var env _visitors_c0 _visitors_c1
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Switch (_visitors_c0,_visitors_c1) ->
              self#on_Switch env _visitors_c0 _visitors_c1
          | Foreach (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
          | Def_inline _visitors_c0 -> self#on_Def_inline env _visitors_c0
          | Noop  -> self#on_Noop env
          | Markup (_visitors_c0,_visitors_c1) ->
              self#on_Markup env _visitors_c0 _visitors_c1
          | Using _visitors_c0 -> self#on_Using env _visitors_c0
          | Declare (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Declare env _visitors_c0 _visitors_c1 _visitors_c2
        method on_As_v env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_As_kv env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_as_expr env _visitors_this =
          match _visitors_this with
          | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
          | As_kv (_visitors_c0,_visitors_c1) ->
              self#on_As_kv env _visitors_c0 _visitors_c1
        method on_Xhp_simple env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Xhp_spread env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_xhp_attribute env _visitors_this =
          match _visitors_this with
          | Xhp_simple (_visitors_c0,_visitors_c1) ->
              self#on_Xhp_simple env _visitors_c0 _visitors_c1
          | Xhp_spread _visitors_c0 -> self#on_Xhp_spread env _visitors_c0
        method on_block env = self#on_list self#on_stmt env
        method on_expr env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_t env _visitors_c0  in
          let _visitors_s1 = self#on_expr_ env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Array env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_afield env _visitors_c0  in
          _visitors_s0
        method on_Varray env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Darray env _visitors_c0 =
          let _visitors_s0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_s0 = self#on_expr env _visitors_c0  in
                   let _visitors_s1 = self#on_expr env _visitors_c1  in
                   self#plus _visitors_s0 _visitors_s1) env _visitors_c0
             in
          _visitors_s0
        method on_Shape env _visitors_c0 =
          let _visitors_s0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_s0 =
                     self#on_shape_field_name env _visitors_c0  in
                   let _visitors_s1 = self#on_expr env _visitors_c1  in
                   self#plus _visitors_s0 _visitors_s1) env _visitors_c0
             in
          _visitors_s0
        method on_Collection env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_afield env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Null env = self#zero
        method on_True env = self#zero
        method on_False env = self#zero
        method on_Omitted env = self#zero
        method on_Id env _visitors_c0 =
          let _visitors_s0 = self#on_id env _visitors_c0  in _visitors_s0
        method on_Id_type_arguments env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Lvar env _visitors_c0 =
          let _visitors_s0 = self#on_id env _visitors_c0  in _visitors_s0
        method on_Lvarvar env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_int env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Clone env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_og_null_flavor env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Array_get env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Class_get env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Class_const env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_pstring env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_expr env _visitors_c2  in
          let _visitors_s3 = self#on_list self#on_expr env _visitors_c3  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_Int env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_Float env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_String env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_String2 env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Yield env _visitors_c0 =
          let _visitors_s0 = self#on_afield env _visitors_c0  in _visitors_s0
        method on_Yield_break env = self#zero
        method on_Yield_from env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Await env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Suspend env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_List env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Expr_list env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Cast env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Unop env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_uop env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Binop env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_bop env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Pipe env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_NullCoalesce env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_InstanceOf env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Is env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_BracedExpr env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_ParenthesizedExpr env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_class_ env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Efun env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_fun_ env _visitors_c0  in
          let _visitors_s1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_s0 = self#on_id env _visitors_c0  in
                   let _visitors_s1 = self#on_bool env _visitors_c1  in
                   self#plus _visitors_s0 _visitors_s1) env _visitors_c1
             in
          self#plus _visitors_s0 _visitors_s1
        method on_Lfun env _visitors_c0 =
          let _visitors_s0 = self#on_fun_ env _visitors_c0  in _visitors_s0
        method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 =
            self#on_list self#on_xhp_attribute env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_Unsafeexpr env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Import env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_import_flavor env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Callconv env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_param_kind env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Execution_operator env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_expr_ env _visitors_this =
          match _visitors_this with
          | Array _visitors_c0 -> self#on_Array env _visitors_c0
          | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
          | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
          | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
          | Collection (_visitors_c0,_visitors_c1) ->
              self#on_Collection env _visitors_c0 _visitors_c1
          | Null  -> self#on_Null env
          | True  -> self#on_True env
          | False  -> self#on_False env
          | Omitted  -> self#on_Omitted env
          | Id _visitors_c0 -> self#on_Id env _visitors_c0
          | Id_type_arguments (_visitors_c0,_visitors_c1) ->
              self#on_Id_type_arguments env _visitors_c0 _visitors_c1
          | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
          | Lvarvar (_visitors_c0,_visitors_c1) ->
              self#on_Lvarvar env _visitors_c0 _visitors_c1
          | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
          | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
          | Array_get (_visitors_c0,_visitors_c1) ->
              self#on_Array_get env _visitors_c0 _visitors_c1
          | Class_get (_visitors_c0,_visitors_c1) ->
              self#on_Class_get env _visitors_c0 _visitors_c1
          | Class_const (_visitors_c0,_visitors_c1) ->
              self#on_Class_const env _visitors_c0 _visitors_c1
          | Call (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Int _visitors_c0 -> self#on_Int env _visitors_c0
          | Float _visitors_c0 -> self#on_Float env _visitors_c0
          | String _visitors_c0 -> self#on_String env _visitors_c0
          | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
          | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
          | Yield_break  -> self#on_Yield_break env
          | Yield_from _visitors_c0 -> self#on_Yield_from env _visitors_c0
          | Await _visitors_c0 -> self#on_Await env _visitors_c0
          | Suspend _visitors_c0 -> self#on_Suspend env _visitors_c0
          | List _visitors_c0 -> self#on_List env _visitors_c0
          | Expr_list _visitors_c0 -> self#on_Expr_list env _visitors_c0
          | Cast (_visitors_c0,_visitors_c1) ->
              self#on_Cast env _visitors_c0 _visitors_c1
          | Unop (_visitors_c0,_visitors_c1) ->
              self#on_Unop env _visitors_c0 _visitors_c1
          | Binop (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Binop env _visitors_c0 _visitors_c1 _visitors_c2
          | Pipe (_visitors_c0,_visitors_c1) ->
              self#on_Pipe env _visitors_c0 _visitors_c1
          | Eif (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Eif env _visitors_c0 _visitors_c1 _visitors_c2
          | NullCoalesce (_visitors_c0,_visitors_c1) ->
              self#on_NullCoalesce env _visitors_c0 _visitors_c1
          | InstanceOf (_visitors_c0,_visitors_c1) ->
              self#on_InstanceOf env _visitors_c0 _visitors_c1
          | Is (_visitors_c0,_visitors_c1) ->
              self#on_Is env _visitors_c0 _visitors_c1
          | BracedExpr _visitors_c0 -> self#on_BracedExpr env _visitors_c0
          | ParenthesizedExpr _visitors_c0 ->
              self#on_ParenthesizedExpr env _visitors_c0
          | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
          | NewAnonClass (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2
          | Efun (_visitors_c0,_visitors_c1) ->
              self#on_Efun env _visitors_c0 _visitors_c1
          | Lfun _visitors_c0 -> self#on_Lfun env _visitors_c0
          | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
          | Unsafeexpr _visitors_c0 -> self#on_Unsafeexpr env _visitors_c0
          | Import (_visitors_c0,_visitors_c1) ->
              self#on_Import env _visitors_c0 _visitors_c1
          | Callconv (_visitors_c0,_visitors_c1) ->
              self#on_Callconv env _visitors_c0 _visitors_c1
          | Execution_operator _visitors_c0 ->
              self#on_Execution_operator env _visitors_c0
        method on_Include env = self#zero
        method on_Require env = self#zero
        method on_IncludeOnce env = self#zero
        method on_RequireOnce env = self#zero
        method on_import_flavor env _visitors_this =
          match _visitors_this with
          | Include  -> self#on_Include env
          | Require  -> self#on_Require env
          | IncludeOnce  -> self#on_IncludeOnce env
          | RequireOnce  -> self#on_RequireOnce env
        method on_AFvalue env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_AFkvalue env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_afield env _visitors_this =
          match _visitors_this with
          | AFvalue _visitors_c0 -> self#on_AFvalue env _visitors_c0
          | AFkvalue (_visitors_c0,_visitors_c1) ->
              self#on_AFkvalue env _visitors_c0 _visitors_c1
        method on_Default env _visitors_c0 =
          let _visitors_s0 = self#on_block env _visitors_c0  in _visitors_s0
        method on_Case env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_block env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_case env _visitors_this =
          match _visitors_this with
          | Default _visitors_c0 -> self#on_Default env _visitors_c0
          | Case (_visitors_c0,_visitors_c1) ->
              self#on_Case env _visitors_c0 _visitors_c1
        method on_catch env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          let _visitors_s2 = self#on_block env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_field env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_attr env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] map =
      object (self : 'self)
        inherit  [_] map_base
        method on_program env = self#on_list self#on_def env
        method on_Fun env _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          Fun _visitors_r0
        method on_Class env _visitors_c0 =
          let _visitors_r0 = self#on_class_ env _visitors_c0  in
          Class _visitors_r0
        method on_Stmt env _visitors_c0 =
          let _visitors_r0 = self#on_stmt env _visitors_c0  in
          Stmt _visitors_r0
        method on_Typedef env _visitors_c0 =
          let _visitors_r0 = self#on_typedef env _visitors_c0  in
          Typedef _visitors_r0
        method on_Constant env _visitors_c0 =
          let _visitors_r0 = self#on_gconst env _visitors_c0  in
          Constant _visitors_r0
        method on_Namespace env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_program env _visitors_c1  in
          Namespace (_visitors_r0, _visitors_r1)
        method on_NamespaceUse env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_ns_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_id env _visitors_c1  in
                   let _visitors_r2 = self#on_id env _visitors_c2  in
                   (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_c0
             in
          NamespaceUse _visitors_r0
        method on_SetNamespaceEnv env _visitors_c0 =
          let _visitors_r0 = self#on_env env _visitors_c0  in
          SetNamespaceEnv _visitors_r0
        method on_def env _visitors_this =
          match _visitors_this with
          | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
          | Class _visitors_c0 -> self#on_Class env _visitors_c0
          | Stmt _visitors_c0 -> self#on_Stmt env _visitors_c0
          | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
          | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
          | Namespace (_visitors_c0,_visitors_c1) ->
              self#on_Namespace env _visitors_c0 _visitors_c1
          | NamespaceUse _visitors_c0 ->
              self#on_NamespaceUse env _visitors_c0
          | SetNamespaceEnv _visitors_c0 ->
              self#on_SetNamespaceEnv env _visitors_c0
        method on_typedef env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.t_id  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.t_tparams  in
          let _visitors_r2 =
            self#on_tconstraint env _visitors_this.t_constraint  in
          let _visitors_r3 = self#on_typedef_kind env _visitors_this.t_kind
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.t_user_attributes
             in
          let _visitors_r5 = self#on_env env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_mode env _visitors_this.t_mode  in
          {
            t_id = _visitors_r0;
            t_tparams = _visitors_r1;
            t_constraint = _visitors_r2;
            t_kind = _visitors_r3;
            t_user_attributes = _visitors_r4;
            t_namespace = _visitors_r5;
            t_mode = _visitors_r6
          }
        method on_gconst env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_env env _visitors_this.cst_namespace  in
          {
            cst_mode = _visitors_r0;
            cst_kind = _visitors_r1;
            cst_name = _visitors_r2;
            cst_type = _visitors_r3;
            cst_value = _visitors_r4;
            cst_namespace = _visitors_r5
          }
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 =
                     self#on_constraint_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_hint env _visitors_c1  in
                   (_visitors_r0, _visitors_r1)) env _visitors_c2
             in
          (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_tconstraint env = self#on_option self#on_hint env
        method on_Alias env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          Alias _visitors_r0
        method on_NewType env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          NewType _visitors_r0
        method on_typedef_kind env _visitors_this =
          match _visitors_this with
          | Alias _visitors_c0 -> self#on_Alias env _visitors_c0
          | NewType _visitors_c0 -> self#on_NewType env _visitors_c0
        method on_class_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.c_mode  in
          let _visitors_r1 =
            self#on_list self#on_user_attribute env
              _visitors_this.c_user_attributes
             in
          let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
          let _visitors_r3 = self#on_class_kind env _visitors_this.c_kind  in
          let _visitors_r4 = self#on_bool env _visitors_this.c_is_xhp  in
          let _visitors_r5 = self#on_id env _visitors_this.c_name  in
          let _visitors_r6 =
            self#on_list self#on_tparam env _visitors_this.c_tparams  in
          let _visitors_r7 =
            self#on_list self#on_hint env _visitors_this.c_extends  in
          let _visitors_r8 =
            self#on_list self#on_hint env _visitors_this.c_implements  in
          let _visitors_r9 =
            self#on_list self#on_class_elt env _visitors_this.c_body  in
          let _visitors_r10 = self#on_env env _visitors_this.c_namespace  in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_t env _visitors_this.c_span  in
          let _visitors_r13 =
            self#on_option self#on_string env _visitors_this.c_doc_comment
             in
          {
            c_mode = _visitors_r0;
            c_user_attributes = _visitors_r1;
            c_final = _visitors_r2;
            c_kind = _visitors_r3;
            c_is_xhp = _visitors_r4;
            c_name = _visitors_r5;
            c_tparams = _visitors_r6;
            c_extends = _visitors_r7;
            c_implements = _visitors_r8;
            c_body = _visitors_r9;
            c_namespace = _visitors_r10;
            c_enum = _visitors_r11;
            c_span = _visitors_r12;
            c_doc_comment = _visitors_r13
          }
        method on_enum_ env _visitors_this =
          let _visitors_r0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          { e_base = _visitors_r0; e_constraint = _visitors_r1 }
        method on_user_attribute env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.ua_name  in
          let _visitors_r1 =
            self#on_list self#on_expr env _visitors_this.ua_params  in
          { ua_name = _visitors_r0; ua_params = _visitors_r1 }
        method on_Const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   (_visitors_r0, _visitors_r1)) env _visitors_c1
             in
          Const (_visitors_r0, _visitors_r1)
        method on_AbsConst env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          AbsConst (_visitors_r0, _visitors_r1)
        method on_Attributes env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_class_attr env _visitors_c0
             in
          Attributes _visitors_r0
        method on_TypeConst env _visitors_c0 =
          let _visitors_r0 = self#on_typeconst env _visitors_c0  in
          TypeConst _visitors_r0
        method on_ClassUse env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          ClassUse _visitors_r0
        method on_ClassUseAlias env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_id env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_kind env _visitors_c3  in
          ClassUseAlias
            (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_ClassUsePrecedence env _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in
          ClassUsePrecedence (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_XhpAttrUse env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          XhpAttrUse _visitors_r0
        method on_ClassTraitRequire env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_trait_req_kind env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          ClassTraitRequire (_visitors_r0, _visitors_r1)
        method on_ClassVars env _visitors_c0 =
          let _visitors_r0 = self#on_class_vars_ env _visitors_c0  in
          ClassVars _visitors_r0
        method on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_class_var env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in
          let _visitors_r3 =
            self#on_option
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_t env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   let _visitors_r2 =
                     self#on_list self#on_expr env _visitors_c2  in
                   (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_c3
             in
          XhpAttr (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Method env _visitors_c0 =
          let _visitors_r0 = self#on_method_ env _visitors_c0  in
          Method _visitors_r0
        method on_XhpCategory env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_pstring env _visitors_c0
             in
          XhpCategory _visitors_r0
        method on_XhpChild env _visitors_c0 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          XhpChild _visitors_r0
        method on_class_elt env _visitors_this =
          match _visitors_this with
          | Const (_visitors_c0,_visitors_c1) ->
              self#on_Const env _visitors_c0 _visitors_c1
          | AbsConst (_visitors_c0,_visitors_c1) ->
              self#on_AbsConst env _visitors_c0 _visitors_c1
          | Attributes _visitors_c0 -> self#on_Attributes env _visitors_c0
          | TypeConst _visitors_c0 -> self#on_TypeConst env _visitors_c0
          | ClassUse _visitors_c0 -> self#on_ClassUse env _visitors_c0
          | ClassUseAlias
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_ClassUseAlias env _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | ClassUsePrecedence (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_ClassUsePrecedence env _visitors_c0 _visitors_c1
                _visitors_c2
          | XhpAttrUse _visitors_c0 -> self#on_XhpAttrUse env _visitors_c0
          | ClassTraitRequire (_visitors_c0,_visitors_c1) ->
              self#on_ClassTraitRequire env _visitors_c0 _visitors_c1
          | ClassVars _visitors_c0 -> self#on_ClassVars env _visitors_c0
          | XhpAttr (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Method _visitors_c0 -> self#on_Method env _visitors_c0
          | XhpCategory _visitors_c0 -> self#on_XhpCategory env _visitors_c0
          | XhpChild _visitors_c0 -> self#on_XhpChild env _visitors_c0
        method on_ChildName env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          ChildName _visitors_r0
        method on_ChildList env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_xhp_child env _visitors_c0
             in
          ChildList _visitors_r0
        method on_ChildUnary env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child_op env _visitors_c1  in
          ChildUnary (_visitors_r0, _visitors_r1)
        method on_ChildBinary env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in
          ChildBinary (_visitors_r0, _visitors_r1)
        method on_xhp_child env _visitors_this =
          match _visitors_this with
          | ChildName _visitors_c0 -> self#on_ChildName env _visitors_c0
          | ChildList _visitors_c0 -> self#on_ChildList env _visitors_c0
          | ChildUnary (_visitors_c0,_visitors_c1) ->
              self#on_ChildUnary env _visitors_c0 _visitors_c1
          | ChildBinary (_visitors_c0,_visitors_c1) ->
              self#on_ChildBinary env _visitors_c0 _visitors_c1
        method on_ChildStar env = ChildStar
        method on_ChildPlus env = ChildPlus
        method on_ChildQuestion env = ChildQuestion
        method on_xhp_child_op env _visitors_this =
          match _visitors_this with
          | ChildStar  -> self#on_ChildStar env
          | ChildPlus  -> self#on_ChildPlus env
          | ChildQuestion  -> self#on_ChildQuestion env
        method on_CA_name env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          CA_name _visitors_r0
        method on_CA_field env _visitors_c0 =
          let _visitors_r0 = self#on_ca_field env _visitors_c0  in
          CA_field _visitors_r0
        method on_class_attr env _visitors_this =
          match _visitors_this with
          | CA_name _visitors_c0 -> self#on_CA_name env _visitors_c0
          | CA_field _visitors_c0 -> self#on_CA_field env _visitors_c0
        method on_ca_field env _visitors_this =
          let _visitors_r0 = self#on_ca_type env _visitors_this.ca_type  in
          let _visitors_r1 = self#on_id env _visitors_this.ca_id  in
          let _visitors_r2 =
            self#on_option self#on_expr env _visitors_this.ca_value  in
          let _visitors_r3 = self#on_bool env _visitors_this.ca_required  in
          {
            ca_type = _visitors_r0;
            ca_id = _visitors_r1;
            ca_value = _visitors_r2;
            ca_required = _visitors_r3
          }
        method on_CA_hint env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          CA_hint _visitors_r0
        method on_CA_enum env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_string env _visitors_c0  in
          CA_enum _visitors_r0
        method on_ca_type env _visitors_this =
          match _visitors_this with
          | CA_hint _visitors_c0 -> self#on_CA_hint env _visitors_c0
          | CA_enum _visitors_c0 -> self#on_CA_enum env _visitors_c0
        method on_class_var env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_expr env _visitors_c2  in
          (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_class_vars_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_r2 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r3 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          {
            cv_kinds = _visitors_r0;
            cv_hint = _visitors_r1;
            cv_names = _visitors_r2;
            cv_doc_comment = _visitors_r3;
            cv_user_attributes = _visitors_r4
          }
        method on_method_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.m_kind  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.m_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in
                   (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_this.m_constrs
             in
          let _visitors_r3 = self#on_id env _visitors_this.m_name  in
          let _visitors_r4 =
            self#on_list self#on_fun_param env _visitors_this.m_params  in
          let _visitors_r5 = self#on_block env _visitors_this.m_body  in
          let _visitors_r6 =
            self#on_list self#on_user_attribute env
              _visitors_this.m_user_attributes
             in
          let _visitors_r7 =
            self#on_option self#on_hint env _visitors_this.m_ret  in
          let _visitors_r8 = self#on_bool env _visitors_this.m_ret_by_ref  in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.m_fun_kind
             in
          let _visitors_r10 = self#on_t env _visitors_this.m_span  in
          let _visitors_r11 =
            self#on_option self#on_string env _visitors_this.m_doc_comment
             in
          {
            m_kind = _visitors_r0;
            m_tparams = _visitors_r1;
            m_constrs = _visitors_r2;
            m_name = _visitors_r3;
            m_params = _visitors_r4;
            m_body = _visitors_r5;
            m_user_attributes = _visitors_r6;
            m_ret = _visitors_r7;
            m_ret_by_ref = _visitors_r8;
            m_fun_kind = _visitors_r9;
            m_span = _visitors_r10;
            m_doc_comment = _visitors_r11
          }
        method on_typeconst env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.tconst_abstract
             in
          let _visitors_r1 = self#on_id env _visitors_this.tconst_name  in
          let _visitors_r2 =
            self#on_list self#on_tparam env _visitors_this.tconst_tparams  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.tconst_constraint
             in
          let _visitors_r4 =
            self#on_option self#on_hint env _visitors_this.tconst_type  in
          let _visitors_r5 = self#on_t env _visitors_this.tconst_span  in
          {
            tconst_abstract = _visitors_r0;
            tconst_name = _visitors_r1;
            tconst_tparams = _visitors_r2;
            tconst_constraint = _visitors_r3;
            tconst_type = _visitors_r4;
            tconst_span = _visitors_r5
          }
        method on_is_reference env = self#on_bool env
        method on_is_variadic env = self#on_bool env
        method on_fun_param env _visitors_this =
          let _visitors_r0 =
            self#on_option self#on_hint env _visitors_this.param_hint  in
          let _visitors_r1 =
            self#on_is_reference env _visitors_this.param_is_reference  in
          let _visitors_r2 =
            self#on_is_variadic env _visitors_this.param_is_variadic  in
          let _visitors_r3 = self#on_id env _visitors_this.param_id  in
          let _visitors_r4 =
            self#on_option self#on_expr env _visitors_this.param_expr  in
          let _visitors_r5 =
            self#on_option self#on_kind env _visitors_this.param_modifier  in
          let _visitors_r6 =
            self#on_option self#on_param_kind env
              _visitors_this.param_callconv
             in
          let _visitors_r7 =
            self#on_list self#on_user_attribute env
              _visitors_this.param_user_attributes
             in
          {
            param_hint = _visitors_r0;
            param_is_reference = _visitors_r1;
            param_is_variadic = _visitors_r2;
            param_id = _visitors_r3;
            param_expr = _visitors_r4;
            param_modifier = _visitors_r5;
            param_callconv = _visitors_r6;
            param_user_attributes = _visitors_r7
          }
        method on_fun_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.f_mode  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.f_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in
                   (_visitors_r0, _visitors_r1, _visitors_r2)) env
              _visitors_this.f_constrs
             in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.f_ret  in
          let _visitors_r4 = self#on_bool env _visitors_this.f_ret_by_ref  in
          let _visitors_r5 = self#on_id env _visitors_this.f_name  in
          let _visitors_r6 =
            self#on_list self#on_fun_param env _visitors_this.f_params  in
          let _visitors_r7 = self#on_block env _visitors_this.f_body  in
          let _visitors_r8 =
            self#on_list self#on_user_attribute env
              _visitors_this.f_user_attributes
             in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.f_fun_kind
             in
          let _visitors_r10 = self#on_env env _visitors_this.f_namespace  in
          let _visitors_r11 = self#on_t env _visitors_this.f_span  in
          let _visitors_r12 =
            self#on_option self#on_string env _visitors_this.f_doc_comment
             in
          let _visitors_r13 = self#on_bool env _visitors_this.f_static  in
          {
            f_mode = _visitors_r0;
            f_tparams = _visitors_r1;
            f_constrs = _visitors_r2;
            f_ret = _visitors_r3;
            f_ret_by_ref = _visitors_r4;
            f_name = _visitors_r5;
            f_params = _visitors_r6;
            f_body = _visitors_r7;
            f_user_attributes = _visitors_r8;
            f_fun_kind = _visitors_r9;
            f_namespace = _visitors_r10;
            f_span = _visitors_r11;
            f_doc_comment = _visitors_r12;
            f_static = _visitors_r13
          }
        method on_is_coroutine env = self#on_bool env
        method on_hint env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_hint_ env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_Hvariadic env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          Hvariadic _visitors_r0
        method on_Hnon_variadic env = Hnon_variadic
        method on_variadic_hint env _visitors_this =
          match _visitors_this with
          | Hvariadic _visitors_c0 -> self#on_Hvariadic env _visitors_c0
          | Hnon_variadic  -> self#on_Hnon_variadic env
        method on_Hoption env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          Hoption _visitors_r0
        method on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 _visitors_c4 =
          let _visitors_r0 = self#on_is_coroutine env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c2
             in
          let _visitors_r3 = self#on_variadic_hint env _visitors_c3  in
          let _visitors_r4 = self#on_hint env _visitors_c4  in
          Hfun
            (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
              _visitors_r4)
        method on_Htuple env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          Htuple _visitors_r0
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          Happly (_visitors_r0, _visitors_r1)
        method on_Hshape env _visitors_c0 =
          let _visitors_r0 = self#on_shape_info env _visitors_c0  in
          Hshape _visitors_r0
        method on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in
          Haccess (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Hsoft env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          Hsoft _visitors_r0
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2
          | Hsoft _visitors_c0 -> self#on_Hsoft env _visitors_c0
        method on_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.si_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_list self#on_shape_field env
              _visitors_this.si_shape_field_list
             in
          {
            si_allows_unknown_fields = _visitors_r0;
            si_shape_field_list = _visitors_r1
          }
        method on_shape_field env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sf_optional  in
          let _visitors_r1 =
            self#on_shape_field_name env _visitors_this.sf_name  in
          let _visitors_r2 = self#on_hint env _visitors_this.sf_hint  in
          {
            sf_optional = _visitors_r0;
            sf_name = _visitors_r1;
            sf_hint = _visitors_r2
          }
        method on_using_stmt env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.us_is_block_scoped  in
          let _visitors_r1 = self#on_bool env _visitors_this.us_has_await  in
          let _visitors_r2 = self#on_expr env _visitors_this.us_expr  in
          let _visitors_r3 = self#on_block env _visitors_this.us_block  in
          {
            us_is_block_scoped = _visitors_r0;
            us_has_await = _visitors_r1;
            us_expr = _visitors_r2;
            us_block = _visitors_r3
          }
        method on_Unsafe env = Unsafe
        method on_Fallthrough env = Fallthrough
        method on_Expr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Expr _visitors_r0
        method on_Block env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          Block _visitors_r0
        method on_Break env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          Break (_visitors_r0, _visitors_r1)
        method on_Continue env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          Continue (_visitors_r0, _visitors_r1)
        method on_Throw env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Throw _visitors_r0
        method on_Return env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          Return (_visitors_r0, _visitors_r1)
        method on_GotoLabel env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          GotoLabel _visitors_r0
        method on_Goto env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          Goto _visitors_r0
        method on_Static_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Static_var _visitors_r0
        method on_Global_var env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          Global_var (_visitors_r0, _visitors_r1)
        method on_If env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          If (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Do env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Do (_visitors_r0, _visitors_r1)
        method on_While env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          While (_visitors_r0, _visitors_r1)
        method on_For env _visitors_c0 _visitors_c1 _visitors_c2 _visitors_c3
          _visitors_c4 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_expr env _visitors_c3  in
          let _visitors_r4 = self#on_block env _visitors_c4  in
          For
            (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
              _visitors_r4)
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_case env _visitors_c1  in
          Switch (_visitors_r0, _visitors_r1)
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_t env _visitors_c1  in
          let _visitors_r2 = self#on_as_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in
          Foreach (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_catch env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          Try (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Def_inline env _visitors_c0 =
          let _visitors_r0 = self#on_def env _visitors_c0  in
          Def_inline _visitors_r0
        method on_Noop env = Noop
        method on_Markup env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          Markup (_visitors_r0, _visitors_r1)
        method on_Using env _visitors_c0 =
          let _visitors_r0 = self#on_using_stmt env _visitors_c0  in
          Using _visitors_r0
        method on_Declare env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_bool env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          Declare (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_stmt env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break (_visitors_c0,_visitors_c1) ->
              self#on_Break env _visitors_c0 _visitors_c1
          | Continue (_visitors_c0,_visitors_c1) ->
              self#on_Continue env _visitors_c0 _visitors_c1
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return (_visitors_c0,_visitors_c1) ->
              self#on_Return env _visitors_c0 _visitors_c1
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var (_visitors_c0,_visitors_c1) ->
              self#on_Global_var env _visitors_c0 _visitors_c1
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Switch (_visitors_c0,_visitors_c1) ->
              self#on_Switch env _visitors_c0 _visitors_c1
          | Foreach (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
          | Def_inline _visitors_c0 -> self#on_Def_inline env _visitors_c0
          | Noop  -> self#on_Noop env
          | Markup (_visitors_c0,_visitors_c1) ->
              self#on_Markup env _visitors_c0 _visitors_c1
          | Using _visitors_c0 -> self#on_Using env _visitors_c0
          | Declare (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Declare env _visitors_c0 _visitors_c1 _visitors_c2
        method on_As_v env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          As_v _visitors_r0
        method on_As_kv env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          As_kv (_visitors_r0, _visitors_r1)
        method on_as_expr env _visitors_this =
          match _visitors_this with
          | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
          | As_kv (_visitors_c0,_visitors_c1) ->
              self#on_As_kv env _visitors_c0 _visitors_c1
        method on_Xhp_simple env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Xhp_simple (_visitors_r0, _visitors_r1)
        method on_Xhp_spread env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Xhp_spread _visitors_r0
        method on_xhp_attribute env _visitors_this =
          match _visitors_this with
          | Xhp_simple (_visitors_c0,_visitors_c1) ->
              self#on_Xhp_simple env _visitors_c0 _visitors_c1
          | Xhp_spread _visitors_c0 -> self#on_Xhp_spread env _visitors_c0
        method on_block env = self#on_list self#on_stmt env
        method on_expr env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr_ env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_Array env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_afield env _visitors_c0  in
          Array _visitors_r0
        method on_Varray env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Varray _visitors_r0
        method on_Darray env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_expr env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   (_visitors_r0, _visitors_r1)) env _visitors_c0
             in
          Darray _visitors_r0
        method on_Shape env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 =
                     self#on_shape_field_name env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in
                   (_visitors_r0, _visitors_r1)) env _visitors_c0
             in
          Shape _visitors_r0
        method on_Collection env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_afield env _visitors_c1  in
          Collection (_visitors_r0, _visitors_r1)
        method on_Null env = Null
        method on_True env = True
        method on_False env = False
        method on_Omitted env = Omitted
        method on_Id env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in Id _visitors_r0
        method on_Id_type_arguments env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          Id_type_arguments (_visitors_r0, _visitors_r1)
        method on_Lvar env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          Lvar _visitors_r0
        method on_Lvarvar env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_int env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          Lvarvar (_visitors_r0, _visitors_r1)
        method on_Clone env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Clone _visitors_r0
        method on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_og_null_flavor env _visitors_c2  in
          Obj_get (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Array_get env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          Array_get (_visitors_r0, _visitors_r1)
        method on_Class_get env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Class_get (_visitors_r0, _visitors_r1)
        method on_Class_const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          Class_const (_visitors_r0, _visitors_r1)
        method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_expr env _visitors_c3  in
          Call (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Int env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          Int _visitors_r0
        method on_Float env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          Float _visitors_r0
        method on_String env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          String _visitors_r0
        method on_String2 env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          String2 _visitors_r0
        method on_Yield env _visitors_c0 =
          let _visitors_r0 = self#on_afield env _visitors_c0  in
          Yield _visitors_r0
        method on_Yield_break env = Yield_break
        method on_Yield_from env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Yield_from _visitors_r0
        method on_Await env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Await _visitors_r0
        method on_Suspend env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Suspend _visitors_r0
        method on_List env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          List _visitors_r0
        method on_Expr_list env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Expr_list _visitors_r0
        method on_Cast env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Cast (_visitors_r0, _visitors_r1)
        method on_Unop env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_uop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Unop (_visitors_r0, _visitors_r1)
        method on_Binop env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_bop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          Binop (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Pipe env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Pipe (_visitors_r0, _visitors_r1)
        method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          Eif (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_NullCoalesce env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          NullCoalesce (_visitors_r0, _visitors_r1)
        method on_InstanceOf env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          InstanceOf (_visitors_r0, _visitors_r1)
        method on_Is env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          Is (_visitors_r0, _visitors_r1)
        method on_BracedExpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          BracedExpr _visitors_r0
        method on_ParenthesizedExpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          ParenthesizedExpr _visitors_r0
        method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          New (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_class_ env _visitors_c2  in
          NewAnonClass (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Efun env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   (_visitors_r0, _visitors_r1)) env _visitors_c1
             in
          Efun (_visitors_r0, _visitors_r1)
        method on_Lfun env _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          Lfun _visitors_r0
        method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 =
            self#on_list self#on_xhp_attribute env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          Xml (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_Unsafeexpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Unsafeexpr _visitors_r0
        method on_Import env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_import_flavor env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Import (_visitors_r0, _visitors_r1)
        method on_Callconv env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_param_kind env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          Callconv (_visitors_r0, _visitors_r1)
        method on_Execution_operator env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Execution_operator _visitors_r0
        method on_expr_ env _visitors_this =
          match _visitors_this with
          | Array _visitors_c0 -> self#on_Array env _visitors_c0
          | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
          | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
          | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
          | Collection (_visitors_c0,_visitors_c1) ->
              self#on_Collection env _visitors_c0 _visitors_c1
          | Null  -> self#on_Null env
          | True  -> self#on_True env
          | False  -> self#on_False env
          | Omitted  -> self#on_Omitted env
          | Id _visitors_c0 -> self#on_Id env _visitors_c0
          | Id_type_arguments (_visitors_c0,_visitors_c1) ->
              self#on_Id_type_arguments env _visitors_c0 _visitors_c1
          | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
          | Lvarvar (_visitors_c0,_visitors_c1) ->
              self#on_Lvarvar env _visitors_c0 _visitors_c1
          | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
          | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
          | Array_get (_visitors_c0,_visitors_c1) ->
              self#on_Array_get env _visitors_c0 _visitors_c1
          | Class_get (_visitors_c0,_visitors_c1) ->
              self#on_Class_get env _visitors_c0 _visitors_c1
          | Class_const (_visitors_c0,_visitors_c1) ->
              self#on_Class_const env _visitors_c0 _visitors_c1
          | Call (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Int _visitors_c0 -> self#on_Int env _visitors_c0
          | Float _visitors_c0 -> self#on_Float env _visitors_c0
          | String _visitors_c0 -> self#on_String env _visitors_c0
          | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
          | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
          | Yield_break  -> self#on_Yield_break env
          | Yield_from _visitors_c0 -> self#on_Yield_from env _visitors_c0
          | Await _visitors_c0 -> self#on_Await env _visitors_c0
          | Suspend _visitors_c0 -> self#on_Suspend env _visitors_c0
          | List _visitors_c0 -> self#on_List env _visitors_c0
          | Expr_list _visitors_c0 -> self#on_Expr_list env _visitors_c0
          | Cast (_visitors_c0,_visitors_c1) ->
              self#on_Cast env _visitors_c0 _visitors_c1
          | Unop (_visitors_c0,_visitors_c1) ->
              self#on_Unop env _visitors_c0 _visitors_c1
          | Binop (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Binop env _visitors_c0 _visitors_c1 _visitors_c2
          | Pipe (_visitors_c0,_visitors_c1) ->
              self#on_Pipe env _visitors_c0 _visitors_c1
          | Eif (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Eif env _visitors_c0 _visitors_c1 _visitors_c2
          | NullCoalesce (_visitors_c0,_visitors_c1) ->
              self#on_NullCoalesce env _visitors_c0 _visitors_c1
          | InstanceOf (_visitors_c0,_visitors_c1) ->
              self#on_InstanceOf env _visitors_c0 _visitors_c1
          | Is (_visitors_c0,_visitors_c1) ->
              self#on_Is env _visitors_c0 _visitors_c1
          | BracedExpr _visitors_c0 -> self#on_BracedExpr env _visitors_c0
          | ParenthesizedExpr _visitors_c0 ->
              self#on_ParenthesizedExpr env _visitors_c0
          | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
          | NewAnonClass (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2
          | Efun (_visitors_c0,_visitors_c1) ->
              self#on_Efun env _visitors_c0 _visitors_c1
          | Lfun _visitors_c0 -> self#on_Lfun env _visitors_c0
          | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
          | Unsafeexpr _visitors_c0 -> self#on_Unsafeexpr env _visitors_c0
          | Import (_visitors_c0,_visitors_c1) ->
              self#on_Import env _visitors_c0 _visitors_c1
          | Callconv (_visitors_c0,_visitors_c1) ->
              self#on_Callconv env _visitors_c0 _visitors_c1
          | Execution_operator _visitors_c0 ->
              self#on_Execution_operator env _visitors_c0
        method on_Include env = Include
        method on_Require env = Require
        method on_IncludeOnce env = IncludeOnce
        method on_RequireOnce env = RequireOnce
        method on_import_flavor env _visitors_this =
          match _visitors_this with
          | Include  -> self#on_Include env
          | Require  -> self#on_Require env
          | IncludeOnce  -> self#on_IncludeOnce env
          | RequireOnce  -> self#on_RequireOnce env
        method on_AFvalue env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          AFvalue _visitors_r0
        method on_AFkvalue env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          AFkvalue (_visitors_r0, _visitors_r1)
        method on_afield env _visitors_this =
          match _visitors_this with
          | AFvalue _visitors_c0 -> self#on_AFvalue env _visitors_c0
          | AFkvalue (_visitors_c0,_visitors_c1) ->
              self#on_AFkvalue env _visitors_c0 _visitors_c1
        method on_Default env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          Default _visitors_r0
        method on_Case env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          Case (_visitors_r0, _visitors_r1)
        method on_case env _visitors_this =
          match _visitors_this with
          | Default _visitors_c0 -> self#on_Default env _visitors_c0
          | Case (_visitors_c0,_visitors_c1) ->
              self#on_Case env _visitors_c0 _visitors_c1
        method on_catch env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in
          (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_field env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_attr env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] iter =
      object (self : 'self)
        inherit  [_] iter_base
        method on_program env = self#on_list self#on_def env
        method on_Fun env _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in ()
        method on_Class env _visitors_c0 =
          let _visitors_r0 = self#on_class_ env _visitors_c0  in ()
        method on_Stmt env _visitors_c0 =
          let _visitors_r0 = self#on_stmt env _visitors_c0  in ()
        method on_Typedef env _visitors_c0 =
          let _visitors_r0 = self#on_typedef env _visitors_c0  in ()
        method on_Constant env _visitors_c0 =
          let _visitors_r0 = self#on_gconst env _visitors_c0  in ()
        method on_Namespace env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_program env _visitors_c1  in ()
        method on_NamespaceUse env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_ns_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_id env _visitors_c1  in
                   let _visitors_r2 = self#on_id env _visitors_c2  in ()) env
              _visitors_c0
             in
          ()
        method on_SetNamespaceEnv env _visitors_c0 =
          let _visitors_r0 = self#on_env env _visitors_c0  in ()
        method on_def env _visitors_this =
          match _visitors_this with
          | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
          | Class _visitors_c0 -> self#on_Class env _visitors_c0
          | Stmt _visitors_c0 -> self#on_Stmt env _visitors_c0
          | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
          | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
          | Namespace (_visitors_c0,_visitors_c1) ->
              self#on_Namespace env _visitors_c0 _visitors_c1
          | NamespaceUse _visitors_c0 ->
              self#on_NamespaceUse env _visitors_c0
          | SetNamespaceEnv _visitors_c0 ->
              self#on_SetNamespaceEnv env _visitors_c0
        method on_typedef env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.t_id  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.t_tparams  in
          let _visitors_r2 =
            self#on_tconstraint env _visitors_this.t_constraint  in
          let _visitors_r3 = self#on_typedef_kind env _visitors_this.t_kind
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.t_user_attributes
             in
          let _visitors_r5 = self#on_env env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_mode env _visitors_this.t_mode  in ()
        method on_gconst env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_env env _visitors_this.cst_namespace  in
          ()
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 =
                     self#on_constraint_kind env _visitors_c0  in
                   let _visitors_r1 = self#on_hint env _visitors_c1  in ())
              env _visitors_c2
             in
          ()
        method on_tconstraint env = self#on_option self#on_hint env
        method on_Alias env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_NewType env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_typedef_kind env _visitors_this =
          match _visitors_this with
          | Alias _visitors_c0 -> self#on_Alias env _visitors_c0
          | NewType _visitors_c0 -> self#on_NewType env _visitors_c0
        method on_class_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.c_mode  in
          let _visitors_r1 =
            self#on_list self#on_user_attribute env
              _visitors_this.c_user_attributes
             in
          let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
          let _visitors_r3 = self#on_class_kind env _visitors_this.c_kind  in
          let _visitors_r4 = self#on_bool env _visitors_this.c_is_xhp  in
          let _visitors_r5 = self#on_id env _visitors_this.c_name  in
          let _visitors_r6 =
            self#on_list self#on_tparam env _visitors_this.c_tparams  in
          let _visitors_r7 =
            self#on_list self#on_hint env _visitors_this.c_extends  in
          let _visitors_r8 =
            self#on_list self#on_hint env _visitors_this.c_implements  in
          let _visitors_r9 =
            self#on_list self#on_class_elt env _visitors_this.c_body  in
          let _visitors_r10 = self#on_env env _visitors_this.c_namespace  in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_t env _visitors_this.c_span  in
          let _visitors_r13 =
            self#on_option self#on_string env _visitors_this.c_doc_comment
             in
          ()
        method on_enum_ env _visitors_this =
          let _visitors_r0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          ()
        method on_user_attribute env _visitors_this =
          let _visitors_r0 = self#on_id env _visitors_this.ua_name  in
          let _visitors_r1 =
            self#on_list self#on_expr env _visitors_this.ua_params  in
          ()
        method on_Const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in ())
              env _visitors_c1
             in
          ()
        method on_AbsConst env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in ()
        method on_Attributes env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_class_attr env _visitors_c0
             in
          ()
        method on_TypeConst env _visitors_c0 =
          let _visitors_r0 = self#on_typeconst env _visitors_c0  in ()
        method on_ClassUse env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_ClassUseAlias env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_id env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_kind env _visitors_c3  in
          ()
        method on_ClassUsePrecedence env _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in ()
        method on_XhpAttrUse env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_ClassTraitRequire env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_trait_req_kind env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in ()
        method on_ClassVars env _visitors_c0 =
          let _visitors_r0 = self#on_class_vars_ env _visitors_c0  in ()
        method on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_class_var env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in
          let _visitors_r3 =
            self#on_option
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_t env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   let _visitors_r2 =
                     self#on_list self#on_expr env _visitors_c2  in
                   ()) env _visitors_c3
             in
          ()
        method on_Method env _visitors_c0 =
          let _visitors_r0 = self#on_method_ env _visitors_c0  in ()
        method on_XhpCategory env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_pstring env _visitors_c0
             in
          ()
        method on_XhpChild env _visitors_c0 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in ()
        method on_class_elt env _visitors_this =
          match _visitors_this with
          | Const (_visitors_c0,_visitors_c1) ->
              self#on_Const env _visitors_c0 _visitors_c1
          | AbsConst (_visitors_c0,_visitors_c1) ->
              self#on_AbsConst env _visitors_c0 _visitors_c1
          | Attributes _visitors_c0 -> self#on_Attributes env _visitors_c0
          | TypeConst _visitors_c0 -> self#on_TypeConst env _visitors_c0
          | ClassUse _visitors_c0 -> self#on_ClassUse env _visitors_c0
          | ClassUseAlias
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_ClassUseAlias env _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
          | ClassUsePrecedence (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_ClassUsePrecedence env _visitors_c0 _visitors_c1
                _visitors_c2
          | XhpAttrUse _visitors_c0 -> self#on_XhpAttrUse env _visitors_c0
          | ClassTraitRequire (_visitors_c0,_visitors_c1) ->
              self#on_ClassTraitRequire env _visitors_c0 _visitors_c1
          | ClassVars _visitors_c0 -> self#on_ClassVars env _visitors_c0
          | XhpAttr (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_XhpAttr env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Method _visitors_c0 -> self#on_Method env _visitors_c0
          | XhpCategory _visitors_c0 -> self#on_XhpCategory env _visitors_c0
          | XhpChild _visitors_c0 -> self#on_XhpChild env _visitors_c0
        method on_ChildName env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in ()
        method on_ChildList env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_xhp_child env _visitors_c0
             in
          ()
        method on_ChildUnary env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child_op env _visitors_c1  in ()
        method on_ChildBinary env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_xhp_child env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in ()
        method on_xhp_child env _visitors_this =
          match _visitors_this with
          | ChildName _visitors_c0 -> self#on_ChildName env _visitors_c0
          | ChildList _visitors_c0 -> self#on_ChildList env _visitors_c0
          | ChildUnary (_visitors_c0,_visitors_c1) ->
              self#on_ChildUnary env _visitors_c0 _visitors_c1
          | ChildBinary (_visitors_c0,_visitors_c1) ->
              self#on_ChildBinary env _visitors_c0 _visitors_c1
        method on_ChildStar env = ()
        method on_ChildPlus env = ()
        method on_ChildQuestion env = ()
        method on_xhp_child_op env _visitors_this =
          match _visitors_this with
          | ChildStar  -> self#on_ChildStar env
          | ChildPlus  -> self#on_ChildPlus env
          | ChildQuestion  -> self#on_ChildQuestion env
        method on_CA_name env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in ()
        method on_CA_field env _visitors_c0 =
          let _visitors_r0 = self#on_ca_field env _visitors_c0  in ()
        method on_class_attr env _visitors_this =
          match _visitors_this with
          | CA_name _visitors_c0 -> self#on_CA_name env _visitors_c0
          | CA_field _visitors_c0 -> self#on_CA_field env _visitors_c0
        method on_ca_field env _visitors_this =
          let _visitors_r0 = self#on_ca_type env _visitors_this.ca_type  in
          let _visitors_r1 = self#on_id env _visitors_this.ca_id  in
          let _visitors_r2 =
            self#on_option self#on_expr env _visitors_this.ca_value  in
          let _visitors_r3 = self#on_bool env _visitors_this.ca_required  in
          ()
        method on_CA_hint env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_CA_enum env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_string env _visitors_c0  in
          ()
        method on_ca_type env _visitors_this =
          match _visitors_this with
          | CA_hint _visitors_c0 -> self#on_CA_hint env _visitors_c0
          | CA_enum _visitors_c0 -> self#on_CA_enum env _visitors_c0
        method on_class_var env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_expr env _visitors_c2  in
          ()
        method on_class_vars_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_r2 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r3 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r4 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          ()
        method on_method_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.m_kind  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.m_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in ())
              env _visitors_this.m_constrs
             in
          let _visitors_r3 = self#on_id env _visitors_this.m_name  in
          let _visitors_r4 =
            self#on_list self#on_fun_param env _visitors_this.m_params  in
          let _visitors_r5 = self#on_block env _visitors_this.m_body  in
          let _visitors_r6 =
            self#on_list self#on_user_attribute env
              _visitors_this.m_user_attributes
             in
          let _visitors_r7 =
            self#on_option self#on_hint env _visitors_this.m_ret  in
          let _visitors_r8 = self#on_bool env _visitors_this.m_ret_by_ref  in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.m_fun_kind
             in
          let _visitors_r10 = self#on_t env _visitors_this.m_span  in
          let _visitors_r11 =
            self#on_option self#on_string env _visitors_this.m_doc_comment
             in
          ()
        method on_typeconst env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.tconst_abstract
             in
          let _visitors_r1 = self#on_id env _visitors_this.tconst_name  in
          let _visitors_r2 =
            self#on_list self#on_tparam env _visitors_this.tconst_tparams  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.tconst_constraint
             in
          let _visitors_r4 =
            self#on_option self#on_hint env _visitors_this.tconst_type  in
          let _visitors_r5 = self#on_t env _visitors_this.tconst_span  in ()
        method on_is_reference env = self#on_bool env
        method on_is_variadic env = self#on_bool env
        method on_fun_param env _visitors_this =
          let _visitors_r0 =
            self#on_option self#on_hint env _visitors_this.param_hint  in
          let _visitors_r1 =
            self#on_is_reference env _visitors_this.param_is_reference  in
          let _visitors_r2 =
            self#on_is_variadic env _visitors_this.param_is_variadic  in
          let _visitors_r3 = self#on_id env _visitors_this.param_id  in
          let _visitors_r4 =
            self#on_option self#on_expr env _visitors_this.param_expr  in
          let _visitors_r5 =
            self#on_option self#on_kind env _visitors_this.param_modifier  in
          let _visitors_r6 =
            self#on_option self#on_param_kind env
              _visitors_this.param_callconv
             in
          let _visitors_r7 =
            self#on_list self#on_user_attribute env
              _visitors_this.param_user_attributes
             in
          ()
        method on_fun_ env _visitors_this =
          let _visitors_r0 = self#on_mode env _visitors_this.f_mode  in
          let _visitors_r1 =
            self#on_list self#on_tparam env _visitors_this.f_tparams  in
          let _visitors_r2 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1,_visitors_c2)  ->
                   let _visitors_r0 = self#on_hint env _visitors_c0  in
                   let _visitors_r1 =
                     self#on_constraint_kind env _visitors_c1  in
                   let _visitors_r2 = self#on_hint env _visitors_c2  in ())
              env _visitors_this.f_constrs
             in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.f_ret  in
          let _visitors_r4 = self#on_bool env _visitors_this.f_ret_by_ref  in
          let _visitors_r5 = self#on_id env _visitors_this.f_name  in
          let _visitors_r6 =
            self#on_list self#on_fun_param env _visitors_this.f_params  in
          let _visitors_r7 = self#on_block env _visitors_this.f_body  in
          let _visitors_r8 =
            self#on_list self#on_user_attribute env
              _visitors_this.f_user_attributes
             in
          let _visitors_r9 = self#on_fun_kind env _visitors_this.f_fun_kind
             in
          let _visitors_r10 = self#on_env env _visitors_this.f_namespace  in
          let _visitors_r11 = self#on_t env _visitors_this.f_span  in
          let _visitors_r12 =
            self#on_option self#on_string env _visitors_this.f_doc_comment
             in
          let _visitors_r13 = self#on_bool env _visitors_this.f_static  in ()
        method on_is_coroutine env = self#on_bool env
        method on_hint env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_hint_ env _visitors_c1  in ()
        method on_Hvariadic env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          ()
        method on_Hnon_variadic env = ()
        method on_variadic_hint env _visitors_this =
          match _visitors_this with
          | Hvariadic _visitors_c0 -> self#on_Hvariadic env _visitors_c0
          | Hnon_variadic  -> self#on_Hnon_variadic env
        method on_Hoption env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 _visitors_c4 =
          let _visitors_r0 = self#on_is_coroutine env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c2
             in
          let _visitors_r3 = self#on_variadic_hint env _visitors_c3  in
          let _visitors_r4 = self#on_hint env _visitors_c4  in ()
        method on_Htuple env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          ()
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          ()
        method on_Hshape env _visitors_c0 =
          let _visitors_r0 = self#on_shape_info env _visitors_c0  in ()
        method on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_id env _visitors_c2  in ()
        method on_Hsoft env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Haccess env _visitors_c0 _visitors_c1 _visitors_c2
          | Hsoft _visitors_c0 -> self#on_Hsoft env _visitors_c0
        method on_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.si_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_list self#on_shape_field env
              _visitors_this.si_shape_field_list
             in
          ()
        method on_shape_field env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sf_optional  in
          let _visitors_r1 =
            self#on_shape_field_name env _visitors_this.sf_name  in
          let _visitors_r2 = self#on_hint env _visitors_this.sf_hint  in ()
        method on_using_stmt env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.us_is_block_scoped  in
          let _visitors_r1 = self#on_bool env _visitors_this.us_has_await  in
          let _visitors_r2 = self#on_expr env _visitors_this.us_expr  in
          let _visitors_r3 = self#on_block env _visitors_this.us_block  in ()
        method on_Unsafe env = ()
        method on_Fallthrough env = ()
        method on_Expr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Block env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in ()
        method on_Break env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          ()
        method on_Continue env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          ()
        method on_Throw env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Return env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          ()
        method on_GotoLabel env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_Goto env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_Static_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Global_var env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          ()
        method on_If env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in ()
        method on_Do env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_While env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in ()
        method on_For env _visitors_c0 _visitors_c1 _visitors_c2 _visitors_c3
          _visitors_c4 =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_expr env _visitors_c3  in
          let _visitors_r4 = self#on_block env _visitors_c4  in ()
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_case env _visitors_c1  in
          ()
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_t env _visitors_c1  in
          let _visitors_r2 = self#on_as_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in ()
        method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_catch env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in ()
        method on_Def_inline env _visitors_c0 =
          let _visitors_r0 = self#on_def env _visitors_c0  in ()
        method on_Noop env = ()
        method on_Markup env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          ()
        method on_Using env _visitors_c0 =
          let _visitors_r0 = self#on_using_stmt env _visitors_c0  in ()
        method on_Declare env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_bool env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in ()
        method on_stmt env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break (_visitors_c0,_visitors_c1) ->
              self#on_Break env _visitors_c0 _visitors_c1
          | Continue (_visitors_c0,_visitors_c1) ->
              self#on_Continue env _visitors_c0 _visitors_c1
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return (_visitors_c0,_visitors_c1) ->
              self#on_Return env _visitors_c0 _visitors_c1
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var (_visitors_c0,_visitors_c1) ->
              self#on_Global_var env _visitors_c0 _visitors_c1
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
              ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4
          | Switch (_visitors_c0,_visitors_c1) ->
              self#on_Switch env _visitors_c0 _visitors_c1
          | Foreach (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
          | Def_inline _visitors_c0 -> self#on_Def_inline env _visitors_c0
          | Noop  -> self#on_Noop env
          | Markup (_visitors_c0,_visitors_c1) ->
              self#on_Markup env _visitors_c0 _visitors_c1
          | Using _visitors_c0 -> self#on_Using env _visitors_c0
          | Declare (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Declare env _visitors_c0 _visitors_c1 _visitors_c2
        method on_As_v env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_As_kv env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_as_expr env _visitors_this =
          match _visitors_this with
          | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
          | As_kv (_visitors_c0,_visitors_c1) ->
              self#on_As_kv env _visitors_c0 _visitors_c1
        method on_Xhp_simple env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Xhp_spread env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_xhp_attribute env _visitors_this =
          match _visitors_this with
          | Xhp_simple (_visitors_c0,_visitors_c1) ->
              self#on_Xhp_simple env _visitors_c0 _visitors_c1
          | Xhp_spread _visitors_c0 -> self#on_Xhp_spread env _visitors_c0
        method on_block env = self#on_list self#on_stmt env
        method on_expr env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_t env _visitors_c0  in
          let _visitors_r1 = self#on_expr_ env _visitors_c1  in ()
        method on_Array env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_afield env _visitors_c0  in
          ()
        method on_Varray env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Darray env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_expr env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in ())
              env _visitors_c0
             in
          ()
        method on_Shape env _visitors_c0 =
          let _visitors_r0 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 =
                     self#on_shape_field_name env _visitors_c0  in
                   let _visitors_r1 = self#on_expr env _visitors_c1  in ())
              env _visitors_c0
             in
          ()
        method on_Collection env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_afield env _visitors_c1  in
          ()
        method on_Null env = ()
        method on_True env = ()
        method on_False env = ()
        method on_Omitted env = ()
        method on_Id env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in ()
        method on_Id_type_arguments env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          ()
        method on_Lvar env _visitors_c0 =
          let _visitors_r0 = self#on_id env _visitors_c0  in ()
        method on_Lvarvar env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_int env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in ()
        method on_Clone env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_og_null_flavor env _visitors_c2  in ()
        method on_Array_get env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          ()
        method on_Class_get env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Class_const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
        method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_list self#on_expr env _visitors_c3  in
          ()
        method on_Int env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_Float env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_String env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_String2 env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Yield env _visitors_c0 =
          let _visitors_r0 = self#on_afield env _visitors_c0  in ()
        method on_Yield_break env = ()
        method on_Yield_from env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Await env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Suspend env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_List env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Expr_list env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Cast env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Unop env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_uop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Binop env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_bop env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in ()
        method on_Pipe env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in ()
        method on_NullCoalesce env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_InstanceOf env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Is env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in ()
        method on_BracedExpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_ParenthesizedExpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          ()
        method on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_class_ env _visitors_c2  in ()
        method on_Efun env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in
          let _visitors_r1 =
            self#on_list
              (fun env  ->
                 fun (_visitors_c0,_visitors_c1)  ->
                   let _visitors_r0 = self#on_id env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in ())
              env _visitors_c1
             in
          ()
        method on_Lfun env _visitors_c0 =
          let _visitors_r0 = self#on_fun_ env _visitors_c0  in ()
        method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 =
            self#on_list self#on_xhp_attribute env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_expr env _visitors_c2  in
          ()
        method on_Unsafeexpr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Import env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_import_flavor env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Callconv env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_param_kind env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_Execution_operator env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_expr_ env _visitors_this =
          match _visitors_this with
          | Array _visitors_c0 -> self#on_Array env _visitors_c0
          | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
          | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
          | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
          | Collection (_visitors_c0,_visitors_c1) ->
              self#on_Collection env _visitors_c0 _visitors_c1
          | Null  -> self#on_Null env
          | True  -> self#on_True env
          | False  -> self#on_False env
          | Omitted  -> self#on_Omitted env
          | Id _visitors_c0 -> self#on_Id env _visitors_c0
          | Id_type_arguments (_visitors_c0,_visitors_c1) ->
              self#on_Id_type_arguments env _visitors_c0 _visitors_c1
          | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
          | Lvarvar (_visitors_c0,_visitors_c1) ->
              self#on_Lvarvar env _visitors_c0 _visitors_c1
          | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
          | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
          | Array_get (_visitors_c0,_visitors_c1) ->
              self#on_Array_get env _visitors_c0 _visitors_c1
          | Class_get (_visitors_c0,_visitors_c1) ->
              self#on_Class_get env _visitors_c0 _visitors_c1
          | Class_const (_visitors_c0,_visitors_c1) ->
              self#on_Class_const env _visitors_c0 _visitors_c1
          | Call (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
          | Int _visitors_c0 -> self#on_Int env _visitors_c0
          | Float _visitors_c0 -> self#on_Float env _visitors_c0
          | String _visitors_c0 -> self#on_String env _visitors_c0
          | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
          | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
          | Yield_break  -> self#on_Yield_break env
          | Yield_from _visitors_c0 -> self#on_Yield_from env _visitors_c0
          | Await _visitors_c0 -> self#on_Await env _visitors_c0
          | Suspend _visitors_c0 -> self#on_Suspend env _visitors_c0
          | List _visitors_c0 -> self#on_List env _visitors_c0
          | Expr_list _visitors_c0 -> self#on_Expr_list env _visitors_c0
          | Cast (_visitors_c0,_visitors_c1) ->
              self#on_Cast env _visitors_c0 _visitors_c1
          | Unop (_visitors_c0,_visitors_c1) ->
              self#on_Unop env _visitors_c0 _visitors_c1
          | Binop (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Binop env _visitors_c0 _visitors_c1 _visitors_c2
          | Pipe (_visitors_c0,_visitors_c1) ->
              self#on_Pipe env _visitors_c0 _visitors_c1
          | Eif (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Eif env _visitors_c0 _visitors_c1 _visitors_c2
          | NullCoalesce (_visitors_c0,_visitors_c1) ->
              self#on_NullCoalesce env _visitors_c0 _visitors_c1
          | InstanceOf (_visitors_c0,_visitors_c1) ->
              self#on_InstanceOf env _visitors_c0 _visitors_c1
          | Is (_visitors_c0,_visitors_c1) ->
              self#on_Is env _visitors_c0 _visitors_c1
          | BracedExpr _visitors_c0 -> self#on_BracedExpr env _visitors_c0
          | ParenthesizedExpr _visitors_c0 ->
              self#on_ParenthesizedExpr env _visitors_c0
          | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
          | NewAnonClass (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_NewAnonClass env _visitors_c0 _visitors_c1 _visitors_c2
          | Efun (_visitors_c0,_visitors_c1) ->
              self#on_Efun env _visitors_c0 _visitors_c1
          | Lfun _visitors_c0 -> self#on_Lfun env _visitors_c0
          | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
          | Unsafeexpr _visitors_c0 -> self#on_Unsafeexpr env _visitors_c0
          | Import (_visitors_c0,_visitors_c1) ->
              self#on_Import env _visitors_c0 _visitors_c1
          | Callconv (_visitors_c0,_visitors_c1) ->
              self#on_Callconv env _visitors_c0 _visitors_c1
          | Execution_operator _visitors_c0 ->
              self#on_Execution_operator env _visitors_c0
        method on_Include env = ()
        method on_Require env = ()
        method on_IncludeOnce env = ()
        method on_RequireOnce env = ()
        method on_import_flavor env _visitors_this =
          match _visitors_this with
          | Include  -> self#on_Include env
          | Require  -> self#on_Require env
          | IncludeOnce  -> self#on_IncludeOnce env
          | RequireOnce  -> self#on_RequireOnce env
        method on_AFvalue env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_AFkvalue env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_afield env _visitors_this =
          match _visitors_this with
          | AFvalue _visitors_c0 -> self#on_AFvalue env _visitors_c0
          | AFkvalue (_visitors_c0,_visitors_c1) ->
              self#on_AFkvalue env _visitors_c0 _visitors_c1
        method on_Default env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in ()
        method on_Case env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_block env _visitors_c1  in ()
        method on_case env _visitors_this =
          match _visitors_this with
          | Default _visitors_c0 -> self#on_Default env _visitors_c0
          | Case (_visitors_c0,_visitors_c1) ->
              self#on_Case env _visitors_c0 _visitors_c1
        method on_catch env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_block env _visitors_c2  in ()
        method on_field env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
        method on_attr env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in ()
      end
    [@@@VISITORS.END ]
  end
type any =
  | AHint of hint 
  | AExpr of expr 
  | AStmt of stmt 
  | ADef of def 
  | AProgram of program 
