(* @generated from aast.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2004-present, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<90445139d4f25220585a0ee69256b9abaad8265f>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2015, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the MIT license found in the\n * LICENSE file in the \"hack\" directory of this source tree.\n *\n "]
include Aast_defs
module type AnnotationType  =
  sig type t val pp : Format.formatter -> t -> unit end
module type ASTAnnotationTypes  =
  sig
    module ExprAnnotation : AnnotationType
    module EnvAnnotation : AnnotationType
    module ClassIdAnnotation : AnnotationType
  end
module AnnotatedAST(Annotations:ASTAnnotationTypes) =
  struct
    module ExprAnnotation = Annotations.ExprAnnotation
    module EnvAnnotation = Annotations.EnvAnnotation
    module ClassIdAnnotation = Annotations.ClassIdAnnotation
    type program = def list[@@deriving
                             ((show { with_path = false }),
                               (visitors
                                  {
                                    variety = "iter";
                                    nude = true;
                                    visit_prefix = "on_";
                                    ancestors = ["iter_defs"]
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
                                    variety = "endo";
                                    nude = true;
                                    visit_prefix = "on_";
                                    ancestors = ["endo_defs"]
                                  }))]
    and expr_annotation = ((ExprAnnotation.t)[@visitors.opaque ])
    and env_annotation = ((EnvAnnotation.t)[@visitors.opaque ])
    and class_id_annotation = ((ClassIdAnnotation.t)[@visitors.opaque ])
    and stmt =
      | Fallthrough 
      | Expr of expr 
      | Break of pos 
      | Continue of pos 
      | Throw of is_terminal * expr 
      | Return of pos * expr option 
      | GotoLabel of pstring 
      | Goto of pstring 
      | Static_var of expr list 
      | Global_var of expr list 
      | If of expr * block * block 
      | Do of block * expr 
      | While of expr * block 
      | Using of bool * expr * block 
      | For of expr * expr * expr * block 
      | Switch of expr * case list 
      | Foreach of expr * as_expr * block 
      | Try of block * catch list * block 
      | Noop 
    and as_expr =
      | As_v of expr 
      | As_kv of expr * expr 
      | Await_as_v of pos * expr 
      | Await_as_kv of pos * expr * expr 
    and block = stmt list
    and class_id = (class_id_annotation * class_id_)
    and class_id_ =
      | CIparent 
      | CIself 
      | CIstatic 
      | CIexpr of expr 
      | CI of instantiated_sid 
    and expr = (expr_annotation * expr_)
    and expr_ =
      | Array of afield list 
      | Darray of (expr * expr) list 
      | Varray of expr list 
      | Shape of expr shape_map 
      | ValCollection of vc_kind * expr list 
      | KeyValCollection of kvc_kind * field list 
      | Null 
      | This 
      | True 
      | False 
      | Id of sid 
      | Lvar of lid 
      | Dollar of expr 
      | Dollardollar of lid 
      | Clone of expr 
      | Obj_get of expr * expr * og_null_flavor 
      | Array_get of expr * expr option 
      | Class_get of class_id * pstring 
      | Class_const of class_id * pstring 
      | Call of call_type * expr * hint list * expr list * expr list 
      | Int of pstring 
      | Float of pstring 
      | String of pstring 
      | String2 of expr list 
      | Yield of afield 
      | Yield_break 
      | Await of expr 
      | Suspend of expr 
      | List of expr list 
      | Expr_list of expr list 
      | Cast of hint * expr 
      | Unop of Ast.uop * expr 
      | Binop of Ast.bop * expr * expr
      [@ocaml.doc
        " The ID of the $$ that is implicitly declared by this pipe. "]
      | Pipe of lid * expr * expr 
      | Eif of expr * expr option * expr 
      | NullCoalesce of expr * expr 
      | InstanceOf of expr * class_id 
      | Is of expr * hint 
      | As of expr * hint * bool 
      | New of class_id * expr list * expr list 
      | Efun of fun_ * lid list 
      | Xml of sid * xhp_attribute list * expr list 
      | Callconv of Ast.param_kind * expr 
      | Lplaceholder of pos 
      | Fun_id of sid 
      | Method_id of expr * pstring 
      | Method_caller of sid * pstring 
      | Smethod_id of sid * pstring 
      | Special_func of special_func 
      | Pair of expr * expr 
      | Assert of assert_expr 
      | Typename of sid 
      | Any 
    and assert_expr =
      | AE_assert of expr 
    and case =
      | Default of block 
      | Case of expr * block 
    and catch = (sid * lid * block)
    and field = (expr * expr)
    and afield =
      | AFvalue of expr 
      | AFkvalue of expr * expr 
    and xhp_attribute =
      | Xhp_simple of pstring * expr 
      | Xhp_spread of expr 
    and special_func =
      | Gena of expr 
      | Genva of expr list 
      | Gen_array_rec of expr 
    and is_reference = bool
    and is_variadic = bool
    and fun_param =
      {
      param_annotation: expr_annotation ;
      param_hint: hint option ;
      param_is_reference: is_reference ;
      param_is_variadic: is_variadic ;
      param_pos: pos ;
      param_name: string ;
      param_expr: expr option ;
      param_callconv: Ast.param_kind option ;
      param_user_attributes: user_attribute list }
    and fun_variadicity =
      | FVvariadicArg of fun_param 
      | FVellipsis 
      | FVnonVariadic 
    and fun_ =
      {
      f_annotation: env_annotation ;
      f_mode: FileInfo.mode [@opaque ];
      f_ret: hint option ;
      f_name: sid ;
      f_tparams: tparam list ;
      f_where_constraints: where_constraint list ;
      f_variadic: fun_variadicity ;
      f_params: fun_param list ;
      f_body: func_body ;
      f_fun_kind: Ast.fun_kind ;
      f_user_attributes: user_attribute list ;
      f_ret_by_ref: bool }
    and func_body =
      | UnnamedBody of func_unnamed_body 
      | NamedBody of func_named_body 
    and func_unnamed_body =
      {
      fub_ast: Ast.block [@opaque ];
      fub_tparams: Ast.tparam list [@opaque ];
      fub_namespace: Namespace_env.env [@opaque ]}
    and func_named_body = {
      fnb_nast: block ;
      fnb_unsafe: bool }
    and user_attribute = {
      ua_name: sid ;
      ua_params: expr list }
    and static_var = class_var
    and static_method = method_
    and class_ =
      {
      c_annotation: env_annotation ;
      c_mode: FileInfo.mode [@opaque ];
      c_final: bool ;
      c_is_xhp: bool ;
      c_kind: Ast.class_kind ;
      c_name: sid ;
      c_tparams: (tparam list * (Ast.constraint_kind * Ast.hint) list SMap.t)
        [@opaque ];
      c_extends: hint list ;
      c_uses: hint list ;
      c_xhp_attr_uses: hint list ;
      c_xhp_category: pstring list ;
      c_req_extends: hint list ;
      c_req_implements: hint list ;
      c_implements: hint list ;
      c_consts: class_const list ;
      c_typeconsts: class_typeconst list ;
      c_static_vars: static_var list ;
      c_vars: class_var list ;
      c_constructor: method_ option ;
      c_static_methods: static_method list ;
      c_methods: method_ list ;
      c_user_attributes: user_attribute list ;
      c_enum: enum_ option }
    and class_const = (hint option * sid * expr option)
    and class_typeconst =
      {
      c_tconst_name: sid ;
      c_tconst_constraint: hint option ;
      c_tconst_type: hint option }
    and class_var =
      {
      cv_final: bool ;
      cv_is_xhp: bool ;
      cv_visibility: visibility ;
      cv_type: hint option ;
      cv_id: sid ;
      cv_expr: expr option ;
      cv_user_attributes: user_attribute list }
    and method_ =
      {
      m_annotation: env_annotation ;
      m_final: bool ;
      m_abstract: bool ;
      m_visibility: visibility ;
      m_name: sid ;
      m_tparams: tparam list ;
      m_where_constraints: where_constraint list ;
      m_variadic: fun_variadicity ;
      m_params: fun_param list ;
      m_body: func_body ;
      m_fun_kind: Ast.fun_kind ;
      m_user_attributes: user_attribute list ;
      m_ret: hint option ;
      m_ret_by_ref: bool }
    and typedef =
      {
      t_annotation: env_annotation ;
      t_name: sid ;
      t_tparams: tparam list ;
      t_constraint: hint option ;
      t_kind: hint ;
      t_user_attributes: user_attribute list ;
      t_mode: FileInfo.mode [@opaque ];
      t_vis: typedef_visibility }
    and gconst =
      {
      cst_annotation: env_annotation ;
      cst_mode: FileInfo.mode [@opaque ];
      cst_name: sid ;
      cst_type: hint option ;
      cst_value: expr option ;
      cst_is_define: bool }
    and def =
      | Fun of fun_ 
      | Class of class_ 
      | Typedef of typedef 
      | Constant of gconst 
    let rec pp_program :
      Format.formatter -> program -> Ppx_deriving_runtime.unit =
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
    
    and pp_expr_annotation :
      Format.formatter -> expr_annotation -> Ppx_deriving_runtime.unit =
      let __0 () = ExprAnnotation.pp  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show_expr_annotation : expr_annotation -> Ppx_deriving_runtime.string
      = fun x  -> Format.asprintf "%a" pp_expr_annotation x
    
    and pp_env_annotation :
      Format.formatter -> env_annotation -> Ppx_deriving_runtime.unit =
      let __0 () = EnvAnnotation.pp  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show_env_annotation : env_annotation -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_env_annotation x
    
    and pp_class_id_annotation :
      Format.formatter -> class_id_annotation -> Ppx_deriving_runtime.unit =
      let __0 () = ClassIdAnnotation.pp  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show_class_id_annotation :
      class_id_annotation -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_id_annotation x
    
    and pp_stmt : Format.formatter -> stmt -> Ppx_deriving_runtime.unit =
      let __31 () = pp_block
      
      and __30 () = pp_catch
      
      and __29 () = pp_block
      
      and __28 () = pp_block
      
      and __27 () = pp_as_expr
      
      and __26 () = pp_expr
      
      and __25 () = pp_case
      
      and __24 () = pp_expr
      
      and __23 () = pp_block
      
      and __22 () = pp_expr
      
      and __21 () = pp_expr
      
      and __20 () = pp_expr
      
      and __19 () = pp_block
      
      and __18 () = pp_expr
      
      and __17 () = pp_block
      
      and __16 () = pp_expr
      
      and __15 () = pp_expr
      
      and __14 () = pp_block
      
      and __13 () = pp_block
      
      and __12 () = pp_block
      
      and __11 () = pp_expr
      
      and __10 () = pp_expr
      
      and __9 () = pp_expr
      
      and __8 () = pp_pstring
      
      and __7 () = pp_pstring
      
      and __6 () = pp_expr
      
      and __5 () = pp_pos
      
      and __4 () = pp_expr
      
      and __3 () = pp_is_terminal
      
      and __2 () = pp_pos
      
      and __1 () = pp_pos
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | Fallthrough  -> Format.pp_print_string fmt "Fallthrough"
            | Expr a0 ->
                (Format.fprintf fmt "(@[<2>Expr@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Break a0 ->
                (Format.fprintf fmt "(@[<2>Break@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Continue a0 ->
                (Format.fprintf fmt "(@[<2>Continue@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Throw (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Throw (@,";
                 (((__3 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__4 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Return (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Return (@,";
                 (((__5 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((function
                    | None  -> Format.pp_print_string fmt "None"
                    | Some x ->
                        (Format.pp_print_string fmt "(Some ";
                         ((__6 ()) fmt) x;
                         Format.pp_print_string fmt ")"))) a1);
                 Format.fprintf fmt "@,))@]")
            | GotoLabel a0 ->
                (Format.fprintf fmt "(@[<2>GotoLabel@ ";
                 ((__7 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Goto a0 ->
                (Format.fprintf fmt "(@[<2>Goto@ ";
                 ((__8 ()) fmt) a0;
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
                               ((__9 ()) fmt) x;
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
                               ((__10 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | If (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>If (@,";
                 ((((__11 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__12 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__13 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Do (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Do (@,";
                 (((__14 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__15 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | While (a0,a1) ->
                (Format.fprintf fmt "(@[<2>While (@,";
                 (((__16 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__17 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Using (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Using (@,";
                 (((Format.fprintf fmt "%B") a0;
                   Format.fprintf fmt ",@ ";
                   ((__18 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__19 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | For (a0,a1,a2,a3) ->
                (Format.fprintf fmt "(@[<2>For (@,";
                 (((((__20 ()) fmt) a0;
                    Format.fprintf fmt ",@ ";
                    ((__21 ()) fmt) a1);
                   Format.fprintf fmt ",@ ";
                   ((__22 ()) fmt) a2);
                  Format.fprintf fmt ",@ ";
                  ((__23 ()) fmt) a3);
                 Format.fprintf fmt "@,))@]")
            | Switch (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Switch (@,";
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
            | Foreach (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Foreach (@,";
                 ((((__26 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__27 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__28 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Try (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Try (@,";
                 ((((__29 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((fun x  ->
                       Format.fprintf fmt "@[<2>[";
                       ignore
                         (List.fold_left
                            (fun sep  ->
                               fun x  ->
                                 if sep then Format.fprintf fmt ";@ ";
                                 ((__30 ()) fmt) x;
                                 true) false x);
                       Format.fprintf fmt "@,]@]")) a1);
                  Format.fprintf fmt ",@ ";
                  ((__31 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Noop  -> Format.pp_print_string fmt "Noop")
        [@ocaml.warning "-A"])
    
    and show_stmt : stmt -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_stmt x
    
    and pp_as_expr : Format.formatter -> as_expr -> Ppx_deriving_runtime.unit
      =
      let __7 () = pp_expr
      
      and __6 () = pp_expr
      
      and __5 () = pp_pos
      
      and __4 () = pp_expr
      
      and __3 () = pp_pos
      
      and __2 () = pp_expr
      
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
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Await_as_v (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Await_as_v (@,";
                 (((__3 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__4 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Await_as_kv (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Await_as_kv (@,";
                 ((((__5 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__6 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__7 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]"))
        [@ocaml.warning "-A"])
    
    and show_as_expr : as_expr -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_as_expr x
    
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
    
    and pp_class_id :
      Format.formatter -> class_id -> Ppx_deriving_runtime.unit =
      let __1 () = pp_class_id_
      
      and __0 () = pp_class_id_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun (a0,a1)  ->
              Format.fprintf fmt "(@[";
              (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
              Format.fprintf fmt "@])")
        [@ocaml.warning "-A"])
    
    and show_class_id : class_id -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_id x
    
    and pp_class_id_ :
      Format.formatter -> class_id_ -> Ppx_deriving_runtime.unit =
      let __1 () = pp_instantiated_sid
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | CIparent  -> Format.pp_print_string fmt "CIparent"
            | CIself  -> Format.pp_print_string fmt "CIself"
            | CIstatic  -> Format.pp_print_string fmt "CIstatic"
            | CIexpr a0 ->
                (Format.fprintf fmt "(@[<2>CIexpr@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | CI a0 ->
                (Format.fprintf fmt "(@[<2>CI@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_class_id_ : class_id_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_id_ x
    
    and pp_expr : Format.formatter -> expr -> Ppx_deriving_runtime.unit =
      let __1 () = pp_expr_
      
      and __0 () = pp_expr_annotation
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
      let __81 () = pp_sid
      
      and __80 () = pp_assert_expr
      
      and __79 () = pp_expr
      
      and __78 () = pp_expr
      
      and __77 () = pp_special_func
      
      and __76 () = pp_pstring
      
      and __75 () = pp_sid
      
      and __74 () = pp_pstring
      
      and __73 () = pp_sid
      
      and __72 () = pp_pstring
      
      and __71 () = pp_expr
      
      and __70 () = pp_sid
      
      and __69 () = pp_pos
      
      and __68 () = pp_expr
      
      and __67 () = Ast.pp_param_kind
      
      and __66 () = pp_expr
      
      and __65 () = pp_xhp_attribute
      
      and __64 () = pp_sid
      
      and __63 () = pp_lid
      
      and __62 () = pp_fun_
      
      and __61 () = pp_expr
      
      and __60 () = pp_expr
      
      and __59 () = pp_class_id
      
      and __58 () = pp_hint
      
      and __57 () = pp_expr
      
      and __56 () = pp_hint
      
      and __55 () = pp_expr
      
      and __54 () = pp_class_id
      
      and __53 () = pp_expr
      
      and __52 () = pp_expr
      
      and __51 () = pp_expr
      
      and __50 () = pp_expr
      
      and __49 () = pp_expr
      
      and __48 () = pp_expr
      
      and __47 () = pp_expr
      
      and __46 () = pp_expr
      
      and __45 () = pp_lid
      
      and __44 () = pp_expr
      
      and __43 () = pp_expr
      
      and __42 () = Ast.pp_bop
      
      and __41 () = pp_expr
      
      and __40 () = Ast.pp_uop
      
      and __39 () = pp_expr
      
      and __38 () = pp_hint
      
      and __37 () = pp_expr
      
      and __36 () = pp_expr
      
      and __35 () = pp_expr
      
      and __34 () = pp_expr
      
      and __33 () = pp_afield
      
      and __32 () = pp_expr
      
      and __31 () = pp_pstring
      
      and __30 () = pp_pstring
      
      and __29 () = pp_pstring
      
      and __28 () = pp_expr
      
      and __27 () = pp_expr
      
      and __26 () = pp_hint
      
      and __25 () = pp_expr
      
      and __24 () = pp_call_type
      
      and __23 () = pp_pstring
      
      and __22 () = pp_class_id
      
      and __21 () = pp_pstring
      
      and __20 () = pp_class_id
      
      and __19 () = pp_expr
      
      and __18 () = pp_expr
      
      and __17 () = pp_og_null_flavor
      
      and __16 () = pp_expr
      
      and __15 () = pp_expr
      
      and __14 () = pp_expr
      
      and __13 () = pp_lid
      
      and __12 () = pp_expr
      
      and __11 () = pp_lid
      
      and __10 () = pp_sid
      
      and __9 () = pp_field
      
      and __8 () = pp_kvc_kind
      
      and __7 () = pp_expr
      
      and __6 () = pp_vc_kind
      
      and __5 () = pp_shape_map
      
      and __4 () = pp_expr
      
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
                                   (((__1 ()) fmt) a0;
                                    Format.fprintf fmt ",@ ";
                                    ((__2 ()) fmt) a1);
                                   Format.fprintf fmt "@])")) x;
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
                               ((__3 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | Shape a0 ->
                (Format.fprintf fmt "(@[<2>Shape@ ";
                 ((__5 ()) (fun fmt  -> (__4 ()) fmt) fmt) a0;
                 Format.fprintf fmt "@])")
            | ValCollection (a0,a1) ->
                (Format.fprintf fmt "(@[<2>ValCollection (@,";
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
            | KeyValCollection (a0,a1) ->
                (Format.fprintf fmt "(@[<2>KeyValCollection (@,";
                 (((__8 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__9 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) a1);
                 Format.fprintf fmt "@,))@]")
            | Null  -> Format.pp_print_string fmt "Null"
            | This  -> Format.pp_print_string fmt "This"
            | True  -> Format.pp_print_string fmt "True"
            | False  -> Format.pp_print_string fmt "False"
            | Id a0 ->
                (Format.fprintf fmt "(@[<2>Id@ ";
                 ((__10 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Lvar a0 ->
                (Format.fprintf fmt "(@[<2>Lvar@ ";
                 ((__11 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Dollar a0 ->
                (Format.fprintf fmt "(@[<2>Dollar@ ";
                 ((__12 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Dollardollar a0 ->
                (Format.fprintf fmt "(@[<2>Dollardollar@ ";
                 ((__13 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Clone a0 ->
                (Format.fprintf fmt "(@[<2>Clone@ ";
                 ((__14 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Obj_get (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Obj_get (@,";
                 ((((__15 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__16 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__17 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Array_get (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Array_get (@,";
                 (((__18 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((function
                    | None  -> Format.pp_print_string fmt "None"
                    | Some x ->
                        (Format.pp_print_string fmt "(Some ";
                         ((__19 ()) fmt) x;
                         Format.pp_print_string fmt ")"))) a1);
                 Format.fprintf fmt "@,))@]")
            | Class_get (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Class_get (@,";
                 (((__20 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__21 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Class_const (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Class_const (@,";
                 (((__22 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__23 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Call (a0,a1,a2,a3,a4) ->
                (Format.fprintf fmt "(@[<2>Call (@,";
                 ((((((__24 ()) fmt) a0;
                     Format.fprintf fmt ",@ ";
                     ((__25 ()) fmt) a1);
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
                        Format.fprintf fmt "@,]@]")) a2);
                   Format.fprintf fmt ",@ ";
                   ((fun x  ->
                       Format.fprintf fmt "@[<2>[";
                       ignore
                         (List.fold_left
                            (fun sep  ->
                               fun x  ->
                                 if sep then Format.fprintf fmt ";@ ";
                                 ((__27 ()) fmt) x;
                                 true) false x);
                       Format.fprintf fmt "@,]@]")) a3);
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
                      Format.fprintf fmt "@,]@]")) a4);
                 Format.fprintf fmt "@,))@]")
            | Int a0 ->
                (Format.fprintf fmt "(@[<2>Int@ ";
                 ((__29 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Float a0 ->
                (Format.fprintf fmt "(@[<2>Float@ ";
                 ((__30 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | String a0 ->
                (Format.fprintf fmt "(@[<2>String@ ";
                 ((__31 ()) fmt) a0;
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
                               ((__32 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | Yield a0 ->
                (Format.fprintf fmt "(@[<2>Yield@ ";
                 ((__33 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Yield_break  -> Format.pp_print_string fmt "Yield_break"
            | Await a0 ->
                (Format.fprintf fmt "(@[<2>Await@ ";
                 ((__34 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Suspend a0 ->
                (Format.fprintf fmt "(@[<2>Suspend@ ";
                 ((__35 ()) fmt) a0;
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
                               ((__36 ()) fmt) x;
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
                               ((__37 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | Cast (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Cast (@,";
                 (((__38 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__39 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Unop (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Unop (@,";
                 (((__40 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__41 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Binop (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Binop (@,";
                 ((((__42 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__43 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__44 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Pipe (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Pipe (@,";
                 ((((__45 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__46 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__47 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Eif (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Eif (@,";
                 ((((__48 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((function
                     | None  -> Format.pp_print_string fmt "None"
                     | Some x ->
                         (Format.pp_print_string fmt "(Some ";
                          ((__49 ()) fmt) x;
                          Format.pp_print_string fmt ")"))) a1);
                  Format.fprintf fmt ",@ ";
                  ((__50 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | NullCoalesce (a0,a1) ->
                (Format.fprintf fmt "(@[<2>NullCoalesce (@,";
                 (((__51 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__52 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | InstanceOf (a0,a1) ->
                (Format.fprintf fmt "(@[<2>InstanceOf (@,";
                 (((__53 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__54 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Is (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Is (@,";
                 (((__55 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__56 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | As (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>As (@,";
                 ((((__57 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__58 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  (Format.fprintf fmt "%B") a2);
                 Format.fprintf fmt "@,))@]")
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
            | Efun (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Efun (@,";
                 (((__62 ()) fmt) a0;
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
                 Format.fprintf fmt "@,))@]")
            | Xml (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>Xml (@,";
                 ((((__64 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((fun x  ->
                       Format.fprintf fmt "@[<2>[";
                       ignore
                         (List.fold_left
                            (fun sep  ->
                               fun x  ->
                                 if sep then Format.fprintf fmt ";@ ";
                                 ((__65 ()) fmt) x;
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
                                ((__66 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) a2);
                 Format.fprintf fmt "@,))@]")
            | Callconv (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Callconv (@,";
                 (((__67 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__68 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Lplaceholder a0 ->
                (Format.fprintf fmt "(@[<2>Lplaceholder@ ";
                 ((__69 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Fun_id a0 ->
                (Format.fprintf fmt "(@[<2>Fun_id@ ";
                 ((__70 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Method_id (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Method_id (@,";
                 (((__71 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__72 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Method_caller (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Method_caller (@,";
                 (((__73 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__74 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Smethod_id (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Smethod_id (@,";
                 (((__75 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__76 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Special_func a0 ->
                (Format.fprintf fmt "(@[<2>Special_func@ ";
                 ((__77 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Pair (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Pair (@,";
                 (((__78 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__79 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Assert a0 ->
                (Format.fprintf fmt "(@[<2>Assert@ ";
                 ((__80 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Typename a0 ->
                (Format.fprintf fmt "(@[<2>Typename@ ";
                 ((__81 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Any  -> Format.pp_print_string fmt "Any")
        [@ocaml.warning "-A"])
    
    and show_expr_ : expr_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_expr_ x
    
    and pp_assert_expr :
      Format.formatter -> assert_expr -> Ppx_deriving_runtime.unit =
      let __0 () = pp_expr  in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | AE_assert a0 ->
                (Format.fprintf fmt "(@[<2>AE_assert@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_assert_expr : assert_expr -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_assert_expr x
    
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
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]"))
        [@ocaml.warning "-A"])
    
    and show_case : case -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_case x
    
    and pp_catch : Format.formatter -> catch -> Ppx_deriving_runtime.unit =
      let __2 () = pp_block
      
      and __1 () = pp_lid
      
      and __0 () = pp_sid
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun (a0,a1,a2)  ->
              Format.fprintf fmt "(@[";
              ((((__0 ()) fmt) a0;
                Format.fprintf fmt ",@ ";
                ((__1 ()) fmt) a1);
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
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]"))
        [@ocaml.warning "-A"])
    
    and show_afield : afield -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_afield x
    
    and pp_xhp_attribute :
      Format.formatter -> xhp_attribute -> Ppx_deriving_runtime.unit =
      let __2 () = pp_expr
      
      and __1 () = pp_expr
      
      and __0 () = pp_pstring
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | Xhp_simple (a0,a1) ->
                (Format.fprintf fmt "(@[<2>Xhp_simple (@,";
                 (((__0 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__1 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Xhp_spread a0 ->
                (Format.fprintf fmt "(@[<2>Xhp_spread@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_xhp_attribute : xhp_attribute -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_xhp_attribute x
    
    and pp_special_func :
      Format.formatter -> special_func -> Ppx_deriving_runtime.unit =
      let __2 () = pp_expr
      
      and __1 () = pp_expr
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | Gena a0 ->
                (Format.fprintf fmt "(@[<2>Gena@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Genva a0 ->
                (Format.fprintf fmt "(@[<2>Genva@ ";
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
            | Gen_array_rec a0 ->
                (Format.fprintf fmt "(@[<2>Gen_array_rec@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_special_func : special_func -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_special_func x
    
    and (pp_is_reference :
          Format.formatter -> is_reference -> Ppx_deriving_runtime.unit)
      =
      ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
      [@ocaml.warning "-A"])
    
    and show_is_reference : is_reference -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_is_reference x
    
    and (pp_is_variadic :
          Format.formatter -> is_variadic -> Ppx_deriving_runtime.unit)
      =
      ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
      [@ocaml.warning "-A"])
    
    and show_is_variadic : is_variadic -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_is_variadic x
    
    and pp_fun_param :
      Format.formatter -> fun_param -> Ppx_deriving_runtime.unit =
      let __7 () = pp_user_attribute
      
      and __6 () = Ast.pp_param_kind
      
      and __5 () = pp_expr
      
      and __4 () = pp_pos
      
      and __3 () = pp_is_variadic
      
      and __2 () = pp_is_reference
      
      and __1 () = pp_hint
      
      and __0 () = pp_expr_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((((((((Format.fprintf fmt "@[%s =@ "
                         "AnnotatedAST.param_annotation";
                       ((__0 ()) fmt) x.param_annotation;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "param_hint";
                      ((function
                        | None  -> Format.pp_print_string fmt "None"
                        | Some x ->
                            (Format.pp_print_string fmt "(Some ";
                             ((__1 ()) fmt) x;
                             Format.pp_print_string fmt ")"))) x.param_hint;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "param_is_reference";
                     ((__2 ()) fmt) x.param_is_reference;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "param_is_variadic";
                    ((__3 ()) fmt) x.param_is_variadic;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "param_pos";
                   ((__4 ()) fmt) x.param_pos;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "param_name";
                  (Format.fprintf fmt "%S") x.param_name;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "param_expr";
                 ((function
                   | None  -> Format.pp_print_string fmt "None"
                   | Some x ->
                       (Format.pp_print_string fmt "(Some ";
                        ((__5 ()) fmt) x;
                        Format.pp_print_string fmt ")"))) x.param_expr;
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
    
    and pp_fun_variadicity :
      Format.formatter -> fun_variadicity -> Ppx_deriving_runtime.unit =
      let __0 () = pp_fun_param  in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | FVvariadicArg a0 ->
                (Format.fprintf fmt "(@[<2>FVvariadicArg@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | FVellipsis  -> Format.pp_print_string fmt "FVellipsis"
            | FVnonVariadic  -> Format.pp_print_string fmt "FVnonVariadic")
        [@ocaml.warning "-A"])
    
    and show_fun_variadicity : fun_variadicity -> Ppx_deriving_runtime.string
      = fun x  -> Format.asprintf "%a" pp_fun_variadicity x
    
    and pp_fun_ : Format.formatter -> fun_ -> Ppx_deriving_runtime.unit =
      let __9 () = pp_user_attribute
      
      and __8 () = Ast.pp_fun_kind
      
      and __7 () = pp_func_body
      
      and __6 () = pp_fun_param
      
      and __5 () = pp_fun_variadicity
      
      and __4 () = pp_where_constraint
      
      and __3 () = pp_tparam
      
      and __2 () = pp_sid
      
      and __1 () = pp_hint
      
      and __0 () = pp_env_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((((((Format.fprintf fmt "@[%s =@ "
                            "AnnotatedAST.f_annotation";
                          ((__0 ()) fmt) x.f_annotation;
                          Format.fprintf fmt "@]");
                         Format.fprintf fmt ";@ ";
                         Format.fprintf fmt "@[%s =@ " "f_mode";
                         ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                           x.f_mode;
                         Format.fprintf fmt "@]");
                        Format.fprintf fmt ";@ ";
                        Format.fprintf fmt "@[%s =@ " "f_ret";
                        ((function
                          | None  -> Format.pp_print_string fmt "None"
                          | Some x ->
                              (Format.pp_print_string fmt "(Some ";
                               ((__1 ()) fmt) x;
                               Format.pp_print_string fmt ")"))) x.f_ret;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "f_name";
                       ((__2 ()) fmt) x.f_name;
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
                                    ((__3 ()) fmt) x;
                                    true) false x);
                          Format.fprintf fmt "@,]@]")) x.f_tparams;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "f_where_constraints";
                     ((fun x  ->
                         Format.fprintf fmt "@[<2>[";
                         ignore
                           (List.fold_left
                              (fun sep  ->
                                 fun x  ->
                                   if sep then Format.fprintf fmt ";@ ";
                                   ((__4 ()) fmt) x;
                                   true) false x);
                         Format.fprintf fmt "@,]@]")) x.f_where_constraints;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "f_variadic";
                    ((__5 ()) fmt) x.f_variadic;
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
                                 ((__6 ()) fmt) x;
                                 true) false x);
                       Format.fprintf fmt "@,]@]")) x.f_params;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "f_body";
                  ((__7 ()) fmt) x.f_body;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "f_fun_kind";
                 ((__8 ()) fmt) x.f_fun_kind;
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
               Format.fprintf fmt "@[%s =@ " "f_ret_by_ref";
               (Format.fprintf fmt "%B") x.f_ret_by_ref;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_fun_ : fun_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_fun_ x
    
    and pp_func_body :
      Format.formatter -> func_body -> Ppx_deriving_runtime.unit =
      let __1 () = pp_func_named_body
      
      and __0 () = pp_func_unnamed_body
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | UnnamedBody a0 ->
                (Format.fprintf fmt "(@[<2>UnnamedBody@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | NamedBody a0 ->
                (Format.fprintf fmt "(@[<2>NamedBody@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_func_body : func_body -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_func_body x
    
    and (pp_func_unnamed_body :
          Format.formatter -> func_unnamed_body -> Ppx_deriving_runtime.unit)
      =
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.fub_ast";
                 ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                   x.fub_ast;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "fub_tparams";
                ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                  x.fub_tparams;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "fub_namespace";
               ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                 x.fub_namespace;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
      [@ocaml.warning "-A"])
    
    and show_func_unnamed_body :
      func_unnamed_body -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_func_unnamed_body x
    
    and pp_func_named_body :
      Format.formatter -> func_named_body -> Ppx_deriving_runtime.unit =
      let __0 () = pp_block  in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.fnb_nast";
                ((__0 ()) fmt) x.fnb_nast;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "fnb_unsafe";
               (Format.fprintf fmt "%B") x.fnb_unsafe;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_func_named_body : func_named_body -> Ppx_deriving_runtime.string
      = fun x  -> Format.asprintf "%a" pp_func_named_body x
    
    and pp_user_attribute :
      Format.formatter -> user_attribute -> Ppx_deriving_runtime.unit =
      let __1 () = pp_expr
      
      and __0 () = pp_sid
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.ua_name";
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
    
    and pp_static_var :
      Format.formatter -> static_var -> Ppx_deriving_runtime.unit =
      let __0 () = pp_class_var  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show_static_var : static_var -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_static_var x
    
    and pp_static_method :
      Format.formatter -> static_method -> Ppx_deriving_runtime.unit =
      let __0 () = pp_method_  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show_static_method : static_method -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_static_method x
    
    and pp_class_ : Format.formatter -> class_ -> Ppx_deriving_runtime.unit =
      let __18 () = pp_enum_
      
      and __17 () = pp_user_attribute
      
      and __16 () = pp_method_
      
      and __15 () = pp_static_method
      
      and __14 () = pp_method_
      
      and __13 () = pp_class_var
      
      and __12 () = pp_static_var
      
      and __11 () = pp_class_typeconst
      
      and __10 () = pp_class_const
      
      and __9 () = pp_hint
      
      and __8 () = pp_hint
      
      and __7 () = pp_hint
      
      and __6 () = pp_pstring
      
      and __5 () = pp_hint
      
      and __4 () = pp_hint
      
      and __3 () = pp_hint
      
      and __2 () = pp_sid
      
      and __1 () = Ast.pp_class_kind
      
      and __0 () = pp_env_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((((((((((((((((((((((Format.fprintf fmt "@[%s =@ "
                                       "AnnotatedAST.c_annotation";
                                     ((__0 ()) fmt) x.c_annotation;
                                     Format.fprintf fmt "@]");
                                    Format.fprintf fmt ";@ ";
                                    Format.fprintf fmt "@[%s =@ " "c_mode";
                                    ((fun _  ->
                                        Format.pp_print_string fmt "<opaque>"))
                                      x.c_mode;
                                    Format.fprintf fmt "@]");
                                   Format.fprintf fmt ";@ ";
                                   Format.fprintf fmt "@[%s =@ " "c_final";
                                   (Format.fprintf fmt "%B") x.c_final;
                                   Format.fprintf fmt "@]");
                                  Format.fprintf fmt ";@ ";
                                  Format.fprintf fmt "@[%s =@ " "c_is_xhp";
                                  (Format.fprintf fmt "%B") x.c_is_xhp;
                                  Format.fprintf fmt "@]");
                                 Format.fprintf fmt ";@ ";
                                 Format.fprintf fmt "@[%s =@ " "c_kind";
                                 ((__1 ()) fmt) x.c_kind;
                                 Format.fprintf fmt "@]");
                                Format.fprintf fmt ";@ ";
                                Format.fprintf fmt "@[%s =@ " "c_name";
                                ((__2 ()) fmt) x.c_name;
                                Format.fprintf fmt "@]");
                               Format.fprintf fmt ";@ ";
                               Format.fprintf fmt "@[%s =@ " "c_tparams";
                               ((fun _  ->
                                   Format.pp_print_string fmt "<opaque>"))
                                 x.c_tparams;
                               Format.fprintf fmt "@]");
                              Format.fprintf fmt ";@ ";
                              Format.fprintf fmt "@[%s =@ " "c_extends";
                              ((fun x  ->
                                  Format.fprintf fmt "@[<2>[";
                                  ignore
                                    (List.fold_left
                                       (fun sep  ->
                                          fun x  ->
                                            if sep
                                            then Format.fprintf fmt ";@ ";
                                            ((__3 ()) fmt) x;
                                            true) false x);
                                  Format.fprintf fmt "@,]@]")) x.c_extends;
                              Format.fprintf fmt "@]");
                             Format.fprintf fmt ";@ ";
                             Format.fprintf fmt "@[%s =@ " "c_uses";
                             ((fun x  ->
                                 Format.fprintf fmt "@[<2>[";
                                 ignore
                                   (List.fold_left
                                      (fun sep  ->
                                         fun x  ->
                                           if sep
                                           then Format.fprintf fmt ";@ ";
                                           ((__4 ()) fmt) x;
                                           true) false x);
                                 Format.fprintf fmt "@,]@]")) x.c_uses;
                             Format.fprintf fmt "@]");
                            Format.fprintf fmt ";@ ";
                            Format.fprintf fmt "@[%s =@ " "c_xhp_attr_uses";
                            ((fun x  ->
                                Format.fprintf fmt "@[<2>[";
                                ignore
                                  (List.fold_left
                                     (fun sep  ->
                                        fun x  ->
                                          if sep
                                          then Format.fprintf fmt ";@ ";
                                          ((__5 ()) fmt) x;
                                          true) false x);
                                Format.fprintf fmt "@,]@]"))
                              x.c_xhp_attr_uses;
                            Format.fprintf fmt "@]");
                           Format.fprintf fmt ";@ ";
                           Format.fprintf fmt "@[%s =@ " "c_xhp_category";
                           ((fun x  ->
                               Format.fprintf fmt "@[<2>[";
                               ignore
                                 (List.fold_left
                                    (fun sep  ->
                                       fun x  ->
                                         if sep then Format.fprintf fmt ";@ ";
                                         ((__6 ()) fmt) x;
                                         true) false x);
                               Format.fprintf fmt "@,]@]")) x.c_xhp_category;
                           Format.fprintf fmt "@]");
                          Format.fprintf fmt ";@ ";
                          Format.fprintf fmt "@[%s =@ " "c_req_extends";
                          ((fun x  ->
                              Format.fprintf fmt "@[<2>[";
                              ignore
                                (List.fold_left
                                   (fun sep  ->
                                      fun x  ->
                                        if sep then Format.fprintf fmt ";@ ";
                                        ((__7 ()) fmt) x;
                                        true) false x);
                              Format.fprintf fmt "@,]@]")) x.c_req_extends;
                          Format.fprintf fmt "@]");
                         Format.fprintf fmt ";@ ";
                         Format.fprintf fmt "@[%s =@ " "c_req_implements";
                         ((fun x  ->
                             Format.fprintf fmt "@[<2>[";
                             ignore
                               (List.fold_left
                                  (fun sep  ->
                                     fun x  ->
                                       if sep then Format.fprintf fmt ";@ ";
                                       ((__8 ()) fmt) x;
                                       true) false x);
                             Format.fprintf fmt "@,]@]")) x.c_req_implements;
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
                                      ((__9 ()) fmt) x;
                                      true) false x);
                            Format.fprintf fmt "@,]@]")) x.c_implements;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "c_consts";
                       ((fun x  ->
                           Format.fprintf fmt "@[<2>[";
                           ignore
                             (List.fold_left
                                (fun sep  ->
                                   fun x  ->
                                     if sep then Format.fprintf fmt ";@ ";
                                     ((__10 ()) fmt) x;
                                     true) false x);
                           Format.fprintf fmt "@,]@]")) x.c_consts;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "c_typeconsts";
                      ((fun x  ->
                          Format.fprintf fmt "@[<2>[";
                          ignore
                            (List.fold_left
                               (fun sep  ->
                                  fun x  ->
                                    if sep then Format.fprintf fmt ";@ ";
                                    ((__11 ()) fmt) x;
                                    true) false x);
                          Format.fprintf fmt "@,]@]")) x.c_typeconsts;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "c_static_vars";
                     ((fun x  ->
                         Format.fprintf fmt "@[<2>[";
                         ignore
                           (List.fold_left
                              (fun sep  ->
                                 fun x  ->
                                   if sep then Format.fprintf fmt ";@ ";
                                   ((__12 ()) fmt) x;
                                   true) false x);
                         Format.fprintf fmt "@,]@]")) x.c_static_vars;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "c_vars";
                    ((fun x  ->
                        Format.fprintf fmt "@[<2>[";
                        ignore
                          (List.fold_left
                             (fun sep  ->
                                fun x  ->
                                  if sep then Format.fprintf fmt ";@ ";
                                  ((__13 ()) fmt) x;
                                  true) false x);
                        Format.fprintf fmt "@,]@]")) x.c_vars;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "c_constructor";
                   ((function
                     | None  -> Format.pp_print_string fmt "None"
                     | Some x ->
                         (Format.pp_print_string fmt "(Some ";
                          ((__14 ()) fmt) x;
                          Format.pp_print_string fmt ")"))) x.c_constructor;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "c_static_methods";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__15 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) x.c_static_methods;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "c_methods";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__16 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) x.c_methods;
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
                              ((__17 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.c_user_attributes;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "c_enum";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__18 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.c_enum;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_class_ : class_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_ x
    
    and pp_class_const :
      Format.formatter -> class_const -> Ppx_deriving_runtime.unit =
      let __2 () = pp_expr
      
      and __1 () = pp_sid
      
      and __0 () = pp_hint
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun (a0,a1,a2)  ->
              Format.fprintf fmt "(@[";
              ((((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__0 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) a0;
                Format.fprintf fmt ",@ ";
                ((__1 ()) fmt) a1);
               Format.fprintf fmt ",@ ";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__2 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) a2);
              Format.fprintf fmt "@])")
        [@ocaml.warning "-A"])
    
    and show_class_const : class_const -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_const x
    
    and pp_class_typeconst :
      Format.formatter -> class_typeconst -> Ppx_deriving_runtime.unit =
      let __2 () = pp_hint
      
      and __1 () = pp_hint
      
      and __0 () = pp_sid
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.c_tconst_name";
                 ((__0 ()) fmt) x.c_tconst_name;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "c_tconst_constraint";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__1 ()) fmt) x;
                       Format.pp_print_string fmt ")")))
                  x.c_tconst_constraint;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "c_tconst_type";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__2 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.c_tconst_type;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_class_typeconst : class_typeconst -> Ppx_deriving_runtime.string
      = fun x  -> Format.asprintf "%a" pp_class_typeconst x
    
    and pp_class_var :
      Format.formatter -> class_var -> Ppx_deriving_runtime.unit =
      let __4 () = pp_user_attribute
      
      and __3 () = pp_expr
      
      and __2 () = pp_sid
      
      and __1 () = pp_hint
      
      and __0 () = pp_visibility
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.cv_final";
                     (Format.fprintf fmt "%B") x.cv_final;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "cv_is_xhp";
                    (Format.fprintf fmt "%B") x.cv_is_xhp;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "cv_visibility";
                   ((__0 ()) fmt) x.cv_visibility;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "cv_type";
                  ((function
                    | None  -> Format.pp_print_string fmt "None"
                    | Some x ->
                        (Format.pp_print_string fmt "(Some ";
                         ((__1 ()) fmt) x;
                         Format.pp_print_string fmt ")"))) x.cv_type;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "cv_id";
                 ((__2 ()) fmt) x.cv_id;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "cv_expr";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__3 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) x.cv_expr;
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
    
    and show_class_var : class_var -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_var x
    
    and pp_method_ : Format.formatter -> method_ -> Ppx_deriving_runtime.unit
      =
      let __10 () = pp_hint
      
      and __9 () = pp_user_attribute
      
      and __8 () = Ast.pp_fun_kind
      
      and __7 () = pp_func_body
      
      and __6 () = pp_fun_param
      
      and __5 () = pp_fun_variadicity
      
      and __4 () = pp_where_constraint
      
      and __3 () = pp_tparam
      
      and __2 () = pp_sid
      
      and __1 () = pp_visibility
      
      and __0 () = pp_env_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((((((((Format.fprintf fmt "@[%s =@ "
                              "AnnotatedAST.m_annotation";
                            ((__0 ()) fmt) x.m_annotation;
                            Format.fprintf fmt "@]");
                           Format.fprintf fmt ";@ ";
                           Format.fprintf fmt "@[%s =@ " "m_final";
                           (Format.fprintf fmt "%B") x.m_final;
                           Format.fprintf fmt "@]");
                          Format.fprintf fmt ";@ ";
                          Format.fprintf fmt "@[%s =@ " "m_abstract";
                          (Format.fprintf fmt "%B") x.m_abstract;
                          Format.fprintf fmt "@]");
                         Format.fprintf fmt ";@ ";
                         Format.fprintf fmt "@[%s =@ " "m_visibility";
                         ((__1 ()) fmt) x.m_visibility;
                         Format.fprintf fmt "@]");
                        Format.fprintf fmt ";@ ";
                        Format.fprintf fmt "@[%s =@ " "m_name";
                        ((__2 ()) fmt) x.m_name;
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
                                     ((__3 ()) fmt) x;
                                     true) false x);
                           Format.fprintf fmt "@,]@]")) x.m_tparams;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "m_where_constraints";
                      ((fun x  ->
                          Format.fprintf fmt "@[<2>[";
                          ignore
                            (List.fold_left
                               (fun sep  ->
                                  fun x  ->
                                    if sep then Format.fprintf fmt ";@ ";
                                    ((__4 ()) fmt) x;
                                    true) false x);
                          Format.fprintf fmt "@,]@]")) x.m_where_constraints;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "m_variadic";
                     ((__5 ()) fmt) x.m_variadic;
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
                  Format.fprintf fmt "@[%s =@ " "m_fun_kind";
                  ((__8 ()) fmt) x.m_fun_kind;
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
                               ((__9 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) x.m_user_attributes;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "m_ret";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__10 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) x.m_ret;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "m_ret_by_ref";
               (Format.fprintf fmt "%B") x.m_ret_by_ref;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_method_ : method_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_method_ x
    
    and pp_typedef : Format.formatter -> typedef -> Ppx_deriving_runtime.unit
      =
      let __6 () = pp_typedef_visibility
      
      and __5 () = pp_user_attribute
      
      and __4 () = pp_hint
      
      and __3 () = pp_hint
      
      and __2 () = pp_tparam
      
      and __1 () = pp_sid
      
      and __0 () = pp_env_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((Format.fprintf fmt "@[%s =@ "
                        "AnnotatedAST.t_annotation";
                      ((__0 ()) fmt) x.t_annotation;
                      Format.fprintf fmt "@]");
                     Format.fprintf fmt ";@ ";
                     Format.fprintf fmt "@[%s =@ " "t_name";
                     ((__1 ()) fmt) x.t_name;
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
                                  ((__2 ()) fmt) x;
                                  true) false x);
                        Format.fprintf fmt "@,]@]")) x.t_tparams;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "t_constraint";
                   ((function
                     | None  -> Format.pp_print_string fmt "None"
                     | Some x ->
                         (Format.pp_print_string fmt "(Some ";
                          ((__3 ()) fmt) x;
                          Format.pp_print_string fmt ")"))) x.t_constraint;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "t_kind";
                  ((__4 ()) fmt) x.t_kind;
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
                               ((__5 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) x.t_user_attributes;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "t_mode";
                ((fun _  -> Format.pp_print_string fmt "<opaque>")) x.t_mode;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "t_vis";
               ((__6 ()) fmt) x.t_vis;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_typedef : typedef -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_typedef x
    
    and pp_gconst : Format.formatter -> gconst -> Ppx_deriving_runtime.unit =
      let __3 () = pp_expr
      
      and __2 () = pp_hint
      
      and __1 () = pp_sid
      
      and __0 () = pp_env_annotation
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((Format.fprintf fmt "@[%s =@ "
                      "AnnotatedAST.cst_annotation";
                    ((__0 ()) fmt) x.cst_annotation;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "cst_mode";
                   ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                     x.cst_mode;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "cst_name";
                  ((__1 ()) fmt) x.cst_name;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "cst_type";
                 ((function
                   | None  -> Format.pp_print_string fmt "None"
                   | Some x ->
                       (Format.pp_print_string fmt "(Some ";
                        ((__2 ()) fmt) x;
                        Format.pp_print_string fmt ")"))) x.cst_type;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "cst_value";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__3 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) x.cst_value;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "cst_is_define";
               (Format.fprintf fmt "%B") x.cst_is_define;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_gconst : gconst -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_gconst x
    
    and pp_def : Format.formatter -> def -> Ppx_deriving_runtime.unit =
      let __3 () = pp_gconst
      
      and __2 () = pp_typedef
      
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
            | Typedef a0 ->
                (Format.fprintf fmt "(@[<2>Typedef@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Constant a0 ->
                (Format.fprintf fmt "(@[<2>Constant@ ";
                 ((__3 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_def : def -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_def x
    
    include
      struct
        [@@@ocaml.warning "-4-26-27"]
        [@@@VISITORS.BEGIN ]
        class virtual ['self] iter =
          object (self : 'self)
            inherit  [_] iter_defs
            method on_program env = self#on_list self#on_def env
            method on_expr_annotation env _visitors_this = ()
            method on_env_annotation env _visitors_this = ()
            method on_class_id_annotation env _visitors_this = ()
            method on_Fallthrough env = ()
            method on_Expr env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_Break env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in ()
            method on_Continue env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in ()
            method on_Throw env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_is_terminal env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Return env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              ()
            method on_GotoLabel env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
            method on_Goto env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
            method on_Static_var env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              ()
            method on_Global_var env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Using env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_bool env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in ()
            method on_For env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              let _visitors_r3 = self#on_block env _visitors_c3  in ()
            method on_Switch env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_case env _visitors_c1
                 in
              ()
            method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_as_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in ()
            method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_block env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_catch env _visitors_c1
                 in
              let _visitors_r2 = self#on_block env _visitors_c2  in ()
            method on_Noop env = ()
            method on_stmt env _visitors_this =
              match _visitors_this with
              | Fallthrough  -> self#on_Fallthrough env
              | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
              | Break _visitors_c0 -> self#on_Break env _visitors_c0
              | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
              | Throw (_visitors_c0,_visitors_c1) ->
                  self#on_Throw env _visitors_c0 _visitors_c1
              | Return (_visitors_c0,_visitors_c1) ->
                  self#on_Return env _visitors_c0 _visitors_c1
              | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
              | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
              | Static_var _visitors_c0 ->
                  self#on_Static_var env _visitors_c0
              | Global_var _visitors_c0 ->
                  self#on_Global_var env _visitors_c0
              | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
              | Do (_visitors_c0,_visitors_c1) ->
                  self#on_Do env _visitors_c0 _visitors_c1
              | While (_visitors_c0,_visitors_c1) ->
                  self#on_While env _visitors_c0 _visitors_c1
              | Using (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Using env _visitors_c0 _visitors_c1 _visitors_c2
              | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
                  self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3
              | Switch (_visitors_c0,_visitors_c1) ->
                  self#on_Switch env _visitors_c0 _visitors_c1
              | Foreach (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
              | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
              | Noop  -> self#on_Noop env
            method on_As_v env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_As_kv env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Await_as_v env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Await_as_kv env _visitors_c0 _visitors_c1 _visitors_c2
              =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in ()
            method on_as_expr env _visitors_this =
              match _visitors_this with
              | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
              | As_kv (_visitors_c0,_visitors_c1) ->
                  self#on_As_kv env _visitors_c0 _visitors_c1
              | Await_as_v (_visitors_c0,_visitors_c1) ->
                  self#on_Await_as_v env _visitors_c0 _visitors_c1
              | Await_as_kv (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Await_as_kv env _visitors_c0 _visitors_c1
                    _visitors_c2
            method on_block env = self#on_list self#on_stmt env
            method on_class_id env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_class_id_annotation env _visitors_c0
                 in
              let _visitors_r1 = self#on_class_id_ env _visitors_c1  in ()
            method on_CIparent env = ()
            method on_CIself env = ()
            method on_CIstatic env = ()
            method on_CIexpr env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_CI env _visitors_c0 =
              let _visitors_r0 = self#on_instantiated_sid env _visitors_c0
                 in
              ()
            method on_class_id_ env _visitors_this =
              match _visitors_this with
              | CIparent  -> self#on_CIparent env
              | CIself  -> self#on_CIself env
              | CIstatic  -> self#on_CIstatic env
              | CIexpr _visitors_c0 -> self#on_CIexpr env _visitors_c0
              | CI _visitors_c0 -> self#on_CI env _visitors_c0
            method on_expr env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_expr_annotation env _visitors_c0  in
              let _visitors_r1 = self#on_expr_ env _visitors_c1  in ()
            method on_Array env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_afield env _visitors_c0
                 in
              ()
            method on_Darray env _visitors_c0 =
              let _visitors_r0 =
                self#on_list
                  (fun env  ->
                     fun (_visitors_c0,_visitors_c1)  ->
                       let _visitors_r0 = self#on_expr env _visitors_c0  in
                       let _visitors_r1 = self#on_expr env _visitors_c1  in
                       ()) env _visitors_c0
                 in
              ()
            method on_Varray env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              ()
            method on_Shape env _visitors_c0 =
              let _visitors_r0 =
                self#on_shape_map self#on_expr env _visitors_c0  in
              ()
            method on_ValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_vc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              ()
            method on_KeyValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_kvc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_field env _visitors_c1
                 in
              ()
            method on_Null env = ()
            method on_This env = ()
            method on_True env = ()
            method on_False env = ()
            method on_Id env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in ()
            method on_Lvar env _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in ()
            method on_Dollar env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_Dollardollar env _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in ()
            method on_Clone env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_og_null_flavor env _visitors_c2  in
              ()
            method on_Array_get env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              ()
            method on_Class_get env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
            method on_Class_const env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
            method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 _visitors_c4 =
              let _visitors_r0 = self#on_call_type env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_hint env _visitors_c2
                 in
              let _visitors_r3 = self#on_list self#on_expr env _visitors_c3
                 in
              let _visitors_r4 = self#on_list self#on_expr env _visitors_c4
                 in
              ()
            method on_Int env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
            method on_Float env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
            method on_String env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
            method on_String2 env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              ()
            method on_Yield env _visitors_c0 =
              let _visitors_r0 = self#on_afield env _visitors_c0  in ()
            method on_Yield_break env = ()
            method on_Await env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_Suspend env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_List env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              ()
            method on_Expr_list env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in ()
            method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_expr env _visitors_c2  in ()
            method on_NullCoalesce env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_InstanceOf env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_class_id env _visitors_c1  in ()
            method on_Is env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_hint env _visitors_c1  in ()
            method on_As env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_hint env _visitors_c1  in
              let _visitors_r2 = self#on_bool env _visitors_c2  in ()
            method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              ()
            method on_Efun env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_fun_ env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_lid env _visitors_c1
                 in
              ()
            method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 =
                self#on_list self#on_xhp_attribute env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              ()
            method on_Callconv env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_param_kind env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Lplaceholder env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in ()
            method on_Fun_id env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in ()
            method on_Method_id env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
            method on_Method_caller env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
            method on_Smethod_id env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
            method on_Special_func env _visitors_c0 =
              let _visitors_r0 = self#on_special_func env _visitors_c0  in ()
            method on_Pair env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Assert env _visitors_c0 =
              let _visitors_r0 = self#on_assert_expr env _visitors_c0  in ()
            method on_Typename env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in ()
            method on_Any env = ()
            method on_expr_ env _visitors_this =
              match _visitors_this with
              | Array _visitors_c0 -> self#on_Array env _visitors_c0
              | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
              | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
              | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
              | ValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_ValCollection env _visitors_c0 _visitors_c1
              | KeyValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_KeyValCollection env _visitors_c0 _visitors_c1
              | Null  -> self#on_Null env
              | This  -> self#on_This env
              | True  -> self#on_True env
              | False  -> self#on_False env
              | Id _visitors_c0 -> self#on_Id env _visitors_c0
              | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
              | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
              | Dollardollar _visitors_c0 ->
                  self#on_Dollardollar env _visitors_c0
              | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
              | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
              | Array_get (_visitors_c0,_visitors_c1) ->
                  self#on_Array_get env _visitors_c0 _visitors_c1
              | Class_get (_visitors_c0,_visitors_c1) ->
                  self#on_Class_get env _visitors_c0 _visitors_c1
              | Class_const (_visitors_c0,_visitors_c1) ->
                  self#on_Class_const env _visitors_c0 _visitors_c1
              | Call
                  (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
                  ->
                  self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3 _visitors_c4
              | Int _visitors_c0 -> self#on_Int env _visitors_c0
              | Float _visitors_c0 -> self#on_Float env _visitors_c0
              | String _visitors_c0 -> self#on_String env _visitors_c0
              | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
              | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
              | Yield_break  -> self#on_Yield_break env
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
              | Pipe (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2
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
              | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
              | Efun (_visitors_c0,_visitors_c1) ->
                  self#on_Efun env _visitors_c0 _visitors_c1
              | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
              | Callconv (_visitors_c0,_visitors_c1) ->
                  self#on_Callconv env _visitors_c0 _visitors_c1
              | Lplaceholder _visitors_c0 ->
                  self#on_Lplaceholder env _visitors_c0
              | Fun_id _visitors_c0 -> self#on_Fun_id env _visitors_c0
              | Method_id (_visitors_c0,_visitors_c1) ->
                  self#on_Method_id env _visitors_c0 _visitors_c1
              | Method_caller (_visitors_c0,_visitors_c1) ->
                  self#on_Method_caller env _visitors_c0 _visitors_c1
              | Smethod_id (_visitors_c0,_visitors_c1) ->
                  self#on_Smethod_id env _visitors_c0 _visitors_c1
              | Special_func _visitors_c0 ->
                  self#on_Special_func env _visitors_c0
              | Pair (_visitors_c0,_visitors_c1) ->
                  self#on_Pair env _visitors_c0 _visitors_c1
              | Assert _visitors_c0 -> self#on_Assert env _visitors_c0
              | Typename _visitors_c0 -> self#on_Typename env _visitors_c0
              | Any  -> self#on_Any env
            method on_AE_assert env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_assert_expr env _visitors_this =
              match _visitors_this with
              | AE_assert _visitors_c0 -> self#on_AE_assert env _visitors_c0
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
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_lid env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in ()
            method on_field env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
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
            method on_Xhp_simple env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in ()
            method on_Xhp_spread env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_xhp_attribute env _visitors_this =
              match _visitors_this with
              | Xhp_simple (_visitors_c0,_visitors_c1) ->
                  self#on_Xhp_simple env _visitors_c0 _visitors_c1
              | Xhp_spread _visitors_c0 ->
                  self#on_Xhp_spread env _visitors_c0
            method on_Gena env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_Genva env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              ()
            method on_Gen_array_rec env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in ()
            method on_special_func env _visitors_this =
              match _visitors_this with
              | Gena _visitors_c0 -> self#on_Gena env _visitors_c0
              | Genva _visitors_c0 -> self#on_Genva env _visitors_c0
              | Gen_array_rec _visitors_c0 ->
                  self#on_Gen_array_rec env _visitors_c0
            method on_is_reference env = self#on_bool env
            method on_is_variadic env = self#on_bool env
            method on_fun_param env _visitors_this =
              let _visitors_r0 =
                self#on_expr_annotation env _visitors_this.param_annotation
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env _visitors_this.param_hint  in
              let _visitors_r2 =
                self#on_is_reference env _visitors_this.param_is_reference
                 in
              let _visitors_r3 =
                self#on_is_variadic env _visitors_this.param_is_variadic  in
              let _visitors_r4 = self#on_pos env _visitors_this.param_pos  in
              let _visitors_r5 = self#on_string env _visitors_this.param_name
                 in
              let _visitors_r6 =
                self#on_option self#on_expr env _visitors_this.param_expr  in
              let _visitors_r7 =
                self#on_option self#on_param_kind env
                  _visitors_this.param_callconv
                 in
              let _visitors_r8 =
                self#on_list self#on_user_attribute env
                  _visitors_this.param_user_attributes
                 in
              ()
            method on_FVvariadicArg env _visitors_c0 =
              let _visitors_r0 = self#on_fun_param env _visitors_c0  in ()
            method on_FVellipsis env = ()
            method on_FVnonVariadic env = ()
            method on_fun_variadicity env _visitors_this =
              match _visitors_this with
              | FVvariadicArg _visitors_c0 ->
                  self#on_FVvariadicArg env _visitors_c0
              | FVellipsis  -> self#on_FVellipsis env
              | FVnonVariadic  -> self#on_FVnonVariadic env
            method on_fun_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.f_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> ()) _visitors_this.f_mode  in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.f_ret  in
              let _visitors_r3 = self#on_sid env _visitors_this.f_name  in
              let _visitors_r4 =
                self#on_list self#on_tparam env _visitors_this.f_tparams  in
              let _visitors_r5 =
                self#on_list self#on_where_constraint env
                  _visitors_this.f_where_constraints
                 in
              let _visitors_r6 =
                self#on_fun_variadicity env _visitors_this.f_variadic  in
              let _visitors_r7 =
                self#on_list self#on_fun_param env _visitors_this.f_params
                 in
              let _visitors_r8 = self#on_func_body env _visitors_this.f_body
                 in
              let _visitors_r9 =
                self#on_fun_kind env _visitors_this.f_fun_kind  in
              let _visitors_r10 =
                self#on_list self#on_user_attribute env
                  _visitors_this.f_user_attributes
                 in
              let _visitors_r11 =
                self#on_bool env _visitors_this.f_ret_by_ref  in
              ()
            method on_UnnamedBody env _visitors_c0 =
              let _visitors_r0 = self#on_func_unnamed_body env _visitors_c0
                 in
              ()
            method on_NamedBody env _visitors_c0 =
              let _visitors_r0 = self#on_func_named_body env _visitors_c0  in
              ()
            method on_func_body env _visitors_this =
              match _visitors_this with
              | UnnamedBody _visitors_c0 ->
                  self#on_UnnamedBody env _visitors_c0
              | NamedBody _visitors_c0 -> self#on_NamedBody env _visitors_c0
            method on_func_unnamed_body env _visitors_this =
              let _visitors_r0 =
                (fun _visitors_this  -> ()) _visitors_this.fub_ast  in
              let _visitors_r1 =
                (fun _visitors_this  -> ()) _visitors_this.fub_tparams  in
              let _visitors_r2 =
                (fun _visitors_this  -> ()) _visitors_this.fub_namespace  in
              ()
            method on_func_named_body env _visitors_this =
              let _visitors_r0 = self#on_block env _visitors_this.fnb_nast
                 in
              let _visitors_r1 = self#on_bool env _visitors_this.fnb_unsafe
                 in
              ()
            method on_user_attribute env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.ua_name  in
              let _visitors_r1 =
                self#on_list self#on_expr env _visitors_this.ua_params  in
              ()
            method on_static_var env = self#on_class_var env
            method on_static_method env = self#on_method_ env
            method on_class_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.c_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> ()) _visitors_this.c_mode  in
              let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
              let _visitors_r3 = self#on_bool env _visitors_this.c_is_xhp  in
              let _visitors_r4 = self#on_class_kind env _visitors_this.c_kind
                 in
              let _visitors_r5 = self#on_sid env _visitors_this.c_name  in
              let _visitors_r6 =
                (fun _visitors_this  -> ()) _visitors_this.c_tparams  in
              let _visitors_r7 =
                self#on_list self#on_hint env _visitors_this.c_extends  in
              let _visitors_r8 =
                self#on_list self#on_hint env _visitors_this.c_uses  in
              let _visitors_r9 =
                self#on_list self#on_hint env _visitors_this.c_xhp_attr_uses
                 in
              let _visitors_r10 =
                self#on_list self#on_pstring env
                  _visitors_this.c_xhp_category
                 in
              let _visitors_r11 =
                self#on_list self#on_hint env _visitors_this.c_req_extends
                 in
              let _visitors_r12 =
                self#on_list self#on_hint env _visitors_this.c_req_implements
                 in
              let _visitors_r13 =
                self#on_list self#on_hint env _visitors_this.c_implements  in
              let _visitors_r14 =
                self#on_list self#on_class_const env _visitors_this.c_consts
                 in
              let _visitors_r15 =
                self#on_list self#on_class_typeconst env
                  _visitors_this.c_typeconsts
                 in
              let _visitors_r16 =
                self#on_list self#on_static_var env
                  _visitors_this.c_static_vars
                 in
              let _visitors_r17 =
                self#on_list self#on_class_var env _visitors_this.c_vars  in
              let _visitors_r18 =
                self#on_option self#on_method_ env
                  _visitors_this.c_constructor
                 in
              let _visitors_r19 =
                self#on_list self#on_static_method env
                  _visitors_this.c_static_methods
                 in
              let _visitors_r20 =
                self#on_list self#on_method_ env _visitors_this.c_methods  in
              let _visitors_r21 =
                self#on_list self#on_user_attribute env
                  _visitors_this.c_user_attributes
                 in
              let _visitors_r22 =
                self#on_option self#on_enum_ env _visitors_this.c_enum  in
              ()
            method on_class_const env
              (_visitors_c0,_visitors_c1,_visitors_c2) =
              let _visitors_r0 = self#on_option self#on_hint env _visitors_c0
                 in
              let _visitors_r1 = self#on_sid env _visitors_c1  in
              let _visitors_r2 = self#on_option self#on_expr env _visitors_c2
                 in
              ()
            method on_class_typeconst env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.c_tconst_name
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env
                  _visitors_this.c_tconst_constraint
                 in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.c_tconst_type
                 in
              ()
            method on_class_var env _visitors_this =
              let _visitors_r0 = self#on_bool env _visitors_this.cv_final  in
              let _visitors_r1 = self#on_bool env _visitors_this.cv_is_xhp
                 in
              let _visitors_r2 =
                self#on_visibility env _visitors_this.cv_visibility  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cv_type  in
              let _visitors_r4 = self#on_sid env _visitors_this.cv_id  in
              let _visitors_r5 =
                self#on_option self#on_expr env _visitors_this.cv_expr  in
              let _visitors_r6 =
                self#on_list self#on_user_attribute env
                  _visitors_this.cv_user_attributes
                 in
              ()
            method on_method_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.m_annotation  in
              let _visitors_r1 = self#on_bool env _visitors_this.m_final  in
              let _visitors_r2 = self#on_bool env _visitors_this.m_abstract
                 in
              let _visitors_r3 =
                self#on_visibility env _visitors_this.m_visibility  in
              let _visitors_r4 = self#on_sid env _visitors_this.m_name  in
              let _visitors_r5 =
                self#on_list self#on_tparam env _visitors_this.m_tparams  in
              let _visitors_r6 =
                self#on_list self#on_where_constraint env
                  _visitors_this.m_where_constraints
                 in
              let _visitors_r7 =
                self#on_fun_variadicity env _visitors_this.m_variadic  in
              let _visitors_r8 =
                self#on_list self#on_fun_param env _visitors_this.m_params
                 in
              let _visitors_r9 = self#on_func_body env _visitors_this.m_body
                 in
              let _visitors_r10 =
                self#on_fun_kind env _visitors_this.m_fun_kind  in
              let _visitors_r11 =
                self#on_list self#on_user_attribute env
                  _visitors_this.m_user_attributes
                 in
              let _visitors_r12 =
                self#on_option self#on_hint env _visitors_this.m_ret  in
              let _visitors_r13 =
                self#on_bool env _visitors_this.m_ret_by_ref  in
              ()
            method on_typedef env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.t_annotation  in
              let _visitors_r1 = self#on_sid env _visitors_this.t_name  in
              let _visitors_r2 =
                self#on_list self#on_tparam env _visitors_this.t_tparams  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.t_constraint
                 in
              let _visitors_r4 = self#on_hint env _visitors_this.t_kind  in
              let _visitors_r5 =
                self#on_list self#on_user_attribute env
                  _visitors_this.t_user_attributes
                 in
              let _visitors_r6 =
                (fun _visitors_this  -> ()) _visitors_this.t_mode  in
              let _visitors_r7 =
                self#on_typedef_visibility env _visitors_this.t_vis  in
              ()
            method on_gconst env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.cst_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> ()) _visitors_this.cst_mode  in
              let _visitors_r2 = self#on_sid env _visitors_this.cst_name  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cst_type  in
              let _visitors_r4 =
                self#on_option self#on_expr env _visitors_this.cst_value  in
              let _visitors_r5 =
                self#on_bool env _visitors_this.cst_is_define  in
              ()
            method on_Fun env _visitors_c0 =
              let _visitors_r0 = self#on_fun_ env _visitors_c0  in ()
            method on_Class env _visitors_c0 =
              let _visitors_r0 = self#on_class_ env _visitors_c0  in ()
            method on_Typedef env _visitors_c0 =
              let _visitors_r0 = self#on_typedef env _visitors_c0  in ()
            method on_Constant env _visitors_c0 =
              let _visitors_r0 = self#on_gconst env _visitors_c0  in ()
            method on_def env _visitors_this =
              match _visitors_this with
              | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
              | Class _visitors_c0 -> self#on_Class env _visitors_c0
              | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
              | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
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
            method on_expr_annotation env _visitors_this = self#zero
            method on_env_annotation env _visitors_this = self#zero
            method on_class_id_annotation env _visitors_this = self#zero
            method on_Fallthrough env = self#zero
            method on_Expr env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_Break env _visitors_c0 =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              _visitors_s0
            method on_Continue env _visitors_c0 =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              _visitors_s0
            method on_Throw env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_is_terminal env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Return env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              let _visitors_s1 = self#on_option self#on_expr env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_GotoLabel env _visitors_c0 =
              let _visitors_s0 = self#on_pstring env _visitors_c0  in
              _visitors_s0
            method on_Goto env _visitors_c0 =
              let _visitors_s0 = self#on_pstring env _visitors_c0  in
              _visitors_s0
            method on_Static_var env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
              _visitors_s0
            method on_Global_var env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Using env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_bool env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_block env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_For env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_expr env _visitors_c2  in
              let _visitors_s3 = self#on_block env _visitors_c3  in
              self#plus
                (self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2)
                _visitors_s3
            method on_Switch env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_case env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_as_expr env _visitors_c1  in
              let _visitors_s2 = self#on_block env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_block env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_catch env _visitors_c1
                 in
              let _visitors_s2 = self#on_block env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Noop env = self#zero
            method on_stmt env _visitors_this =
              match _visitors_this with
              | Fallthrough  -> self#on_Fallthrough env
              | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
              | Break _visitors_c0 -> self#on_Break env _visitors_c0
              | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
              | Throw (_visitors_c0,_visitors_c1) ->
                  self#on_Throw env _visitors_c0 _visitors_c1
              | Return (_visitors_c0,_visitors_c1) ->
                  self#on_Return env _visitors_c0 _visitors_c1
              | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
              | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
              | Static_var _visitors_c0 ->
                  self#on_Static_var env _visitors_c0
              | Global_var _visitors_c0 ->
                  self#on_Global_var env _visitors_c0
              | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
              | Do (_visitors_c0,_visitors_c1) ->
                  self#on_Do env _visitors_c0 _visitors_c1
              | While (_visitors_c0,_visitors_c1) ->
                  self#on_While env _visitors_c0 _visitors_c1
              | Using (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Using env _visitors_c0 _visitors_c1 _visitors_c2
              | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
                  self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3
              | Switch (_visitors_c0,_visitors_c1) ->
                  self#on_Switch env _visitors_c0 _visitors_c1
              | Foreach (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
              | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
              | Noop  -> self#on_Noop env
            method on_As_v env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_As_kv env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Await_as_v env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Await_as_kv env _visitors_c0 _visitors_c1 _visitors_c2
              =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_expr env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_as_expr env _visitors_this =
              match _visitors_this with
              | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
              | As_kv (_visitors_c0,_visitors_c1) ->
                  self#on_As_kv env _visitors_c0 _visitors_c1
              | Await_as_v (_visitors_c0,_visitors_c1) ->
                  self#on_Await_as_v env _visitors_c0 _visitors_c1
              | Await_as_kv (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Await_as_kv env _visitors_c0 _visitors_c1
                    _visitors_c2
            method on_block env = self#on_list self#on_stmt env
            method on_class_id env (_visitors_c0,_visitors_c1) =
              let _visitors_s0 = self#on_class_id_annotation env _visitors_c0
                 in
              let _visitors_s1 = self#on_class_id_ env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_CIparent env = self#zero
            method on_CIself env = self#zero
            method on_CIstatic env = self#zero
            method on_CIexpr env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_CI env _visitors_c0 =
              let _visitors_s0 = self#on_instantiated_sid env _visitors_c0
                 in
              _visitors_s0
            method on_class_id_ env _visitors_this =
              match _visitors_this with
              | CIparent  -> self#on_CIparent env
              | CIself  -> self#on_CIself env
              | CIstatic  -> self#on_CIstatic env
              | CIexpr _visitors_c0 -> self#on_CIexpr env _visitors_c0
              | CI _visitors_c0 -> self#on_CI env _visitors_c0
            method on_expr env (_visitors_c0,_visitors_c1) =
              let _visitors_s0 = self#on_expr_annotation env _visitors_c0  in
              let _visitors_s1 = self#on_expr_ env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Array env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_afield env _visitors_c0
                 in
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
            method on_Varray env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
              _visitors_s0
            method on_Shape env _visitors_c0 =
              let _visitors_s0 =
                self#on_shape_map self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_ValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_vc_kind env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_expr env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_KeyValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_kvc_kind env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_field env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_Null env = self#zero
            method on_This env = self#zero
            method on_True env = self#zero
            method on_False env = self#zero
            method on_Id env _visitors_c0 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              _visitors_s0
            method on_Lvar env _visitors_c0 =
              let _visitors_s0 = self#on_lid env _visitors_c0  in
              _visitors_s0
            method on_Dollar env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_Dollardollar env _visitors_c0 =
              let _visitors_s0 = self#on_lid env _visitors_c0  in
              _visitors_s0
            method on_Clone env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_og_null_flavor env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Array_get env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_option self#on_expr env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_Class_get env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_class_id env _visitors_c0  in
              let _visitors_s1 = self#on_pstring env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Class_const env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_class_id env _visitors_c0  in
              let _visitors_s1 = self#on_pstring env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 _visitors_c4 =
              let _visitors_s0 = self#on_call_type env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_list self#on_hint env _visitors_c2
                 in
              let _visitors_s3 = self#on_list self#on_expr env _visitors_c3
                 in
              let _visitors_s4 = self#on_list self#on_expr env _visitors_c4
                 in
              self#plus
                (self#plus
                   (self#plus (self#plus _visitors_s0 _visitors_s1)
                      _visitors_s2) _visitors_s3) _visitors_s4
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
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
              _visitors_s0
            method on_Yield env _visitors_c0 =
              let _visitors_s0 = self#on_afield env _visitors_c0  in
              _visitors_s0
            method on_Yield_break env = self#zero
            method on_Await env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_Suspend env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_List env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
              _visitors_s0
            method on_Expr_list env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_lid env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              let _visitors_s2 = self#on_expr env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_option self#on_expr env _visitors_c1
                 in
              let _visitors_s2 = self#on_expr env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_NullCoalesce env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_InstanceOf env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_class_id env _visitors_c1  in
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
            method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_class_id env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_expr env _visitors_c1
                 in
              let _visitors_s2 = self#on_list self#on_expr env _visitors_c2
                 in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Efun env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_fun_ env _visitors_c0  in
              let _visitors_s1 = self#on_list self#on_lid env _visitors_c1
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              let _visitors_s1 =
                self#on_list self#on_xhp_attribute env _visitors_c1  in
              let _visitors_s2 = self#on_list self#on_expr env _visitors_c2
                 in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_Callconv env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_param_kind env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Lplaceholder env _visitors_c0 =
              let _visitors_s0 = self#on_pos env _visitors_c0  in
              _visitors_s0
            method on_Fun_id env _visitors_c0 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              _visitors_s0
            method on_Method_id env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_pstring env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Method_caller env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              let _visitors_s1 = self#on_pstring env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Smethod_id env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              let _visitors_s1 = self#on_pstring env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Special_func env _visitors_c0 =
              let _visitors_s0 = self#on_special_func env _visitors_c0  in
              _visitors_s0
            method on_Pair env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Assert env _visitors_c0 =
              let _visitors_s0 = self#on_assert_expr env _visitors_c0  in
              _visitors_s0
            method on_Typename env _visitors_c0 =
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              _visitors_s0
            method on_Any env = self#zero
            method on_expr_ env _visitors_this =
              match _visitors_this with
              | Array _visitors_c0 -> self#on_Array env _visitors_c0
              | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
              | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
              | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
              | ValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_ValCollection env _visitors_c0 _visitors_c1
              | KeyValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_KeyValCollection env _visitors_c0 _visitors_c1
              | Null  -> self#on_Null env
              | This  -> self#on_This env
              | True  -> self#on_True env
              | False  -> self#on_False env
              | Id _visitors_c0 -> self#on_Id env _visitors_c0
              | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
              | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
              | Dollardollar _visitors_c0 ->
                  self#on_Dollardollar env _visitors_c0
              | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
              | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
              | Array_get (_visitors_c0,_visitors_c1) ->
                  self#on_Array_get env _visitors_c0 _visitors_c1
              | Class_get (_visitors_c0,_visitors_c1) ->
                  self#on_Class_get env _visitors_c0 _visitors_c1
              | Class_const (_visitors_c0,_visitors_c1) ->
                  self#on_Class_const env _visitors_c0 _visitors_c1
              | Call
                  (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
                  ->
                  self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3 _visitors_c4
              | Int _visitors_c0 -> self#on_Int env _visitors_c0
              | Float _visitors_c0 -> self#on_Float env _visitors_c0
              | String _visitors_c0 -> self#on_String env _visitors_c0
              | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
              | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
              | Yield_break  -> self#on_Yield_break env
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
              | Pipe (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2
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
              | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
              | Efun (_visitors_c0,_visitors_c1) ->
                  self#on_Efun env _visitors_c0 _visitors_c1
              | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
              | Callconv (_visitors_c0,_visitors_c1) ->
                  self#on_Callconv env _visitors_c0 _visitors_c1
              | Lplaceholder _visitors_c0 ->
                  self#on_Lplaceholder env _visitors_c0
              | Fun_id _visitors_c0 -> self#on_Fun_id env _visitors_c0
              | Method_id (_visitors_c0,_visitors_c1) ->
                  self#on_Method_id env _visitors_c0 _visitors_c1
              | Method_caller (_visitors_c0,_visitors_c1) ->
                  self#on_Method_caller env _visitors_c0 _visitors_c1
              | Smethod_id (_visitors_c0,_visitors_c1) ->
                  self#on_Smethod_id env _visitors_c0 _visitors_c1
              | Special_func _visitors_c0 ->
                  self#on_Special_func env _visitors_c0
              | Pair (_visitors_c0,_visitors_c1) ->
                  self#on_Pair env _visitors_c0 _visitors_c1
              | Assert _visitors_c0 -> self#on_Assert env _visitors_c0
              | Typename _visitors_c0 -> self#on_Typename env _visitors_c0
              | Any  -> self#on_Any env
            method on_AE_assert env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_assert_expr env _visitors_this =
              match _visitors_this with
              | AE_assert _visitors_c0 -> self#on_AE_assert env _visitors_c0
            method on_Default env _visitors_c0 =
              let _visitors_s0 = self#on_block env _visitors_c0  in
              _visitors_s0
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
              let _visitors_s0 = self#on_sid env _visitors_c0  in
              let _visitors_s1 = self#on_lid env _visitors_c1  in
              let _visitors_s2 = self#on_block env _visitors_c2  in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_field env (_visitors_c0,_visitors_c1) =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_AFvalue env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_AFkvalue env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_afield env _visitors_this =
              match _visitors_this with
              | AFvalue _visitors_c0 -> self#on_AFvalue env _visitors_c0
              | AFkvalue (_visitors_c0,_visitors_c1) ->
                  self#on_AFkvalue env _visitors_c0 _visitors_c1
            method on_Xhp_simple env _visitors_c0 _visitors_c1 =
              let _visitors_s0 = self#on_pstring env _visitors_c0  in
              let _visitors_s1 = self#on_expr env _visitors_c1  in
              self#plus _visitors_s0 _visitors_s1
            method on_Xhp_spread env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_xhp_attribute env _visitors_this =
              match _visitors_this with
              | Xhp_simple (_visitors_c0,_visitors_c1) ->
                  self#on_Xhp_simple env _visitors_c0 _visitors_c1
              | Xhp_spread _visitors_c0 ->
                  self#on_Xhp_spread env _visitors_c0
            method on_Gena env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_Genva env _visitors_c0 =
              let _visitors_s0 = self#on_list self#on_expr env _visitors_c0
                 in
              _visitors_s0
            method on_Gen_array_rec env _visitors_c0 =
              let _visitors_s0 = self#on_expr env _visitors_c0  in
              _visitors_s0
            method on_special_func env _visitors_this =
              match _visitors_this with
              | Gena _visitors_c0 -> self#on_Gena env _visitors_c0
              | Genva _visitors_c0 -> self#on_Genva env _visitors_c0
              | Gen_array_rec _visitors_c0 ->
                  self#on_Gen_array_rec env _visitors_c0
            method on_is_reference env = self#on_bool env
            method on_is_variadic env = self#on_bool env
            method on_fun_param env _visitors_this =
              let _visitors_s0 =
                self#on_expr_annotation env _visitors_this.param_annotation
                 in
              let _visitors_s1 =
                self#on_option self#on_hint env _visitors_this.param_hint  in
              let _visitors_s2 =
                self#on_is_reference env _visitors_this.param_is_reference
                 in
              let _visitors_s3 =
                self#on_is_variadic env _visitors_this.param_is_variadic  in
              let _visitors_s4 = self#on_pos env _visitors_this.param_pos  in
              let _visitors_s5 = self#on_string env _visitors_this.param_name
                 in
              let _visitors_s6 =
                self#on_option self#on_expr env _visitors_this.param_expr  in
              let _visitors_s7 =
                self#on_option self#on_param_kind env
                  _visitors_this.param_callconv
                 in
              let _visitors_s8 =
                self#on_list self#on_user_attribute env
                  _visitors_this.param_user_attributes
                 in
              self#plus
                (self#plus
                   (self#plus
                      (self#plus
                         (self#plus
                            (self#plus
                               (self#plus
                                  (self#plus _visitors_s0 _visitors_s1)
                                  _visitors_s2) _visitors_s3) _visitors_s4)
                         _visitors_s5) _visitors_s6) _visitors_s7)
                _visitors_s8
            method on_FVvariadicArg env _visitors_c0 =
              let _visitors_s0 = self#on_fun_param env _visitors_c0  in
              _visitors_s0
            method on_FVellipsis env = self#zero
            method on_FVnonVariadic env = self#zero
            method on_fun_variadicity env _visitors_this =
              match _visitors_this with
              | FVvariadicArg _visitors_c0 ->
                  self#on_FVvariadicArg env _visitors_c0
              | FVellipsis  -> self#on_FVellipsis env
              | FVnonVariadic  -> self#on_FVnonVariadic env
            method on_fun_ env _visitors_this =
              let _visitors_s0 =
                self#on_env_annotation env _visitors_this.f_annotation  in
              let _visitors_s1 =
                (fun _visitors_this  -> self#zero) _visitors_this.f_mode  in
              let _visitors_s2 =
                self#on_option self#on_hint env _visitors_this.f_ret  in
              let _visitors_s3 = self#on_sid env _visitors_this.f_name  in
              let _visitors_s4 =
                self#on_list self#on_tparam env _visitors_this.f_tparams  in
              let _visitors_s5 =
                self#on_list self#on_where_constraint env
                  _visitors_this.f_where_constraints
                 in
              let _visitors_s6 =
                self#on_fun_variadicity env _visitors_this.f_variadic  in
              let _visitors_s7 =
                self#on_list self#on_fun_param env _visitors_this.f_params
                 in
              let _visitors_s8 = self#on_func_body env _visitors_this.f_body
                 in
              let _visitors_s9 =
                self#on_fun_kind env _visitors_this.f_fun_kind  in
              let _visitors_s10 =
                self#on_list self#on_user_attribute env
                  _visitors_this.f_user_attributes
                 in
              let _visitors_s11 =
                self#on_bool env _visitors_this.f_ret_by_ref  in
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
                                           (self#plus _visitors_s0
                                              _visitors_s1) _visitors_s2)
                                        _visitors_s3) _visitors_s4)
                                  _visitors_s5) _visitors_s6) _visitors_s7)
                         _visitors_s8) _visitors_s9) _visitors_s10)
                _visitors_s11
            method on_UnnamedBody env _visitors_c0 =
              let _visitors_s0 = self#on_func_unnamed_body env _visitors_c0
                 in
              _visitors_s0
            method on_NamedBody env _visitors_c0 =
              let _visitors_s0 = self#on_func_named_body env _visitors_c0  in
              _visitors_s0
            method on_func_body env _visitors_this =
              match _visitors_this with
              | UnnamedBody _visitors_c0 ->
                  self#on_UnnamedBody env _visitors_c0
              | NamedBody _visitors_c0 -> self#on_NamedBody env _visitors_c0
            method on_func_unnamed_body env _visitors_this =
              let _visitors_s0 =
                (fun _visitors_this  -> self#zero) _visitors_this.fub_ast  in
              let _visitors_s1 =
                (fun _visitors_this  -> self#zero) _visitors_this.fub_tparams
                 in
              let _visitors_s2 =
                (fun _visitors_this  -> self#zero)
                  _visitors_this.fub_namespace
                 in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_func_named_body env _visitors_this =
              let _visitors_s0 = self#on_block env _visitors_this.fnb_nast
                 in
              let _visitors_s1 = self#on_bool env _visitors_this.fnb_unsafe
                 in
              self#plus _visitors_s0 _visitors_s1
            method on_user_attribute env _visitors_this =
              let _visitors_s0 = self#on_sid env _visitors_this.ua_name  in
              let _visitors_s1 =
                self#on_list self#on_expr env _visitors_this.ua_params  in
              self#plus _visitors_s0 _visitors_s1
            method on_static_var env = self#on_class_var env
            method on_static_method env = self#on_method_ env
            method on_class_ env _visitors_this =
              let _visitors_s0 =
                self#on_env_annotation env _visitors_this.c_annotation  in
              let _visitors_s1 =
                (fun _visitors_this  -> self#zero) _visitors_this.c_mode  in
              let _visitors_s2 = self#on_bool env _visitors_this.c_final  in
              let _visitors_s3 = self#on_bool env _visitors_this.c_is_xhp  in
              let _visitors_s4 = self#on_class_kind env _visitors_this.c_kind
                 in
              let _visitors_s5 = self#on_sid env _visitors_this.c_name  in
              let _visitors_s6 =
                (fun _visitors_this  -> self#zero) _visitors_this.c_tparams
                 in
              let _visitors_s7 =
                self#on_list self#on_hint env _visitors_this.c_extends  in
              let _visitors_s8 =
                self#on_list self#on_hint env _visitors_this.c_uses  in
              let _visitors_s9 =
                self#on_list self#on_hint env _visitors_this.c_xhp_attr_uses
                 in
              let _visitors_s10 =
                self#on_list self#on_pstring env
                  _visitors_this.c_xhp_category
                 in
              let _visitors_s11 =
                self#on_list self#on_hint env _visitors_this.c_req_extends
                 in
              let _visitors_s12 =
                self#on_list self#on_hint env _visitors_this.c_req_implements
                 in
              let _visitors_s13 =
                self#on_list self#on_hint env _visitors_this.c_implements  in
              let _visitors_s14 =
                self#on_list self#on_class_const env _visitors_this.c_consts
                 in
              let _visitors_s15 =
                self#on_list self#on_class_typeconst env
                  _visitors_this.c_typeconsts
                 in
              let _visitors_s16 =
                self#on_list self#on_static_var env
                  _visitors_this.c_static_vars
                 in
              let _visitors_s17 =
                self#on_list self#on_class_var env _visitors_this.c_vars  in
              let _visitors_s18 =
                self#on_option self#on_method_ env
                  _visitors_this.c_constructor
                 in
              let _visitors_s19 =
                self#on_list self#on_static_method env
                  _visitors_this.c_static_methods
                 in
              let _visitors_s20 =
                self#on_list self#on_method_ env _visitors_this.c_methods  in
              let _visitors_s21 =
                self#on_list self#on_user_attribute env
                  _visitors_this.c_user_attributes
                 in
              let _visitors_s22 =
                self#on_option self#on_enum_ env _visitors_this.c_enum  in
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
                                                                    _visitors_s0
                                                                    _visitors_s1)
                                                                    _visitors_s2)
                                                                    _visitors_s3)
                                                                    _visitors_s4)
                                                                   _visitors_s5)
                                                                _visitors_s6)
                                                             _visitors_s7)
                                                          _visitors_s8)
                                                       _visitors_s9)
                                                    _visitors_s10)
                                                 _visitors_s11) _visitors_s12)
                                           _visitors_s13) _visitors_s14)
                                     _visitors_s15) _visitors_s16)
                               _visitors_s17) _visitors_s18) _visitors_s19)
                      _visitors_s20) _visitors_s21) _visitors_s22
            method on_class_const env
              (_visitors_c0,_visitors_c1,_visitors_c2) =
              let _visitors_s0 = self#on_option self#on_hint env _visitors_c0
                 in
              let _visitors_s1 = self#on_sid env _visitors_c1  in
              let _visitors_s2 = self#on_option self#on_expr env _visitors_c2
                 in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_class_typeconst env _visitors_this =
              let _visitors_s0 = self#on_sid env _visitors_this.c_tconst_name
                 in
              let _visitors_s1 =
                self#on_option self#on_hint env
                  _visitors_this.c_tconst_constraint
                 in
              let _visitors_s2 =
                self#on_option self#on_hint env _visitors_this.c_tconst_type
                 in
              self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
            method on_class_var env _visitors_this =
              let _visitors_s0 = self#on_bool env _visitors_this.cv_final  in
              let _visitors_s1 = self#on_bool env _visitors_this.cv_is_xhp
                 in
              let _visitors_s2 =
                self#on_visibility env _visitors_this.cv_visibility  in
              let _visitors_s3 =
                self#on_option self#on_hint env _visitors_this.cv_type  in
              let _visitors_s4 = self#on_sid env _visitors_this.cv_id  in
              let _visitors_s5 =
                self#on_option self#on_expr env _visitors_this.cv_expr  in
              let _visitors_s6 =
                self#on_list self#on_user_attribute env
                  _visitors_this.cv_user_attributes
                 in
              self#plus
                (self#plus
                   (self#plus
                      (self#plus
                         (self#plus (self#plus _visitors_s0 _visitors_s1)
                            _visitors_s2) _visitors_s3) _visitors_s4)
                   _visitors_s5) _visitors_s6
            method on_method_ env _visitors_this =
              let _visitors_s0 =
                self#on_env_annotation env _visitors_this.m_annotation  in
              let _visitors_s1 = self#on_bool env _visitors_this.m_final  in
              let _visitors_s2 = self#on_bool env _visitors_this.m_abstract
                 in
              let _visitors_s3 =
                self#on_visibility env _visitors_this.m_visibility  in
              let _visitors_s4 = self#on_sid env _visitors_this.m_name  in
              let _visitors_s5 =
                self#on_list self#on_tparam env _visitors_this.m_tparams  in
              let _visitors_s6 =
                self#on_list self#on_where_constraint env
                  _visitors_this.m_where_constraints
                 in
              let _visitors_s7 =
                self#on_fun_variadicity env _visitors_this.m_variadic  in
              let _visitors_s8 =
                self#on_list self#on_fun_param env _visitors_this.m_params
                 in
              let _visitors_s9 = self#on_func_body env _visitors_this.m_body
                 in
              let _visitors_s10 =
                self#on_fun_kind env _visitors_this.m_fun_kind  in
              let _visitors_s11 =
                self#on_list self#on_user_attribute env
                  _visitors_this.m_user_attributes
                 in
              let _visitors_s12 =
                self#on_option self#on_hint env _visitors_this.m_ret  in
              let _visitors_s13 =
                self#on_bool env _visitors_this.m_ret_by_ref  in
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
                                                    _visitors_s1)
                                                 _visitors_s2) _visitors_s3)
                                           _visitors_s4) _visitors_s5)
                                     _visitors_s6) _visitors_s7) _visitors_s8)
                            _visitors_s9) _visitors_s10) _visitors_s11)
                   _visitors_s12) _visitors_s13
            method on_typedef env _visitors_this =
              let _visitors_s0 =
                self#on_env_annotation env _visitors_this.t_annotation  in
              let _visitors_s1 = self#on_sid env _visitors_this.t_name  in
              let _visitors_s2 =
                self#on_list self#on_tparam env _visitors_this.t_tparams  in
              let _visitors_s3 =
                self#on_option self#on_hint env _visitors_this.t_constraint
                 in
              let _visitors_s4 = self#on_hint env _visitors_this.t_kind  in
              let _visitors_s5 =
                self#on_list self#on_user_attribute env
                  _visitors_this.t_user_attributes
                 in
              let _visitors_s6 =
                (fun _visitors_this  -> self#zero) _visitors_this.t_mode  in
              let _visitors_s7 =
                self#on_typedef_visibility env _visitors_this.t_vis  in
              self#plus
                (self#plus
                   (self#plus
                      (self#plus
                         (self#plus
                            (self#plus (self#plus _visitors_s0 _visitors_s1)
                               _visitors_s2) _visitors_s3) _visitors_s4)
                      _visitors_s5) _visitors_s6) _visitors_s7
            method on_gconst env _visitors_this =
              let _visitors_s0 =
                self#on_env_annotation env _visitors_this.cst_annotation  in
              let _visitors_s1 =
                (fun _visitors_this  -> self#zero) _visitors_this.cst_mode
                 in
              let _visitors_s2 = self#on_sid env _visitors_this.cst_name  in
              let _visitors_s3 =
                self#on_option self#on_hint env _visitors_this.cst_type  in
              let _visitors_s4 =
                self#on_option self#on_expr env _visitors_this.cst_value  in
              let _visitors_s5 =
                self#on_bool env _visitors_this.cst_is_define  in
              self#plus
                (self#plus
                   (self#plus
                      (self#plus (self#plus _visitors_s0 _visitors_s1)
                         _visitors_s2) _visitors_s3) _visitors_s4)
                _visitors_s5
            method on_Fun env _visitors_c0 =
              let _visitors_s0 = self#on_fun_ env _visitors_c0  in
              _visitors_s0
            method on_Class env _visitors_c0 =
              let _visitors_s0 = self#on_class_ env _visitors_c0  in
              _visitors_s0
            method on_Typedef env _visitors_c0 =
              let _visitors_s0 = self#on_typedef env _visitors_c0  in
              _visitors_s0
            method on_Constant env _visitors_c0 =
              let _visitors_s0 = self#on_gconst env _visitors_c0  in
              _visitors_s0
            method on_def env _visitors_this =
              match _visitors_this with
              | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
              | Class _visitors_c0 -> self#on_Class env _visitors_c0
              | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
              | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
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
            method on_expr_annotation env _visitors_this = _visitors_this
            method on_env_annotation env _visitors_this = _visitors_this
            method on_class_id_annotation env _visitors_this = _visitors_this
            method on_Fallthrough env = Fallthrough
            method on_Expr env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Expr _visitors_r0
            method on_Break env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              Break _visitors_r0
            method on_Continue env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              Continue _visitors_r0
            method on_Throw env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_is_terminal env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              Throw (_visitors_r0, _visitors_r1)
            method on_Return env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              Return (_visitors_r0, _visitors_r1)
            method on_GotoLabel env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in
              GotoLabel _visitors_r0
            method on_Goto env _visitors_c0 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in
              Goto _visitors_r0
            method on_Static_var env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              Static_var _visitors_r0
            method on_Global_var env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Using env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_bool env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              Using (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_For env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              let _visitors_r3 = self#on_block env _visitors_c3  in
              For (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
            method on_Switch env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_case env _visitors_c1
                 in
              Switch (_visitors_r0, _visitors_r1)
            method on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_as_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              Foreach (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Try env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_block env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_catch env _visitors_c1
                 in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              Try (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Noop env = Noop
            method on_stmt env _visitors_this =
              match _visitors_this with
              | Fallthrough  -> self#on_Fallthrough env
              | Expr _visitors_c0 -> self#on_Expr env _visitors_c0
              | Break _visitors_c0 -> self#on_Break env _visitors_c0
              | Continue _visitors_c0 -> self#on_Continue env _visitors_c0
              | Throw (_visitors_c0,_visitors_c1) ->
                  self#on_Throw env _visitors_c0 _visitors_c1
              | Return (_visitors_c0,_visitors_c1) ->
                  self#on_Return env _visitors_c0 _visitors_c1
              | GotoLabel _visitors_c0 -> self#on_GotoLabel env _visitors_c0
              | Goto _visitors_c0 -> self#on_Goto env _visitors_c0
              | Static_var _visitors_c0 ->
                  self#on_Static_var env _visitors_c0
              | Global_var _visitors_c0 ->
                  self#on_Global_var env _visitors_c0
              | If (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_If env _visitors_c0 _visitors_c1 _visitors_c2
              | Do (_visitors_c0,_visitors_c1) ->
                  self#on_Do env _visitors_c0 _visitors_c1
              | While (_visitors_c0,_visitors_c1) ->
                  self#on_While env _visitors_c0 _visitors_c1
              | Using (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Using env _visitors_c0 _visitors_c1 _visitors_c2
              | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) ->
                  self#on_For env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3
              | Switch (_visitors_c0,_visitors_c1) ->
                  self#on_Switch env _visitors_c0 _visitors_c1
              | Foreach (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Foreach env _visitors_c0 _visitors_c1 _visitors_c2
              | Try (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Try env _visitors_c0 _visitors_c1 _visitors_c2
              | Noop  -> self#on_Noop env
            method on_As_v env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              As_v _visitors_r0
            method on_As_kv env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              As_kv (_visitors_r0, _visitors_r1)
            method on_Await_as_v env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              Await_as_v (_visitors_r0, _visitors_r1)
            method on_Await_as_kv env _visitors_c0 _visitors_c1 _visitors_c2
              =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              Await_as_kv (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_as_expr env _visitors_this =
              match _visitors_this with
              | As_v _visitors_c0 -> self#on_As_v env _visitors_c0
              | As_kv (_visitors_c0,_visitors_c1) ->
                  self#on_As_kv env _visitors_c0 _visitors_c1
              | Await_as_v (_visitors_c0,_visitors_c1) ->
                  self#on_Await_as_v env _visitors_c0 _visitors_c1
              | Await_as_kv (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Await_as_kv env _visitors_c0 _visitors_c1
                    _visitors_c2
            method on_block env = self#on_list self#on_stmt env
            method on_class_id env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_class_id_annotation env _visitors_c0
                 in
              let _visitors_r1 = self#on_class_id_ env _visitors_c1  in
              (_visitors_r0, _visitors_r1)
            method on_CIparent env = CIparent
            method on_CIself env = CIself
            method on_CIstatic env = CIstatic
            method on_CIexpr env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              CIexpr _visitors_r0
            method on_CI env _visitors_c0 =
              let _visitors_r0 = self#on_instantiated_sid env _visitors_c0
                 in
              CI _visitors_r0
            method on_class_id_ env _visitors_this =
              match _visitors_this with
              | CIparent  -> self#on_CIparent env
              | CIself  -> self#on_CIself env
              | CIstatic  -> self#on_CIstatic env
              | CIexpr _visitors_c0 -> self#on_CIexpr env _visitors_c0
              | CI _visitors_c0 -> self#on_CI env _visitors_c0
            method on_expr env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_expr_annotation env _visitors_c0  in
              let _visitors_r1 = self#on_expr_ env _visitors_c1  in
              (_visitors_r0, _visitors_r1)
            method on_Array env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_afield env _visitors_c0
                 in
              Array _visitors_r0
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
            method on_Varray env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              Varray _visitors_r0
            method on_Shape env _visitors_c0 =
              let _visitors_r0 =
                self#on_shape_map self#on_expr env _visitors_c0  in
              Shape _visitors_r0
            method on_ValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_vc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              ValCollection (_visitors_r0, _visitors_r1)
            method on_KeyValCollection env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_kvc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_field env _visitors_c1
                 in
              KeyValCollection (_visitors_r0, _visitors_r1)
            method on_Null env = Null
            method on_This env = This
            method on_True env = True
            method on_False env = False
            method on_Id env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              Id _visitors_r0
            method on_Lvar env _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              Lvar _visitors_r0
            method on_Dollar env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Dollar _visitors_r0
            method on_Dollardollar env _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              Dollardollar _visitors_r0
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
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              Array_get (_visitors_r0, _visitors_r1)
            method on_Class_get env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              Class_get (_visitors_r0, _visitors_r1)
            method on_Class_const env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              Class_const (_visitors_r0, _visitors_r1)
            method on_Call env _visitors_c0 _visitors_c1 _visitors_c2
              _visitors_c3 _visitors_c4 =
              let _visitors_r0 = self#on_call_type env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_hint env _visitors_c2
                 in
              let _visitors_r3 = self#on_list self#on_expr env _visitors_c3
                 in
              let _visitors_r4 = self#on_list self#on_expr env _visitors_c4
                 in
              Call
                (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
                  _visitors_r4)
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
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              String2 _visitors_r0
            method on_Yield env _visitors_c0 =
              let _visitors_r0 = self#on_afield env _visitors_c0  in
              Yield _visitors_r0
            method on_Yield_break env = Yield_break
            method on_Await env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Await _visitors_r0
            method on_Suspend env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Suspend _visitors_r0
            method on_List env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              List _visitors_r0
            method on_Expr_list env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
            method on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              Pipe (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Eif env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              Eif (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_NullCoalesce env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              NullCoalesce (_visitors_r0, _visitors_r1)
            method on_InstanceOf env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_class_id env _visitors_c1  in
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
            method on_New env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              New (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Efun env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_fun_ env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_lid env _visitors_c1
                 in
              Efun (_visitors_r0, _visitors_r1)
            method on_Xml env _visitors_c0 _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 =
                self#on_list self#on_xhp_attribute env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              Xml (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Callconv env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_param_kind env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              Callconv (_visitors_r0, _visitors_r1)
            method on_Lplaceholder env _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              Lplaceholder _visitors_r0
            method on_Fun_id env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              Fun_id _visitors_r0
            method on_Method_id env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              Method_id (_visitors_r0, _visitors_r1)
            method on_Method_caller env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              Method_caller (_visitors_r0, _visitors_r1)
            method on_Smethod_id env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              Smethod_id (_visitors_r0, _visitors_r1)
            method on_Special_func env _visitors_c0 =
              let _visitors_r0 = self#on_special_func env _visitors_c0  in
              Special_func _visitors_r0
            method on_Pair env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              Pair (_visitors_r0, _visitors_r1)
            method on_Assert env _visitors_c0 =
              let _visitors_r0 = self#on_assert_expr env _visitors_c0  in
              Assert _visitors_r0
            method on_Typename env _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              Typename _visitors_r0
            method on_Any env = Any
            method on_expr_ env _visitors_this =
              match _visitors_this with
              | Array _visitors_c0 -> self#on_Array env _visitors_c0
              | Darray _visitors_c0 -> self#on_Darray env _visitors_c0
              | Varray _visitors_c0 -> self#on_Varray env _visitors_c0
              | Shape _visitors_c0 -> self#on_Shape env _visitors_c0
              | ValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_ValCollection env _visitors_c0 _visitors_c1
              | KeyValCollection (_visitors_c0,_visitors_c1) ->
                  self#on_KeyValCollection env _visitors_c0 _visitors_c1
              | Null  -> self#on_Null env
              | This  -> self#on_This env
              | True  -> self#on_True env
              | False  -> self#on_False env
              | Id _visitors_c0 -> self#on_Id env _visitors_c0
              | Lvar _visitors_c0 -> self#on_Lvar env _visitors_c0
              | Dollar _visitors_c0 -> self#on_Dollar env _visitors_c0
              | Dollardollar _visitors_c0 ->
                  self#on_Dollardollar env _visitors_c0
              | Clone _visitors_c0 -> self#on_Clone env _visitors_c0
              | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Obj_get env _visitors_c0 _visitors_c1 _visitors_c2
              | Array_get (_visitors_c0,_visitors_c1) ->
                  self#on_Array_get env _visitors_c0 _visitors_c1
              | Class_get (_visitors_c0,_visitors_c1) ->
                  self#on_Class_get env _visitors_c0 _visitors_c1
              | Class_const (_visitors_c0,_visitors_c1) ->
                  self#on_Class_const env _visitors_c0 _visitors_c1
              | Call
                  (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
                  ->
                  self#on_Call env _visitors_c0 _visitors_c1 _visitors_c2
                    _visitors_c3 _visitors_c4
              | Int _visitors_c0 -> self#on_Int env _visitors_c0
              | Float _visitors_c0 -> self#on_Float env _visitors_c0
              | String _visitors_c0 -> self#on_String env _visitors_c0
              | String2 _visitors_c0 -> self#on_String2 env _visitors_c0
              | Yield _visitors_c0 -> self#on_Yield env _visitors_c0
              | Yield_break  -> self#on_Yield_break env
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
              | Pipe (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Pipe env _visitors_c0 _visitors_c1 _visitors_c2
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
              | New (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_New env _visitors_c0 _visitors_c1 _visitors_c2
              | Efun (_visitors_c0,_visitors_c1) ->
                  self#on_Efun env _visitors_c0 _visitors_c1
              | Xml (_visitors_c0,_visitors_c1,_visitors_c2) ->
                  self#on_Xml env _visitors_c0 _visitors_c1 _visitors_c2
              | Callconv (_visitors_c0,_visitors_c1) ->
                  self#on_Callconv env _visitors_c0 _visitors_c1
              | Lplaceholder _visitors_c0 ->
                  self#on_Lplaceholder env _visitors_c0
              | Fun_id _visitors_c0 -> self#on_Fun_id env _visitors_c0
              | Method_id (_visitors_c0,_visitors_c1) ->
                  self#on_Method_id env _visitors_c0 _visitors_c1
              | Method_caller (_visitors_c0,_visitors_c1) ->
                  self#on_Method_caller env _visitors_c0 _visitors_c1
              | Smethod_id (_visitors_c0,_visitors_c1) ->
                  self#on_Smethod_id env _visitors_c0 _visitors_c1
              | Special_func _visitors_c0 ->
                  self#on_Special_func env _visitors_c0
              | Pair (_visitors_c0,_visitors_c1) ->
                  self#on_Pair env _visitors_c0 _visitors_c1
              | Assert _visitors_c0 -> self#on_Assert env _visitors_c0
              | Typename _visitors_c0 -> self#on_Typename env _visitors_c0
              | Any  -> self#on_Any env
            method on_AE_assert env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              AE_assert _visitors_r0
            method on_assert_expr env _visitors_this =
              match _visitors_this with
              | AE_assert _visitors_c0 -> self#on_AE_assert env _visitors_c0
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
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_lid env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_field env (_visitors_c0,_visitors_c1) =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              (_visitors_r0, _visitors_r1)
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
            method on_Xhp_simple env _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              Xhp_simple (_visitors_r0, _visitors_r1)
            method on_Xhp_spread env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Xhp_spread _visitors_r0
            method on_xhp_attribute env _visitors_this =
              match _visitors_this with
              | Xhp_simple (_visitors_c0,_visitors_c1) ->
                  self#on_Xhp_simple env _visitors_c0 _visitors_c1
              | Xhp_spread _visitors_c0 ->
                  self#on_Xhp_spread env _visitors_c0
            method on_Gena env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Gena _visitors_r0
            method on_Genva env _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              Genva _visitors_r0
            method on_Gen_array_rec env _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              Gen_array_rec _visitors_r0
            method on_special_func env _visitors_this =
              match _visitors_this with
              | Gena _visitors_c0 -> self#on_Gena env _visitors_c0
              | Genva _visitors_c0 -> self#on_Genva env _visitors_c0
              | Gen_array_rec _visitors_c0 ->
                  self#on_Gen_array_rec env _visitors_c0
            method on_is_reference env = self#on_bool env
            method on_is_variadic env = self#on_bool env
            method on_fun_param env _visitors_this =
              let _visitors_r0 =
                self#on_expr_annotation env _visitors_this.param_annotation
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env _visitors_this.param_hint  in
              let _visitors_r2 =
                self#on_is_reference env _visitors_this.param_is_reference
                 in
              let _visitors_r3 =
                self#on_is_variadic env _visitors_this.param_is_variadic  in
              let _visitors_r4 = self#on_pos env _visitors_this.param_pos  in
              let _visitors_r5 = self#on_string env _visitors_this.param_name
                 in
              let _visitors_r6 =
                self#on_option self#on_expr env _visitors_this.param_expr  in
              let _visitors_r7 =
                self#on_option self#on_param_kind env
                  _visitors_this.param_callconv
                 in
              let _visitors_r8 =
                self#on_list self#on_user_attribute env
                  _visitors_this.param_user_attributes
                 in
              {
                param_annotation = _visitors_r0;
                param_hint = _visitors_r1;
                param_is_reference = _visitors_r2;
                param_is_variadic = _visitors_r3;
                param_pos = _visitors_r4;
                param_name = _visitors_r5;
                param_expr = _visitors_r6;
                param_callconv = _visitors_r7;
                param_user_attributes = _visitors_r8
              }
            method on_FVvariadicArg env _visitors_c0 =
              let _visitors_r0 = self#on_fun_param env _visitors_c0  in
              FVvariadicArg _visitors_r0
            method on_FVellipsis env = FVellipsis
            method on_FVnonVariadic env = FVnonVariadic
            method on_fun_variadicity env _visitors_this =
              match _visitors_this with
              | FVvariadicArg _visitors_c0 ->
                  self#on_FVvariadicArg env _visitors_c0
              | FVellipsis  -> self#on_FVellipsis env
              | FVnonVariadic  -> self#on_FVnonVariadic env
            method on_fun_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.f_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.f_mode
                 in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.f_ret  in
              let _visitors_r3 = self#on_sid env _visitors_this.f_name  in
              let _visitors_r4 =
                self#on_list self#on_tparam env _visitors_this.f_tparams  in
              let _visitors_r5 =
                self#on_list self#on_where_constraint env
                  _visitors_this.f_where_constraints
                 in
              let _visitors_r6 =
                self#on_fun_variadicity env _visitors_this.f_variadic  in
              let _visitors_r7 =
                self#on_list self#on_fun_param env _visitors_this.f_params
                 in
              let _visitors_r8 = self#on_func_body env _visitors_this.f_body
                 in
              let _visitors_r9 =
                self#on_fun_kind env _visitors_this.f_fun_kind  in
              let _visitors_r10 =
                self#on_list self#on_user_attribute env
                  _visitors_this.f_user_attributes
                 in
              let _visitors_r11 =
                self#on_bool env _visitors_this.f_ret_by_ref  in
              {
                f_annotation = _visitors_r0;
                f_mode = _visitors_r1;
                f_ret = _visitors_r2;
                f_name = _visitors_r3;
                f_tparams = _visitors_r4;
                f_where_constraints = _visitors_r5;
                f_variadic = _visitors_r6;
                f_params = _visitors_r7;
                f_body = _visitors_r8;
                f_fun_kind = _visitors_r9;
                f_user_attributes = _visitors_r10;
                f_ret_by_ref = _visitors_r11
              }
            method on_UnnamedBody env _visitors_c0 =
              let _visitors_r0 = self#on_func_unnamed_body env _visitors_c0
                 in
              UnnamedBody _visitors_r0
            method on_NamedBody env _visitors_c0 =
              let _visitors_r0 = self#on_func_named_body env _visitors_c0  in
              NamedBody _visitors_r0
            method on_func_body env _visitors_this =
              match _visitors_this with
              | UnnamedBody _visitors_c0 ->
                  self#on_UnnamedBody env _visitors_c0
              | NamedBody _visitors_c0 -> self#on_NamedBody env _visitors_c0
            method on_func_unnamed_body env _visitors_this =
              let _visitors_r0 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_ast
                 in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_tparams
                 in
              let _visitors_r2 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_namespace
                 in
              {
                fub_ast = _visitors_r0;
                fub_tparams = _visitors_r1;
                fub_namespace = _visitors_r2
              }
            method on_func_named_body env _visitors_this =
              let _visitors_r0 = self#on_block env _visitors_this.fnb_nast
                 in
              let _visitors_r1 = self#on_bool env _visitors_this.fnb_unsafe
                 in
              { fnb_nast = _visitors_r0; fnb_unsafe = _visitors_r1 }
            method on_user_attribute env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.ua_name  in
              let _visitors_r1 =
                self#on_list self#on_expr env _visitors_this.ua_params  in
              { ua_name = _visitors_r0; ua_params = _visitors_r1 }
            method on_static_var env = self#on_class_var env
            method on_static_method env = self#on_method_ env
            method on_class_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.c_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.c_mode
                 in
              let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
              let _visitors_r3 = self#on_bool env _visitors_this.c_is_xhp  in
              let _visitors_r4 = self#on_class_kind env _visitors_this.c_kind
                 in
              let _visitors_r5 = self#on_sid env _visitors_this.c_name  in
              let _visitors_r6 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.c_tparams
                 in
              let _visitors_r7 =
                self#on_list self#on_hint env _visitors_this.c_extends  in
              let _visitors_r8 =
                self#on_list self#on_hint env _visitors_this.c_uses  in
              let _visitors_r9 =
                self#on_list self#on_hint env _visitors_this.c_xhp_attr_uses
                 in
              let _visitors_r10 =
                self#on_list self#on_pstring env
                  _visitors_this.c_xhp_category
                 in
              let _visitors_r11 =
                self#on_list self#on_hint env _visitors_this.c_req_extends
                 in
              let _visitors_r12 =
                self#on_list self#on_hint env _visitors_this.c_req_implements
                 in
              let _visitors_r13 =
                self#on_list self#on_hint env _visitors_this.c_implements  in
              let _visitors_r14 =
                self#on_list self#on_class_const env _visitors_this.c_consts
                 in
              let _visitors_r15 =
                self#on_list self#on_class_typeconst env
                  _visitors_this.c_typeconsts
                 in
              let _visitors_r16 =
                self#on_list self#on_static_var env
                  _visitors_this.c_static_vars
                 in
              let _visitors_r17 =
                self#on_list self#on_class_var env _visitors_this.c_vars  in
              let _visitors_r18 =
                self#on_option self#on_method_ env
                  _visitors_this.c_constructor
                 in
              let _visitors_r19 =
                self#on_list self#on_static_method env
                  _visitors_this.c_static_methods
                 in
              let _visitors_r20 =
                self#on_list self#on_method_ env _visitors_this.c_methods  in
              let _visitors_r21 =
                self#on_list self#on_user_attribute env
                  _visitors_this.c_user_attributes
                 in
              let _visitors_r22 =
                self#on_option self#on_enum_ env _visitors_this.c_enum  in
              {
                c_annotation = _visitors_r0;
                c_mode = _visitors_r1;
                c_final = _visitors_r2;
                c_is_xhp = _visitors_r3;
                c_kind = _visitors_r4;
                c_name = _visitors_r5;
                c_tparams = _visitors_r6;
                c_extends = _visitors_r7;
                c_uses = _visitors_r8;
                c_xhp_attr_uses = _visitors_r9;
                c_xhp_category = _visitors_r10;
                c_req_extends = _visitors_r11;
                c_req_implements = _visitors_r12;
                c_implements = _visitors_r13;
                c_consts = _visitors_r14;
                c_typeconsts = _visitors_r15;
                c_static_vars = _visitors_r16;
                c_vars = _visitors_r17;
                c_constructor = _visitors_r18;
                c_static_methods = _visitors_r19;
                c_methods = _visitors_r20;
                c_user_attributes = _visitors_r21;
                c_enum = _visitors_r22
              }
            method on_class_const env
              (_visitors_c0,_visitors_c1,_visitors_c2) =
              let _visitors_r0 = self#on_option self#on_hint env _visitors_c0
                 in
              let _visitors_r1 = self#on_sid env _visitors_c1  in
              let _visitors_r2 = self#on_option self#on_expr env _visitors_c2
                 in
              (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_class_typeconst env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.c_tconst_name
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env
                  _visitors_this.c_tconst_constraint
                 in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.c_tconst_type
                 in
              {
                c_tconst_name = _visitors_r0;
                c_tconst_constraint = _visitors_r1;
                c_tconst_type = _visitors_r2
              }
            method on_class_var env _visitors_this =
              let _visitors_r0 = self#on_bool env _visitors_this.cv_final  in
              let _visitors_r1 = self#on_bool env _visitors_this.cv_is_xhp
                 in
              let _visitors_r2 =
                self#on_visibility env _visitors_this.cv_visibility  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cv_type  in
              let _visitors_r4 = self#on_sid env _visitors_this.cv_id  in
              let _visitors_r5 =
                self#on_option self#on_expr env _visitors_this.cv_expr  in
              let _visitors_r6 =
                self#on_list self#on_user_attribute env
                  _visitors_this.cv_user_attributes
                 in
              {
                cv_final = _visitors_r0;
                cv_is_xhp = _visitors_r1;
                cv_visibility = _visitors_r2;
                cv_type = _visitors_r3;
                cv_id = _visitors_r4;
                cv_expr = _visitors_r5;
                cv_user_attributes = _visitors_r6
              }
            method on_method_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.m_annotation  in
              let _visitors_r1 = self#on_bool env _visitors_this.m_final  in
              let _visitors_r2 = self#on_bool env _visitors_this.m_abstract
                 in
              let _visitors_r3 =
                self#on_visibility env _visitors_this.m_visibility  in
              let _visitors_r4 = self#on_sid env _visitors_this.m_name  in
              let _visitors_r5 =
                self#on_list self#on_tparam env _visitors_this.m_tparams  in
              let _visitors_r6 =
                self#on_list self#on_where_constraint env
                  _visitors_this.m_where_constraints
                 in
              let _visitors_r7 =
                self#on_fun_variadicity env _visitors_this.m_variadic  in
              let _visitors_r8 =
                self#on_list self#on_fun_param env _visitors_this.m_params
                 in
              let _visitors_r9 = self#on_func_body env _visitors_this.m_body
                 in
              let _visitors_r10 =
                self#on_fun_kind env _visitors_this.m_fun_kind  in
              let _visitors_r11 =
                self#on_list self#on_user_attribute env
                  _visitors_this.m_user_attributes
                 in
              let _visitors_r12 =
                self#on_option self#on_hint env _visitors_this.m_ret  in
              let _visitors_r13 =
                self#on_bool env _visitors_this.m_ret_by_ref  in
              {
                m_annotation = _visitors_r0;
                m_final = _visitors_r1;
                m_abstract = _visitors_r2;
                m_visibility = _visitors_r3;
                m_name = _visitors_r4;
                m_tparams = _visitors_r5;
                m_where_constraints = _visitors_r6;
                m_variadic = _visitors_r7;
                m_params = _visitors_r8;
                m_body = _visitors_r9;
                m_fun_kind = _visitors_r10;
                m_user_attributes = _visitors_r11;
                m_ret = _visitors_r12;
                m_ret_by_ref = _visitors_r13
              }
            method on_typedef env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.t_annotation  in
              let _visitors_r1 = self#on_sid env _visitors_this.t_name  in
              let _visitors_r2 =
                self#on_list self#on_tparam env _visitors_this.t_tparams  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.t_constraint
                 in
              let _visitors_r4 = self#on_hint env _visitors_this.t_kind  in
              let _visitors_r5 =
                self#on_list self#on_user_attribute env
                  _visitors_this.t_user_attributes
                 in
              let _visitors_r6 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.t_mode
                 in
              let _visitors_r7 =
                self#on_typedef_visibility env _visitors_this.t_vis  in
              {
                t_annotation = _visitors_r0;
                t_name = _visitors_r1;
                t_tparams = _visitors_r2;
                t_constraint = _visitors_r3;
                t_kind = _visitors_r4;
                t_user_attributes = _visitors_r5;
                t_mode = _visitors_r6;
                t_vis = _visitors_r7
              }
            method on_gconst env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.cst_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.cst_mode
                 in
              let _visitors_r2 = self#on_sid env _visitors_this.cst_name  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cst_type  in
              let _visitors_r4 =
                self#on_option self#on_expr env _visitors_this.cst_value  in
              let _visitors_r5 =
                self#on_bool env _visitors_this.cst_is_define  in
              {
                cst_annotation = _visitors_r0;
                cst_mode = _visitors_r1;
                cst_name = _visitors_r2;
                cst_type = _visitors_r3;
                cst_value = _visitors_r4;
                cst_is_define = _visitors_r5
              }
            method on_Fun env _visitors_c0 =
              let _visitors_r0 = self#on_fun_ env _visitors_c0  in
              Fun _visitors_r0
            method on_Class env _visitors_c0 =
              let _visitors_r0 = self#on_class_ env _visitors_c0  in
              Class _visitors_r0
            method on_Typedef env _visitors_c0 =
              let _visitors_r0 = self#on_typedef env _visitors_c0  in
              Typedef _visitors_r0
            method on_Constant env _visitors_c0 =
              let _visitors_r0 = self#on_gconst env _visitors_c0  in
              Constant _visitors_r0
            method on_def env _visitors_this =
              match _visitors_this with
              | Fun _visitors_c0 -> self#on_Fun env _visitors_c0
              | Class _visitors_c0 -> self#on_Class env _visitors_c0
              | Typedef _visitors_c0 -> self#on_Typedef env _visitors_c0
              | Constant _visitors_c0 -> self#on_Constant env _visitors_c0
          end
        [@@@VISITORS.END ]
      end
    include
      struct
        [@@@ocaml.warning "-4-26-27"]
        [@@@VISITORS.BEGIN ]
        class virtual ['self] endo =
          object (self : 'self)
            inherit  [_] endo_defs
            method on_program env = self#on_list self#on_def env
            method on_expr_annotation env _visitors_this = _visitors_this
            method on_env_annotation env _visitors_this = _visitors_this
            method on_class_id_annotation env _visitors_this = _visitors_this
            method on_Fallthrough env _visitors_this =
              if true then _visitors_this else Fallthrough
            method on_Expr env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Expr _visitors_r0
            method on_Break env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Break _visitors_r0
            method on_Continue env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Continue _visitors_r0
            method on_Throw env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_is_terminal env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Throw (_visitors_r0, _visitors_r1)
            method on_Return env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
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
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Static_var _visitors_r0
            method on_Global_var env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
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
            method on_Using env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_bool env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Using (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_For env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 _visitors_c3 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              let _visitors_r3 = self#on_block env _visitors_c3  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_c2 _visitors_r2)
                        (Pervasives.(==) _visitors_c3 _visitors_r3)))
              then _visitors_this
              else
                For (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3)
            method on_Switch env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_case env _visitors_c1
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Switch (_visitors_r0, _visitors_r1)
            method on_Foreach env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_as_expr env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Foreach (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Try env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_block env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_catch env _visitors_c1
                 in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Try (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Noop env _visitors_this =
              if true then _visitors_this else Noop
            method on_stmt env _visitors_this =
              match _visitors_this with
              | Fallthrough  as _visitors_this ->
                  self#on_Fallthrough env _visitors_this
              | Expr _visitors_c0 as _visitors_this ->
                  self#on_Expr env _visitors_this _visitors_c0
              | Break _visitors_c0 as _visitors_this ->
                  self#on_Break env _visitors_this _visitors_c0
              | Continue _visitors_c0 as _visitors_this ->
                  self#on_Continue env _visitors_this _visitors_c0
              | Throw (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Throw env _visitors_this _visitors_c0 _visitors_c1
              | Return (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Return env _visitors_this _visitors_c0 _visitors_c1
              | GotoLabel _visitors_c0 as _visitors_this ->
                  self#on_GotoLabel env _visitors_this _visitors_c0
              | Goto _visitors_c0 as _visitors_this ->
                  self#on_Goto env _visitors_this _visitors_c0
              | Static_var _visitors_c0 as _visitors_this ->
                  self#on_Static_var env _visitors_this _visitors_c0
              | Global_var _visitors_c0 as _visitors_this ->
                  self#on_Global_var env _visitors_this _visitors_c0
              | If (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this
                  ->
                  self#on_If env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Do (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Do env _visitors_this _visitors_c0 _visitors_c1
              | While (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_While env _visitors_this _visitors_c0 _visitors_c1
              | Using (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Using env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | For (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3) as
                  _visitors_this ->
                  self#on_For env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2 _visitors_c3
              | Switch (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Switch env _visitors_this _visitors_c0 _visitors_c1
              | Foreach (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Foreach env _visitors_this _visitors_c0
                    _visitors_c1 _visitors_c2
              | Try (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Try env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Noop  as _visitors_this -> self#on_Noop env _visitors_this
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
            method on_Await_as_v env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Await_as_v (_visitors_r0, _visitors_r1)
            method on_Await_as_kv env _visitors_this _visitors_c0
              _visitors_c1 _visitors_c2 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Await_as_kv (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_as_expr env _visitors_this =
              match _visitors_this with
              | As_v _visitors_c0 as _visitors_this ->
                  self#on_As_v env _visitors_this _visitors_c0
              | As_kv (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_As_kv env _visitors_this _visitors_c0 _visitors_c1
              | Await_as_v (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Await_as_v env _visitors_this _visitors_c0
                    _visitors_c1
              | Await_as_kv (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Await_as_kv env _visitors_this _visitors_c0
                    _visitors_c1 _visitors_c2
            method on_block env = self#on_list self#on_stmt env
            method on_class_id env
              ((_visitors_c0,_visitors_c1) as _visitors_this) =
              let _visitors_r0 = self#on_class_id_annotation env _visitors_c0
                 in
              let _visitors_r1 = self#on_class_id_ env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else (_visitors_r0, _visitors_r1)
            method on_CIparent env _visitors_this =
              if true then _visitors_this else CIparent
            method on_CIself env _visitors_this =
              if true then _visitors_this else CIself
            method on_CIstatic env _visitors_this =
              if true then _visitors_this else CIstatic
            method on_CIexpr env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else CIexpr _visitors_r0
            method on_CI env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_instantiated_sid env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else CI _visitors_r0
            method on_class_id_ env _visitors_this =
              match _visitors_this with
              | CIparent  as _visitors_this ->
                  self#on_CIparent env _visitors_this
              | CIself  as _visitors_this ->
                  self#on_CIself env _visitors_this
              | CIstatic  as _visitors_this ->
                  self#on_CIstatic env _visitors_this
              | CIexpr _visitors_c0 as _visitors_this ->
                  self#on_CIexpr env _visitors_this _visitors_c0
              | CI _visitors_c0 as _visitors_this ->
                  self#on_CI env _visitors_this _visitors_c0
            method on_expr env
              ((_visitors_c0,_visitors_c1) as _visitors_this) =
              let _visitors_r0 = self#on_expr_annotation env _visitors_c0  in
              let _visitors_r1 = self#on_expr_ env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else (_visitors_r0, _visitors_r1)
            method on_Array env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_afield env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Array _visitors_r0
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
            method on_Varray env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Varray _visitors_r0
            method on_Shape env _visitors_this _visitors_c0 =
              let _visitors_r0 =
                self#on_shape_map self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Shape _visitors_r0
            method on_ValCollection env _visitors_this _visitors_c0
              _visitors_c1 =
              let _visitors_r0 = self#on_vc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else ValCollection (_visitors_r0, _visitors_r1)
            method on_KeyValCollection env _visitors_this _visitors_c0
              _visitors_c1 =
              let _visitors_r0 = self#on_kvc_kind env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_field env _visitors_c1
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else KeyValCollection (_visitors_r0, _visitors_r1)
            method on_Null env _visitors_this =
              if true then _visitors_this else Null
            method on_This env _visitors_this =
              if true then _visitors_this else This
            method on_True env _visitors_this =
              if true then _visitors_this else True
            method on_False env _visitors_this =
              if true then _visitors_this else False
            method on_Id env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Id _visitors_r0
            method on_Lvar env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Lvar _visitors_r0
            method on_Dollar env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Dollar _visitors_r0
            method on_Dollardollar env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Dollardollar _visitors_r0
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
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Obj_get (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Array_get env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Array_get (_visitors_r0, _visitors_r1)
            method on_Class_get env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Class_get (_visitors_r0, _visitors_r1)
            method on_Class_const env _visitors_this _visitors_c0
              _visitors_c1 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Class_const (_visitors_r0, _visitors_r1)
            method on_Call env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 _visitors_c3 _visitors_c4 =
              let _visitors_r0 = self#on_call_type env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_hint env _visitors_c2
                 in
              let _visitors_r3 = self#on_list self#on_expr env _visitors_c3
                 in
              let _visitors_r4 = self#on_list self#on_expr env _visitors_c4
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_c2 _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_c3 _visitors_r3)
                           (Pervasives.(==) _visitors_c4 _visitors_r4))))
              then _visitors_this
              else
                Call
                  (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
                    _visitors_r4)
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
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else List _visitors_r0
            method on_Expr_list env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
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
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Binop (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Pipe env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_lid env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Pipe (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Eif env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_option self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_expr env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Eif (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_NullCoalesce env _visitors_this _visitors_c0
              _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else NullCoalesce (_visitors_r0, _visitors_r1)
            method on_InstanceOf env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_class_id env _visitors_c1  in
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
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else As (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_New env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_class_id env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_expr env _visitors_c1
                 in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else New (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Efun env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_fun_ env _visitors_c0  in
              let _visitors_r1 = self#on_list self#on_lid env _visitors_c1
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Efun (_visitors_r0, _visitors_r1)
            method on_Xml env _visitors_this _visitors_c0 _visitors_c1
              _visitors_c2 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 =
                self#on_list self#on_xhp_attribute env _visitors_c1  in
              let _visitors_r2 = self#on_list self#on_expr env _visitors_c2
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else Xml (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_Callconv env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_param_kind env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Callconv (_visitors_r0, _visitors_r1)
            method on_Lplaceholder env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_pos env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Lplaceholder _visitors_r0
            method on_Fun_id env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Fun_id _visitors_r0
            method on_Method_id env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Method_id (_visitors_r0, _visitors_r1)
            method on_Method_caller env _visitors_this _visitors_c0
              _visitors_c1 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Method_caller (_visitors_r0, _visitors_r1)
            method on_Smethod_id env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_pstring env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Smethod_id (_visitors_r0, _visitors_r1)
            method on_Special_func env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_special_func env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Special_func _visitors_r0
            method on_Pair env _visitors_this _visitors_c0 _visitors_c1 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else Pair (_visitors_r0, _visitors_r1)
            method on_Assert env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_assert_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Assert _visitors_r0
            method on_Typename env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Typename _visitors_r0
            method on_Any env _visitors_this =
              if true then _visitors_this else Any
            method on_expr_ env _visitors_this =
              match _visitors_this with
              | Array _visitors_c0 as _visitors_this ->
                  self#on_Array env _visitors_this _visitors_c0
              | Darray _visitors_c0 as _visitors_this ->
                  self#on_Darray env _visitors_this _visitors_c0
              | Varray _visitors_c0 as _visitors_this ->
                  self#on_Varray env _visitors_this _visitors_c0
              | Shape _visitors_c0 as _visitors_this ->
                  self#on_Shape env _visitors_this _visitors_c0
              | ValCollection (_visitors_c0,_visitors_c1) as _visitors_this
                  ->
                  self#on_ValCollection env _visitors_this _visitors_c0
                    _visitors_c1
              | KeyValCollection (_visitors_c0,_visitors_c1) as
                  _visitors_this ->
                  self#on_KeyValCollection env _visitors_this _visitors_c0
                    _visitors_c1
              | Null  as _visitors_this -> self#on_Null env _visitors_this
              | This  as _visitors_this -> self#on_This env _visitors_this
              | True  as _visitors_this -> self#on_True env _visitors_this
              | False  as _visitors_this -> self#on_False env _visitors_this
              | Id _visitors_c0 as _visitors_this ->
                  self#on_Id env _visitors_this _visitors_c0
              | Lvar _visitors_c0 as _visitors_this ->
                  self#on_Lvar env _visitors_this _visitors_c0
              | Dollar _visitors_c0 as _visitors_this ->
                  self#on_Dollar env _visitors_this _visitors_c0
              | Dollardollar _visitors_c0 as _visitors_this ->
                  self#on_Dollardollar env _visitors_this _visitors_c0
              | Clone _visitors_c0 as _visitors_this ->
                  self#on_Clone env _visitors_this _visitors_c0
              | Obj_get (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Obj_get env _visitors_this _visitors_c0
                    _visitors_c1 _visitors_c2
              | Array_get (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Array_get env _visitors_this _visitors_c0
                    _visitors_c1
              | Class_get (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Class_get env _visitors_this _visitors_c0
                    _visitors_c1
              | Class_const (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Class_const env _visitors_this _visitors_c0
                    _visitors_c1
              | Call
                  (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4)
                  as _visitors_this ->
                  self#on_Call env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2 _visitors_c3 _visitors_c4
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
              | Binop (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Binop env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Pipe (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Pipe env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Eif (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Eif env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | NullCoalesce (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_NullCoalesce env _visitors_this _visitors_c0
                    _visitors_c1
              | InstanceOf (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_InstanceOf env _visitors_this _visitors_c0
                    _visitors_c1
              | Is (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Is env _visitors_this _visitors_c0 _visitors_c1
              | As (_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this
                  ->
                  self#on_As env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | New (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_New env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Efun (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Efun env _visitors_this _visitors_c0 _visitors_c1
              | Xml (_visitors_c0,_visitors_c1,_visitors_c2) as
                  _visitors_this ->
                  self#on_Xml env _visitors_this _visitors_c0 _visitors_c1
                    _visitors_c2
              | Callconv (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Callconv env _visitors_this _visitors_c0
                    _visitors_c1
              | Lplaceholder _visitors_c0 as _visitors_this ->
                  self#on_Lplaceholder env _visitors_this _visitors_c0
              | Fun_id _visitors_c0 as _visitors_this ->
                  self#on_Fun_id env _visitors_this _visitors_c0
              | Method_id (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Method_id env _visitors_this _visitors_c0
                    _visitors_c1
              | Method_caller (_visitors_c0,_visitors_c1) as _visitors_this
                  ->
                  self#on_Method_caller env _visitors_this _visitors_c0
                    _visitors_c1
              | Smethod_id (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Smethod_id env _visitors_this _visitors_c0
                    _visitors_c1
              | Special_func _visitors_c0 as _visitors_this ->
                  self#on_Special_func env _visitors_this _visitors_c0
              | Pair (_visitors_c0,_visitors_c1) as _visitors_this ->
                  self#on_Pair env _visitors_this _visitors_c0 _visitors_c1
              | Assert _visitors_c0 as _visitors_this ->
                  self#on_Assert env _visitors_this _visitors_c0
              | Typename _visitors_c0 as _visitors_this ->
                  self#on_Typename env _visitors_this _visitors_c0
              | Any  as _visitors_this -> self#on_Any env _visitors_this
            method on_AE_assert env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else AE_assert _visitors_r0
            method on_assert_expr env _visitors_this =
              match _visitors_this with
              | AE_assert _visitors_c0 as _visitors_this ->
                  self#on_AE_assert env _visitors_this _visitors_c0
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
              let _visitors_r0 = self#on_sid env _visitors_c0  in
              let _visitors_r1 = self#on_lid env _visitors_c1  in
              let _visitors_r2 = self#on_block env _visitors_c2  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_field env
              ((_visitors_c0,_visitors_c1) as _visitors_this) =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              let _visitors_r1 = self#on_expr env _visitors_c1  in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(==) _visitors_c1 _visitors_r1)
              then _visitors_this
              else (_visitors_r0, _visitors_r1)
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
                  self#on_AFkvalue env _visitors_this _visitors_c0
                    _visitors_c1
            method on_Xhp_simple env _visitors_this _visitors_c0 _visitors_c1
              =
              let _visitors_r0 = self#on_pstring env _visitors_c0  in
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
                  self#on_Xhp_simple env _visitors_this _visitors_c0
                    _visitors_c1
              | Xhp_spread _visitors_c0 as _visitors_this ->
                  self#on_Xhp_spread env _visitors_this _visitors_c0
            method on_Gena env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Gena _visitors_r0
            method on_Genva env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_list self#on_expr env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Genva _visitors_r0
            method on_Gen_array_rec env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_expr env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else Gen_array_rec _visitors_r0
            method on_special_func env _visitors_this =
              match _visitors_this with
              | Gena _visitors_c0 as _visitors_this ->
                  self#on_Gena env _visitors_this _visitors_c0
              | Genva _visitors_c0 as _visitors_this ->
                  self#on_Genva env _visitors_this _visitors_c0
              | Gen_array_rec _visitors_c0 as _visitors_this ->
                  self#on_Gen_array_rec env _visitors_this _visitors_c0
            method on_is_reference env = self#on_bool env
            method on_is_variadic env = self#on_bool env
            method on_fun_param env _visitors_this =
              let _visitors_r0 =
                self#on_expr_annotation env _visitors_this.param_annotation
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env _visitors_this.param_hint  in
              let _visitors_r2 =
                self#on_is_reference env _visitors_this.param_is_reference
                 in
              let _visitors_r3 =
                self#on_is_variadic env _visitors_this.param_is_variadic  in
              let _visitors_r4 = self#on_pos env _visitors_this.param_pos  in
              let _visitors_r5 = self#on_string env _visitors_this.param_name
                 in
              let _visitors_r6 =
                self#on_option self#on_expr env _visitors_this.param_expr  in
              let _visitors_r7 =
                self#on_option self#on_param_kind env
                  _visitors_this.param_callconv
                 in
              let _visitors_r8 =
                self#on_list self#on_user_attribute env
                  _visitors_this.param_user_attributes
                 in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.param_annotation
                     _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.param_hint _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.param_is_reference
                           _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.param_is_variadic
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.param_pos
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==) _visitors_this.param_name
                                    _visitors_r5)
                                 (Pervasives.(&&)
                                    (Pervasives.(==)
                                       _visitors_this.param_expr _visitors_r6)
                                    (Pervasives.(&&)
                                       (Pervasives.(==)
                                          _visitors_this.param_callconv
                                          _visitors_r7)
                                       (Pervasives.(==)
                                          _visitors_this.param_user_attributes
                                          _visitors_r8))))))))
              then _visitors_this
              else
                {
                  param_annotation = _visitors_r0;
                  param_hint = _visitors_r1;
                  param_is_reference = _visitors_r2;
                  param_is_variadic = _visitors_r3;
                  param_pos = _visitors_r4;
                  param_name = _visitors_r5;
                  param_expr = _visitors_r6;
                  param_callconv = _visitors_r7;
                  param_user_attributes = _visitors_r8
                }
            method on_FVvariadicArg env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_fun_param env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else FVvariadicArg _visitors_r0
            method on_FVellipsis env _visitors_this =
              if true then _visitors_this else FVellipsis
            method on_FVnonVariadic env _visitors_this =
              if true then _visitors_this else FVnonVariadic
            method on_fun_variadicity env _visitors_this =
              match _visitors_this with
              | FVvariadicArg _visitors_c0 as _visitors_this ->
                  self#on_FVvariadicArg env _visitors_this _visitors_c0
              | FVellipsis  as _visitors_this ->
                  self#on_FVellipsis env _visitors_this
              | FVnonVariadic  as _visitors_this ->
                  self#on_FVnonVariadic env _visitors_this
            method on_fun_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.f_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.f_mode
                 in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.f_ret  in
              let _visitors_r3 = self#on_sid env _visitors_this.f_name  in
              let _visitors_r4 =
                self#on_list self#on_tparam env _visitors_this.f_tparams  in
              let _visitors_r5 =
                self#on_list self#on_where_constraint env
                  _visitors_this.f_where_constraints
                 in
              let _visitors_r6 =
                self#on_fun_variadicity env _visitors_this.f_variadic  in
              let _visitors_r7 =
                self#on_list self#on_fun_param env _visitors_this.f_params
                 in
              let _visitors_r8 = self#on_func_body env _visitors_this.f_body
                 in
              let _visitors_r9 =
                self#on_fun_kind env _visitors_this.f_fun_kind  in
              let _visitors_r10 =
                self#on_list self#on_user_attribute env
                  _visitors_this.f_user_attributes
                 in
              let _visitors_r11 =
                self#on_bool env _visitors_this.f_ret_by_ref  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.f_annotation _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.f_mode _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.f_ret _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.f_name
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.f_tparams
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==)
                                    _visitors_this.f_where_constraints
                                    _visitors_r5)
                                 (Pervasives.(&&)
                                    (Pervasives.(==)
                                       _visitors_this.f_variadic _visitors_r6)
                                    (Pervasives.(&&)
                                       (Pervasives.(==)
                                          _visitors_this.f_params
                                          _visitors_r7)
                                       (Pervasives.(&&)
                                          (Pervasives.(==)
                                             _visitors_this.f_body
                                             _visitors_r8)
                                          (Pervasives.(&&)
                                             (Pervasives.(==)
                                                _visitors_this.f_fun_kind
                                                _visitors_r9)
                                             (Pervasives.(&&)
                                                (Pervasives.(==)
                                                   _visitors_this.f_user_attributes
                                                   _visitors_r10)
                                                (Pervasives.(==)
                                                   _visitors_this.f_ret_by_ref
                                                   _visitors_r11)))))))))))
              then _visitors_this
              else
                {
                  f_annotation = _visitors_r0;
                  f_mode = _visitors_r1;
                  f_ret = _visitors_r2;
                  f_name = _visitors_r3;
                  f_tparams = _visitors_r4;
                  f_where_constraints = _visitors_r5;
                  f_variadic = _visitors_r6;
                  f_params = _visitors_r7;
                  f_body = _visitors_r8;
                  f_fun_kind = _visitors_r9;
                  f_user_attributes = _visitors_r10;
                  f_ret_by_ref = _visitors_r11
                }
            method on_UnnamedBody env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_func_unnamed_body env _visitors_c0
                 in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else UnnamedBody _visitors_r0
            method on_NamedBody env _visitors_this _visitors_c0 =
              let _visitors_r0 = self#on_func_named_body env _visitors_c0  in
              if Pervasives.(==) _visitors_c0 _visitors_r0
              then _visitors_this
              else NamedBody _visitors_r0
            method on_func_body env _visitors_this =
              match _visitors_this with
              | UnnamedBody _visitors_c0 as _visitors_this ->
                  self#on_UnnamedBody env _visitors_this _visitors_c0
              | NamedBody _visitors_c0 as _visitors_this ->
                  self#on_NamedBody env _visitors_this _visitors_c0
            method on_func_unnamed_body env _visitors_this =
              let _visitors_r0 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_ast
                 in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_tparams
                 in
              let _visitors_r2 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.fub_namespace
                 in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.fub_ast _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.fub_tparams _visitors_r1)
                     (Pervasives.(==) _visitors_this.fub_namespace
                        _visitors_r2))
              then _visitors_this
              else
                {
                  fub_ast = _visitors_r0;
                  fub_tparams = _visitors_r1;
                  fub_namespace = _visitors_r2
                }
            method on_func_named_body env _visitors_this =
              let _visitors_r0 = self#on_block env _visitors_this.fnb_nast
                 in
              let _visitors_r1 = self#on_bool env _visitors_this.fnb_unsafe
                 in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.fnb_nast _visitors_r0)
                  (Pervasives.(==) _visitors_this.fnb_unsafe _visitors_r1)
              then _visitors_this
              else { fnb_nast = _visitors_r0; fnb_unsafe = _visitors_r1 }
            method on_user_attribute env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.ua_name  in
              let _visitors_r1 =
                self#on_list self#on_expr env _visitors_this.ua_params  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.ua_name _visitors_r0)
                  (Pervasives.(==) _visitors_this.ua_params _visitors_r1)
              then _visitors_this
              else { ua_name = _visitors_r0; ua_params = _visitors_r1 }
            method on_static_var env = self#on_class_var env
            method on_static_method env = self#on_method_ env
            method on_class_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.c_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.c_mode
                 in
              let _visitors_r2 = self#on_bool env _visitors_this.c_final  in
              let _visitors_r3 = self#on_bool env _visitors_this.c_is_xhp  in
              let _visitors_r4 = self#on_class_kind env _visitors_this.c_kind
                 in
              let _visitors_r5 = self#on_sid env _visitors_this.c_name  in
              let _visitors_r6 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.c_tparams
                 in
              let _visitors_r7 =
                self#on_list self#on_hint env _visitors_this.c_extends  in
              let _visitors_r8 =
                self#on_list self#on_hint env _visitors_this.c_uses  in
              let _visitors_r9 =
                self#on_list self#on_hint env _visitors_this.c_xhp_attr_uses
                 in
              let _visitors_r10 =
                self#on_list self#on_pstring env
                  _visitors_this.c_xhp_category
                 in
              let _visitors_r11 =
                self#on_list self#on_hint env _visitors_this.c_req_extends
                 in
              let _visitors_r12 =
                self#on_list self#on_hint env _visitors_this.c_req_implements
                 in
              let _visitors_r13 =
                self#on_list self#on_hint env _visitors_this.c_implements  in
              let _visitors_r14 =
                self#on_list self#on_class_const env _visitors_this.c_consts
                 in
              let _visitors_r15 =
                self#on_list self#on_class_typeconst env
                  _visitors_this.c_typeconsts
                 in
              let _visitors_r16 =
                self#on_list self#on_static_var env
                  _visitors_this.c_static_vars
                 in
              let _visitors_r17 =
                self#on_list self#on_class_var env _visitors_this.c_vars  in
              let _visitors_r18 =
                self#on_option self#on_method_ env
                  _visitors_this.c_constructor
                 in
              let _visitors_r19 =
                self#on_list self#on_static_method env
                  _visitors_this.c_static_methods
                 in
              let _visitors_r20 =
                self#on_list self#on_method_ env _visitors_this.c_methods  in
              let _visitors_r21 =
                self#on_list self#on_user_attribute env
                  _visitors_this.c_user_attributes
                 in
              let _visitors_r22 =
                self#on_option self#on_enum_ env _visitors_this.c_enum  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.c_annotation _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.c_mode _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.c_final _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.c_is_xhp
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.c_kind
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==) _visitors_this.c_name
                                    _visitors_r5)
                                 (Pervasives.(&&)
                                    (Pervasives.(==) _visitors_this.c_tparams
                                       _visitors_r6)
                                    (Pervasives.(&&)
                                       (Pervasives.(==)
                                          _visitors_this.c_extends
                                          _visitors_r7)
                                       (Pervasives.(&&)
                                          (Pervasives.(==)
                                             _visitors_this.c_uses
                                             _visitors_r8)
                                          (Pervasives.(&&)
                                             (Pervasives.(==)
                                                _visitors_this.c_xhp_attr_uses
                                                _visitors_r9)
                                             (Pervasives.(&&)
                                                (Pervasives.(==)
                                                   _visitors_this.c_xhp_category
                                                   _visitors_r10)
                                                (Pervasives.(&&)
                                                   (Pervasives.(==)
                                                      _visitors_this.c_req_extends
                                                      _visitors_r11)
                                                   (Pervasives.(&&)
                                                      (Pervasives.(==)
                                                         _visitors_this.c_req_implements
                                                         _visitors_r12)
                                                      (Pervasives.(&&)
                                                         (Pervasives.(==)
                                                            _visitors_this.c_implements
                                                            _visitors_r13)
                                                         (Pervasives.(&&)
                                                            (Pervasives.(==)
                                                               _visitors_this.c_consts
                                                               _visitors_r14)
                                                            (Pervasives.(&&)
                                                               (Pervasives.(==)
                                                                  _visitors_this.c_typeconsts
                                                                  _visitors_r15)
                                                               (Pervasives.(&&)
                                                                  (Pervasives.(==)
                                                                    _visitors_this.c_static_vars
                                                                    _visitors_r16)
                                                                  (Pervasives.(&&)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_vars
                                                                    _visitors_r17)
                                                                    (Pervasives.(&&)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_constructor
                                                                    _visitors_r18)
                                                                    (Pervasives.(&&)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_static_methods
                                                                    _visitors_r19)
                                                                    (Pervasives.(&&)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_methods
                                                                    _visitors_r20)
                                                                    (Pervasives.(&&)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_user_attributes
                                                                    _visitors_r21)
                                                                    (Pervasives.(==)
                                                                    _visitors_this.c_enum
                                                                    _visitors_r22))))))))))))))))))))))
              then _visitors_this
              else
                {
                  c_annotation = _visitors_r0;
                  c_mode = _visitors_r1;
                  c_final = _visitors_r2;
                  c_is_xhp = _visitors_r3;
                  c_kind = _visitors_r4;
                  c_name = _visitors_r5;
                  c_tparams = _visitors_r6;
                  c_extends = _visitors_r7;
                  c_uses = _visitors_r8;
                  c_xhp_attr_uses = _visitors_r9;
                  c_xhp_category = _visitors_r10;
                  c_req_extends = _visitors_r11;
                  c_req_implements = _visitors_r12;
                  c_implements = _visitors_r13;
                  c_consts = _visitors_r14;
                  c_typeconsts = _visitors_r15;
                  c_static_vars = _visitors_r16;
                  c_vars = _visitors_r17;
                  c_constructor = _visitors_r18;
                  c_static_methods = _visitors_r19;
                  c_methods = _visitors_r20;
                  c_user_attributes = _visitors_r21;
                  c_enum = _visitors_r22
                }
            method on_class_const env
              ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
              let _visitors_r0 = self#on_option self#on_hint env _visitors_c0
                 in
              let _visitors_r1 = self#on_sid env _visitors_c1  in
              let _visitors_r2 = self#on_option self#on_expr env _visitors_c2
                 in
              if
                Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_c1 _visitors_r1)
                     (Pervasives.(==) _visitors_c2 _visitors_r2))
              then _visitors_this
              else (_visitors_r0, _visitors_r1, _visitors_r2)
            method on_class_typeconst env _visitors_this =
              let _visitors_r0 = self#on_sid env _visitors_this.c_tconst_name
                 in
              let _visitors_r1 =
                self#on_option self#on_hint env
                  _visitors_this.c_tconst_constraint
                 in
              let _visitors_r2 =
                self#on_option self#on_hint env _visitors_this.c_tconst_type
                 in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.c_tconst_name _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.c_tconst_constraint
                        _visitors_r1)
                     (Pervasives.(==) _visitors_this.c_tconst_type
                        _visitors_r2))
              then _visitors_this
              else
                {
                  c_tconst_name = _visitors_r0;
                  c_tconst_constraint = _visitors_r1;
                  c_tconst_type = _visitors_r2
                }
            method on_class_var env _visitors_this =
              let _visitors_r0 = self#on_bool env _visitors_this.cv_final  in
              let _visitors_r1 = self#on_bool env _visitors_this.cv_is_xhp
                 in
              let _visitors_r2 =
                self#on_visibility env _visitors_this.cv_visibility  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cv_type  in
              let _visitors_r4 = self#on_sid env _visitors_this.cv_id  in
              let _visitors_r5 =
                self#on_option self#on_expr env _visitors_this.cv_expr  in
              let _visitors_r6 =
                self#on_list self#on_user_attribute env
                  _visitors_this.cv_user_attributes
                 in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.cv_final _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.cv_is_xhp _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.cv_visibility
                           _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.cv_type
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.cv_id
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==) _visitors_this.cv_expr
                                    _visitors_r5)
                                 (Pervasives.(==)
                                    _visitors_this.cv_user_attributes
                                    _visitors_r6))))))
              then _visitors_this
              else
                {
                  cv_final = _visitors_r0;
                  cv_is_xhp = _visitors_r1;
                  cv_visibility = _visitors_r2;
                  cv_type = _visitors_r3;
                  cv_id = _visitors_r4;
                  cv_expr = _visitors_r5;
                  cv_user_attributes = _visitors_r6
                }
            method on_method_ env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.m_annotation  in
              let _visitors_r1 = self#on_bool env _visitors_this.m_final  in
              let _visitors_r2 = self#on_bool env _visitors_this.m_abstract
                 in
              let _visitors_r3 =
                self#on_visibility env _visitors_this.m_visibility  in
              let _visitors_r4 = self#on_sid env _visitors_this.m_name  in
              let _visitors_r5 =
                self#on_list self#on_tparam env _visitors_this.m_tparams  in
              let _visitors_r6 =
                self#on_list self#on_where_constraint env
                  _visitors_this.m_where_constraints
                 in
              let _visitors_r7 =
                self#on_fun_variadicity env _visitors_this.m_variadic  in
              let _visitors_r8 =
                self#on_list self#on_fun_param env _visitors_this.m_params
                 in
              let _visitors_r9 = self#on_func_body env _visitors_this.m_body
                 in
              let _visitors_r10 =
                self#on_fun_kind env _visitors_this.m_fun_kind  in
              let _visitors_r11 =
                self#on_list self#on_user_attribute env
                  _visitors_this.m_user_attributes
                 in
              let _visitors_r12 =
                self#on_option self#on_hint env _visitors_this.m_ret  in
              let _visitors_r13 =
                self#on_bool env _visitors_this.m_ret_by_ref  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.m_annotation _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.m_final _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.m_abstract
                           _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.m_visibility
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.m_name
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==) _visitors_this.m_tparams
                                    _visitors_r5)
                                 (Pervasives.(&&)
                                    (Pervasives.(==)
                                       _visitors_this.m_where_constraints
                                       _visitors_r6)
                                    (Pervasives.(&&)
                                       (Pervasives.(==)
                                          _visitors_this.m_variadic
                                          _visitors_r7)
                                       (Pervasives.(&&)
                                          (Pervasives.(==)
                                             _visitors_this.m_params
                                             _visitors_r8)
                                          (Pervasives.(&&)
                                             (Pervasives.(==)
                                                _visitors_this.m_body
                                                _visitors_r9)
                                             (Pervasives.(&&)
                                                (Pervasives.(==)
                                                   _visitors_this.m_fun_kind
                                                   _visitors_r10)
                                                (Pervasives.(&&)
                                                   (Pervasives.(==)
                                                      _visitors_this.m_user_attributes
                                                      _visitors_r11)
                                                   (Pervasives.(&&)
                                                      (Pervasives.(==)
                                                         _visitors_this.m_ret
                                                         _visitors_r12)
                                                      (Pervasives.(==)
                                                         _visitors_this.m_ret_by_ref
                                                         _visitors_r13)))))))))))))
              then _visitors_this
              else
                {
                  m_annotation = _visitors_r0;
                  m_final = _visitors_r1;
                  m_abstract = _visitors_r2;
                  m_visibility = _visitors_r3;
                  m_name = _visitors_r4;
                  m_tparams = _visitors_r5;
                  m_where_constraints = _visitors_r6;
                  m_variadic = _visitors_r7;
                  m_params = _visitors_r8;
                  m_body = _visitors_r9;
                  m_fun_kind = _visitors_r10;
                  m_user_attributes = _visitors_r11;
                  m_ret = _visitors_r12;
                  m_ret_by_ref = _visitors_r13
                }
            method on_typedef env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.t_annotation  in
              let _visitors_r1 = self#on_sid env _visitors_this.t_name  in
              let _visitors_r2 =
                self#on_list self#on_tparam env _visitors_this.t_tparams  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.t_constraint
                 in
              let _visitors_r4 = self#on_hint env _visitors_this.t_kind  in
              let _visitors_r5 =
                self#on_list self#on_user_attribute env
                  _visitors_this.t_user_attributes
                 in
              let _visitors_r6 =
                (fun _visitors_this  -> _visitors_this) _visitors_this.t_mode
                 in
              let _visitors_r7 =
                self#on_typedef_visibility env _visitors_this.t_vis  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.t_annotation _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.t_name _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.t_tparams
                           _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.t_constraint
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.t_kind
                                 _visitors_r4)
                              (Pervasives.(&&)
                                 (Pervasives.(==)
                                    _visitors_this.t_user_attributes
                                    _visitors_r5)
                                 (Pervasives.(&&)
                                    (Pervasives.(==) _visitors_this.t_mode
                                       _visitors_r6)
                                    (Pervasives.(==) _visitors_this.t_vis
                                       _visitors_r7)))))))
              then _visitors_this
              else
                {
                  t_annotation = _visitors_r0;
                  t_name = _visitors_r1;
                  t_tparams = _visitors_r2;
                  t_constraint = _visitors_r3;
                  t_kind = _visitors_r4;
                  t_user_attributes = _visitors_r5;
                  t_mode = _visitors_r6;
                  t_vis = _visitors_r7
                }
            method on_gconst env _visitors_this =
              let _visitors_r0 =
                self#on_env_annotation env _visitors_this.cst_annotation  in
              let _visitors_r1 =
                (fun _visitors_this  -> _visitors_this)
                  _visitors_this.cst_mode
                 in
              let _visitors_r2 = self#on_sid env _visitors_this.cst_name  in
              let _visitors_r3 =
                self#on_option self#on_hint env _visitors_this.cst_type  in
              let _visitors_r4 =
                self#on_option self#on_expr env _visitors_this.cst_value  in
              let _visitors_r5 =
                self#on_bool env _visitors_this.cst_is_define  in
              if
                Pervasives.(&&)
                  (Pervasives.(==) _visitors_this.cst_annotation _visitors_r0)
                  (Pervasives.(&&)
                     (Pervasives.(==) _visitors_this.cst_mode _visitors_r1)
                     (Pervasives.(&&)
                        (Pervasives.(==) _visitors_this.cst_name _visitors_r2)
                        (Pervasives.(&&)
                           (Pervasives.(==) _visitors_this.cst_type
                              _visitors_r3)
                           (Pervasives.(&&)
                              (Pervasives.(==) _visitors_this.cst_value
                                 _visitors_r4)
                              (Pervasives.(==) _visitors_this.cst_is_define
                                 _visitors_r5)))))
              then _visitors_this
              else
                {
                  cst_annotation = _visitors_r0;
                  cst_mode = _visitors_r1;
                  cst_name = _visitors_r2;
                  cst_type = _visitors_r3;
                  cst_value = _visitors_r4;
                  cst_is_define = _visitors_r5
                }
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
            method on_def env _visitors_this =
              match _visitors_this with
              | Fun _visitors_c0 as _visitors_this ->
                  self#on_Fun env _visitors_this _visitors_c0
              | Class _visitors_c0 as _visitors_this ->
                  self#on_Class env _visitors_this _visitors_c0
              | Typedef _visitors_c0 as _visitors_this ->
                  self#on_Typedef env _visitors_this _visitors_c0
              | Constant _visitors_c0 as _visitors_this ->
                  self#on_Constant env _visitors_this _visitors_c0
          end
        [@@@VISITORS.END ]
      end
    let expr_to_string expr =
      match expr with
      | Any  -> "Any"
      | Array _ -> "Array"
      | Darray _ -> "Darray"
      | Varray _ -> "Varray"
      | Shape _ -> "Shape"
      | ValCollection _ -> "ValCollection"
      | KeyValCollection _ -> "KeyValCollection"
      | This  -> "This"
      | Id _ -> "Id"
      | Lvar _ -> "Lvar"
      | Dollar _ -> "Dollar"
      | Lplaceholder _ -> "Lplaceholder"
      | Dollardollar _ -> "Dollardollar"
      | Fun_id _ -> "Fun_id"
      | Method_id _ -> "Method_id"
      | Method_caller _ -> "Method_caller"
      | Smethod_id _ -> "Smethod_id"
      | Obj_get _ -> "Obj_get"
      | Array_get _ -> "Array_get"
      | Class_get _ -> "Class_get"
      | Class_const _ -> "Class_const"
      | Call _ -> "Call"
      | True  -> "True"
      | False  -> "False"
      | Int _ -> "Int"
      | Float _ -> "Float"
      | Null  -> "Null"
      | String _ -> "String"
      | String2 _ -> "String2"
      | Special_func _ -> "Special_func"
      | Yield_break  -> "Yield_break"
      | Yield _ -> "Yield"
      | Await _ -> "Await"
      | Suspend _ -> "Suspend"
      | List _ -> "List"
      | Pair _ -> "Pair"
      | Expr_list _ -> "Expr_list"
      | Cast _ -> "Cast"
      | Unop _ -> "Unop"
      | Binop _ -> "Binop"
      | Pipe _ -> "Pipe"
      | Eif _ -> "Eif"
      | NullCoalesce _ -> "NullCoalesce"
      | InstanceOf _ -> "InstanceOf"
      | Is _ -> "Is"
      | As _ -> "As"
      | New _ -> "New"
      | Efun _ -> "Efun"
      | Xml _ -> "Xml"
      | Callconv _ -> "Callconv"
      | Assert _ -> "Assert"
      | Clone _ -> "Clone"
      | Typename _ -> "Typename" 
    module Visitor =
      struct
        class type ['a] visitor_type =
          object
            method  on_block : 'a -> block -> 'a
            method  on_break : 'a -> Pos.t -> 'a
            method  on_case : 'a -> case -> 'a
            method  on_catch : 'a -> catch -> 'a
            method  on_continue : 'a -> Pos.t -> 'a
            method  on_do : 'a -> block -> expr -> 'a
            method  on_expr : 'a -> expr -> 'a
            method  on_expr_ : 'a -> expr_ -> 'a
            method  on_for : 'a -> expr -> expr -> expr -> block -> 'a
            method  on_foreach : 'a -> expr -> as_expr -> block -> 'a
            method  on_if : 'a -> expr -> block -> block -> 'a
            method  on_noop : 'a -> 'a
            method  on_fallthrough : 'a -> 'a
            method  on_return : 'a -> Pos.t -> expr option -> 'a
            method  on_goto_label : 'a -> pstring -> 'a
            method  on_goto : 'a -> pstring -> 'a
            method  on_static_var : 'a -> expr list -> 'a
            method  on_global_var : 'a -> expr list -> 'a
            method  on_stmt : 'a -> stmt -> 'a
            method  on_switch : 'a -> expr -> case list -> 'a
            method  on_throw : 'a -> is_terminal -> expr -> 'a
            method  on_try : 'a -> block -> catch list -> block -> 'a
            method  on_while : 'a -> expr -> block -> 'a
            method  on_using : 'a -> bool -> expr -> block -> 'a
            method  on_as_expr : 'a -> as_expr -> 'a
            method  on_array : 'a -> afield list -> 'a
            method  on_shape : 'a -> expr ShapeMap.t -> 'a
            method  on_valCollection : 'a -> vc_kind -> expr list -> 'a
            method  on_keyValCollection : 'a -> kvc_kind -> field list -> 'a
            method  on_this : 'a -> 'a
            method  on_id : 'a -> sid -> 'a
            method  on_lvar : 'a -> id -> 'a
            method  on_dollar : 'a -> expr -> 'a
            method  on_dollardollar : 'a -> id -> 'a
            method  on_fun_id : 'a -> sid -> 'a
            method  on_method_id : 'a -> expr -> pstring -> 'a
            method  on_smethod_id : 'a -> sid -> pstring -> 'a
            method  on_method_caller : 'a -> sid -> pstring -> 'a
            method  on_obj_get : 'a -> expr -> expr -> 'a
            method  on_array_get : 'a -> expr -> expr option -> 'a
            method  on_class_get : 'a -> class_id -> pstring -> 'a
            method  on_class_const : 'a -> class_id -> pstring -> 'a
            method  on_call :
              'a -> call_type -> expr -> expr list -> expr list -> 'a
            method  on_true : 'a -> 'a
            method  on_false : 'a -> 'a
            method  on_int : 'a -> pstring -> 'a
            method  on_float : 'a -> pstring -> 'a
            method  on_null : 'a -> 'a
            method  on_string : 'a -> pstring -> 'a
            method  on_string2 : 'a -> expr list -> 'a
            method  on_special_func : 'a -> special_func -> 'a
            method  on_yield_break : 'a -> 'a
            method  on_yield : 'a -> afield -> 'a
            method  on_await : 'a -> expr -> 'a
            method  on_suspend : 'a -> expr -> 'a
            method  on_list : 'a -> expr list -> 'a
            method  on_pair : 'a -> expr -> expr -> 'a
            method  on_expr_list : 'a -> expr list -> 'a
            method  on_cast : 'a -> hint -> expr -> 'a
            method  on_unop : 'a -> Ast.uop -> expr -> 'a
            method  on_binop : 'a -> Ast.bop -> expr -> expr -> 'a
            method  on_pipe : 'a -> id -> expr -> expr -> 'a
            method  on_eif : 'a -> expr -> expr option -> expr -> 'a
            method  on_nullCoalesce : 'a -> expr -> expr -> 'a
            method  on_typename : 'a -> sid -> 'a
            method  on_instanceOf : 'a -> expr -> class_id -> 'a
            method  on_is : 'a -> expr -> hint -> 'a
            method  on_as : 'a -> expr -> hint -> bool -> 'a
            method  on_class_id : 'a -> class_id -> 'a
            method  on_class_id_ : 'a -> class_id_ -> 'a
            method  on_new : 'a -> class_id -> expr list -> expr list -> 'a
            method  on_efun : 'a -> fun_ -> id list -> 'a
            method  on_xml :
              'a -> sid -> xhp_attribute list -> expr list -> 'a
            method  on_param_kind : 'a -> Ast.param_kind -> 'a
            method  on_callconv : 'a -> Ast.param_kind -> expr -> 'a
            method  on_assert : 'a -> assert_expr -> 'a
            method  on_clone : 'a -> expr -> 'a
            method  on_field : 'a -> field -> 'a
            method  on_afield : 'a -> afield -> 'a
            method  on_func_named_body : 'a -> func_named_body -> 'a
            method  on_func_unnamed_body : 'a -> func_unnamed_body -> 'a
            method  on_func_body : 'a -> func_body -> 'a
            method  on_method_ : 'a -> method_ -> 'a
            method  on_fun_ : 'a -> fun_ -> 'a
            method  on_class_ : 'a -> class_ -> 'a
            method  on_gconst : 'a -> gconst -> 'a
            method  on_typedef : 'a -> typedef -> 'a
            method  on_hint : 'a -> hint -> 'a
            method  on_def : 'a -> def -> 'a
            method  on_program : 'a -> program -> 'a
          end
        class virtual ['a] visitor : ['a] visitor_type =
          object (this)
            method on_break acc _ = acc
            method on_continue acc _ = acc
            method on_noop acc = acc
            method on_fallthrough acc = acc
            method on_goto_label acc _ = acc
            method on_goto acc _ = acc
            method on_throw acc _ e = let acc = this#on_expr acc e  in acc
            method on_return acc _ eopt =
              match eopt with | None  -> acc | Some e -> this#on_expr acc e
            method on_static_var acc el = List.fold_left this#on_expr acc el
            method on_global_var acc el = List.fold_left this#on_expr acc el
            method on_if acc e b1 b2 =
              let acc = this#on_expr acc e  in
              let acc = this#on_block acc b1  in
              let acc = this#on_block acc b2  in acc
            method on_do acc b e =
              let acc = this#on_block acc b  in
              let acc = this#on_expr acc e  in acc
            method on_while acc e b =
              let acc = this#on_expr acc e  in
              let acc = this#on_block acc b  in acc
            method on_using acc _has_await e b =
              let acc = this#on_expr acc e  in
              let acc = this#on_block acc b  in acc
            method on_for acc e1 e2 e3 b =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in
              let acc = this#on_expr acc e3  in
              let acc = this#on_block acc b  in acc
            method on_switch acc e cl =
              let acc = this#on_expr acc e  in
              let acc = List.fold_left this#on_case acc cl  in acc
            method on_foreach acc e ae b =
              let acc = this#on_expr acc e  in
              let acc = this#on_as_expr acc ae  in
              let acc = this#on_block acc b  in acc
            method on_try acc b cl fb =
              let acc = this#on_block acc b  in
              let acc = List.fold_left this#on_catch acc cl  in
              let acc = this#on_block acc fb  in acc
            method on_block acc b = List.fold_left this#on_stmt acc b
            method on_case acc =
              function
              | Default b -> let acc = this#on_block acc b  in acc
              | Case (e,b) ->
                  let acc = this#on_expr acc e  in
                  let acc = this#on_block acc b  in acc
            method on_as_expr acc =
              function
              | As_v e|Await_as_v (_,e) ->
                  let acc = this#on_expr acc e  in acc
              | As_kv (e1,e2)|Await_as_kv (_,e1,e2) ->
                  let acc = this#on_expr acc e1  in
                  let acc = this#on_expr acc e2  in acc
            method on_catch acc (_,_,b) = this#on_block acc b
            method on_stmt acc =
              function
              | Expr e -> this#on_expr acc e
              | Break p -> this#on_break acc p
              | Continue p -> this#on_continue acc p
              | Throw (is_term,e) -> this#on_throw acc is_term e
              | Return (p,eopt) -> this#on_return acc p eopt
              | GotoLabel label -> this#on_goto_label acc label
              | Goto label -> this#on_goto acc label
              | If (e,b1,b2) -> this#on_if acc e b1 b2
              | Do (b,e) -> this#on_do acc b e
              | While (e,b) -> this#on_while acc e b
              | Using (has_await,e,b) -> this#on_using acc has_await e b
              | For (e1,e2,e3,b) -> this#on_for acc e1 e2 e3 b
              | Switch (e,cl) -> this#on_switch acc e cl
              | Foreach (e,ae,b) -> this#on_foreach acc e ae b
              | Try (b,cl,fb) -> this#on_try acc b cl fb
              | Noop  -> this#on_noop acc
              | Fallthrough  -> this#on_fallthrough acc
              | Static_var el -> this#on_static_var acc el
              | Global_var el -> this#on_global_var acc el
            method on_expr acc (_,e) = this#on_expr_ acc e
            method on_expr_ acc e =
              match e with
              | Any  -> acc
              | Array afl -> this#on_array acc afl
              | Darray fieldl -> List.fold_left this#on_field acc fieldl
              | Varray el -> List.fold_left this#on_expr acc el
              | Shape sh -> this#on_shape acc sh
              | True  -> this#on_true acc
              | False  -> this#on_false acc
              | Int n -> this#on_int acc n
              | Float n -> this#on_float acc n
              | Null  -> this#on_null acc
              | String s -> this#on_string acc s
              | This  -> this#on_this acc
              | Id sid -> this#on_id acc sid
              | Lplaceholder _pos -> acc
              | Dollardollar id -> this#on_dollardollar acc id
              | Lvar id -> this#on_lvar acc id
              | Dollar e -> this#on_dollar acc e
              | Fun_id sid -> this#on_fun_id acc sid
              | Method_id (expr,pstr) -> this#on_method_id acc expr pstr
              | Method_caller (sid,pstr) ->
                  this#on_method_caller acc sid pstr
              | Smethod_id (sid,pstr) -> this#on_smethod_id acc sid pstr
              | Yield_break  -> this#on_yield_break acc
              | Yield e -> this#on_yield acc e
              | Await e -> this#on_await acc e
              | Suspend e -> this#on_suspend acc e
              | List el -> this#on_list acc el
              | Assert ae -> this#on_assert acc ae
              | Clone e -> this#on_clone acc e
              | Expr_list el -> this#on_expr_list acc el
              | Special_func sf -> this#on_special_func acc sf
              | Obj_get (e1,e2,_) -> this#on_obj_get acc e1 e2
              | Array_get (e1,e2) -> this#on_array_get acc e1 e2
              | Class_get (cid,id) -> this#on_class_get acc cid id
              | Class_const (cid,id) -> this#on_class_const acc cid id
              | Call (ct,e,_,el,uel) -> this#on_call acc ct e el uel
              | String2 el -> this#on_string2 acc el
              | Pair (e1,e2) -> this#on_pair acc e1 e2
              | Cast (hint,e) -> this#on_cast acc hint e
              | Unop (uop,e) -> this#on_unop acc uop e
              | Binop (bop,e1,e2) -> this#on_binop acc bop e1 e2
              | Pipe (id,e1,e2) -> this#on_pipe acc id e1 e2
              | Eif (e1,e2,e3) -> this#on_eif acc e1 e2 e3
              | NullCoalesce (e1,e2) -> this#on_nullCoalesce acc e1 e2
              | InstanceOf (e1,e2) -> this#on_instanceOf acc e1 e2
              | Is (e,h) -> this#on_is acc e h
              | As (e,h,b) -> this#on_as acc e h b
              | Typename n -> this#on_typename acc n
              | New (cid,el,uel) -> this#on_new acc cid el uel
              | Efun (f,idl) -> this#on_efun acc f idl
              | Xml (sid,attrl,el) -> this#on_xml acc sid attrl el
              | Callconv (kind,e) -> this#on_callconv acc kind e
              | ValCollection (s,el) -> this#on_valCollection acc s el
              | KeyValCollection (s,fl) -> this#on_keyValCollection acc s fl
            method on_array acc afl = List.fold_left this#on_afield acc afl
            method on_shape acc sm =
              ShapeMap.fold
                (fun _  ->
                   fun e  -> fun acc  -> let acc = this#on_expr acc e  in acc)
                sm acc
            method on_valCollection acc _ el =
              List.fold_left this#on_expr acc el
            method on_keyValCollection acc _ fieldl =
              List.fold_left this#on_field acc fieldl
            method on_this acc = acc
            method on_id acc _ = acc
            method on_lvar acc _ = acc
            method on_dollardollar acc id = this#on_lvar acc id
            method on_fun_id acc _ = acc
            method on_method_id acc _ _ = acc
            method on_smethod_id acc _ _ = acc
            method on_method_caller acc _ _ = acc
            method on_typename acc _ = acc
            method on_obj_get acc e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_array_get acc e e_opt =
              let acc = this#on_expr acc e  in
              let acc =
                match e_opt with
                | None  -> acc
                | Some e -> this#on_expr acc e  in
              acc
            method on_class_get acc cid _ = this#on_class_id acc cid
            method on_class_const acc cid _ = this#on_class_id acc cid
            method on_call acc _ e el uel =
              let acc = this#on_expr acc e  in
              let acc = List.fold_left this#on_expr acc el  in
              let acc = List.fold_left this#on_expr acc uel  in acc
            method on_true acc = acc
            method on_false acc = acc
            method on_int acc _ = acc
            method on_float acc _ = acc
            method on_null acc = acc
            method on_string acc _ = acc
            method on_string2 acc el =
              let acc = List.fold_left this#on_expr acc el  in acc
            method on_special_func acc =
              function
              | Gena e|Gen_array_rec e -> this#on_expr acc e
              | Genva el -> List.fold_left this#on_expr acc el
            method on_yield_break acc = acc
            method on_yield acc e = this#on_afield acc e
            method on_await acc e = this#on_expr acc e
            method on_dollar acc e = this#on_expr acc e
            method on_suspend acc e = this#on_expr acc e
            method on_list acc el = List.fold_left this#on_expr acc el
            method on_pair acc e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_expr_list acc el =
              let acc = List.fold_left this#on_expr acc el  in acc
            method on_cast acc _ e = this#on_expr acc e
            method on_unop acc _ e = this#on_expr acc e
            method on_binop acc _ e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_pipe acc _id e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_eif acc e1 e2 e3 =
              let acc = this#on_expr acc e1  in
              let acc =
                match e2 with | None  -> acc | Some e -> this#on_expr acc e
                 in
              let acc = this#on_expr acc e3  in acc
            method on_nullCoalesce acc e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_instanceOf acc e1 e2 =
              let acc = this#on_expr acc e1  in
              let acc = this#on_class_id acc e2  in acc
            method on_is acc e _ = this#on_expr acc e
            method on_as acc e _ _ = this#on_expr acc e
            method on_class_id acc (_,cid) = this#on_class_id_ acc cid
            method on_class_id_ acc =
              function | CIexpr e -> this#on_expr acc e | _ -> acc
            method on_new acc cid el uel =
              let acc = this#on_class_id acc cid  in
              let acc = List.fold_left this#on_expr acc el  in
              let acc = List.fold_left this#on_expr acc uel  in acc
            method on_efun acc f _ =
              match f.f_body with
              | UnnamedBody _ ->
                  failwith
                    "lambdas expected to be named in the context of the surrounding function"
              | NamedBody { fnb_nast = n;_} -> this#on_block acc n
            method on_xml acc _ attrl el =
              let acc =
                List.fold_left
                  (fun acc  ->
                     fun attr  ->
                       match attr with
                       | Xhp_simple (_,e)|Xhp_spread e -> this#on_expr acc e)
                  acc attrl
                 in
              let acc = List.fold_left this#on_expr acc el  in acc
            method on_param_kind acc _ = acc
            method on_callconv acc kind e =
              let acc = this#on_param_kind acc kind  in
              let acc = this#on_expr acc e  in acc
            method on_assert acc =
              function | AE_assert e -> this#on_expr acc e
            method on_clone acc e = this#on_expr acc e
            method on_field acc (e1,e2) =
              let acc = this#on_expr acc e1  in
              let acc = this#on_expr acc e2  in acc
            method on_afield acc =
              function
              | AFvalue e -> this#on_expr acc e
              | AFkvalue (e1,e2) ->
                  let acc = this#on_expr acc e1  in
                  let acc = this#on_expr acc e2  in acc
            method on_hint acc _ = acc
            method on_fun_ acc f =
              let acc = this#on_id acc f.f_name  in
              let acc = this#on_func_body acc f.f_body  in
              let acc =
                match f.f_ret with
                | Some h -> this#on_hint acc h
                | None  -> acc  in
              acc
            method on_func_named_body acc fnb =
              this#on_block acc fnb.fnb_nast
            method on_func_unnamed_body acc _ = acc
            method on_func_body acc =
              function
              | UnnamedBody unb -> this#on_func_unnamed_body acc unb
              | NamedBody nb -> this#on_func_named_body acc nb
            method on_method_ acc m =
              let acc = this#on_id acc m.m_name  in
              let acc = this#on_func_body acc m.m_body  in acc
            method on_class_ acc c =
              let acc = this#on_id acc c.c_name  in
              let acc = List.fold_left this#on_hint acc c.c_extends  in
              let acc = List.fold_left this#on_hint acc c.c_uses  in
              let acc = List.fold_left this#on_hint acc c.c_implements  in
              let acc =
                match c.c_constructor with
                | Some ctor -> this#on_method_ acc ctor
                | None  -> acc  in
              let acc = List.fold_left this#on_method_ acc c.c_methods  in
              let acc = List.fold_left this#on_method_ acc c.c_static_methods
                 in
              acc
            method on_gconst acc g =
              let acc = this#on_id acc g.cst_name  in
              let acc =
                match g.cst_value with
                | Some e -> this#on_expr acc e
                | None  -> acc  in
              let acc =
                match g.cst_type with
                | Some h -> this#on_hint acc h
                | None  -> acc  in
              acc
            method on_typedef acc t =
              let acc = this#on_id acc t.t_name  in
              let acc = this#on_hint acc t.t_kind  in
              let acc =
                match t.t_constraint with
                | Some c -> this#on_hint acc c
                | None  -> acc  in
              acc
            method on_def acc =
              function
              | Fun f -> this#on_fun_ acc f
              | Class c -> this#on_class_ acc c
              | Typedef t -> this#on_typedef acc t
              | Constant g -> this#on_gconst acc g
            method on_program acc p =
              let acc =
                List.fold_left (fun acc  -> fun d  -> this#on_def acc d) acc
                  p
                 in
              acc
          end
        module HasReturn : sig val block : block -> bool end =
          struct
            let visitor =
              object
                inherit  [bool] visitor
                method! on_expr acc _ = acc
                method! on_return _ _ _ = true
              end 
            let block b = visitor#on_block false b 
          end 
        class loop_visitor =
          object
            inherit  [bool] visitor
            method! on_expr acc _ = acc
            method! on_for acc _ _ _ _ = acc
            method! on_foreach acc _ _ _ = acc
            method! on_do acc _ _ = acc
            method! on_while acc _ _ = acc
            method! on_switch acc _ _ = acc
          end
        module HasContinue : sig val block : block -> bool end =
          struct
            let visitor =
              object inherit  loop_visitor method! on_continue _ _ = true end 
            let block b = visitor#on_block false b 
          end 
        module HasBreak : sig val block : block -> bool end =
          struct
            let visitor =
              object inherit  loop_visitor method! on_break _ _ = true end 
            let block b = visitor#on_block false b 
          end 
      end
  end
