(* @generated from nast.src.ml by hphp/hack/tools/ppx/ppx_gen. *)
(* SourceShasum<<b7dbe543a8400884fee6a3df1b5e0b0068b65eba>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2015, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
module SN = Naming_special_names
type id =
  (((Pos.t* Local_id.t))[@printer
                          fun fmt  -> fun (_,id)  -> Local_id.pp fmt id])
[@@deriving show]
let rec pp_id : Format.formatter -> id -> Ppx_deriving_runtime.unit =
  let __0 () =
    ((let fprintf = Format.fprintf  in
      fun fmt  -> fun (_,id)  -> Local_id.pp fmt id)
    [@ocaml.warning "-26"])  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_id : id -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_id x

type sid = Ast.id[@@deriving show]
let rec pp_sid : Format.formatter -> sid -> Ppx_deriving_runtime.unit =
  let __0 () = Ast.pp_id  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_sid : sid -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_sid x

type pstring = Ast.pstring[@@deriving show]
let rec pp_pstring : Format.formatter -> pstring -> Ppx_deriving_runtime.unit
  =
  let __0 () = Ast.pp_pstring  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_pstring : pstring -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_pstring x

type is_terminal = bool[@@deriving show]
let rec (pp_is_terminal :
          Format.formatter -> is_terminal -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_terminal : is_terminal -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_terminal x

type call_type =
  | Cnormal 
  | Cuser_func [@@deriving show]
let rec (pp_call_type :
          Format.formatter -> call_type -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Cnormal  -> Format.pp_print_string fmt "Cnormal"
        | Cuser_func  -> Format.pp_print_string fmt "Cuser_func")
  [@ocaml.warning "-A"])

and show_call_type : call_type -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_call_type x

type shape_field_name = Ast.shape_field_name
module ShapeMap = Ast.ShapeMap
type hint =
  (((Pos.t* hint_))[@printer fun fmt  -> fun (_,h)  -> pp_hint_ fmt h])
and hint_ =
  | Hoption of hint 
  | Hfun of hint list* bool* hint 
  | Htuple of hint list 
  | Happly of sid* hint list 
  | Hshape of nast_shape_info 
  | Haccess of hint* sid list 
  | Hany 
  | Hmixed 
  | Habstr of string 
  | Harray of hint option* hint option 
  | Hdarray of hint* hint 
  | Hvarray of hint 
  | Hdarray_or_varray of hint 
  | Hprim of tprim 
  | Hthis 
and tprim =
  | Tvoid 
  | Tint 
  | Tbool 
  | Tfloat 
  | Tstring 
  | Tresource 
  | Tnum 
  | Tarraykey 
  | Tnoreturn 
and shape_field_info = {
  sfi_optional: bool ;
  sfi_hint: hint }
and nast_shape_info =
  {
  nsi_allows_unknown_fields: bool ;
  nsi_field_map: shape_field_info ShapeMap.t }[@@deriving show]
let rec pp_hint : Format.formatter -> hint -> Ppx_deriving_runtime.unit =
  let __0 () =
    ((let fprintf = Format.fprintf  in
      fun fmt  -> fun (_,h)  -> pp_hint_ fmt h)
    [@ocaml.warning "-26"])  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_hint : hint -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_hint x

and pp_hint_ : Format.formatter -> hint_ -> Ppx_deriving_runtime.unit =
  let __15 () = pp_tprim
  
  and __14 () = pp_hint
  
  and __13 () = pp_hint
  
  and __12 () = pp_hint
  
  and __11 () = pp_hint
  
  and __10 () = pp_hint
  
  and __9 () = pp_hint
  
  and __8 () = pp_sid
  
  and __7 () = pp_hint
  
  and __6 () = pp_nast_shape_info
  
  and __5 () = pp_hint
  
  and __4 () = pp_sid
  
  and __3 () = pp_hint
  
  and __2 () = pp_hint
  
  and __1 () = pp_hint
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Hoption a0 ->
            (Format.fprintf fmt "(@[<2>Hoption@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hfun (a0,a1,a2) ->
            (Format.fprintf fmt "(@[<2>Hfun (@,";
             ((((fun x  ->
                   Format.fprintf fmt "@[<2>[";
                   ignore
                     (List.fold_left
                        (fun sep  ->
                           fun x  ->
                             if sep then Format.fprintf fmt ";@ ";
                             ((__1 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) a0;
               Format.fprintf fmt ",@ ";
               (Format.fprintf fmt "%B") a1);
              Format.fprintf fmt ",@ ";
              ((__2 ()) fmt) a2);
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
                           ((__3 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Happly (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Happly (@,";
             (((__4 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((fun x  ->
                  Format.fprintf fmt "@[<2>[";
                  ignore
                    (List.fold_left
                       (fun sep  ->
                          fun x  ->
                            if sep then Format.fprintf fmt ";@ ";
                            ((__5 ()) fmt) x;
                            true) false x);
                  Format.fprintf fmt "@,]@]")) a1);
             Format.fprintf fmt "@,))@]")
        | Hshape a0 ->
            (Format.fprintf fmt "(@[<2>Hshape@ ";
             ((__6 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Haccess (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Haccess (@,";
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
        | Hany  -> Format.pp_print_string fmt "Hany"
        | Hmixed  -> Format.pp_print_string fmt "Hmixed"
        | Habstr a0 ->
            (Format.fprintf fmt "(@[<2>Habstr@ ";
             (Format.fprintf fmt "%S") a0;
             Format.fprintf fmt "@])")
        | Harray (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Harray (@,";
             (((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__9 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a0;
              Format.fprintf fmt ",@ ";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__10 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a1);
             Format.fprintf fmt "@,))@]")
        | Hdarray (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Hdarray (@,";
             (((__11 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__12 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Hvarray a0 ->
            (Format.fprintf fmt "(@[<2>Hvarray@ ";
             ((__13 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hdarray_or_varray a0 ->
            (Format.fprintf fmt "(@[<2>Hdarray_or_varray@ ";
             ((__14 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hprim a0 ->
            (Format.fprintf fmt "(@[<2>Hprim@ ";
             ((__15 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hthis  -> Format.pp_print_string fmt "Hthis")
    [@ocaml.warning "-A"])

and show_hint_ : hint_ -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_hint_ x

and (pp_tprim : Format.formatter -> tprim -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Tvoid  -> Format.pp_print_string fmt "Tvoid"
        | Tint  -> Format.pp_print_string fmt "Tint"
        | Tbool  -> Format.pp_print_string fmt "Tbool"
        | Tfloat  -> Format.pp_print_string fmt "Tfloat"
        | Tstring  -> Format.pp_print_string fmt "Tstring"
        | Tresource  -> Format.pp_print_string fmt "Tresource"
        | Tnum  -> Format.pp_print_string fmt "Tnum"
        | Tarraykey  -> Format.pp_print_string fmt "Tarraykey"
        | Tnoreturn  -> Format.pp_print_string fmt "Tnoreturn")
  [@ocaml.warning "-A"])

and show_tprim : tprim -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_tprim x

and pp_shape_field_info :
  Format.formatter -> shape_field_info -> Ppx_deriving_runtime.unit =
  let __0 () = pp_hint  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((Format.fprintf fmt "@[%s =@ " "sfi_optional";
            (Format.fprintf fmt "%B") x.sfi_optional;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "sfi_hint";
           ((__0 ()) fmt) x.sfi_hint;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_shape_field_info : shape_field_info -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_shape_field_info x

and pp_nast_shape_info :
  Format.formatter -> nast_shape_info -> Ppx_deriving_runtime.unit =
  let __1 () = ShapeMap.pp
  
  and __0 () = pp_shape_field_info
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun x  ->
          Format.fprintf fmt "@[<2>{ ";
          ((Format.fprintf fmt "@[%s =@ " "nsi_allows_unknown_fields";
            (Format.fprintf fmt "%B") x.nsi_allows_unknown_fields;
            Format.fprintf fmt "@]");
           Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[%s =@ " "nsi_field_map";
           ((__1 ()) (fun fmt  -> (__0 ()) fmt) fmt) x.nsi_field_map;
           Format.fprintf fmt "@]");
          Format.fprintf fmt "@ }@]")
    [@ocaml.warning "-A"])

and show_nast_shape_info : nast_shape_info -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_nast_shape_info x

type og_null_flavor =
  | OG_nullthrows 
  | OG_nullsafe [@@deriving show]
let rec (pp_og_null_flavor :
          Format.formatter -> og_null_flavor -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | OG_nullthrows  -> Format.pp_print_string fmt "OG_nullthrows"
        | OG_nullsafe  -> Format.pp_print_string fmt "OG_nullsafe")
  [@ocaml.warning "-A"])

and show_og_null_flavor : og_null_flavor -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_og_null_flavor x

type kvc_kind = [ `Map  | `ImmMap  | `Dict ]
let pp_kvc_kind fmt _ = Format.pp_print_string fmt "<kvc_kind>" 
type vc_kind =
  [ `Vector  | `ImmVector  | `Vec  | `Set  | `ImmSet  | `Pair  | `Keyset ]
let pp_vc_kind fmt _ = Format.pp_print_string fmt "<vc_kind>" 
type tparam = (Ast.variance* sid* (Ast.constraint_kind* hint) list)[@@deriving
                                                                    show]
let rec pp_tparam : Format.formatter -> tparam -> Ppx_deriving_runtime.unit =
  let __3 () = pp_hint
  
  and __2 () = Ast.pp_constraint_kind
  
  and __1 () = pp_sid
  
  and __0 () = Ast.pp_variance
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

type visibility =
  | Private 
  | Public 
  | Protected [@@deriving show]
let rec (pp_visibility :
          Format.formatter -> visibility -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Private  -> Format.pp_print_string fmt "Private"
        | Public  -> Format.pp_print_string fmt "Public"
        | Protected  -> Format.pp_print_string fmt "Protected")
  [@ocaml.warning "-A"])

and show_visibility : visibility -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_visibility x

type typedef_visibility =
  | Transparent 
  | Opaque [@@deriving show]
let rec (pp_typedef_visibility :
          Format.formatter -> typedef_visibility -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Transparent  -> Format.pp_print_string fmt "Transparent"
        | Opaque  -> Format.pp_print_string fmt "Opaque")
  [@ocaml.warning "-A"])

and show_typedef_visibility :
  typedef_visibility -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_typedef_visibility x

type enum_ = {
  e_base: hint ;
  e_constraint: hint option }[@@deriving show]
let rec pp_enum_ : Format.formatter -> enum_ -> Ppx_deriving_runtime.unit =
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

type instantiated_sid = (sid* hint list)[@@deriving show]
let rec pp_instantiated_sid :
  Format.formatter -> instantiated_sid -> Ppx_deriving_runtime.unit =
  let __1 () = pp_hint
  
  and __0 () = pp_sid
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0;
           Format.fprintf fmt ",@ ";
           ((fun x  ->
               Format.fprintf fmt "@[<2>[";
               ignore
                 (List.fold_left
                    (fun sep  ->
                       fun x  ->
                         if sep then Format.fprintf fmt ";@ ";
                         ((__1 ()) fmt) x;
                         true) false x);
               Format.fprintf fmt "@,]@]")) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_instantiated_sid : instantiated_sid -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_instantiated_sid x

type where_constraint = (hint* Ast.constraint_kind* hint)[@@deriving show]
let rec pp_where_constraint :
  Format.formatter -> where_constraint -> Ppx_deriving_runtime.unit =
  let __2 () = pp_hint
  
  and __1 () = Ast.pp_constraint_kind
  
  and __0 () = pp_hint
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

and show_where_constraint : where_constraint -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_where_constraint x

module type AnnotationType  =
  sig type t val pp : Format.formatter -> t -> unit end
module AnnotatedAST(Annotation:AnnotationType) =
  struct
    type stmt =
      | Fallthrough 
      | Expr of expr 
      | Break of Pos.t 
      | Continue of Pos.t 
      | Throw of is_terminal* expr 
      | Return of Pos.t* expr option 
      | GotoLabel of pstring 
      | Goto of pstring 
      | Static_var of expr list 
      | Global_var of expr list 
      | If of expr* block* block 
      | Do of block* expr 
      | While of expr* block 
      | For of expr* expr* expr* block 
      | Switch of expr* case list 
      | Foreach of expr* as_expr* block 
      | Try of block* catch list* block 
      | Noop 
    and as_expr =
      | As_v of expr 
      | As_kv of expr* expr 
      | Await_as_v of Pos.t* expr 
      | Await_as_kv of Pos.t* expr* expr 
    and block = stmt list
    and class_id =
      | CIparent 
      | CIself 
      | CIstatic 
      | CIexpr of expr 
      | CI of instantiated_sid 
    and expr = (Annotation.t* expr_)
    and expr_ =
      | Array of afield list 
      | Darray of (expr* expr) list 
      | Varray of expr list 
      | Shape of expr ShapeMap.t 
      | ValCollection of vc_kind* expr list 
      | KeyValCollection of kvc_kind* field list 
      | Null 
      | This 
      | True 
      | False 
      | Id of sid 
      | Lvar of id 
      | Lvarvar of int* id 
      | Dollardollar of id 
      | Clone of expr 
      | Obj_get of expr* expr* og_null_flavor 
      | Array_get of expr* expr option 
      | Class_get of class_id* pstring 
      | Class_const of class_id* pstring 
      | Call of call_type* expr* expr list* expr list 
      | Int of pstring 
      | Float of pstring 
      | String of pstring 
      | String2 of expr list 
      | Yield of afield 
      | Yield_break 
      | Await of expr 
      | List of expr list 
      | Expr_list of expr list 
      | Cast of hint* expr 
      | Unop of Ast.uop* expr 
      | Binop of Ast.bop* expr* expr
      [@ocaml.doc
        " The ID of the $$ that is implicitly declared by this pipe. "]
      | Pipe of id* expr* expr 
      | Eif of expr* expr option* expr 
      | NullCoalesce of expr* expr 
      | InstanceOf of expr* class_id 
      | New of class_id* expr list* expr list 
      | Efun of fun_* id list 
      | Xml of sid* (pstring* expr) list* expr list 
      | Lplaceholder of Pos.t 
      | Fun_id of sid 
      | Method_id of expr* pstring 
      | Method_caller of sid* pstring 
      | Smethod_id of sid* pstring 
      | Special_func of special_func 
      | Pair of expr* expr 
      | Assert of assert_expr 
      | Typename of sid 
      | Any 
    and assert_expr =
      | AE_assert of expr 
    and case =
      | Default of block 
      | Case of expr* block 
    and catch = (sid* id* block)
    and field = (expr* expr)
    and afield =
      | AFvalue of expr 
      | AFkvalue of expr* expr 
    and special_func =
      | Gena of expr 
      | Genva of expr list 
      | Gen_array_rec of expr 
    and is_reference = bool
    and is_variadic = bool
    and fun_param =
      {
      param_hint: hint option ;
      param_is_reference: is_reference ;
      param_is_variadic: is_variadic ;
      param_pos: Pos.t ;
      param_name: string ;
      param_expr: expr option }
    and fun_variadicity =
      | FVvariadicArg of fun_param 
      | FVellipsis 
      | FVnonVariadic 
    and fun_ =
      {
      f_mode: FileInfo.mode [@opaque ];
      f_ret: hint option ;
      f_name: sid ;
      f_tparams: tparam list ;
      f_where_constraints: where_constraint list ;
      f_variadic: fun_variadicity ;
      f_params: fun_param list ;
      f_body: func_body ;
      f_fun_kind: Ast.fun_kind ;
      f_user_attributes: user_attribute list }
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
      ua_params: expr list }[@@deriving show]
    let rec pp_stmt : Format.formatter -> stmt -> Ppx_deriving_runtime.unit =
      let __29 () = pp_block
      
      and __28 () = pp_catch
      
      and __27 () = pp_block
      
      and __26 () = pp_block
      
      and __25 () = pp_as_expr
      
      and __24 () = pp_expr
      
      and __23 () = pp_case
      
      and __22 () = pp_expr
      
      and __21 () = pp_block
      
      and __20 () = pp_expr
      
      and __19 () = pp_expr
      
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
      
      and __5 () = Pos.pp
      
      and __4 () = pp_expr
      
      and __3 () = pp_is_terminal
      
      and __2 () = Pos.pp
      
      and __1 () = Pos.pp
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | Fallthrough  ->
                Format.pp_print_string fmt "AnnotatedAST.Fallthrough"
            | Expr a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Expr@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Break a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Break@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Continue a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Continue@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Throw (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Throw (@,";
                 (((__3 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__4 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Return (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Return (@,";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.GotoLabel@ ";
                 ((__7 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Goto a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Goto@ ";
                 ((__8 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Static_var a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Static_var@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Global_var@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.If (@,";
                 ((((__11 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__12 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__13 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Do (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Do (@,";
                 (((__14 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__15 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | While (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.While (@,";
                 (((__16 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__17 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | For (a0,a1,a2,a3) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.For (@,";
                 (((((__18 ()) fmt) a0;
                    Format.fprintf fmt ",@ ";
                    ((__19 ()) fmt) a1);
                   Format.fprintf fmt ",@ ";
                   ((__20 ()) fmt) a2);
                  Format.fprintf fmt ",@ ";
                  ((__21 ()) fmt) a3);
                 Format.fprintf fmt "@,))@]")
            | Switch (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Switch (@,";
                 (((__22 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__23 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) a1);
                 Format.fprintf fmt "@,))@]")
            | Foreach (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Foreach (@,";
                 ((((__24 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__25 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__26 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Try (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Try (@,";
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
            | Noop  -> Format.pp_print_string fmt "AnnotatedAST.Noop")
        [@ocaml.warning "-A"])
    
    and show_stmt : stmt -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_stmt x
    
    and pp_as_expr : Format.formatter -> as_expr -> Ppx_deriving_runtime.unit
      =
      let __7 () = pp_expr
      
      and __6 () = pp_expr
      
      and __5 () = Pos.pp
      
      and __4 () = pp_expr
      
      and __3 () = Pos.pp
      
      and __2 () = pp_expr
      
      and __1 () = pp_expr
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | As_v a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.As_v@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | As_kv (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.As_kv (@,";
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Await_as_v (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Await_as_v (@,";
                 (((__3 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__4 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Await_as_kv (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Await_as_kv (@,";
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
      let __1 () = pp_instantiated_sid
      
      and __0 () = pp_expr
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | CIparent  -> Format.pp_print_string fmt "AnnotatedAST.CIparent"
            | CIself  -> Format.pp_print_string fmt "AnnotatedAST.CIself"
            | CIstatic  -> Format.pp_print_string fmt "AnnotatedAST.CIstatic"
            | CIexpr a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.CIexpr@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | CI a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.CI@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_class_id : class_id -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_id x
    
    and pp_expr : Format.formatter -> expr -> Ppx_deriving_runtime.unit =
      let __1 () = pp_expr_
      
      and __0 () = Annotation.pp
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
      let __74 () = pp_sid
      
      and __73 () = pp_assert_expr
      
      and __72 () = pp_expr
      
      and __71 () = pp_expr
      
      and __70 () = pp_special_func
      
      and __69 () = pp_pstring
      
      and __68 () = pp_sid
      
      and __67 () = pp_pstring
      
      and __66 () = pp_sid
      
      and __65 () = pp_pstring
      
      and __64 () = pp_expr
      
      and __63 () = pp_sid
      
      and __62 () = Pos.pp
      
      and __61 () = pp_expr
      
      and __60 () = pp_expr
      
      and __59 () = pp_pstring
      
      and __58 () = pp_sid
      
      and __57 () = pp_id
      
      and __56 () = pp_fun_
      
      and __55 () = pp_expr
      
      and __54 () = pp_expr
      
      and __53 () = pp_class_id
      
      and __52 () = pp_class_id
      
      and __51 () = pp_expr
      
      and __50 () = pp_expr
      
      and __49 () = pp_expr
      
      and __48 () = pp_expr
      
      and __47 () = pp_expr
      
      and __46 () = pp_expr
      
      and __45 () = pp_expr
      
      and __44 () = pp_expr
      
      and __43 () = pp_id
      
      and __42 () = pp_expr
      
      and __41 () = pp_expr
      
      and __40 () = Ast.pp_bop
      
      and __39 () = pp_expr
      
      and __38 () = Ast.pp_uop
      
      and __37 () = pp_expr
      
      and __36 () = pp_hint
      
      and __35 () = pp_expr
      
      and __34 () = pp_expr
      
      and __33 () = pp_expr
      
      and __32 () = pp_afield
      
      and __31 () = pp_expr
      
      and __30 () = pp_pstring
      
      and __29 () = pp_pstring
      
      and __28 () = pp_pstring
      
      and __27 () = pp_expr
      
      and __26 () = pp_expr
      
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
      
      and __13 () = pp_id
      
      and __12 () = pp_id
      
      and __11 () = pp_id
      
      and __10 () = pp_sid
      
      and __9 () = pp_field
      
      and __8 () = pp_kvc_kind
      
      and __7 () = pp_expr
      
      and __6 () = pp_vc_kind
      
      and __5 () = ShapeMap.pp
      
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Array@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Darray@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Varray@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Shape@ ";
                 ((__5 ()) (fun fmt  -> (__4 ()) fmt) fmt) a0;
                 Format.fprintf fmt "@])")
            | ValCollection (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.ValCollection (@,";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.KeyValCollection (@,";
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
            | Null  -> Format.pp_print_string fmt "AnnotatedAST.Null"
            | This  -> Format.pp_print_string fmt "AnnotatedAST.This"
            | True  -> Format.pp_print_string fmt "AnnotatedAST.True"
            | False  -> Format.pp_print_string fmt "AnnotatedAST.False"
            | Id a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Id@ ";
                 ((__10 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Lvar a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Lvar@ ";
                 ((__11 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Lvarvar (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Lvarvar (@,";
                 ((Format.fprintf fmt "%d") a0;
                  Format.fprintf fmt ",@ ";
                  ((__12 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Dollardollar a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Dollardollar@ ";
                 ((__13 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Clone a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Clone@ ";
                 ((__14 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Obj_get (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Obj_get (@,";
                 ((((__15 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__16 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__17 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Array_get (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Array_get (@,";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Class_get (@,";
                 (((__20 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__21 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Class_const (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Class_const (@,";
                 (((__22 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__23 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Call (a0,a1,a2,a3) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Call (@,";
                 (((((__24 ()) fmt) a0;
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
                 Format.fprintf fmt "@,))@]")
            | Int a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Int@ ";
                 ((__28 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Float a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Float@ ";
                 ((__29 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | String a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.String@ ";
                 ((__30 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | String2 a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.String2@ ";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__31 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | Yield a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Yield@ ";
                 ((__32 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Yield_break  ->
                Format.pp_print_string fmt "AnnotatedAST.Yield_break"
            | Await a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Await@ ";
                 ((__33 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | List a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.List@ ";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__34 ()) fmt) x;
                               true) false x);
                     Format.fprintf fmt "@,]@]")) a0;
                 Format.fprintf fmt "@])")
            | Expr_list a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Expr_list@ ";
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
            | Cast (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Cast (@,";
                 (((__36 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__37 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Unop (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Unop (@,";
                 (((__38 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__39 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Binop (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Binop (@,";
                 ((((__40 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__41 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__42 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Pipe (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Pipe (@,";
                 ((((__43 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((__44 ()) fmt) a1);
                  Format.fprintf fmt ",@ ";
                  ((__45 ()) fmt) a2);
                 Format.fprintf fmt "@,))@]")
            | Eif (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Eif (@,";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.NullCoalesce (@,";
                 (((__49 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__50 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | InstanceOf (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.InstanceOf (@,";
                 (((__51 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__52 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | New (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.New (@,";
                 ((((__53 ()) fmt) a0;
                   Format.fprintf fmt ",@ ";
                   ((fun x  ->
                       Format.fprintf fmt "@[<2>[";
                       ignore
                         (List.fold_left
                            (fun sep  ->
                               fun x  ->
                                 if sep then Format.fprintf fmt ";@ ";
                                 ((__54 ()) fmt) x;
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
                                ((__55 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) a2);
                 Format.fprintf fmt "@,))@]")
            | Efun (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Efun (@,";
                 (((__56 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((fun x  ->
                      Format.fprintf fmt "@[<2>[";
                      ignore
                        (List.fold_left
                           (fun sep  ->
                              fun x  ->
                                if sep then Format.fprintf fmt ";@ ";
                                ((__57 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) a1);
                 Format.fprintf fmt "@,))@]")
            | Xml (a0,a1,a2) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Xml (@,";
                 ((((__58 ()) fmt) a0;
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
                                     (((__59 ()) fmt) a0;
                                      Format.fprintf fmt ",@ ";
                                      ((__60 ()) fmt) a1);
                                     Format.fprintf fmt "@])")) x;
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
            | Lplaceholder a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Lplaceholder@ ";
                 ((__62 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Fun_id a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Fun_id@ ";
                 ((__63 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Method_id (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Method_id (@,";
                 (((__64 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__65 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Method_caller (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Method_caller (@,";
                 (((__66 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__67 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Smethod_id (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Smethod_id (@,";
                 (((__68 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__69 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Special_func a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Special_func@ ";
                 ((__70 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Pair (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Pair (@,";
                 (((__71 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__72 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]")
            | Assert a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Assert@ ";
                 ((__73 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Typename a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Typename@ ";
                 ((__74 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Any  -> Format.pp_print_string fmt "AnnotatedAST.Any")
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.AE_assert@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Default@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Case (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Case (@,";
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]"))
        [@ocaml.warning "-A"])
    
    and show_case : case -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_case x
    
    and pp_catch : Format.formatter -> catch -> Ppx_deriving_runtime.unit =
      let __2 () = pp_block
      
      and __1 () = pp_id
      
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.AFvalue@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | AFkvalue (a0,a1) ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.AFkvalue (@,";
                 (((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt "@,))@]"))
        [@ocaml.warning "-A"])
    
    and show_afield : afield -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_afield x
    
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Gena@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Genva a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Genva@ ";
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Gen_array_rec@ ";
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
      let __4 () = pp_expr
      
      and __3 () = Pos.pp
      
      and __2 () = pp_is_variadic
      
      and __1 () = pp_is_reference
      
      and __0 () = pp_hint
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.param_hint";
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
                 Format.fprintf fmt "@[%s =@ " "param_pos";
                 ((__3 ()) fmt) x.param_pos;
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
                      ((__4 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.param_expr;
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.FVvariadicArg@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | FVellipsis  ->
                Format.pp_print_string fmt "AnnotatedAST.FVellipsis"
            | FVnonVariadic  ->
                Format.pp_print_string fmt "AnnotatedAST.FVnonVariadic")
        [@ocaml.warning "-A"])
    
    and show_fun_variadicity : fun_variadicity -> Ppx_deriving_runtime.string
      = fun x  -> Format.asprintf "%a" pp_fun_variadicity x
    
    and pp_fun_ : Format.formatter -> fun_ -> Ppx_deriving_runtime.unit =
      let __8 () = pp_user_attribute
      
      and __7 () = Ast.pp_fun_kind
      
      and __6 () = pp_func_body
      
      and __5 () = pp_fun_param
      
      and __4 () = pp_fun_variadicity
      
      and __3 () = pp_where_constraint
      
      and __2 () = pp_tparam
      
      and __1 () = pp_sid
      
      and __0 () = pp_hint
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.f_mode";
                        ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                          x.f_mode;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "f_ret";
                       ((function
                         | None  -> Format.pp_print_string fmt "None"
                         | Some x ->
                             (Format.pp_print_string fmt "(Some ";
                              ((__0 ()) fmt) x;
                              Format.pp_print_string fmt ")"))) x.f_ret;
                       Format.fprintf fmt "@]");
                      Format.fprintf fmt ";@ ";
                      Format.fprintf fmt "@[%s =@ " "f_name";
                      ((__1 ()) fmt) x.f_name;
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
                                   ((__2 ()) fmt) x;
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
                                  ((__3 ()) fmt) x;
                                  true) false x);
                        Format.fprintf fmt "@,]@]")) x.f_where_constraints;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "f_variadic";
                   ((__4 ()) fmt) x.f_variadic;
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
                                ((__5 ()) fmt) x;
                                true) false x);
                      Format.fprintf fmt "@,]@]")) x.f_params;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "f_body";
                 ((__6 ()) fmt) x.f_body;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "f_fun_kind";
                ((__7 ()) fmt) x.f_fun_kind;
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
                             ((__8 ()) fmt) x;
                             true) false x);
                   Format.fprintf fmt "@,]@]")) x.f_user_attributes;
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
                (Format.fprintf fmt "(@[<2>AnnotatedAST.UnnamedBody@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | NamedBody a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.NamedBody@ ";
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
    
    type class_ =
      {
      c_mode: FileInfo.mode [@opaque ];
      c_final: bool ;
      c_is_xhp: bool ;
      c_kind: Ast.class_kind ;
      c_name: sid ;
      c_tparams: (tparam list* (Ast.constraint_kind* Ast.hint) list SMap.t)
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
      c_static_vars: class_var list ;
      c_vars: class_var list ;
      c_constructor: method_ option ;
      c_static_methods: method_ list ;
      c_methods: method_ list ;
      c_user_attributes: user_attribute list ;
      c_enum: enum_ option }
    and class_const = (hint option* sid* expr option)
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
      cv_expr: expr option }
    and method_ =
      {
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
      m_ret: hint option }
    and typedef =
      {
      t_name: sid ;
      t_tparams: tparam list ;
      t_constraint: hint option ;
      t_kind: hint ;
      t_user_attributes: user_attribute list ;
      t_mode: FileInfo.mode [@opaque ];
      t_vis: typedef_visibility }
    and gconst =
      {
      cst_mode: FileInfo.mode [@opaque ];
      cst_name: sid ;
      cst_type: hint option ;
      cst_value: expr option }[@@deriving show]
    let rec pp_class_ :
      Format.formatter -> class_ -> Ppx_deriving_runtime.unit =
      let __17 () = pp_enum_
      
      and __16 () = pp_user_attribute
      
      and __15 () = pp_method_
      
      and __14 () = pp_method_
      
      and __13 () = pp_method_
      
      and __12 () = pp_class_var
      
      and __11 () = pp_class_var
      
      and __10 () = pp_class_typeconst
      
      and __9 () = pp_class_const
      
      and __8 () = pp_hint
      
      and __7 () = pp_hint
      
      and __6 () = pp_hint
      
      and __5 () = pp_pstring
      
      and __4 () = pp_hint
      
      and __3 () = pp_hint
      
      and __2 () = pp_hint
      
      and __1 () = pp_sid
      
      and __0 () = Ast.pp_class_kind
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((((((((((((((((Format.fprintf fmt "@[%s =@ "
                                      "AnnotatedAST.c_mode";
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
                                 ((__0 ()) fmt) x.c_kind;
                                 Format.fprintf fmt "@]");
                                Format.fprintf fmt ";@ ";
                                Format.fprintf fmt "@[%s =@ " "c_name";
                                ((__1 ()) fmt) x.c_name;
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
                                            ((__2 ()) fmt) x;
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
                                           ((__3 ()) fmt) x;
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
                                          ((__4 ()) fmt) x;
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
                                         ((__5 ()) fmt) x;
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
                                        ((__6 ()) fmt) x;
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
                                       ((__7 ()) fmt) x;
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
                                      ((__8 ()) fmt) x;
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
                                     ((__9 ()) fmt) x;
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
                                    ((__10 ()) fmt) x;
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
                                   ((__11 ()) fmt) x;
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
                                  ((__12 ()) fmt) x;
                                  true) false x);
                        Format.fprintf fmt "@,]@]")) x.c_vars;
                    Format.fprintf fmt "@]");
                   Format.fprintf fmt ";@ ";
                   Format.fprintf fmt "@[%s =@ " "c_constructor";
                   ((function
                     | None  -> Format.pp_print_string fmt "None"
                     | Some x ->
                         (Format.pp_print_string fmt "(Some ";
                          ((__13 ()) fmt) x;
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
                                ((__14 ()) fmt) x;
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
                               ((__15 ()) fmt) x;
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
                              ((__16 ()) fmt) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) x.c_user_attributes;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "c_enum";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__17 ()) fmt) x;
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
      let __3 () = pp_expr
      
      and __2 () = pp_sid
      
      and __1 () = pp_hint
      
      and __0 () = pp_visibility
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.cv_final";
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
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_class_var : class_var -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_class_var x
    
    and pp_method_ : Format.formatter -> method_ -> Ppx_deriving_runtime.unit
      =
      let __9 () = pp_hint
      
      and __8 () = pp_user_attribute
      
      and __7 () = Ast.pp_fun_kind
      
      and __6 () = pp_func_body
      
      and __5 () = pp_fun_param
      
      and __4 () = pp_fun_variadicity
      
      and __3 () = pp_where_constraint
      
      and __2 () = pp_tparam
      
      and __1 () = pp_sid
      
      and __0 () = pp_visibility
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((((((((((Format.fprintf fmt "@[%s =@ "
                            "AnnotatedAST.m_final";
                          (Format.fprintf fmt "%B") x.m_final;
                          Format.fprintf fmt "@]");
                         Format.fprintf fmt ";@ ";
                         Format.fprintf fmt "@[%s =@ " "m_abstract";
                         (Format.fprintf fmt "%B") x.m_abstract;
                         Format.fprintf fmt "@]");
                        Format.fprintf fmt ";@ ";
                        Format.fprintf fmt "@[%s =@ " "m_visibility";
                        ((__0 ()) fmt) x.m_visibility;
                        Format.fprintf fmt "@]");
                       Format.fprintf fmt ";@ ";
                       Format.fprintf fmt "@[%s =@ " "m_name";
                       ((__1 ()) fmt) x.m_name;
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
                                    ((__2 ()) fmt) x;
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
                                   ((__3 ()) fmt) x;
                                   true) false x);
                         Format.fprintf fmt "@,]@]")) x.m_where_constraints;
                     Format.fprintf fmt "@]");
                    Format.fprintf fmt ";@ ";
                    Format.fprintf fmt "@[%s =@ " "m_variadic";
                    ((__4 ()) fmt) x.m_variadic;
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
                                 ((__5 ()) fmt) x;
                                 true) false x);
                       Format.fprintf fmt "@,]@]")) x.m_params;
                   Format.fprintf fmt "@]");
                  Format.fprintf fmt ";@ ";
                  Format.fprintf fmt "@[%s =@ " "m_body";
                  ((__6 ()) fmt) x.m_body;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "m_fun_kind";
                 ((__7 ()) fmt) x.m_fun_kind;
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
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_method_ : method_ -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_method_ x
    
    and pp_typedef : Format.formatter -> typedef -> Ppx_deriving_runtime.unit
      =
      let __5 () = pp_typedef_visibility
      
      and __4 () = pp_user_attribute
      
      and __3 () = pp_hint
      
      and __2 () = pp_hint
      
      and __1 () = pp_tparam
      
      and __0 () = pp_sid
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              (((((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.t_name";
                     ((__0 ()) fmt) x.t_name;
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
                   ((function
                     | None  -> Format.pp_print_string fmt "None"
                     | Some x ->
                         (Format.pp_print_string fmt "(Some ";
                          ((__2 ()) fmt) x;
                          Format.pp_print_string fmt ")"))) x.t_constraint;
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
                Format.fprintf fmt "@[%s =@ " "t_mode";
                ((fun _  -> Format.pp_print_string fmt "<opaque>")) x.t_mode;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "t_vis";
               ((__5 ()) fmt) x.t_vis;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_typedef : typedef -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_typedef x
    
    and pp_gconst : Format.formatter -> gconst -> Ppx_deriving_runtime.unit =
      let __2 () = pp_expr
      
      and __1 () = pp_hint
      
      and __0 () = pp_sid
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            fun x  ->
              Format.fprintf fmt "@[<2>{ ";
              ((((Format.fprintf fmt "@[%s =@ " "AnnotatedAST.cst_mode";
                  ((fun _  -> Format.pp_print_string fmt "<opaque>"))
                    x.cst_mode;
                  Format.fprintf fmt "@]");
                 Format.fprintf fmt ";@ ";
                 Format.fprintf fmt "@[%s =@ " "cst_name";
                 ((__0 ()) fmt) x.cst_name;
                 Format.fprintf fmt "@]");
                Format.fprintf fmt ";@ ";
                Format.fprintf fmt "@[%s =@ " "cst_type";
                ((function
                  | None  -> Format.pp_print_string fmt "None"
                  | Some x ->
                      (Format.pp_print_string fmt "(Some ";
                       ((__1 ()) fmt) x;
                       Format.pp_print_string fmt ")"))) x.cst_type;
                Format.fprintf fmt "@]");
               Format.fprintf fmt ";@ ";
               Format.fprintf fmt "@[%s =@ " "cst_value";
               ((function
                 | None  -> Format.pp_print_string fmt "None"
                 | Some x ->
                     (Format.pp_print_string fmt "(Some ";
                      ((__2 ()) fmt) x;
                      Format.pp_print_string fmt ")"))) x.cst_value;
               Format.fprintf fmt "@]");
              Format.fprintf fmt "@ }@]")
        [@ocaml.warning "-A"])
    
    and show_gconst : gconst -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_gconst x
    
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
      | Lvarvar _ -> "Lvarvar"
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
      | New _ -> "New"
      | Efun _ -> "Efun"
      | Xml _ -> "Xml"
      | Assert _ -> "Assert"
      | Clone _ -> "Clone"
      | Typename _ -> "Typename" 
    type def =
      | Fun of fun_ 
      | Class of class_ 
      | Typedef of typedef 
      | Constant of gconst [@@deriving show]
    let rec pp_def : Format.formatter -> def -> Ppx_deriving_runtime.unit =
      let __3 () = pp_gconst
      
      and __2 () = pp_typedef
      
      and __1 () = pp_class_
      
      and __0 () = pp_fun_
       in
      ((let open! Ppx_deriving_runtime in
          fun fmt  ->
            function
            | Fun a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Fun@ ";
                 ((__0 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Class a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Class@ ";
                 ((__1 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Typedef a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Typedef@ ";
                 ((__2 ()) fmt) a0;
                 Format.fprintf fmt "@])")
            | Constant a0 ->
                (Format.fprintf fmt "(@[<2>AnnotatedAST.Constant@ ";
                 ((__3 ()) fmt) a0;
                 Format.fprintf fmt "@])"))
        [@ocaml.warning "-A"])
    
    and show_def : def -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp_def x
    
    type program = def list[@@deriving show]
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
            method  on_as_expr : 'a -> as_expr -> 'a
            method  on_array : 'a -> afield list -> 'a
            method  on_shape : 'a -> expr ShapeMap.t -> 'a
            method  on_valCollection : 'a -> vc_kind -> expr list -> 'a
            method  on_keyValCollection : 'a -> kvc_kind -> field list -> 'a
            method  on_this : 'a -> 'a
            method  on_id : 'a -> sid -> 'a
            method  on_lvar : 'a -> id -> 'a
            method  on_lvarvar : 'a -> int -> id -> 'a
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
            method  on_class_id : 'a -> class_id -> 'a
            method  on_new : 'a -> class_id -> expr list -> expr list -> 'a
            method  on_efun : 'a -> fun_ -> id list -> 'a
            method  on_xml :
              'a -> sid -> (pstring* expr) list -> expr list -> 'a
            method  on_assert : 'a -> assert_expr -> 'a
            method  on_clone : 'a -> expr -> 'a
            method  on_field : 'a -> field -> 'a
            method  on_afield : 'a -> afield -> 'a
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
              | Lvarvar (n,id) -> this#on_lvarvar acc n id
              | Fun_id sid -> this#on_fun_id acc sid
              | Method_id (expr,pstr) -> this#on_method_id acc expr pstr
              | Method_caller (sid,pstr) ->
                  this#on_method_caller acc sid pstr
              | Smethod_id (sid,pstr) -> this#on_smethod_id acc sid pstr
              | Yield_break  -> this#on_yield_break acc
              | Yield e -> this#on_yield acc e
              | Await e -> this#on_await acc e
              | List el -> this#on_list acc el
              | Assert ae -> this#on_assert acc ae
              | Clone e -> this#on_clone acc e
              | Expr_list el -> this#on_expr_list acc el
              | Special_func sf -> this#on_special_func acc sf
              | Obj_get (e1,e2,_) -> this#on_obj_get acc e1 e2
              | Array_get (e1,e2) -> this#on_array_get acc e1 e2
              | Class_get (cid,id) -> this#on_class_get acc cid id
              | Class_const (cid,id) -> this#on_class_const acc cid id
              | Call (ct,e,el,uel) -> this#on_call acc ct e el uel
              | String2 el -> this#on_string2 acc el
              | Pair (e1,e2) -> this#on_pair acc e1 e2
              | Cast (hint,e) -> this#on_cast acc hint e
              | Unop (uop,e) -> this#on_unop acc uop e
              | Binop (bop,e1,e2) -> this#on_binop acc bop e1 e2
              | Pipe (id,e1,e2) -> this#on_pipe acc id e1 e2
              | Eif (e1,e2,e3) -> this#on_eif acc e1 e2 e3
              | NullCoalesce (e1,e2) -> this#on_nullCoalesce acc e1 e2
              | InstanceOf (e1,e2) -> this#on_instanceOf acc e1 e2
              | Typename n -> this#on_typename acc n
              | New (cid,el,uel) -> this#on_new acc cid el uel
              | Efun (f,idl) -> this#on_efun acc f idl
              | Xml (sid,attrl,el) -> this#on_xml acc sid attrl el
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
            method on_lvarvar acc _ _ = acc
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
            method on_class_id acc =
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
                List.fold_left (fun acc  -> fun (_,e)  -> this#on_expr acc e)
                  acc attrl
                 in
              let acc = List.fold_left this#on_expr acc el  in acc
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
module PosAnnotation =
  struct
    type t = Pos.t[@@deriving show]
    let rec pp : Format.formatter -> t -> Ppx_deriving_runtime.unit =
      let __0 () = Pos.pp  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show : t -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp x
    
  end
module PosAnnotatedAST = AnnotatedAST(PosAnnotation)
include PosAnnotatedAST
let assert_named_body =
  function
  | NamedBody b -> b
  | UnnamedBody _ -> failwith "Expecting a named function body" 
let get_instantiated_sid_name ((_,x),_) = x 
let class_id_to_str =
  function
  | CIparent  -> SN.Classes.cParent
  | CIself  -> SN.Classes.cSelf
  | CIstatic  -> SN.Classes.cStatic
  | CIexpr (_,This ) -> SN.SpecialIdents.this
  | CIexpr (_,Lvar (_,x)) -> "$" ^ (Local_id.to_string x)
  | CIexpr _ -> assert false
  | CI x -> get_instantiated_sid_name x 
let is_kvc_kind name =
  (name = SN.Collections.cMap) ||
    ((name = SN.Collections.cImmMap) ||
       ((name = SN.Collections.cStableMap) || (name = SN.Collections.cDict)))
  
let get_kvc_kind name =
  match name with
  | x when x = SN.Collections.cMap -> `Map
  | x when x = SN.Collections.cImmMap -> `ImmMap
  | x when x = SN.Collections.cDict -> `Dict
  | _ ->
      (Errors.internal_error Pos.none
         ("Invalid KeyValueCollection name: " ^ name);
       `Map)
  
let kvc_kind_to_name kind =
  match kind with
  | `Map -> SN.Collections.cMap
  | `ImmMap -> SN.Collections.cImmMap
  | `Dict -> SN.Collections.cDict 
let is_vc_kind name =
  (name = SN.Collections.cVector) ||
    ((name = SN.Collections.cImmVector) ||
       ((name = SN.Collections.cSet) ||
          ((name = SN.Collections.cImmSet) ||
             ((name = SN.Collections.cKeyset) || (name = SN.Collections.cVec)))))
  
let get_vc_kind name =
  match name with
  | x when x = SN.Collections.cVector -> `Vector
  | x when x = SN.Collections.cImmVector -> `ImmVector
  | x when x = SN.Collections.cVec -> `Vec
  | x when x = SN.Collections.cSet -> `Set
  | x when x = SN.Collections.cImmSet -> `ImmSet
  | x when x = SN.Collections.cKeyset -> `Keyset
  | _ ->
      (Errors.internal_error Pos.none
         ("Invalid ValueCollection name: " ^ name);
       `Set)
  
let vc_kind_to_name kind =
  match kind with
  | `Vector -> SN.Collections.cVector
  | `ImmVector -> SN.Collections.cImmVector
  | `Vec -> SN.Collections.cVec
  | `Set -> SN.Collections.cSet
  | `ImmSet -> SN.Collections.cImmSet
  | `Keyset -> SN.Collections.cKeyset
  | `Pair -> SN.Collections.cPair 
