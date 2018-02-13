(* @generated from aast_defs.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2004-present, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<1b972644f5c9a617b8717e040600b04f0e610f7b>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2017, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
include Aast_defs_visitors_ancestors
module ShapeMap = Ast.ShapeMap
type 'a shape_map = 'a ShapeMap.t[@@deriving show]
let rec pp_shape_map :
  'a .
    (Format.formatter -> 'a -> Ppx_deriving_runtime.unit) ->
      Format.formatter -> 'a shape_map -> Ppx_deriving_runtime.unit
  =
  let __0 () = ShapeMap.pp  in
  ((let open! Ppx_deriving_runtime in
      fun poly_a  -> fun fmt  -> (__0 ()) (fun fmt  -> poly_a fmt) fmt)
    [@ocaml.warning "-A"])

and show_shape_map :
  'a .
    (Format.formatter -> 'a -> Ppx_deriving_runtime.unit) ->
      'a shape_map -> Ppx_deriving_runtime.string
  = fun poly_a  -> fun x  -> Format.asprintf "%a" (pp_shape_map poly_a) x

type pos = Ast.pos[@@deriving show]
let rec pp_pos : Format.formatter -> pos -> Ppx_deriving_runtime.unit =
  let __0 () = Ast.pp_pos  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_pos : pos -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_pos x

type local_id = ((Local_id.t)[@visitors.opaque ])
and lid = (pos * local_id)
and sid = Ast.id
and is_terminal = bool
and call_type =
  | Cnormal [@visitors.name "call_type_Cnormal"]
  | Cuser_func [@visitors.name "call_type_Cuser_func"]
and is_coroutine = bool
and is_reactive = bool
and hint = (pos * hint_)
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
  nsi_field_map: shape_field_info shape_map }
and kvc_kind = (([ `Map  | `ImmMap  | `Dict ])[@visitors.opaque ])
and vc_kind =
  (([ `Vector  | `ImmVector  | `Vec  | `Set  | `ImmSet  | `Pair  | `Keyset ])
  [@visitors.opaque ])
and tparam = (Ast.variance * sid * (Ast.constraint_kind * hint) list)
and visibility =
  | Private [@visitors.name "visibility_Private"]
  | Public [@visitors.name "visibility_Public"]
  | Protected [@visitors.name "visibility_Protected"]
and typedef_visibility =
  | Transparent
  | Opaque
and enum_ = {
  e_base: hint ;
  e_constraint: hint option }
and instantiated_sid = (sid * hint list)
and where_constraint = (hint * Ast.constraint_kind * hint)[@@deriving
                                                            (show,
                                                              (visitors
                                                                 {
                                                                   name =
                                                                    "iter_defs";
                                                                   variety =
                                                                    "iter";
                                                                   nude =
                                                                    true;
                                                                   visit_prefix
                                                                    = "on_";
                                                                   ancestors
                                                                    =
                                                                    ["iter_defs_base"]
                                                                 }),
                                                              (visitors
                                                                 {
                                                                   name =
                                                                    "reduce_defs";
                                                                   variety =
                                                                    "reduce";
                                                                   nude =
                                                                    true;
                                                                   visit_prefix
                                                                    = "on_";
                                                                   ancestors
                                                                    =
                                                                    ["reduce_defs_base"]
                                                                 }),
                                                              (visitors
                                                                 {
                                                                   name =
                                                                    "map_defs";
                                                                   variety =
                                                                    "map";
                                                                   nude =
                                                                    true;
                                                                   visit_prefix
                                                                    = "on_";
                                                                   ancestors
                                                                    =
                                                                    ["map_defs_base"]
                                                                 }),
                                                              (visitors
                                                                 {
                                                                   name =
                                                                    "endo_defs";
                                                                   variety =
                                                                    "endo";
                                                                   nude =
                                                                    true;
                                                                   visit_prefix
                                                                    = "on_";
                                                                   ancestors
                                                                    =
                                                                    ["endo_defs_base"]
                                                                 }))]
let rec pp_local_id :
  Format.formatter -> local_id -> Ppx_deriving_runtime.unit =
  let __0 () = Local_id.pp  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_local_id : local_id -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_local_id x

and pp_lid : Format.formatter -> lid -> Ppx_deriving_runtime.unit =
  let __1 () = pp_local_id

  and __0 () = pp_pos
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__1 ()) fmt) a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_lid : lid -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_lid x

