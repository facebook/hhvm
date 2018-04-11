(* @generated from ast.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2004-present, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<740c9aef7a1ef36cede3244a5bfaa5ff96d3ec7d>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2015, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the MIT license found in the\n * LICENSE file in the \"hack\" directory of this source tree.\n *\n "]
include Ast_defs
type program = def list[@@deriving
                         (show,
                           (visitors
                              {
                                variety = "endo";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["endo_defs"]
                              }),
                           (visitors
                              {
                                variety = "reduce";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["reduce_defs"]
                              }),
                           (visitors
                              {
                                variety = "map";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["map_defs"]
                              }),
                           (visitors
                              {
                                variety = "iter";
                                nude = true;
                                visit_prefix = "on_";
                                ancestors = ["iter_defs"]
                              }))]
and nsenv = ((Namespace_env.env)[@opaque ])
and fimode = ((FileInfo.mode)[@visitors.opaque ])
and def =
  | Fun of fun_ 
  | Class of class_ 
  | Stmt of stmt 
  | Typedef of typedef 
  | Constant of gconst 
  | Namespace of id * program 
  | NamespaceUse of (ns_kind * id * id) list 
  | SetNamespaceEnv of nsenv 
and typedef =
  {
  t_id: id ;
  t_tparams: tparam list ;
  t_constraint: tconstraint ;
  t_kind: typedef_kind ;
  t_user_attributes: user_attribute list ;
  t_namespace: nsenv ;
  t_mode: fimode }
and gconst =
  {
  cst_mode: fimode ;
  cst_kind: cst_kind ;
  cst_name: id ;
  cst_type: hint option ;
  cst_value: expr ;
  cst_namespace: nsenv ;
  cst_span: pos }
and tparam = (variance * id * (constraint_kind * hint) list)
and tconstraint = hint option
and typedef_kind =
  | Alias of hint 
  | NewType of hint 
and class_ =
  {
  c_mode: fimode ;
  c_user_attributes: user_attribute list ;
  c_final: bool ;
  c_kind: class_kind ;
  c_is_xhp: bool ;
  c_name: id ;
  c_tparams: tparam list ;
  c_extends: hint list ;
  c_implements: hint list ;
  c_body: class_elt list ;
  c_namespace: nsenv ;
  c_enum: enum_ option ;
  c_span: pos ;
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
  | XhpAttr of hint option * class_var * bool * (pos * bool * expr list)
  option 
  | Method of method_ 
  | XhpCategory of pos * pstring list 
  | XhpChild of pos * xhp_child 
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
and class_var = (pos * id * expr option)
and class_vars_ =
  {
  cv_kinds: kind list ;
  cv_hint: hint option ;
  cv_is_promoted_variadic: is_variadic ;
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
  m_span: pos ;
  m_doc_comment: string option }
and typeconst =
  {
  tconst_abstract: bool ;
  tconst_name: id ;
  tconst_tparams: tparam list ;
  tconst_constraint: hint option ;
  tconst_type: hint option ;
  tconst_span: pos }
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
  f_mode: fimode ;
  f_tparams: tparam list ;
  f_constrs: (hint * constraint_kind * hint) list ;
  f_ret: hint option ;
  f_ret_by_ref: bool ;
  f_name: id ;
  f_params: fun_param list ;
  f_body: block ;
  f_user_attributes: user_attribute list ;
  f_fun_kind: fun_kind ;
  f_namespace: nsenv ;
  f_span: pos ;
  f_doc_comment: string option ;
  f_static: bool }
and is_coroutine = bool
and hint = (pos * hint_)
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
and stmt = (pos * stmt_)
and stmt_ =
  | Unsafe 
  | Fallthrough 
  | Expr of expr 
  | Block of block 
  | Break of expr option 
  | Continue of expr option 
  | Throw of expr 
  | Return of expr option 
  | GotoLabel of pstring 
  | Goto of pstring 
  | Static_var of expr list 
  | Global_var of expr list 
  | If of expr * block * block 
  | Do of block * expr 
  | While of expr * block 
  | For of expr * expr * expr * block 
  | Switch of expr * case list 
  | Foreach of expr * pos option * as_expr * block 
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
and expr = (pos * expr_)
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
  | Dollar of expr 
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
  | As of expr * hint * bool 
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
let rec pp_program : Format.formatter -> program -> Ppx_deriving_runtime.unit
  =
  let __0 () = pp_def  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>[";
          ignore
            (List.fold_left
               (fun sep  ->
                  fun x  ->
                    if sep then Format.fprintf fmt ";@ ";
                    ((__0 ()) fmt) x;
                    true) false x);
          Format.fprintf fmt "@,]@]")
    [@ocaml.warning "-A"])

and show_program : program -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_program x

and (pp_nsenv : Format.formatter -> nsenv -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  -> fun _  -> Format.pp_print_string fmt "<opaque>")
  [@ocaml.warning "-A"])

and show_nsenv : nsenv -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_nsenv x

and pp_fimode : Format.formatter -> fimode -> Ppx_deriving_runtime.unit =
  let __0 () = FileInfo.pp_mode  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_fimode : fimode -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fimode x

