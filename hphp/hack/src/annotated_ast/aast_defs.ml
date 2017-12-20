(* @generated from aast_defs.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2017, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<3a0a8bb5f816d200877e5418dd6ca90a04571614>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2017, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
type id = (Pos.t * Local_id.t)[@@deriving show]
let rec pp_id : Format.formatter -> id -> Ppx_deriving_runtime.unit =
  let __1 () = Local_id.pp
  
  and __0 () = Pos.pp
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
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
type is_coroutine = bool[@@deriving show]
let rec (pp_is_coroutine :
          Format.formatter -> is_coroutine -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_coroutine : is_coroutine -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_coroutine x

type is_reactive = bool[@@deriving show]
let rec (pp_is_reactive :
          Format.formatter -> is_reactive -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_reactive : is_reactive -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_reactive x

type hint = (Pos.t * hint_)
and variadic_hint =
  | Hvariadic of hint option 
  | Hnon_variadic 
and hint_ =
  | Hoption of hint 
  | Hfun of is_reactive * is_coroutine * hint list * Ast.param_kind option
  list * variadic_hint * hint 
  | Htuple of hint list 
  | Happly of sid * hint list 
  | Hshape of nast_shape_info 
  | Haccess of hint * sid list 
  | Hany 
  | Hmixed 
  | Habstr of string 
  | Harray of hint option * hint option 
  | Hdarray of hint * hint 
  | Hvarray of hint 
  | Hvarray_or_darray of hint 
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
  let __1 () = pp_hint_
  
  and __0 () = Pos.pp
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
  let __19 () = pp_tprim
  
  and __18 () = pp_hint
  
  and __17 () = pp_hint
  
  and __16 () = pp_hint
  
  and __15 () = pp_hint
  
  and __14 () = pp_hint
  
  and __13 () = pp_hint
  
  and __12 () = pp_sid
  
  and __11 () = pp_hint
  
  and __10 () = pp_nast_shape_info
  
  and __9 () = pp_hint
  
  and __8 () = pp_sid
  
  and __7 () = pp_hint
  
  and __6 () = pp_hint
  
  and __5 () = pp_variadic_hint
  
  and __4 () = Ast.pp_param_kind
  
  and __3 () = pp_hint
  
  and __2 () = pp_is_coroutine
  
  and __1 () = pp_is_reactive
  
  and __0 () = pp_hint
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Hoption a0 ->
            (Format.fprintf fmt "(@[<2>Hoption@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hfun (a0,a1,a2,a3,a4,a5) ->
            (Format.fprintf fmt "(@[<2>Hfun (@,";
             (((((((__1 ()) fmt) a0;
                  Format.fprintf fmt ",@ ";
                  ((__2 ()) fmt) a1);
                 Format.fprintf fmt ",@ ";
                 ((fun x  ->
                     Format.fprintf fmt "@[<2>[";
                     ignore
                       (List.fold_left
                          (fun sep  ->
                             fun x  ->
                               if sep then Format.fprintf fmt ";@ ";
                               ((__3 ()) fmt) x;
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
                              ((function
                                | None  -> Format.pp_print_string fmt "None"
                                | Some x ->
                                    (Format.pp_print_string fmt "(Some ";
                                     ((__4 ()) fmt) x;
                                     Format.pp_print_string fmt ")"))) x;
                              true) false x);
                    Format.fprintf fmt "@,]@]")) a3);
               Format.fprintf fmt ",@ ";
               ((__5 ()) fmt) a4);
              Format.fprintf fmt ",@ ";
              ((__6 ()) fmt) a5);
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
                           ((__7 ()) fmt) x;
                           true) false x);
                 Format.fprintf fmt "@,]@]")) a0;
             Format.fprintf fmt "@])")
        | Happly (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Happly (@,";
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
        | Hshape a0 ->
            (Format.fprintf fmt "(@[<2>Hshape@ ";
             ((__10 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Haccess (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Haccess (@,";
             (((__11 ()) fmt) a0;
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
                     ((__13 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a0;
              Format.fprintf fmt ",@ ";
              ((function
                | None  -> Format.pp_print_string fmt "None"
                | Some x ->
                    (Format.pp_print_string fmt "(Some ";
                     ((__14 ()) fmt) x;
                     Format.pp_print_string fmt ")"))) a1);
             Format.fprintf fmt "@,))@]")
        | Hdarray (a0,a1) ->
            (Format.fprintf fmt "(@[<2>Hdarray (@,";
             (((__15 ()) fmt) a0;
              Format.fprintf fmt ",@ ";
              ((__16 ()) fmt) a1);
             Format.fprintf fmt "@,))@]")
        | Hvarray a0 ->
            (Format.fprintf fmt "(@[<2>Hvarray@ ";
             ((__17 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hvarray_or_darray a0 ->
            (Format.fprintf fmt "(@[<2>Hvarray_or_darray@ ";
             ((__18 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | Hprim a0 ->
            (Format.fprintf fmt "(@[<2>Hprim@ ";
             ((__19 ()) fmt) a0;
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
type tparam = (Ast.variance * sid * (Ast.constraint_kind * hint) list)
[@@deriving show]
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

type instantiated_sid = (sid * hint list)[@@deriving show]
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

type where_constraint = (hint * Ast.constraint_kind * hint)[@@deriving show]
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