and pp_sid : Format.formatter -> sid -> Ppx_deriving_runtime.unit =
  let __0 () = Ast.pp_id  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_sid : sid -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_sid x

and (pp_is_terminal :
      Format.formatter -> is_terminal -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_terminal : is_terminal -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_terminal x

and (pp_call_type :
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

and (pp_is_coroutine :
      Format.formatter -> is_coroutine -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_coroutine : is_coroutine -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_coroutine x

and (pp_is_reactive :
      Format.formatter -> is_reactive -> Ppx_deriving_runtime.unit)
  = ((let open! Ppx_deriving_runtime in fun fmt  -> Format.fprintf fmt "%B")
  [@ocaml.warning "-A"])

and show_is_reactive : is_reactive -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_is_reactive x

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
  let __1 () = pp_shape_map

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

and (pp_kvc_kind : Format.formatter -> kvc_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | `Map -> Format.pp_print_string fmt "`Map"
        | `ImmMap -> Format.pp_print_string fmt "`ImmMap"
        | `Dict -> Format.pp_print_string fmt "`Dict")
  [@ocaml.warning "-A"])

and show_kvc_kind : kvc_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_kvc_kind x

and (pp_vc_kind : Format.formatter -> vc_kind -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | `Vector -> Format.pp_print_string fmt "`Vector"
        | `ImmVector -> Format.pp_print_string fmt "`ImmVector"
        | `Vec -> Format.pp_print_string fmt "`Vec"
        | `Set -> Format.pp_print_string fmt "`Set"
        | `ImmSet -> Format.pp_print_string fmt "`ImmSet"
        | `Pair -> Format.pp_print_string fmt "`Pair"
        | `Keyset -> Format.pp_print_string fmt "`Keyset")
  [@ocaml.warning "-A"])

and show_vc_kind : vc_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_vc_kind x

and pp_tparam : Format.formatter -> tparam -> Ppx_deriving_runtime.unit =
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

and (pp_visibility :
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

and (pp_typedef_visibility :
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

and pp_instantiated_sid :
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

and pp_where_constraint :
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

include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] iter_defs =
      object (self : 'self)
        inherit  [_] iter_defs_base
        method on_local_id env _visitors_this = ()
        method on_lid env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_local_id env _visitors_c1  in ()
        method on_sid env = self#on_id env
        method on_is_terminal env = self#on_bool env
        method on_call_type_Cnormal env = ()
        method on_call_type_Cuser_func env = ()
        method on_call_type env _visitors_this =
          match _visitors_this with
          | Cnormal  -> self#on_call_type_Cnormal env
          | Cuser_func  -> self#on_call_type_Cuser_func env
        method on_is_coroutine env = self#on_bool env
        method on_is_reactive env = self#on_bool env
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
          _visitors_c3 _visitors_c4 _visitors_c5 =
          let _visitors_r0 = self#on_is_reactive env _visitors_c0  in
          let _visitors_r1 = self#on_is_coroutine env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_hint env _visitors_c2  in
          let _visitors_r3 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c3
             in
          let _visitors_r4 = self#on_variadic_hint env _visitors_c4  in
          let _visitors_r5 = self#on_hint env _visitors_c5  in ()
        method on_Htuple env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          ()
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          ()
        method on_Hshape env _visitors_c0 =
          let _visitors_r0 = self#on_nast_shape_info env _visitors_c0  in ()
        method on_Haccess env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_sid env _visitors_c1  in ()
        method on_Hany env = ()
        method on_Hmixed env = ()
        method on_Habstr env _visitors_c0 =
          let _visitors_r0 = self#on_string env _visitors_c0  in ()
        method on_Harray env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_hint env _visitors_c1  in
          ()
        method on_Hdarray env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in ()
        method on_Hvarray env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_Hvarray_or_darray env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in ()
        method on_Hprim env _visitors_c0 =
          let _visitors_r0 = self#on_tprim env _visitors_c0  in ()
        method on_Hthis env = ()
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4,_visitors_c5)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4 _visitors_c5
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1) ->
              self#on_Haccess env _visitors_c0 _visitors_c1
          | Hany  -> self#on_Hany env
          | Hmixed  -> self#on_Hmixed env
          | Habstr _visitors_c0 -> self#on_Habstr env _visitors_c0
          | Harray (_visitors_c0,_visitors_c1) ->
              self#on_Harray env _visitors_c0 _visitors_c1
          | Hdarray (_visitors_c0,_visitors_c1) ->
              self#on_Hdarray env _visitors_c0 _visitors_c1
          | Hvarray _visitors_c0 -> self#on_Hvarray env _visitors_c0
          | Hvarray_or_darray _visitors_c0 ->
              self#on_Hvarray_or_darray env _visitors_c0
          | Hprim _visitors_c0 -> self#on_Hprim env _visitors_c0
          | Hthis  -> self#on_Hthis env
        method on_Tvoid env = ()
        method on_Tint env = ()
        method on_Tbool env = ()
        method on_Tfloat env = ()
        method on_Tstring env = ()
        method on_Tresource env = ()
        method on_Tnum env = ()
        method on_Tarraykey env = ()
        method on_Tnoreturn env = ()
        method on_tprim env _visitors_this =
          match _visitors_this with
          | Tvoid  -> self#on_Tvoid env
          | Tint  -> self#on_Tint env
          | Tbool  -> self#on_Tbool env
          | Tfloat  -> self#on_Tfloat env
          | Tstring  -> self#on_Tstring env
          | Tresource  -> self#on_Tresource env
          | Tnum  -> self#on_Tnum env
          | Tarraykey  -> self#on_Tarraykey env
          | Tnoreturn  -> self#on_Tnoreturn env
        method on_shape_field_info env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sfi_optional  in
          let _visitors_r1 = self#on_hint env _visitors_this.sfi_hint  in ()
        method on_nast_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.nsi_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_shape_map self#on_shape_field_info env
              _visitors_this.nsi_field_map
             in
          ()
        method on_kvc_kind env _visitors_this = ()
        method on_vc_kind env _visitors_this = ()
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_sid env _visitors_c1  in
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
        method on_visibility_Private env = ()
        method on_visibility_Public env = ()
        method on_visibility_Protected env = ()
        method on_visibility env _visitors_this =
          match _visitors_this with
          | Private  -> self#on_visibility_Private env
          | Public  -> self#on_visibility_Public env
          | Protected  -> self#on_visibility_Protected env
        method on_Transparent env = ()
        method on_Opaque env = ()
        method on_typedef_visibility env _visitors_this =
          match _visitors_this with
          | Transparent  -> self#on_Transparent env
          | Opaque  -> self#on_Opaque env
        method on_enum_ env _visitors_this =
          let _visitors_r0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          ()
        method on_instantiated_sid env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          ()
        method on_where_constraint env
          (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_constraint_kind env _visitors_c1  in
          let _visitors_r2 = self#on_hint env _visitors_c2  in ()
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] reduce_defs =
      object (self : 'self)
        inherit  [_] reduce_defs_base
        method on_local_id env _visitors_this = self#zero
        method on_lid env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_local_id env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_sid env = self#on_id env
        method on_is_terminal env = self#on_bool env
        method on_call_type_Cnormal env = self#zero
        method on_call_type_Cuser_func env = self#zero
        method on_call_type env _visitors_this =
          match _visitors_this with
          | Cnormal  -> self#on_call_type_Cnormal env
          | Cuser_func  -> self#on_call_type_Cuser_func env
        method on_is_coroutine env = self#on_bool env
        method on_is_reactive env = self#on_bool env
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
          _visitors_c3 _visitors_c4 _visitors_c5 =
          let _visitors_s0 = self#on_is_reactive env _visitors_c0  in
          let _visitors_s1 = self#on_is_coroutine env _visitors_c1  in
          let _visitors_s2 = self#on_list self#on_hint env _visitors_c2  in
          let _visitors_s3 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c3
             in
          let _visitors_s4 = self#on_variadic_hint env _visitors_c4  in
          let _visitors_s5 = self#on_hint env _visitors_c5  in
          self#plus
            (self#plus
               (self#plus
                  (self#plus (self#plus _visitors_s0 _visitors_s1)
                     _visitors_s2) _visitors_s3) _visitors_s4) _visitors_s5
        method on_Htuple env _visitors_c0 =
          let _visitors_s0 = self#on_list self#on_hint env _visitors_c0  in
          _visitors_s0
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_sid env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hshape env _visitors_c0 =
          let _visitors_s0 = self#on_nast_shape_info env _visitors_c0  in
          _visitors_s0
        method on_Haccess env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_sid env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hany env = self#zero
        method on_Hmixed env = self#zero
        method on_Habstr env _visitors_c0 =
          let _visitors_s0 = self#on_string env _visitors_c0  in _visitors_s0
        method on_Harray env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_option self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hdarray env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_Hvarray env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_Hvarray_or_darray env _visitors_c0 =
          let _visitors_s0 = self#on_hint env _visitors_c0  in _visitors_s0
        method on_Hprim env _visitors_c0 =
          let _visitors_s0 = self#on_tprim env _visitors_c0  in _visitors_s0
        method on_Hthis env = self#zero
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4,_visitors_c5)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4 _visitors_c5
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1) ->
              self#on_Haccess env _visitors_c0 _visitors_c1
          | Hany  -> self#on_Hany env
          | Hmixed  -> self#on_Hmixed env
          | Habstr _visitors_c0 -> self#on_Habstr env _visitors_c0
          | Harray (_visitors_c0,_visitors_c1) ->
              self#on_Harray env _visitors_c0 _visitors_c1
          | Hdarray (_visitors_c0,_visitors_c1) ->
              self#on_Hdarray env _visitors_c0 _visitors_c1
          | Hvarray _visitors_c0 -> self#on_Hvarray env _visitors_c0
          | Hvarray_or_darray _visitors_c0 ->
              self#on_Hvarray_or_darray env _visitors_c0
          | Hprim _visitors_c0 -> self#on_Hprim env _visitors_c0
          | Hthis  -> self#on_Hthis env
        method on_Tvoid env = self#zero
        method on_Tint env = self#zero
        method on_Tbool env = self#zero
        method on_Tfloat env = self#zero
        method on_Tstring env = self#zero
        method on_Tresource env = self#zero
        method on_Tnum env = self#zero
        method on_Tarraykey env = self#zero
        method on_Tnoreturn env = self#zero
        method on_tprim env _visitors_this =
          match _visitors_this with
          | Tvoid  -> self#on_Tvoid env
          | Tint  -> self#on_Tint env
          | Tbool  -> self#on_Tbool env
          | Tfloat  -> self#on_Tfloat env
          | Tstring  -> self#on_Tstring env
          | Tresource  -> self#on_Tresource env
          | Tnum  -> self#on_Tnum env
          | Tarraykey  -> self#on_Tarraykey env
          | Tnoreturn  -> self#on_Tnoreturn env
        method on_shape_field_info env _visitors_this =
          let _visitors_s0 = self#on_bool env _visitors_this.sfi_optional  in
          let _visitors_s1 = self#on_hint env _visitors_this.sfi_hint  in
          self#plus _visitors_s0 _visitors_s1
        method on_nast_shape_info env _visitors_this =
          let _visitors_s0 =
            self#on_bool env _visitors_this.nsi_allows_unknown_fields  in
          let _visitors_s1 =
            self#on_shape_map self#on_shape_field_info env
              _visitors_this.nsi_field_map
             in
          self#plus _visitors_s0 _visitors_s1
        method on_kvc_kind env _visitors_this = self#zero
        method on_vc_kind env _visitors_this = self#zero
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_s0 = self#on_variance env _visitors_c0  in
          let _visitors_s1 = self#on_sid env _visitors_c1  in
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
        method on_visibility_Private env = self#zero
        method on_visibility_Public env = self#zero
        method on_visibility_Protected env = self#zero
        method on_visibility env _visitors_this =
          match _visitors_this with
          | Private  -> self#on_visibility_Private env
          | Public  -> self#on_visibility_Public env
          | Protected  -> self#on_visibility_Protected env
        method on_Transparent env = self#zero
        method on_Opaque env = self#zero
        method on_typedef_visibility env _visitors_this =
          match _visitors_this with
          | Transparent  -> self#on_Transparent env
          | Opaque  -> self#on_Opaque env
        method on_enum_ env _visitors_this =
          let _visitors_s0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_s1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          self#plus _visitors_s0 _visitors_s1
        method on_instantiated_sid env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_sid env _visitors_c0  in
          let _visitors_s1 = self#on_list self#on_hint env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_where_constraint env
          (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_s0 = self#on_hint env _visitors_c0  in
          let _visitors_s1 = self#on_constraint_kind env _visitors_c1  in
          let _visitors_s2 = self#on_hint env _visitors_c2  in
          self#plus (self#plus _visitors_s0 _visitors_s1) _visitors_s2
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] map_defs =
      object (self : 'self)
        inherit  [_] map_defs_base
        method on_local_id env _visitors_this = _visitors_this
        method on_lid env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_local_id env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_sid env = self#on_id env
        method on_is_terminal env = self#on_bool env
        method on_call_type_Cnormal env = Cnormal
        method on_call_type_Cuser_func env = Cuser_func
        method on_call_type env _visitors_this =
          match _visitors_this with
          | Cnormal  -> self#on_call_type_Cnormal env
          | Cuser_func  -> self#on_call_type_Cuser_func env
        method on_is_coroutine env = self#on_bool env
        method on_is_reactive env = self#on_bool env
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
          _visitors_c3 _visitors_c4 _visitors_c5 =
          let _visitors_r0 = self#on_is_reactive env _visitors_c0  in
          let _visitors_r1 = self#on_is_coroutine env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_hint env _visitors_c2  in
          let _visitors_r3 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c3
             in
          let _visitors_r4 = self#on_variadic_hint env _visitors_c4  in
          let _visitors_r5 = self#on_hint env _visitors_c5  in
          Hfun
            (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
              _visitors_r4, _visitors_r5)
        method on_Htuple env _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          Htuple _visitors_r0
        method on_Happly env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          Happly (_visitors_r0, _visitors_r1)
        method on_Hshape env _visitors_c0 =
          let _visitors_r0 = self#on_nast_shape_info env _visitors_c0  in
          Hshape _visitors_r0
        method on_Haccess env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_sid env _visitors_c1  in
          Haccess (_visitors_r0, _visitors_r1)
        method on_Hany env = Hany
        method on_Hmixed env = Hmixed
        method on_Habstr env _visitors_c0 =
          let _visitors_r0 = self#on_string env _visitors_c0  in
          Habstr _visitors_r0
        method on_Harray env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_hint env _visitors_c1  in
          Harray (_visitors_r0, _visitors_r1)
        method on_Hdarray env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          Hdarray (_visitors_r0, _visitors_r1)
        method on_Hvarray env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          Hvarray _visitors_r0
        method on_Hvarray_or_darray env _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          Hvarray_or_darray _visitors_r0
        method on_Hprim env _visitors_c0 =
          let _visitors_r0 = self#on_tprim env _visitors_c0  in
          Hprim _visitors_r0
        method on_Hthis env = Hthis
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 -> self#on_Hoption env _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4,_visitors_c5)
              ->
              self#on_Hfun env _visitors_c0 _visitors_c1 _visitors_c2
                _visitors_c3 _visitors_c4 _visitors_c5
          | Htuple _visitors_c0 -> self#on_Htuple env _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) ->
              self#on_Happly env _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 -> self#on_Hshape env _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1) ->
              self#on_Haccess env _visitors_c0 _visitors_c1
          | Hany  -> self#on_Hany env
          | Hmixed  -> self#on_Hmixed env
          | Habstr _visitors_c0 -> self#on_Habstr env _visitors_c0
          | Harray (_visitors_c0,_visitors_c1) ->
              self#on_Harray env _visitors_c0 _visitors_c1
          | Hdarray (_visitors_c0,_visitors_c1) ->
              self#on_Hdarray env _visitors_c0 _visitors_c1
          | Hvarray _visitors_c0 -> self#on_Hvarray env _visitors_c0
          | Hvarray_or_darray _visitors_c0 ->
              self#on_Hvarray_or_darray env _visitors_c0
          | Hprim _visitors_c0 -> self#on_Hprim env _visitors_c0
          | Hthis  -> self#on_Hthis env
        method on_Tvoid env = Tvoid
        method on_Tint env = Tint
        method on_Tbool env = Tbool
        method on_Tfloat env = Tfloat
        method on_Tstring env = Tstring
        method on_Tresource env = Tresource
        method on_Tnum env = Tnum
        method on_Tarraykey env = Tarraykey
        method on_Tnoreturn env = Tnoreturn
        method on_tprim env _visitors_this =
          match _visitors_this with
          | Tvoid  -> self#on_Tvoid env
          | Tint  -> self#on_Tint env
          | Tbool  -> self#on_Tbool env
          | Tfloat  -> self#on_Tfloat env
          | Tstring  -> self#on_Tstring env
          | Tresource  -> self#on_Tresource env
          | Tnum  -> self#on_Tnum env
          | Tarraykey  -> self#on_Tarraykey env
          | Tnoreturn  -> self#on_Tnoreturn env
        method on_shape_field_info env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sfi_optional  in
          let _visitors_r1 = self#on_hint env _visitors_this.sfi_hint  in
          { sfi_optional = _visitors_r0; sfi_hint = _visitors_r1 }
        method on_nast_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.nsi_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_shape_map self#on_shape_field_info env
              _visitors_this.nsi_field_map
             in
          {
            nsi_allows_unknown_fields = _visitors_r0;
            nsi_field_map = _visitors_r1
          }
        method on_kvc_kind env _visitors_this = _visitors_this
        method on_vc_kind env _visitors_this = _visitors_this
        method on_tparam env (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_sid env _visitors_c1  in
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
        method on_visibility_Private env = Private
        method on_visibility_Public env = Public
        method on_visibility_Protected env = Protected
        method on_visibility env _visitors_this =
          match _visitors_this with
          | Private  -> self#on_visibility_Private env
          | Public  -> self#on_visibility_Public env
          | Protected  -> self#on_visibility_Protected env
        method on_Transparent env = Transparent
        method on_Opaque env = Opaque
        method on_typedef_visibility env _visitors_this =
          match _visitors_this with
          | Transparent  -> self#on_Transparent env
          | Opaque  -> self#on_Opaque env
        method on_enum_ env _visitors_this =
          let _visitors_r0 = self#on_hint env _visitors_this.e_base  in
          let _visitors_r1 =
            self#on_option self#on_hint env _visitors_this.e_constraint  in
          { e_base = _visitors_r0; e_constraint = _visitors_r1 }
        method on_instantiated_sid env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_where_constraint env
          (_visitors_c0,_visitors_c1,_visitors_c2) =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_constraint_kind env _visitors_c1  in
          let _visitors_r2 = self#on_hint env _visitors_c2  in
          (_visitors_r0, _visitors_r1, _visitors_r2)
      end
    [@@@VISITORS.END ]
  end
include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] endo_defs =
      object (self : 'self)
        inherit  [_] endo_defs_base
        method on_local_id env _visitors_this = _visitors_this
        method on_lid env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_local_id env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_sid env = self#on_id env
        method on_is_terminal env = self#on_bool env
        method on_call_type_Cnormal env _visitors_this =
          if true then _visitors_this else Cnormal
        method on_call_type_Cuser_func env _visitors_this =
          if true then _visitors_this else Cuser_func
        method on_call_type env _visitors_this =
          match _visitors_this with
          | Cnormal  as _visitors_this ->
              self#on_call_type_Cnormal env _visitors_this
          | Cuser_func  as _visitors_this ->
              self#on_call_type_Cuser_func env _visitors_this
        method on_is_coroutine env = self#on_bool env
        method on_is_reactive env = self#on_bool env
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
          _visitors_c2 _visitors_c3 _visitors_c4 _visitors_c5 =
          let _visitors_r0 = self#on_is_reactive env _visitors_c0  in
          let _visitors_r1 = self#on_is_coroutine env _visitors_c1  in
          let _visitors_r2 = self#on_list self#on_hint env _visitors_c2  in
          let _visitors_r3 =
            self#on_list (self#on_option self#on_param_kind) env _visitors_c3
             in
          let _visitors_r4 = self#on_variadic_hint env _visitors_c4  in
          let _visitors_r5 = self#on_hint env _visitors_c5  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(&&) (Pervasives.(==) _visitors_c2 _visitors_r2)
                    (Pervasives.(&&)
                       (Pervasives.(==) _visitors_c3 _visitors_r3)
                       (Pervasives.(&&)
                          (Pervasives.(==) _visitors_c4 _visitors_r4)
                          (Pervasives.(==) _visitors_c5 _visitors_r5)))))
          then _visitors_this
          else
            Hfun
              (_visitors_r0, _visitors_r1, _visitors_r2, _visitors_r3,
                _visitors_r4, _visitors_r5)
        method on_Htuple env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_list self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Htuple _visitors_r0
        method on_Happly env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Happly (_visitors_r0, _visitors_r1)
        method on_Hshape env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_nast_shape_info env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hshape _visitors_r0
        method on_Haccess env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_sid env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Haccess (_visitors_r0, _visitors_r1)
        method on_Hany env _visitors_this =
          if true then _visitors_this else Hany
        method on_Hmixed env _visitors_this =
          if true then _visitors_this else Hmixed
        method on_Habstr env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_string env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Habstr _visitors_r0
        method on_Harray env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_option self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_option self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Harray (_visitors_r0, _visitors_r1)
        method on_Hdarray env _visitors_this _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else Hdarray (_visitors_r0, _visitors_r1)
        method on_Hvarray env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hvarray _visitors_r0
        method on_Hvarray_or_darray env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hvarray_or_darray _visitors_r0
        method on_Hprim env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_tprim env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Hprim _visitors_r0
        method on_Hthis env _visitors_this =
          if true then _visitors_this else Hthis
        method on_hint_ env _visitors_this =
          match _visitors_this with
          | Hoption _visitors_c0 as _visitors_this ->
              self#on_Hoption env _visitors_this _visitors_c0
          | Hfun
              (_visitors_c0,_visitors_c1,_visitors_c2,_visitors_c3,_visitors_c4,_visitors_c5)
              as _visitors_this ->
              self#on_Hfun env _visitors_this _visitors_c0 _visitors_c1
                _visitors_c2 _visitors_c3 _visitors_c4 _visitors_c5
          | Htuple _visitors_c0 as _visitors_this ->
              self#on_Htuple env _visitors_this _visitors_c0
          | Happly (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Happly env _visitors_this _visitors_c0 _visitors_c1
          | Hshape _visitors_c0 as _visitors_this ->
              self#on_Hshape env _visitors_this _visitors_c0
          | Haccess (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Haccess env _visitors_this _visitors_c0 _visitors_c1
          | Hany  as _visitors_this -> self#on_Hany env _visitors_this
          | Hmixed  as _visitors_this -> self#on_Hmixed env _visitors_this
          | Habstr _visitors_c0 as _visitors_this ->
              self#on_Habstr env _visitors_this _visitors_c0
          | Harray (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Harray env _visitors_this _visitors_c0 _visitors_c1
          | Hdarray (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_Hdarray env _visitors_this _visitors_c0 _visitors_c1
          | Hvarray _visitors_c0 as _visitors_this ->
              self#on_Hvarray env _visitors_this _visitors_c0
          | Hvarray_or_darray _visitors_c0 as _visitors_this ->
              self#on_Hvarray_or_darray env _visitors_this _visitors_c0
          | Hprim _visitors_c0 as _visitors_this ->
              self#on_Hprim env _visitors_this _visitors_c0
          | Hthis  as _visitors_this -> self#on_Hthis env _visitors_this
        method on_Tvoid env _visitors_this =
          if true then _visitors_this else Tvoid
        method on_Tint env _visitors_this =
          if true then _visitors_this else Tint
        method on_Tbool env _visitors_this =
          if true then _visitors_this else Tbool
        method on_Tfloat env _visitors_this =
          if true then _visitors_this else Tfloat
        method on_Tstring env _visitors_this =
          if true then _visitors_this else Tstring
        method on_Tresource env _visitors_this =
          if true then _visitors_this else Tresource
        method on_Tnum env _visitors_this =
          if true then _visitors_this else Tnum
        method on_Tarraykey env _visitors_this =
          if true then _visitors_this else Tarraykey
        method on_Tnoreturn env _visitors_this =
          if true then _visitors_this else Tnoreturn
        method on_tprim env _visitors_this =
          match _visitors_this with
          | Tvoid  as _visitors_this -> self#on_Tvoid env _visitors_this
          | Tint  as _visitors_this -> self#on_Tint env _visitors_this
          | Tbool  as _visitors_this -> self#on_Tbool env _visitors_this
          | Tfloat  as _visitors_this -> self#on_Tfloat env _visitors_this
          | Tstring  as _visitors_this -> self#on_Tstring env _visitors_this
          | Tresource  as _visitors_this ->
              self#on_Tresource env _visitors_this
          | Tnum  as _visitors_this -> self#on_Tnum env _visitors_this
          | Tarraykey  as _visitors_this ->
              self#on_Tarraykey env _visitors_this
          | Tnoreturn  as _visitors_this ->
              self#on_Tnoreturn env _visitors_this
        method on_shape_field_info env _visitors_this =
          let _visitors_r0 = self#on_bool env _visitors_this.sfi_optional  in
          let _visitors_r1 = self#on_hint env _visitors_this.sfi_hint  in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.sfi_optional _visitors_r0)
              (Pervasives.(==) _visitors_this.sfi_hint _visitors_r1)
          then _visitors_this
          else { sfi_optional = _visitors_r0; sfi_hint = _visitors_r1 }
        method on_nast_shape_info env _visitors_this =
          let _visitors_r0 =
            self#on_bool env _visitors_this.nsi_allows_unknown_fields  in
          let _visitors_r1 =
            self#on_shape_map self#on_shape_field_info env
              _visitors_this.nsi_field_map
             in
          if
            Pervasives.(&&)
              (Pervasives.(==) _visitors_this.nsi_allows_unknown_fields
                 _visitors_r0)
              (Pervasives.(==) _visitors_this.nsi_field_map _visitors_r1)
          then _visitors_this
          else
            {
              nsi_allows_unknown_fields = _visitors_r0;
              nsi_field_map = _visitors_r1
            }
        method on_kvc_kind env _visitors_this = _visitors_this
        method on_vc_kind env _visitors_this = _visitors_this
        method on_tparam env
          ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
          let _visitors_r0 = self#on_variance env _visitors_c0  in
          let _visitors_r1 = self#on_sid env _visitors_c1  in
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
        method on_visibility_Private env _visitors_this =
          if true then _visitors_this else Private
        method on_visibility_Public env _visitors_this =
          if true then _visitors_this else Public
        method on_visibility_Protected env _visitors_this =
          if true then _visitors_this else Protected
        method on_visibility env _visitors_this =
          match _visitors_this with
          | Private  as _visitors_this ->
              self#on_visibility_Private env _visitors_this
          | Public  as _visitors_this ->
              self#on_visibility_Public env _visitors_this
          | Protected  as _visitors_this ->
              self#on_visibility_Protected env _visitors_this
        method on_Transparent env _visitors_this =
          if true then _visitors_this else Transparent
        method on_Opaque env _visitors_this =
          if true then _visitors_this else Opaque
        method on_typedef_visibility env _visitors_this =
          match _visitors_this with
          | Transparent  as _visitors_this ->
              self#on_Transparent env _visitors_this
          | Opaque  as _visitors_this -> self#on_Opaque env _visitors_this
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
        method on_instantiated_sid env
          ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_sid env _visitors_c0  in
          let _visitors_r1 = self#on_list self#on_hint env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_where_constraint env
          ((_visitors_c0,_visitors_c1,_visitors_c2) as _visitors_this) =
          let _visitors_r0 = self#on_hint env _visitors_c0  in
          let _visitors_r1 = self#on_constraint_kind env _visitors_c1  in
          let _visitors_r2 = self#on_hint env _visitors_c2  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(&&) (Pervasives.(==) _visitors_c1 _visitors_r1)
                 (Pervasives.(==) _visitors_c2 _visitors_r2))
          then _visitors_this
          else (_visitors_r0, _visitors_r1, _visitors_r2)
      end
    [@@@VISITORS.END ]
  end
type id = lid[@@deriving show]
let rec pp_id : Format.formatter -> id -> Ppx_deriving_runtime.unit =
  let __0 () = pp_lid  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_id : id -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_id x

type pstring = Ast.pstring[@@deriving show]
let rec pp_pstring : Format.formatter -> pstring -> Ppx_deriving_runtime.unit
  =
  let __0 () = Ast.pp_pstring  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_pstring : pstring -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_pstring x

type shape_field_name = Ast.shape_field_name[@@deriving show]
let rec pp_shape_field_name :
  Format.formatter -> shape_field_name -> Ppx_deriving_runtime.unit =
  let __0 () = Ast.pp_shape_field_name  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_shape_field_name : shape_field_name -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_shape_field_name x

type og_null_flavor = Ast.og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
let pp_og_null_flavor fmt flavor =
  (Format.pp_print_string fmt) @@
    (match flavor with
     | OG_nullthrows  -> "OG_nullthrows"
     | OG_nullsafe  -> "OG_nullsafe")

let pp_kvc_kind fmt _ = Format.pp_print_string fmt "<kvc_kind>"
let pp_vc_kind fmt _ = Format.pp_print_string fmt "<vc_kind>"