and pp_def : Format.formatter -> def -> Ppx_deriving_runtime.unit =
  let __10 () = pp_nsenv
  
  and __9 () = pp_id
  
  and __8 () = pp_id
  
  and __7 () = pp_ns_kind
  
  and __6 () = pp_program
  
  and __5 () = pp_id
  
  and __4 () = pp_gconst
  
  and __3 () = pp_typedef
  
  and __2 () = pp_stmt
  
  and __1 () = pp_class_
  
  and __0 () = pp_fun_
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Fun a0 ->
            (Format.fprintf fmt "(@[<2>Fun@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Class a0 ->
            (Format.fprintf fmt "(@[<2>Class@ ";
             ((__1 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Stmt a0 ->
            (Format.fprintf fmt "(@[<2>Stmt@ ";
             ((__2 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Typedef a0 ->
            (Format.fprintf fmt "(@[<2>Typedef@ ";
             ((__3 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Constant a0 ->
            (Format.fprintf fmt "(@[<2>Constant@ ";
             ((__4 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Namespace (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Namespace (@,";
             (((__5 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__6 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | NamespaceUse a0 ->
            (Format.fprintf fmt "(@[<2>NamespaceUse@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((fun (a0,a1,a2)  ->
                               Format.fprintf fmt "(@[";
                               ((((__7 ()) fmt) a0;
                                 Format.fprintf fmt ",@ ";
                                 ((__8 ()) fmt) a1);
                                Format.fprintf fmt ",@ ";
                                ((__9 ()) fmt) a2);
                               Format.fprintf fmt "@])")) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | SetNamespaceEnv a0 ->
            (Format.fprintf fmt "(@[<2>SetNamespaceEnv@ ";
             ((__10 ()) fmt) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_def : def -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_def x

and pp_typedef : Format.formatter -> typedef -> Ppx_deriving_runtime.unit =
  let __6 () = pp_fimode
  
  and __5 () = pp_nsenv
  
  and __4 () = pp_user_attribute
  
  and __3 () = pp_typedef_kind
  
  and __2 () = pp_tconstraint
  
  and __1 () = pp_tparam
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          (((((((Format.fprintf fmt "@[%s =@ " "t_id";
                 ((__0 ()) fmt) x.t_id;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "t_tparams";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__1 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.t_tparams;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "t_constraint";
               ((__2 ()) fmt) x.t_constraint;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "t_kind";
              ((__3 ()) fmt) x.t_kind;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "t_user_attributes";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__4 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) x.t_user_attributes;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "t_namespace";
            ((__5 ()) fmt) x.t_namespace;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "t_mode";
           ((__6 ()) fmt) x.t_mode;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_typedef : typedef -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_typedef x

and pp_gconst : Format.formatter -> gconst -> Ppx_deriving_runtime.unit =
  let __6 () = pp_pos
  
  and __5 () = pp_nsenv
  
  and __4 () = pp_expr
  
  and __3 () = pp_hint
  
  and __2 () = pp_id
  
  and __1 () = pp_cst_kind
  
  and __0 () = pp_fimode
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          (((((((Format.fprintf fmt "@[%s =@ " "cst_mode";
                 ((__0 ()) fmt) x.cst_mode;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "cst_kind";
                ((__1 ()) fmt) x.cst_kind;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "cst_name";
               ((__2 ()) fmt) x.cst_name;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "cst_type";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__3 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) x.cst_type;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "cst_value";
             ((__4 ()) fmt) x.cst_value;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "cst_namespace";
            ((__5 ()) fmt) x.cst_namespace;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "cst_span";
           ((__6 ()) fmt) x.cst_span;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_gconst : gconst -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_gconst x

and pp_tparam : Format.formatter -> tparam -> Ppx_deriving_runtime.unit =
  let __3 () = pp_hint
  
  and __2 () = pp_constraint_kind
  
  and __1 () = pp_id
  
  and __0 () = pp_variance
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1,a2)  ->
          Format.fprintf fmt "(@[";
          ((((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
           Format.fprintf fmt ",@ ";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((fun (a0,a1)  ->
                             Format.fprintf fmt "(@[";
                             (((__2 ()) fmt) a0;
                              Format.fprintf fmt ",@ ";
                              ((__3 ()) fmt) a1);
                             Format.fprintf fmt "@])")) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) a2);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_tparam : tparam -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_tparam x

and pp_tconstraint :
  Format.formatter -> tconstraint -> Ppx_deriving_runtime.unit =
  let __0 () = pp_hint  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | None  -> Format.pp_print_string fmt "None"
        | Some x ->
            (Format.pp_print_string fmt "(Some ";
             ((__0 ()) fmt) x;
             Format.pp_print_string fmt ")"))
    [@ocaml.warning "-A"])

and show_tconstraint : tconstraint -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_tconstraint x

and pp_typedef_kind :
  Format.formatter -> typedef_kind -> Ppx_deriving_runtime.unit =
  let __1 () = pp_hint
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Alias a0 ->
            (Format.fprintf fmt "(@[<2>Alias@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | NewType a0 ->
            (Format.fprintf fmt "(@[<2>NewType@ ";
             ((__1 ()) fmt) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_typedef_kind : typedef_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_typedef_kind x

and pp_class_ : Format.formatter -> class_ -> Ppx_deriving_runtime.unit =
  let __10 () = pp_pos
  
  and __9 () = pp_enum_
  
  and __8 () = pp_nsenv
  
  and __7 () = pp_class_elt
  
  and __6 () = pp_hint
  
  and __5 () = pp_hint
  
  and __4 () = pp_tparam
  
  and __3 () = pp_id
  
  and __2 () = pp_class_kind
  
  and __1 () = pp_user_attribute
  
  and __0 () = pp_fimode
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((((((((((Format.fprintf fmt "@[%s =@ " "c_mode";
                        ((__0 ()) fmt) x.c_mode;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "c_user_attributes";
                       ((fun x  ->
                           Format.fprintf fmt "@[<2>[";
                           ignore
                             (List.fold_left
                                (fun sep  ->
                                   fun x  ->
                                     if sep then Format.fprintf fmt ";@ ";
                                     ((__1 ()) fmt) x;
                                     true) false x);
                           Format.fprintf fmt "@,]@]")) x.c_user_attributes;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "c_final";
                      (Format.fprintf fmt "%B") x.c_final;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "c_kind";
                     ((__2 ()) fmt) x.c_kind;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "c_is_xhp";
                    (Format.fprintf fmt "%B") x.c_is_xhp;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "c_name";
                   ((__3 ()) fmt) x.c_name;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "c_tparams";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__4 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) x.c_tparams;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "c_extends";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__5 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) x.c_extends;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "c_implements";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__6 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.c_implements;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "c_body";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__7 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) x.c_body;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "c_namespace";
              ((__8 ()) fmt) x.c_namespace;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "c_enum";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__9 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) x.c_enum;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "c_span";
            ((__10 ()) fmt) x.c_span;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "c_doc_comment";
           ((function
             | None  -> Format.pp_print_string fmt "None"
             | Some x ->
                 (Format.pp_print_string fmt "(Some ";
                  (Format.fprintf fmt "%S") x;
                  Format.pp_print_string fmt ")"))) x.c_doc_comment;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_class_ : class_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_ x

and pp_enum_ : Format.formatter -> enum_ -> Ppx_deriving_runtime.unit =
  let __1 () = pp_hint
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((Format.fprintf fmt "@[%s =@ " "e_base";
            ((__0 ()) fmt) x.e_base;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "e_constraint";
           ((function
             | None  -> Format.pp_print_string fmt "None"
             | Some x ->
                 (Format.pp_print_string fmt "(Some ";
                  ((__1 ()) fmt) x;
                  Format.pp_print_string fmt ")"))) x.e_constraint;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_enum_ : enum_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_enum_ x

and pp_user_attribute :
  Format.formatter -> user_attribute -> Ppx_deriving_runtime.unit =
  let __1 () = pp_expr
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((Format.fprintf fmt "@[%s =@ " "ua_name";
            ((__0 ()) fmt) x.ua_name;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "ua_params";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((__1 ()) fmt) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) x.ua_params;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_user_attribute : user_attribute -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_user_attribute x

and pp_class_elt : Format.formatter -> class_elt -> Ppx_deriving_runtime.unit
  =
  let __27 () = pp_xhp_child
  
  and __26 () = pp_pos
  
  and __25 () = pp_pstring
  
  and __24 () = pp_pos
  
  and __23 () = pp_method_
  
  and __22 () = pp_expr
  
  and __21 () = pp_pos
  
  and __20 () = pp_class_var
  
  and __19 () = pp_hint
  
  and __18 () = pp_class_vars_
  
  and __17 () = pp_hint
  
  and __16 () = pp_trait_req_kind
  
  and __15 () = pp_hint
  
  and __14 () = pp_id
  
  and __13 () = pp_pstring
  
  and __12 () = pp_id
  
  and __11 () = pp_kind
  
  and __10 () = pp_id
  
  and __9 () = pp_pstring
  
  and __8 () = pp_id
  
  and __7 () = pp_hint
  
  and __6 () = pp_typeconst
  
  and __5 () = pp_class_attr
  
  and __4 () = pp_id
  
  and __3 () = pp_hint
  
  and __2 () = pp_expr
  
  and __1 () = pp_id
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Const (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Const (@,";
             (((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__0 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((fun (a0,a1)  ->
                                Format.fprintf fmt "(@[";
                                (((__1 ()) fmt) a0;
                                 Format.fprintf fmt ",@ ";
                                 ((__2 ()) fmt) a1);
                                Format.fprintf fmt "@])")) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | AbsConst (a0,a1) ->
            (Format.fprintf fmt "(@[<2>AbsConst (@,";
             (((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__3 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a0;
              Format.fprintf fmt ",@ ";
              ((__4 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Attributes a0 ->
            (Format.fprintf fmt "(@[<2>Attributes@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__5 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | TypeConst a0 ->
            (Format.fprintf fmt "(@[<2>TypeConst@ ";
             ((__6 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | ClassUse a0 ->
            (Format.fprintf fmt "(@[<2>ClassUse@ ";
             ((__7 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | ClassUseAlias (a0,a1,a2,a3) ->
            (Format.fprintf fmt "(@[<2>ClassUseAlias (@,";
             (((((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__8 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) a0;
                Format.fprintf fmt ",@ ";
                ((__9 ()) fmt) a1);
               Format.fprintf fmt ",@ ";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__10 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) a2);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__11 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a3);
             Format.fprintf fmt "@,))@]")
        | ClassUsePrecedence (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>ClassUsePrecedence (@,";
             ((((__12 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__13 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__14 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a2);
             Format.fprintf fmt "@,))@]")
        | XhpAttrUse a0 ->
            (Format.fprintf fmt "(@[<2>XhpAttrUse@ ";
             ((__15 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | ClassTraitRequire (a0,a1) ->
            (Format.fprintf fmt "(@[<2>ClassTraitRequire (@,";
             (((__16 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__17 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | ClassVars a0 ->
            (Format.fprintf fmt "(@[<2>ClassVars@ ";
             ((__18 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | XhpAttr (a0,a1,a2,a3) ->
            (Format.fprintf fmt "(@[<2>XhpAttr (@,";
             (((((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__19 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) a0;
                Format.fprintf fmt ",@ ";
                ((__20 ()) fmt) a1);
               Format.fprintf fmt ",@ ";
               (Format.fprintf fmt "%B") a2);
              Format.fprintf fmt ",@ ";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((fun (a0,a1,a2)  ->
                         Format.fprintf fmt "(@[";
                         ((((__21 ()) fmt) a0;
                           Format.fprintf fmt ",@ ";
                           (Format.fprintf fmt "%B") a1);
                          Format.fprintf fmt ",@ ";
                          ((fun x  ->
                              Format.fprintf fmt "@[<2>[";
                              ignore
                                (List.fold_left
                                   (fun sep  ->
                                      fun x  ->
                                        if sep then Format.fprintf fmt ";@ ";
                                        ((__22 ()) fmt) x;
                                        true) false x);
                              Format.fprintf fmt "@,]@]")) a2);
                         Format.fprintf fmt "@])")) x;
                     Format.pp_print_string fmt ")"))) a3);
             Format.fprintf fmt "@,))@]")
        | Method a0 ->
            (Format.fprintf fmt "(@[<2>Method@ ";
             ((__23 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | XhpCategory (a0,a1) ->
            (Format.fprintf fmt "(@[<2>XhpCategory (@,";
             (((__24 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__25 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | XhpChild (a0,a1) ->
            (Format.fprintf fmt "(@[<2>XhpChild (@,";
             (((__26 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__27 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_class_elt : class_elt -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_elt x

and pp_xhp_child : Format.formatter -> xhp_child -> Ppx_deriving_runtime.unit
  =
  let __5 () = pp_xhp_child
  
  and __4 () = pp_xhp_child
  
  and __3 () = pp_xhp_child_op
  
  and __2 () = pp_xhp_child
  
  and __1 () = pp_xhp_child
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | ChildName a0 ->
            (Format.fprintf fmt "(@[<2>ChildName@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | ChildList a0 ->
            (Format.fprintf fmt "(@[<2>ChildList@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__1 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | ChildUnary (a0,a1) ->
            (Format.fprintf fmt "(@[<2>ChildUnary (@,";
             (((__2 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__3 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | ChildBinary (a0,a1) ->
            (Format.fprintf fmt "(@[<2>ChildBinary (@,";
             (((__4 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__5 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_xhp_child : xhp_child -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_xhp_child x

and (pp_xhp_child_op :
      Format.formatter -> xhp_child_op -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | ChildStar  -> Format.pp_print_string fmt "ChildStar"
        | ChildPlus  -> Format.pp_print_string fmt "ChildPlus"
        | ChildQuestion  -> Format.pp_print_string fmt "ChildQuestion")
  [@ocaml.warning "-A"])

and show_xhp_child_op : xhp_child_op -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_xhp_child_op x

and pp_class_attr :
  Format.formatter -> class_attr -> Ppx_deriving_runtime.unit =
  let __1 () = pp_ca_field
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | CA_name a0 ->
            (Format.fprintf fmt "(@[<2>CA_name@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | CA_field a0 ->
            (Format.fprintf fmt "(@[<2>CA_field@ ";
             ((__1 ()) fmt) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_class_attr : class_attr -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_attr x

and pp_ca_field : Format.formatter -> ca_field -> Ppx_deriving_runtime.unit =
  let __2 () = pp_expr
  
  and __1 () = pp_id
  
  and __0 () = pp_ca_type
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((Format.fprintf fmt "@[%s =@ " "ca_type";
              ((__0 ()) fmt) x.ca_type;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "ca_id";
             ((__1 ()) fmt) x.ca_id;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "ca_value";
            ((function
              | None  -> Format.pp_print_string fmt "None"
              | Some x ->
                  (Format.pp_print_string fmt "(Some ";
                   ((__2 ()) fmt) x;
                   Format.pp_print_string fmt ")"))) x.ca_value;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "ca_required";
           (Format.fprintf fmt "%B") x.ca_required;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_ca_field : ca_field -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_ca_field x

and pp_ca_type : Format.formatter -> ca_type -> Ppx_deriving_runtime.unit =
  let __0 () = pp_hint  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | CA_hint a0 ->
            (Format.fprintf fmt "(@[<2>CA_hint@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | CA_enum a0 ->
            (Format.fprintf fmt "(@[<2>CA_enum@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           (Format.fprintf fmt "%S") x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_ca_type : ca_type -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_ca_type x

and pp_class_var : Format.formatter -> class_var -> Ppx_deriving_runtime.unit
  =
  let __2 () = pp_expr
  
  and __1 () = pp_id
  
  and __0 () = pp_pos
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1,a2)  ->
          Format.fprintf fmt "(@[";
          ((((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
           Format.fprintf fmt ",@ ";
           ((function
             | None  -> Format.pp_print_string fmt "None"
             | Some x ->
                 (Format.pp_print_string fmt "(Some ";
                  ((__2 ()) fmt) x;
                  Format.pp_print_string fmt ")"))) a2);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_class_var : class_var -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_var x

and pp_class_vars_ :
  Format.formatter -> class_vars_ -> Ppx_deriving_runtime.unit =
  let __4 () = pp_user_attribute
  
  and __3 () = pp_class_var
  
  and __2 () = pp_is_variadic
  
  and __1 () = pp_hint
  
  and __0 () = pp_kind
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((Format.fprintf fmt "@[%s =@ " "cv_kinds";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__0 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.cv_kinds;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "cv_hint";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__1 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.cv_hint;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "cv_is_promoted_variadic";
              ((__2 ()) fmt) x.cv_is_promoted_variadic;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "cv_names";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__3 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) x.cv_names;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "cv_doc_comment";
            ((function
              | None  -> Format.pp_print_string fmt "None"
              | Some x ->
                  (Format.pp_print_string fmt "(Some ";
                   (Format.fprintf fmt "%S") x;
                   Format.pp_print_string fmt ")"))) x.cv_doc_comment;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "cv_user_attributes";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((__4 ()) fmt) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) x.cv_user_attributes;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_class_vars_ : class_vars_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_vars_ x

and pp_method_ : Format.formatter -> method_ -> Ppx_deriving_runtime.unit =
  let __11 () = pp_pos
  
  and __10 () = pp_fun_kind
  
  and __9 () = pp_hint
  
  and __8 () = pp_user_attribute
  
  and __7 () = pp_block
  
  and __6 () = pp_fun_param
  
  and __5 () = pp_id
  
  and __4 () = pp_hint
  
  and __3 () = pp_constraint_kind
  
  and __2 () = pp_hint
  
  and __1 () = pp_tparam
  
  and __0 () = pp_kind
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((((((((Format.fprintf fmt "@[%s =@ " "m_kind";
                      ((fun x  ->
                          Format.fprintf fmt "@[<2>[";
                          ignore
                            (List.fold_left
                               (fun sep  ->
                                  fun x  ->
                                    if sep then Format.fprintf fmt ";@ ";
                                    ((__0 ()) fmt) x;
                                    true) false x);
                          Format.fprintf fmt "@,]@]")) x.m_kind;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "m_tparams";
                     ((fun x  ->
                         Format.fprintf fmt "@[<2>[";
                         ignore
                           (List.fold_left
                              (fun sep  ->
                                 fun x  ->
                                   if sep then Format.fprintf fmt ";@ ";
                                   ((__1 ()) fmt) x;
                                   true) false x);
                         Format.fprintf fmt "@,]@]")) x.m_tparams;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "m_constrs";
                    ((fun x  ->
                        Format.fprintf fmt "@[<2>[";
                        ignore
                          (List.fold_left
                             (fun sep  ->
                                fun x  ->
                                  if sep then Format.fprintf fmt ";@ ";
                                  ((fun (a0,a1,a2)  ->
                                      Format.fprintf fmt "(@[";
                                      ((((__2 ()) fmt) a0;
                                        Format.fprintf fmt ",@ ";
                                        ((__3 ()) fmt) a1);
                                       Format.fprintf fmt ",@ ";
                                       ((__4 ()) fmt) a2);
                                      Format.fprintf fmt "@])")) x;
                                  true) false x);
                        Format.fprintf fmt "@,]@]")) x.m_constrs;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "m_name";
                   ((__5 ()) fmt) x.m_name;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "m_params";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__6 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) x.m_params;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "m_body";
                 ((__7 ()) fmt) x.m_body;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "m_user_attributes";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__8 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.m_user_attributes;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "m_ret";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__9 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.m_ret;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "m_ret_by_ref";
              (Format.fprintf fmt "%B") x.m_ret_by_ref;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "m_fun_kind";
             ((__10 ()) fmt) x.m_fun_kind;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "m_span";
            ((__11 ()) fmt) x.m_span;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "m_doc_comment";
           ((function
             | None  -> Format.pp_print_string fmt "None"
             | Some x ->
                 (Format.pp_print_string fmt "(Some ";
                  (Format.fprintf fmt "%S") x;
                  Format.pp_print_string fmt ")"))) x.m_doc_comment;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_method_ : method_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_method_ x

and pp_typeconst : Format.formatter -> typeconst -> Ppx_deriving_runtime.unit
  =
  let __4 () = pp_pos
  
  and __3 () = pp_hint
  
  and __2 () = pp_hint
  
  and __1 () = pp_tparam
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((Format.fprintf fmt "@[%s =@ " "tconst_abstract";
                (Format.fprintf fmt "%B") x.tconst_abstract;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "tconst_name";
               ((__0 ()) fmt) x.tconst_name;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "tconst_tparams";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__1 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) x.tconst_tparams;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "tconst_constraint";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__2 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) x.tconst_constraint;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "tconst_type";
            ((function
              | None  -> Format.pp_print_string fmt "None"
              | Some x ->
                  (Format.pp_print_string fmt "(Some ";
                   ((__3 ()) fmt) x;
                   Format.pp_print_string fmt ")"))) x.tconst_type;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "tconst_span";
           ((__4 ()) fmt) x.tconst_span;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_typeconst : typeconst -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_typeconst x

and (pp_is_reference :
      Format.formatter -> is_reference -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_reference : is_reference -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_reference x

and (pp_is_variadic :
      Format.formatter -> is_variadic -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_variadic : is_variadic -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_variadic x

and pp_fun_param : Format.formatter -> fun_param -> Ppx_deriving_runtime.unit
  =
  let __7 () = pp_user_attribute
  
  and __6 () = pp_param_kind
  
  and __5 () = pp_kind
  
  and __4 () = pp_expr
  
  and __3 () = pp_id
  
  and __2 () = pp_is_variadic
  
  and __1 () = pp_is_reference
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((((Format.fprintf fmt "@[%s =@ " "param_hint";
                  ((function
                    | None  -> Format.pp_print_string fmt "None"
                    | Some x ->
                        (Format.pp_print_string fmt "(Some ";
                         ((__0 ()) fmt) x;
                         Format.pp_print_string fmt ")"))) x.param_hint;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "param_is_reference";
                 ((__1 ()) fmt) x.param_is_reference;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "param_is_variadic";
                ((__2 ()) fmt) x.param_is_variadic;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "param_id";
               ((__3 ()) fmt) x.param_id;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "param_expr";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__4 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) x.param_expr;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "param_modifier";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__5 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) x.param_modifier;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "param_callconv";
            ((function
              | None  -> Format.pp_print_string fmt "None"
              | Some x ->
                  (Format.pp_print_string fmt "(Some ";
                   ((__6 ()) fmt) x;
                   Format.pp_print_string fmt ")"))) x.param_callconv;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "param_user_attributes";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((__7 ()) fmt) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) x.param_user_attributes;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_fun_param : fun_param -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_param x

and pp_fun_ : Format.formatter -> fun_ -> Ppx_deriving_runtime.unit =
  let __12 () = pp_pos
  
  and __11 () = pp_nsenv
  
  and __10 () = pp_fun_kind
  
  and __9 () = pp_user_attribute
  
  and __8 () = pp_block
  
  and __7 () = pp_fun_param
  
  and __6 () = pp_id
  
  and __5 () = pp_hint
  
  and __4 () = pp_hint
  
  and __3 () = pp_constraint_kind
  
  and __2 () = pp_hint
  
  and __1 () = pp_tparam
  
  and __0 () = pp_fimode
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((((((((((((Format.fprintf fmt "@[%s =@ " "f_mode";
                        ((__0 ()) fmt) x.f_mode;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "f_tparams";
                       ((fun x  ->
                           Format.fprintf fmt "@[<2>[";
                           ignore
                             (List.fold_left
                                (fun sep  ->
                                   fun x  ->
                                     if sep then Format.fprintf fmt ";@ ";
                                     ((__1 ()) fmt) x;
                                     true) false x);
                           Format.fprintf fmt "@,]@]")) x.f_tparams;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "f_constrs";
                      ((fun x  ->
                          Format.fprintf fmt "@[<2>[";
                          ignore
                            (List.fold_left
                               (fun sep  ->
                                  fun x  ->
                                    if sep then Format.fprintf fmt ";@ ";
                                    ((fun (a0,a1,a2)  ->
                                        Format.fprintf fmt "(@[";
                                        ((((__2 ()) fmt) a0;
                                          Format.fprintf fmt ",@ ";
                                          ((__3 ()) fmt) a1);
                                         Format.fprintf fmt ",@ ";
                                         ((__4 ()) fmt) a2);
                                        Format.fprintf fmt "@])")) x;
                                    true) false x);
                          Format.fprintf fmt "@,]@]")) x.f_constrs;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "f_ret";
                     ((function
                       | None  -> Format.pp_print_string fmt "None"
                       | Some x ->
                           (Format.pp_print_string fmt "(Some ";
                            ((__5 ()) fmt) x;
                            Format.pp_print_string fmt ")"))) x.f_ret;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "f_ret_by_ref";
                    (Format.fprintf fmt "%B") x.f_ret_by_ref;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "f_name";
                   ((__6 ()) fmt) x.f_name;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "f_params";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__7 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) x.f_params;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "f_body";
                 ((__8 ()) fmt) x.f_body;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "f_user_attributes";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__9 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.f_user_attributes;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "f_fun_kind";
               ((__10 ()) fmt) x.f_fun_kind;
               Format.fprintf fmt "@]");
              Format.fprintf fmt ";@ ";
              Format.fprintf fmt "@[%s =@ " "f_namespace";
              ((__11 ()) fmt) x.f_namespace;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "f_span";
             ((__12 ()) fmt) x.f_span;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "f_doc_comment";
            ((function
              | None  -> Format.pp_print_string fmt "None"
              | Some x ->
                  (Format.pp_print_string fmt "(Some ";
                   (Format.fprintf fmt "%S") x;
                   Format.pp_print_string fmt ")"))) x.f_doc_comment;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "f_static";
           (Format.fprintf fmt "%B") x.f_static;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_fun_ : fun_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_ x

and (pp_is_coroutine :
      Format.formatter -> is_coroutine -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_coroutine : is_coroutine -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_coroutine x

and pp_hint : Format.formatter -> hint -> Ppx_deriving_runtime.unit =
  let __1 () = pp_hint_
  
  and __0 () = pp_pos
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_hint : hint -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_hint x

and pp_variadic_hint :
  Format.formatter -> variadic_hint -> Ppx_deriving_runtime.unit =
  let __0 () = pp_hint  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Hvariadic a0 ->
            (Format.fprintf fmt "(@[<2>Hvariadic@ ";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__0 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) a0;
             Format.fprintf fmt "@])")
        | Hnon_variadic  -> Format.pp_print_string fmt "Hnon_variadic")
    [@ocaml.warning "-A"])

and show_variadic_hint : variadic_hint -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_variadic_hint x

and pp_hint_ : Format.formatter -> hint_ -> Ppx_deriving_runtime.unit =
  let __13 () = pp_hint
  
  and __12 () = pp_id
  
  and __11 () = pp_id
  
  and __10 () = pp_id
  
  and __9 () = pp_shape_info
  
  and __8 () = pp_hint
  
  and __7 () = pp_id
  
  and __6 () = pp_hint
  
  and __5 () = pp_hint
  
  and __4 () = pp_variadic_hint
  
  and __3 () = pp_param_kind
  
  and __2 () = pp_hint
  
  and __1 () = pp_is_coroutine
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Hoption a0 ->
            (Format.fprintf fmt "(@[<2>Hoption@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hfun (a0,a1,a2,a3,a4) ->
            (Format.fprintf fmt "(@[<2>Hfun (@,";
             ((((((__1 ()) fmt) a0;
                 Format.fprintf fmt ",@ ";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__2 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a1);
                Format.fprintf fmt ",@ ";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((function
                                | None  -> Format.pp_print_string fmt "None"
                                | Some x ->
                                    (Format.pp_print_string fmt "(Some ";
                                     ((__3 ()) fmt) x;
                                     Format.pp_print_string fmt ")"))) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) a2);
               Format.fprintf fmt ",@ ";
               ((__4 ()) fmt) a3);
              Format.fprintf fmt ",@ ";
              ((__5 ()) fmt) a4);
             Format.fprintf fmt "@,))@]")
        | Htuple a0 ->
            (Format.fprintf fmt "(@[<2>Htuple@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__6 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Happly (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Happly (@,";
             (((__7 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__8 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Hshape a0 ->
            (Format.fprintf fmt "(@[<2>Hshape@ ";
             ((__9 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Haccess (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Haccess (@,";
             ((((__10 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__11 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__12 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a2);
             Format.fprintf fmt "@,))@]")
        | Hsoft a0 ->
            (Format.fprintf fmt "(@[<2>Hsoft@ ";
             ((__13 ()) fmt) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_hint_ : hint_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_hint_ x

and pp_shape_info :
  Format.formatter -> shape_info -> Ppx_deriving_runtime.unit =
  let __0 () = pp_shape_field  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((Format.fprintf fmt "@[%s =@ " "si_allows_unknown_fields";
            (Format.fprintf fmt "%B") x.si_allows_unknown_fields;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "si_shape_field_list";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((__0 ()) fmt) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) x.si_shape_field_list;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_shape_info : shape_info -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_shape_info x

and pp_shape_field :
  Format.formatter -> shape_field -> Ppx_deriving_runtime.unit =
  let __1 () = pp_hint
  
  and __0 () = pp_shape_field_name
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          (((Format.fprintf fmt "@[%s =@ " "sf_optional";
             (Format.fprintf fmt "%B") x.sf_optional;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "sf_name";
            ((__0 ()) fmt) x.sf_name;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "sf_hint";
           ((__1 ()) fmt) x.sf_hint;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_shape_field : shape_field -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_shape_field x

and pp_using_stmt :
  Format.formatter -> using_stmt -> Ppx_deriving_runtime.unit =
  let __1 () = pp_block
  
  and __0 () = pp_expr
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((((Format.fprintf fmt "@[%s =@ " "us_is_block_scoped";
              (Format.fprintf fmt "%B") x.us_is_block_scoped;
              Format.fprintf fmt "@]");
             Format.fprintf fmt ";@ ";
             Format.fprintf fmt "@[%s =@ " "us_has_await";
             (Format.fprintf fmt "%B") x.us_has_await;
             Format.fprintf fmt "@]");
            Format.fprintf fmt ";@ ";
            Format.fprintf fmt "@[%s =@ " "us_expr";
            ((__0 ()) fmt) x.us_expr;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "us_block";
           ((__1 ()) fmt) x.us_block;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_using_stmt : using_stmt -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_using_stmt x

and pp_stmt : Format.formatter -> stmt -> Ppx_deriving_runtime.unit =
  let __1 () = pp_stmt_
  
  and __0 () = pp_pos
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_stmt : stmt -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_stmt x

and pp_stmt_ : Format.formatter -> stmt_ -> Ppx_deriving_runtime.unit =
  let __35 () = pp_block
  
  and __34 () = pp_expr
  
  and __33 () = pp_using_stmt
  
  and __32 () = pp_expr
  
  and __31 () = pp_pstring
  
  and __30 () = pp_def
  
  and __29 () = pp_block
  
  and __28 () = pp_catch
  
  and __27 () = pp_block
  
  and __26 () = pp_block
  
  and __25 () = pp_as_expr
  
  and __24 () = pp_pos
  
  and __23 () = pp_expr
  
  and __22 () = pp_case
  
  and __21 () = pp_expr
  
  and __20 () = pp_block
  
  and __19 () = pp_expr
  
  and __18 () = pp_expr
  
  and __17 () = pp_expr
  
  and __16 () = pp_block
  
  and __15 () = pp_expr
  
  and __14 () = pp_expr
  
  and __13 () = pp_block
  
  and __12 () = pp_block
  
  and __11 () = pp_block
  
  and __10 () = pp_expr
  
  and __9 () = pp_expr
  
  and __8 () = pp_expr
  
  and __7 () = pp_pstring
  
  and __6 () = pp_pstring
  
  and __5 () = pp_expr
  
  and __4 () = pp_expr
  
  and __3 () = pp_expr
  
  and __2 () = pp_expr
  
  and __1 () = pp_block
  
  and __0 () = pp_expr
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Unsafe  -> Format.pp_print_string fmt "Unsafe"
        | Fallthrough  -> Format.pp_print_string fmt "Fallthrough"
        | Expr a0 ->
            (Format.fprintf fmt "(@[<2>Expr@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Block a0 ->
            (Format.fprintf fmt "(@[<2>Block@ ";
             ((__1 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Break a0 ->
            (Format.fprintf fmt "(@[<2>Break@ ";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__2 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) a0;
             Format.fprintf fmt "@])")
        | Continue a0 ->
            (Format.fprintf fmt "(@[<2>Continue@ ";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__3 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) a0;
             Format.fprintf fmt "@])")
        | Throw a0 ->
            (Format.fprintf fmt "(@[<2>Throw@ ";
             ((__4 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Return a0 ->
            (Format.fprintf fmt "(@[<2>Return@ ";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__5 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) a0;
             Format.fprintf fmt "@])")
        | GotoLabel a0 ->
            (Format.fprintf fmt "(@[<2>GotoLabel@ ";
             ((__6 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Goto a0 ->
            (Format.fprintf fmt "(@[<2>Goto@ ";
             ((__7 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Static_var a0 ->
            (Format.fprintf fmt "(@[<2>Static_var@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__8 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Global_var a0 ->
            (Format.fprintf fmt "(@[<2>Global_var@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__9 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | If (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>If (@,";
             ((((__10 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__11 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((__12 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | Do (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Do (@,";
             (((__13 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__14 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | While (a0,a1) ->
            (Format.fprintf fmt "(@[<2>While (@,";
             (((__15 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__16 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | For (a0,a1,a2,a3) ->
            (Format.fprintf fmt "(@[<2>For (@,";
             (((((__17 ()) fmt) a0;
                Format.fprintf fmt ",@ ";
                ((__18 ()) fmt) a1);
               Format.fprintf fmt ",@ ";
               ((__19 ()) fmt) a2);
              Format.fprintf fmt ",@ ";
              ((__20 ()) fmt) a3);
             Format.fprintf fmt "@,))@]")
        | Switch (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Switch (@,";
             (((__21 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__22 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Foreach (a0,a1,a2,a3) ->
            (Format.fprintf fmt "(@[<2>Foreach (@,";
             (((((__23 ()) fmt) a0;
                Format.fprintf fmt ",@ ";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__24 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) a1);
               Format.fprintf fmt ",@ ";
               ((__25 ()) fmt) a2);
              Format.fprintf fmt ",@ ";
              ((__26 ()) fmt) a3);
             Format.fprintf fmt "@,))@]")
        | Try (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Try (@,";
             ((((__27 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__28 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a1);
              Format.fprintf fmt ",@ ";
              ((__29 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | Def_inline a0 ->
            (Format.fprintf fmt "(@[<2>Def_inline@ ";
             ((__30 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Noop  -> Format.pp_print_string fmt "Noop"
        | Markup (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Markup (@,";
             (((__31 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__32 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a1);
             Format.fprintf fmt "@,))@]")
        | Using a0 ->
            (Format.fprintf fmt "(@[<2>Using@ ";
             ((__33 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Declare (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Declare (@,";
             (((Format.fprintf fmt "%B") a0;
               Format.fprintf fmt ",@ ";
               ((__34 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((__35 ()) fmt) a2);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_stmt_ : stmt_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_stmt_ x

and pp_as_expr : Format.formatter -> as_expr -> Ppx_deriving_runtime.unit =
  let __2 () = pp_expr
  
  and __1 () = pp_expr
  
  and __0 () = pp_expr
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | As_v a0 ->
            (Format.fprintf fmt "(@[<2>As_v@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | As_kv (a0,a1) ->
            (Format.fprintf fmt "(@[<2>As_kv (@,";
             (((__1 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__2 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_as_expr : as_expr -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_as_expr x

and pp_xhp_attribute :
  Format.formatter -> xhp_attribute -> Ppx_deriving_runtime.unit =
  let __2 () = pp_expr
  
  and __1 () = pp_expr
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Xhp_simple (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Xhp_simple (@,";
             (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Xhp_spread a0 ->
            (Format.fprintf fmt "(@[<2>Xhp_spread@ ";
             ((__2 ()) fmt) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_xhp_attribute : xhp_attribute -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_xhp_attribute x

and pp_block : Format.formatter -> block -> Ppx_deriving_runtime.unit =
  let __0 () = pp_stmt  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>[";
          ignore
            (List.fold_left
               (fun sep  ->
                  fun x  ->
                    if sep then Format.fprintf fmt ";@ ";
                    ((__0 ()) fmt) x;
                    true) false x);
          Format.fprintf fmt "@,]@]")
    [@ocaml.warning "-A"])

and show_block : block -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_block x

and pp_expr : Format.formatter -> expr -> Ppx_deriving_runtime.unit =
  let __1 () = pp_expr_
  
  and __0 () = pp_pos
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_expr : expr -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_expr x

and pp_expr_ : Format.formatter -> expr_ -> Ppx_deriving_runtime.unit =
  let __76 () = pp_expr
  
  and __75 () = pp_expr
  
  and __74 () = pp_param_kind
  
  and __73 () = pp_expr
  
  and __72 () = pp_import_flavor
  
  and __71 () = pp_expr
  
  and __70 () = pp_expr
  
  and __69 () = pp_xhp_attribute
  
  and __68 () = pp_id
  
  and __67 () = pp_fun_
  
  and __66 () = pp_id
  
  and __65 () = pp_fun_
  
  and __64 () = pp_class_
  
  and __63 () = pp_expr
  
  and __62 () = pp_expr
  
  and __61 () = pp_expr
  
  and __60 () = pp_expr
  
  and __59 () = pp_expr
  
  and __58 () = pp_expr
  
  and __57 () = pp_expr
  
  and __56 () = pp_hint
  
  and __55 () = pp_expr
  
  and __54 () = pp_hint
  
  and __53 () = pp_expr
  
  and __52 () = pp_expr
  
  and __51 () = pp_expr
  
  and __50 () = pp_expr
  
  and __49 () = pp_expr
  
  and __48 () = pp_expr
  
  and __47 () = pp_expr
  
  and __46 () = pp_expr
  
  and __45 () = pp_expr
  
  and __44 () = pp_expr
  
  and __43 () = pp_expr
  
  and __42 () = pp_expr
  
  and __41 () = pp_bop
  
  and __40 () = pp_expr
  
  and __39 () = pp_uop
  
  and __38 () = pp_expr
  
  and __37 () = pp_hint
  
  and __36 () = pp_expr
  
  and __35 () = pp_expr
  
  and __34 () = pp_expr
  
  and __33 () = pp_expr
  
  and __32 () = pp_expr
  
  and __31 () = pp_afield
  
  and __30 () = pp_expr
  
  and __29 () = pp_pstring
  
  and __28 () = pp_pstring
  
  and __27 () = pp_pstring
  
  and __26 () = pp_expr
  
  and __25 () = pp_expr
  
  and __24 () = pp_hint
  
  and __23 () = pp_expr
  
  and __22 () = pp_pstring
  
  and __21 () = pp_expr
  
  and __20 () = pp_expr
  
  and __19 () = pp_expr
  
  and __18 () = pp_expr
  
  and __17 () = pp_expr
  
  and __16 () = pp_og_null_flavor
  
  and __15 () = pp_expr
  
  and __14 () = pp_expr
  
  and __13 () = pp_expr
  
  and __12 () = pp_expr
  
  and __11 () = pp_id
  
  and __10 () = pp_hint
  
  and __9 () = pp_id
  
  and __8 () = pp_id
  
  and __7 () = pp_afield
  
  and __6 () = pp_id
  
  and __5 () = pp_expr
  
  and __4 () = pp_shape_field_name
  
  and __3 () = pp_expr
  
  and __2 () = pp_expr
  
  and __1 () = pp_expr
  
  and __0 () = pp_afield
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Array a0 ->
            (Format.fprintf fmt "(@[<2>Array@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__0 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Varray a0 ->
            (Format.fprintf fmt "(@[<2>Varray@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__1 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Darray a0 ->
            (Format.fprintf fmt "(@[<2>Darray@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((fun (a0,a1)  ->
                               Format.fprintf fmt "(@[";
                               (((__2 ()) fmt) a0;
                                Format.fprintf fmt ",@ ";
                                ((__3 ()) fmt) a1);
                               Format.fprintf fmt "@])")) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Shape a0 ->
            (Format.fprintf fmt "(@[<2>Shape@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((fun (a0,a1)  ->
                               Format.fprintf fmt "(@[";
                               (((__4 ()) fmt) a0;
                                Format.fprintf fmt ",@ ";
                                ((__5 ()) fmt) a1);
                               Format.fprintf fmt "@])")) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Collection (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Collection (@,";
             (((__6 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__7 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Null  -> Format.pp_print_string fmt "Null"
        | True  -> Format.pp_print_string fmt "True"
        | False  -> Format.pp_print_string fmt "False"
        | Omitted  -> Format.pp_print_string fmt "Omitted"
        | Id a0 ->
            (Format.fprintf fmt "(@[<2>Id@ ";
             ((__8 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Id_type_arguments (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Id_type_arguments (@,";
             (((__9 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__10 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Lvar a0 ->
            (Format.fprintf fmt "(@[<2>Lvar@ ";
             ((__11 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Dollar a0 ->
            (Format.fprintf fmt "(@[<2>Dollar@ ";
             ((__12 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Clone a0 ->
            (Format.fprintf fmt "(@[<2>Clone@ ";
             ((__13 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Obj_get (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Obj_get (@,";
             ((((__14 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__15 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((__16 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | Array_get (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Array_get (@,";
             (((__17 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__18 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a1);
             Format.fprintf fmt "@,))@]")
        | Class_get (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Class_get (@,";
             (((__19 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__20 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Class_const (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Class_const (@,";
             (((__21 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__22 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Call (a0,a1,a2,a3) ->
            (Format.fprintf fmt "(@[<2>Call (@,";
             (((((__23 ()) fmt) a0;
                Format.fprintf fmt ",@ ";
                ((fun x  ->
                    Format.fprintf fmt "@[<2>[";
                    ignore
                      (List.fold_left
                         (fun sep  ->
                            fun x  ->
                              if sep then Format.fprintf fmt ";@ ";
                              ((__24 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) a1);
               Format.fprintf fmt ",@ ";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__25 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a2);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__26 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a3);
             Format.fprintf fmt "@,))@]")
        | Int a0 ->
            (Format.fprintf fmt "(@[<2>Int@ ";
             ((__27 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Float a0 ->
            (Format.fprintf fmt "(@[<2>Float@ ";
             ((__28 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | String a0 ->
            (Format.fprintf fmt "(@[<2>String@ ";
             ((__29 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | String2 a0 ->
            (Format.fprintf fmt "(@[<2>String2@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__30 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Yield a0 ->
            (Format.fprintf fmt "(@[<2>Yield@ ";
             ((__31 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Yield_break  -> Format.pp_print_string fmt "Yield_break"
        | Yield_from a0 ->
            (Format.fprintf fmt "(@[<2>Yield_from@ ";
             ((__32 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Await a0 ->
            (Format.fprintf fmt "(@[<2>Await@ ";
             ((__33 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Suspend a0 ->
            (Format.fprintf fmt "(@[<2>Suspend@ ";
             ((__34 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | List a0 ->
            (Format.fprintf fmt "(@[<2>List@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__35 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Expr_list a0 ->
            (Format.fprintf fmt "(@[<2>Expr_list@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__36 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Cast (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Cast (@,";
             (((__37 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__38 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Unop (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Unop (@,";
             (((__39 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__40 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Binop (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Binop (@,";
             ((((__41 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__42 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              ((__43 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | Pipe (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Pipe (@,";
             (((__44 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__45 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Eif (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Eif (@,";
             ((((__46 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__47 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) a1);
              Format.fprintf fmt ",@ ";
              ((__48 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | NullCoalesce (a0,a1) ->
            (Format.fprintf fmt "(@[<2>NullCoalesce (@,";
             (((__49 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__50 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | InstanceOf (a0,a1) ->
            (Format.fprintf fmt "(@[<2>InstanceOf (@,";
             (((__51 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__52 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Is (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Is (@,";
             (((__53 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__54 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | As (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>As (@,";
             ((((__55 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((__56 ()) fmt) a1);
              Format.fprintf fmt ",@ ";
              (Format.fprintf fmt "%B") a2);
             Format.fprintf fmt "@,))@]")
        | BracedExpr a0 ->
            (Format.fprintf fmt "(@[<2>BracedExpr@ ";
             ((__57 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | ParenthesizedExpr a0 ->
            (Format.fprintf fmt "(@[<2>ParenthesizedExpr@ ";
             ((__58 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | New (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>New (@,";
             ((((__59 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__60 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a1);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__61 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a2);
             Format.fprintf fmt "@,))@]")
        | NewAnonClass (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>NewAnonClass (@,";
             ((((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__62 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a0;
               Format.fprintf fmt ",@ ";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__63 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a1);
              Format.fprintf fmt ",@ ";
              ((__64 ()) fmt) a2);
             Format.fprintf fmt "@,))@]")
        | Efun (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Efun (@,";
             (((__65 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((fun (a0,a1)  ->
                                Format.fprintf fmt "(@[";
                                (((__66 ()) fmt) a0;
                                 Format.fprintf fmt ",@ ";
                                 (Format.fprintf fmt "%B") a1);
                                Format.fprintf fmt "@])")) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Lfun a0 ->
            (Format.fprintf fmt "(@[<2>Lfun@ ";
             ((__67 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Xml (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Xml (@,";
             ((((__68 ()) fmt) a0;
               Format.fprintf fmt ",@ ";
               ((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__69 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a1);
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__70 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a2);
             Format.fprintf fmt "@,))@]")
        | Unsafeexpr a0 ->
            (Format.fprintf fmt "(@[<2>Unsafeexpr@ ";
             ((__71 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Import (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Import (@,";
             (((__72 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__73 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Callconv (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Callconv (@,";
             (((__74 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__75 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Execution_operator a0 ->
            (Format.fprintf fmt "(@[<2>Execution_operator@ ";
             ((fun x  ->
                 Format.fprintf fmt "@[<2>[";
                 ignore
                   (List.fold_left
                      (fun sep  ->
                         fun x  ->
                           if sep then Format.fprintf fmt ";@ ";
                           ((__76 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_expr_ : expr_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_expr_ x

and (pp_import_flavor :
      Format.formatter -> import_flavor -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Include  -> Format.pp_print_string fmt "Include"
        | Require  -> Format.pp_print_string fmt "Require"
        | IncludeOnce  -> Format.pp_print_string fmt "IncludeOnce"
        | RequireOnce  -> Format.pp_print_string fmt "RequireOnce")
  [@ocaml.warning "-A"])

and show_import_flavor : import_flavor -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_import_flavor x

and pp_afield : Format.formatter -> afield -> Ppx_deriving_runtime.unit =
  let __2 () = pp_expr
  
  and __1 () = pp_expr
  
  and __0 () = pp_expr
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | AFvalue a0 ->
            (Format.fprintf fmt "(@[<2>AFvalue@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | AFkvalue (a0,a1) ->
            (Format.fprintf fmt "(@[<2>AFkvalue (@,";
             (((__1 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__2 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_afield : afield -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_afield x

and pp_case : Format.formatter -> case -> Ppx_deriving_runtime.unit =
  let __2 () = pp_block
  
  and __1 () = pp_expr
  
  and __0 () = pp_block
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Default a0 ->
            (Format.fprintf fmt "(@[<2>Default@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Case (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Case (@,";
             (((__1 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__2 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_case : case -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_case x

and pp_catch : Format.formatter -> catch -> Ppx_deriving_runtime.unit =
  let __2 () = pp_block
  
  and __1 () = pp_id
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1,a2)  ->
          Format.fprintf fmt "(@[";
          ((((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
           Format.fprintf fmt ",@ ";
           ((__2 ()) fmt) a2);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_catch : catch -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_catch x

and pp_field : Format.formatter -> field -> Ppx_deriving_runtime.unit =
  let __1 () = pp_expr
  
  and __0 () = pp_expr
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_field : field -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_field x

and pp_attr : Format.formatter -> attr -> Ppx_deriving_runtime.unit =
  let __1 () = pp_expr
  
  and __0 () = pp_id
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_attr : attr -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_attr x

include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] endo =
      object (self : 'self)
        inherit  [_] endo_defs
        method on_program env = self#on_list self#on_def env
        method on_nsenv env _visitors_this = _visitors_this
        method on_fimode env _visitors_this = _visitors_this
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
          let _visitors_r0 = self#on_nsenv env _visitors_c0  in
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
          let _visitors_r5 = self#on_nsenv env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_fimode env _visitors_this.t_mode  in
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
          let _visitors_r0 = self#on_fimode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_nsenv env _visitors_this.cst_namespace
             in
          let _visitors_r6 = self#on_pos env _visitors_this.cst_span  in
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
                          (Pervasives.(&&)
                             (Pervasives.(==) _visitors_this.cst_namespace
                                _visitors_r5)
                             (Pervasives.(==) _visitors_this.cst_span
                                _visitors_r6))))))
          then _visitors_this
          else
            {
              cst_mode = _visitors_r0;
              cst_kind = _visitors_r1;
              cst_name = _visitors_r2;
              cst_type = _visitors_r3;
              cst_value = _visitors_r4;
              cst_namespace = _visitors_r5;
              cst_span = _visitors_r6
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
          let _visitors_r0 = self#on_fimode env _visitors_this.c_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.c_namespace
             in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_pos env _visitors_this.c_span  in
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
                   let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_XhpCategory env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_pstring env _visitors_c1
             in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else XhpCategory (_visitors_r0, _visitors_r1)
        method on_XhpChild env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else XhpChild (_visitors_r0, _visitors_r1)
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
          | XhpCategory (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_XhpCategory env _visitors_this _visitors_c0
                _visitors_c1
          | XhpChild (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_XhpChild env _visitors_this _visitors_c0 _visitors_c1
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
            self#on_is_variadic env _visitors_this.cv_is_promoted_variadic
             in
          let _visitors_r3 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r4 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r5 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.cv_kinds _visitors_r0)
              (Pervasives.(&&)
                 (Pervasives.(==) _visitors_this.cv_hint _visitors_r1)
                 (Pervasives.(&&)
                    (Pervasives.(==) _visitors_this.cv_is_promoted_variadic
                       _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_this.cv_names _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_this.cv_doc_comment
                             _visitors_r4)
                          (Pervasives.(==) _visitors_this.cv_user_attributes
                             _visitors_r5)))))
          then _visitors_this
          else
            {
              cv_kinds = _visitors_r0;
              cv_hint = _visitors_r1;
              cv_is_promoted_variadic = _visitors_r2;
              cv_names = _visitors_r3;
              cv_doc_comment = _visitors_r4;
              cv_user_attributes = _visitors_r5
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
          let _visitors_r10 = self#on_pos env _visitors_this.m_span  in
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
          let _visitors_r5 = self#on_pos env _visitors_this.tconst_span  in
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
          let _visitors_r0 = self#on_fimode env _visitors_this.f_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.f_namespace
             in
          let _visitors_r11 = self#on_pos env _visitors_this.f_span  in
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_stmt env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_stmt_ env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
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
        method on_Break env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Break _visitors_r0
        method on_Continue env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Continue _visitors_r0
        method on_Throw env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Throw _visitors_r0
        method on_Return env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Return _visitors_r0
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
        method on_Global_var env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Global_var _visitors_r0
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
          _visitors_c2 _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(==) _visitors_c3 _visitors_r3)))
          then _visitors_this
          else For (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
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
          let _visitors_r1 = self#on_option self#on_pos env _visitors_c1  in
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
        method on_stmt_ env _visitors_this =
          match _visitors_this with
          | Unsafe  as _visitors_this -> self#on_Unsafe env _visitors_this
          | Fallthrough  as _visitors_this ->
              self#on_Fallthrough env _visitors_this
          | Expr _visitors_c0 as _visitors_this ->
              self#on_Expr env _visitors_this _visitors_c0
          | Block _visitors_c0 as _visitors_this ->
              self#on_Block env _visitors_this _visitors_c0
          | Break _visitors_c0 as _visitors_this ->
              self#on_Break env _visitors_this _visitors_c0
          | Continue _visitors_c0 as _visitors_this ->
              self#on_Continue env _visitors_this _visitors_c0
          | Throw _visitors_c0 as _visitors_this ->
              self#on_Throw env _visitors_this _visitors_c0
          | Return _visitors_c0 as _visitors_this ->
              self#on_Return env _visitors_this _visitors_c0
          | GotoLabel _visitors_c0 as _visitors_this ->
              self#on_GotoLabel env _visitors_this _visitors_c0
          | Goto _visitors_c0 as _visitors_this ->
              self#on_Goto env _visitors_this _visitors_c0
          | Static_var _visitors_c0 as _visitors_this ->
              self#on_Static_var env _visitors_this _visitors_c0
          | Global_var _visitors_c0 as _visitors_this ->
              self#on_Global_var env _visitors_this _visitors_c0
          | If (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_If env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
          | Do (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Do env _visitors_this _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_While env _visitors_this _visitors_c0 _visitors_c1
          | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
              _visitors_this ->
              self#on_For env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_Dollar env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Dollar _visitors_r0
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
        method on_As env _visitors_this _visitors_c0 _visitors_c1
          _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else As (_visitors_r0, _visitors_r1, _visitors_r2)
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
          | Dollar _visitors_c0 as _visitors_this ->
              self#on_Dollar env _visitors_this _visitors_c0
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
          | As (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this ->
              self#on_As env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2
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
        inherit  [_] reduce_defs
        method on_program env = self#on_list self#on_def env
        method on_nsenv env _visitors_this = self#zero
        method on_fimode env _visitors_this = self#zero
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
          let _visitors_s0 = self#on_nsenv env _visitors_c0  in _visitors_s0
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
          let _visitors_s5 = self#on_nsenv env _visitors_this.t_namespace  in
          let _visitors_s6 = self#on_fimode env _visitors_this.t_mode  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus (self#plus _visitors_s0 _visitors_s1)
                        _visitors_s2) _visitors_s3) _visitors_s4)
               _visitors_s5) _visitors_s6
        method on_gconst env _visitors_this =
          let _visitors_s0 = self#on_fimode env _visitors_this.cst_mode  in
          let _visitors_s1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_s2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_s3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_s4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_s5 = self#on_nsenv env _visitors_this.cst_namespace
             in
          let _visitors_s6 = self#on_pos env _visitors_this.cst_span  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus
                     (self#plus (self#plus _visitors_s0 _visitors_s1)
                        _visitors_s2) _visitors_s3) _visitors_s4)
               _visitors_s5) _visitors_s6
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
          let _visitors_s0 = self#on_fimode env _visitors_this.c_mode  in
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
          let _visitors_s10 = self#on_nsenv env _visitors_this.c_namespace
             in
          let _visitors_s11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_s12 = self#on_pos env _visitors_this.c_span  in
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
                   let _visitors_s0 = self#on_pos env _visitors_c0  in
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
        method on_XhpCategory env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_pstring env _visitors_c1
             in
          self#plus _visitors_s0 _visitors_s1
        method on_XhpChild env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_xhp_child env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
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
          | XhpCategory (_visitors_c0,_visitors_c1) ->
              self#on_XhpCategory env _visitors_c0 _visitors_c1
          | XhpChild (_visitors_c0,_visitors_c1) ->
              self#on_XhpChild env _visitors_c0 _visitors_c1
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
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_id env _visitors_c1  in
          let _visitors_s2 = self#on_option self#on_expr env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
        method on_class_vars_ env _visitors_this =
          let _visitors_s0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_s1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_s2 =
            self#on_is_variadic env _visitors_this.cv_is_promoted_variadic
             in
          let _visitors_s3 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_s4 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_s5 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          self#plus
            (self#plus
               (self#plus
                  (self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) _visitors_s3) _visitors_s4) _visitors_s5
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
          let _visitors_s10 = self#on_pos env _visitors_this.m_span  in
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
          let _visitors_s5 = self#on_pos env _visitors_this.tconst_span  in
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
          let _visitors_s0 = self#on_fimode env _visitors_this.f_mode  in
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
          let _visitors_s10 = self#on_nsenv env _visitors_this.f_namespace
             in
          let _visitors_s11 = self#on_pos env _visitors_this.f_span  in
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
          let _visitors_s0 = self#on_pos env _visitors_c0  in
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
        method on_stmt env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_stmt_ env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Unsafe env = self#zero
        method on_Fallthrough env = self#zero
        method on_Expr env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Block env _visitors_c0 =
          let _visitors_s0 = self#on_block env _visitors_c0  in _visitors_s0
        method on_Break env _visitors_c0 =
          let _visitors_s0 = self#on_option self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Continue env _visitors_c0 =
          let _visitors_s0 = self#on_option self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Throw env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
        method on_Return env _visitors_c0 =
          let _visitors_s0 = self#on_option self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_GotoLabel env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_Goto env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_Static_var env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
        method on_Global_var env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_expr env _visitors_c0  in
          _visitors_s0
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
          =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_expr env _visitors_c1  in
          let _visitors_s2 = self#on_expr env _visitors_c2  in
          let _visitors_s3 = self#on_block env _visitors_c3  in
          self#plus
            (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
            _visitors_s3
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_case env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_pos env _visitors_c1  in
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
        method on_stmt_ env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break _visitors_c0 -> self#on_Break env _visitors_c0
          | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return _visitors_c0 -> self#on_Return env _visitors_c0
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var _visitors_c0 -> self#on_Global_var env _visitors_c0
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
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
          let _visitors_s0 = self#on_pos env _visitors_c0  in
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
        method on_Dollar env _visitors_c0 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in _visitors_s0
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
        method on_As env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_s0 = self#on_expr env _visitors_c0  in
          let _visitors_s1 = self#on_hint env _visitors_c1  in
          let _visitors_s2 = self#on_bool env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
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
          | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
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
          | As (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_As env _visitors_c0 _visitors_c1 _visitors_c2
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
        inherit  [_] map_defs
        method on_program env = self#on_list self#on_def env
        method on_nsenv env _visitors_this = _visitors_this
        method on_fimode env _visitors_this = _visitors_this
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
          let _visitors_r0 = self#on_nsenv env _visitors_c0  in
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
          let _visitors_r5 = self#on_nsenv env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_fimode env _visitors_this.t_mode  in
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
          let _visitors_r0 = self#on_fimode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_nsenv env _visitors_this.cst_namespace
             in
          let _visitors_r6 = self#on_pos env _visitors_this.cst_span  in
          {
            cst_mode = _visitors_r0;
            cst_kind = _visitors_r1;
            cst_name = _visitors_r2;
            cst_type = _visitors_r3;
            cst_value = _visitors_r4;
            cst_namespace = _visitors_r5;
            cst_span = _visitors_r6
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
          let _visitors_r0 = self#on_fimode env _visitors_this.c_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.c_namespace
             in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_pos env _visitors_this.c_span  in
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
                   let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_XhpCategory env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_pstring env _visitors_c1
             in
          XhpCategory (_visitors_r0, _visitors_r1)
        method on_XhpChild env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in
          XhpChild (_visitors_r0, _visitors_r1)
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
          | XhpCategory (_visitors_c0,_visitors_c1) ->
              self#on_XhpCategory env _visitors_c0 _visitors_c1
          | XhpChild (_visitors_c0,_visitors_c1) ->
              self#on_XhpChild env _visitors_c0 _visitors_c1
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_expr env _visitors_c2  in
          (_visitors_r0, _visitors_r1, _visitors_r2)
        method on_class_vars_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_r2 =
            self#on_is_variadic env _visitors_this.cv_is_promoted_variadic
             in
          let _visitors_r3 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r4 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r5 =
            self#on_list self#on_user_attribute env
              _visitors_this.cv_user_attributes
             in
          {
            cv_kinds = _visitors_r0;
            cv_hint = _visitors_r1;
            cv_is_promoted_variadic = _visitors_r2;
            cv_names = _visitors_r3;
            cv_doc_comment = _visitors_r4;
            cv_user_attributes = _visitors_r5
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
          let _visitors_r10 = self#on_pos env _visitors_this.m_span  in
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
          let _visitors_r5 = self#on_pos env _visitors_this.tconst_span  in
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
          let _visitors_r0 = self#on_fimode env _visitors_this.f_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.f_namespace
             in
          let _visitors_r11 = self#on_pos env _visitors_this.f_span  in
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_stmt env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_stmt_ env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_Unsafe env = Unsafe
        method on_Fallthrough env = Fallthrough
        method on_Expr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Expr _visitors_r0
        method on_Block env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in
          Block _visitors_r0
        method on_Break env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          Break _visitors_r0
        method on_Continue env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          Continue _visitors_r0
        method on_Throw env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Throw _visitors_r0
        method on_Return env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          Return _visitors_r0
        method on_GotoLabel env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          GotoLabel _visitors_r0
        method on_Goto env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          Goto _visitors_r0
        method on_Static_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Static_var _visitors_r0
        method on_Global_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          Global_var _visitors_r0
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
          =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in
          For (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_case env _visitors_c1  in
          Switch (_visitors_r0, _visitors_r1)
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_pos env _visitors_c1  in
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
        method on_stmt_ env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break _visitors_c0 -> self#on_Break env _visitors_c0
          | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return _visitors_c0 -> self#on_Return env _visitors_c0
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var _visitors_c0 -> self#on_Global_var env _visitors_c0
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_Dollar env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          Dollar _visitors_r0
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
        method on_As env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in
          As (_visitors_r0, _visitors_r1, _visitors_r2)
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
          | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
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
          | As (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_As env _visitors_c0 _visitors_c1 _visitors_c2
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
        inherit  [_] iter_defs
        method on_program env = self#on_list self#on_def env
        method on_nsenv env _visitors_this = ()
        method on_fimode env _visitors_this = ()
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
          let _visitors_r0 = self#on_nsenv env _visitors_c0  in ()
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
          let _visitors_r5 = self#on_nsenv env _visitors_this.t_namespace  in
          let _visitors_r6 = self#on_fimode env _visitors_this.t_mode  in ()
        method on_gconst env _visitors_this =
          let _visitors_r0 = self#on_fimode env _visitors_this.cst_mode  in
          let _visitors_r1 = self#on_cst_kind env _visitors_this.cst_kind  in
          let _visitors_r2 = self#on_id env _visitors_this.cst_name  in
          let _visitors_r3 =
            self#on_option self#on_hint env _visitors_this.cst_type  in
          let _visitors_r4 = self#on_expr env _visitors_this.cst_value  in
          let _visitors_r5 = self#on_nsenv env _visitors_this.cst_namespace
             in
          let _visitors_r6 = self#on_pos env _visitors_this.cst_span  in ()
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
          let _visitors_r0 = self#on_fimode env _visitors_this.c_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.c_namespace
             in
          let _visitors_r11 =
            self#on_option self#on_enum_ env _visitors_this.c_enum  in
          let _visitors_r12 = self#on_pos env _visitors_this.c_span  in
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
                   let _visitors_r0 = self#on_pos env _visitors_c0  in
                   let _visitors_r1 = self#on_bool env _visitors_c1  in
                   let _visitors_r2 =
                     self#on_list self#on_expr env _visitors_c2  in
                   ()) env _visitors_c3
             in
          ()
        method on_Method env _visitors_c0 =
          let _visitors_r0 = self#on_method_ env _visitors_c0  in ()
        method on_XhpCategory env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_pstring env _visitors_c1
             in
          ()
        method on_XhpChild env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_xhp_child env _visitors_c1  in ()
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
          | XhpCategory (_visitors_c0,_visitors_c1) ->
              self#on_XhpCategory env _visitors_c0 _visitors_c1
          | XhpChild (_visitors_c0,_visitors_c1) ->
              self#on_XhpChild env _visitors_c0 _visitors_c1
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_id env _visitors_c1  in
          let _visitors_r2 = self#on_option self#on_expr env _visitors_c2  in
          ()
        method on_class_vars_ env _visitors_this =
          let _visitors_r0 =
            self#on_list self#on_kind env _visitors_this.cv_kinds  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.cv_hint  in
          let _visitors_r2 =
            self#on_is_variadic env _visitors_this.cv_is_promoted_variadic
             in
          let _visitors_r3 =
            self#on_list self#on_class_var env _visitors_this.cv_names  in
          let _visitors_r4 =
            self#on_option self#on_string env _visitors_this.cv_doc_comment
             in
          let _visitors_r5 =
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
          let _visitors_r10 = self#on_pos env _visitors_this.m_span  in
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
          let _visitors_r5 = self#on_pos env _visitors_this.tconst_span  in
          ()
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
          let _visitors_r0 = self#on_fimode env _visitors_this.f_mode  in
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
          let _visitors_r10 = self#on_nsenv env _visitors_this.f_namespace
             in
          let _visitors_r11 = self#on_pos env _visitors_this.f_span  in
          let _visitors_r12 =
            self#on_option self#on_string env _visitors_this.f_doc_comment
             in
          let _visitors_r13 = self#on_bool env _visitors_this.f_static  in ()
        method on_is_coroutine env = self#on_bool env
        method on_hint env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_stmt env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_stmt_ env _visitors_c1  in ()
        method on_Unsafe env = ()
        method on_Fallthrough env = ()
        method on_Expr env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Block env _visitors_c0 =
          let _visitors_r0 = self#on_block env _visitors_c0  in ()
        method on_Break env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          ()
        method on_Continue env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          ()
        method on_Throw env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
        method on_Return env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_expr env _visitors_c0  in
          ()
        method on_GotoLabel env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_Goto env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_Static_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
          ()
        method on_Global_var env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_expr env _visitors_c0  in
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
          =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_expr env _visitors_c1  in
          let _visitors_r2 = self#on_expr env _visitors_c2  in
          let _visitors_r3 = self#on_block env _visitors_c3  in ()
        method on_Switch env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_case env _visitors_c1  in
          ()
        method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
          _visitors_c3 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_pos env _visitors_c1  in
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
        method on_stmt_ env _visitors_this =
          match _visitors_this with
          | Unsafe  -> self#on_Unsafe env
          | Fallthrough  -> self#on_Fallthrough env
          | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
          | Block _visitors_c0 -> self#on_Block env _visitors_c0
          | Break _visitors_c0 -> self#on_Break env _visitors_c0
          | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
          | Throw _visitors_c0 -> self#on_Throw env _visitors_c0
          | Return _visitors_c0 -> self#on_Return env _visitors_c0
          | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
          | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
          | Static_var _visitors_c0 -> self#on_Static_var env _visitors_c0
          | Global_var _visitors_c0 -> self#on_Global_var env _visitors_c0
          | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
          | Do (_visitors_c0,_visitors_c1) ->
              self#on_Do env _visitors_c0 _visitors_c1
          | While (_visitors_c0,_visitors_c1) ->
              self#on_While env _visitors_c0 _visitors_c1
          | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
              self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3
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
          let _visitors_r0 = self#on_pos env _visitors_c0  in
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
        method on_Dollar env _visitors_c0 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in ()
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
        method on_As env _visitors_c0 _visitors_c1 _visitors_c2 =
          let _visitors_r0 = self#on_expr env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          let _visitors_r2 = self#on_bool env _visitors_c2  in ()
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
          | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
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
          | As (_visitors_c0,_visitors_c1,_visitors_c2) ->
              self#on_As env _visitors_c0 _visitors_c1 _visitors_c2
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
