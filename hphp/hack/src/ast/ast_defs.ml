(* @generated from ast_defs.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2018, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<d1c8b7bb10a148220cf9d9d912dba33a41672dfe>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2017, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
include Ast_defs_visitors_ancestors
type cst_kind =
  | Cst_define 
  | Cst_const 
and pos = ((Pos.t)[@visitors.opaque ])
and id = (pos * string)
and pstring = (pos * string)
and shape_field_name =
  | SFlit of pstring 
  | SFclass_const of id * pstring 
and variance =
  | Covariant 
  | Contravariant 
  | Invariant 
and ns_kind =
  | NSNamespace 
  | NSClass 
  | NSClassAndNamespace 
  | NSFun 
  | NSConst 
and constraint_kind =
  | Constraint_as 
  | Constraint_eq 
  | Constraint_super 
and class_kind =
  | Cabstract 
  | Cnormal 
  | Cinterface 
  | Ctrait 
  | Cenum 
and trait_req_kind =
  | MustExtend 
  | MustImplement 
and kind =
  | Final 
  | Static 
  | Abstract 
  | Private 
  | Public 
  | Protected 
and param_kind =
  | Pinout 
and og_null_flavor =
  | OG_nullthrows 
  | OG_nullsafe 
and fun_kind =
  | FSync 
  | FAsync 
  | FGenerator 
  | FAsyncGenerator 
  | FCoroutine 
and bop =
  | Plus 
  | Minus 
  | Star 
  | Slash 
  | Eqeq 
  | EQeqeq 
  | Starstar 
  | Diff 
  | Diff2 
  | AMpamp 
  | BArbar 
  | LogXor 
  | Lt 
  | Lte 
  | Gt 
  | Gte 
  | Dot 
  | Amp 
  | Bar 
  | Ltlt 
  | Gtgt 
  | Percent 
  | Xor 
  | Cmp 
  | Eq of bop option 
and uop =
  | Utild 
  | Unot 
  | Uplus 
  | Uminus 
  | Uincr 
  | Udecr 
  | Upincr 
  | Updecr 
  | Uref 
  | Usilence [@@deriving
               (show,
                 (visitors
                    {
                      name = "iter_defs";
                      variety = "iter";
                      nude = true;
                      visit_prefix = "on_";
                      ancestors = ["iter_defs_base"]
                    }),
                 (visitors
                    {
                      name = "endo_defs";
                      variety = "endo";
                      nude = true;
                      visit_prefix = "on_";
                      ancestors = ["endo_defs_base"]
                    }),
                 (visitors
                    {
                      name = "reduce_defs";
                      variety = "reduce";
                      nude = true;
                      visit_prefix = "on_";
                      ancestors = ["reduce_defs_base"]
                    }),
                 (visitors
                    {
                      name = "map_defs";
                      variety = "map";
                      nude = true;
                      visit_prefix = "on_";
                      ancestors = ["map_defs_base"]
                    }))]
let rec (pp_cst_kind :
          Format.formatter -> cst_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Cst_define  -> Format.pp_print_string fmt "Cst_define"
        | Cst_const  -> Format.pp_print_string fmt "Cst_const")
  [@ocaml.warning "-A"])

and show_cst_kind : cst_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_cst_kind x

and pp_pos : Format.formatter -> pos -> Ppx_deriving_runtime.unit =
  let __0 () = Pos.pp  in
  ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
    [@ocaml.warning "-A"])

and show_pos : pos -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_pos x

and pp_id : Format.formatter -> id -> Ppx_deriving_runtime.unit =
  let __0 () = pp_pos  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0;
           Format.fprintf fmt ",@ ";
           (Format.fprintf fmt "%S") a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_id : id -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_id x

and pp_pstring : Format.formatter -> pstring -> Ppx_deriving_runtime.unit =
  let __0 () = pp_pos  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        fun (a0,a1)  ->
          Format.fprintf fmt "(@[";
          (((__0 ()) fmt) a0;
           Format.fprintf fmt ",@ ";
           (Format.fprintf fmt "%S") a1);
          Format.fprintf fmt "@])")
    [@ocaml.warning "-A"])

and show_pstring : pstring -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_pstring x

and pp_shape_field_name :
  Format.formatter -> shape_field_name -> Ppx_deriving_runtime.unit =
  let __2 () = pp_pstring
  
  and __1 () = pp_id
  
  and __0 () = pp_pstring
   in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | SFlit a0 ->
            (Format.fprintf fmt "(@[<2>SFlit@ ";
             ((__0 ()) fmt) a0;
             Format.fprintf fmt "@])")
        | SFclass_const (a0,a1) ->
            (Format.fprintf fmt "(@[<2>SFclass_const (@,";
             (((__1 ()) fmt) a0; Format.fprintf fmt ",@ "; ((__2 ()) fmt) a1);
             Format.fprintf fmt "@,))@]"))
    [@ocaml.warning "-A"])

and show_shape_field_name : shape_field_name -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_shape_field_name x

and (pp_variance : Format.formatter -> variance -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Covariant  -> Format.pp_print_string fmt "Covariant"
        | Contravariant  -> Format.pp_print_string fmt "Contravariant"
        | Invariant  -> Format.pp_print_string fmt "Invariant")
  [@ocaml.warning "-A"])

and show_variance : variance -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_variance x

and (pp_ns_kind : Format.formatter -> ns_kind -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | NSNamespace  -> Format.pp_print_string fmt "NSNamespace"
        | NSClass  -> Format.pp_print_string fmt "NSClass"
        | NSClassAndNamespace  ->
            Format.pp_print_string fmt "NSClassAndNamespace"
        | NSFun  -> Format.pp_print_string fmt "NSFun"
        | NSConst  -> Format.pp_print_string fmt "NSConst")
  [@ocaml.warning "-A"])

and show_ns_kind : ns_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_ns_kind x

and (pp_constraint_kind :
      Format.formatter -> constraint_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Constraint_as  -> Format.pp_print_string fmt "Constraint_as"
        | Constraint_eq  -> Format.pp_print_string fmt "Constraint_eq"
        | Constraint_super  -> Format.pp_print_string fmt "Constraint_super")
  [@ocaml.warning "-A"])

and show_constraint_kind : constraint_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_constraint_kind x

and (pp_class_kind :
      Format.formatter -> class_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Cabstract  -> Format.pp_print_string fmt "Cabstract"
        | Cnormal  -> Format.pp_print_string fmt "Cnormal"
        | Cinterface  -> Format.pp_print_string fmt "Cinterface"
        | Ctrait  -> Format.pp_print_string fmt "Ctrait"
        | Cenum  -> Format.pp_print_string fmt "Cenum")
  [@ocaml.warning "-A"])

and show_class_kind : class_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_class_kind x

and (pp_trait_req_kind :
      Format.formatter -> trait_req_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | MustExtend  -> Format.pp_print_string fmt "MustExtend"
        | MustImplement  -> Format.pp_print_string fmt "MustImplement")
  [@ocaml.warning "-A"])

and show_trait_req_kind : trait_req_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_trait_req_kind x

and (pp_kind : Format.formatter -> kind -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Final  -> Format.pp_print_string fmt "Final"
        | Static  -> Format.pp_print_string fmt "Static"
        | Abstract  -> Format.pp_print_string fmt "Abstract"
        | Private  -> Format.pp_print_string fmt "Private"
        | Public  -> Format.pp_print_string fmt "Public"
        | Protected  -> Format.pp_print_string fmt "Protected")
  [@ocaml.warning "-A"])

and show_kind : kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_kind x

and (pp_param_kind :
      Format.formatter -> param_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  -> function | Pinout  -> Format.pp_print_string fmt "Pinout")
  [@ocaml.warning "-A"])

and show_param_kind : param_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_param_kind x

and (pp_og_null_flavor :
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

and (pp_fun_kind : Format.formatter -> fun_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | FSync  -> Format.pp_print_string fmt "FSync"
        | FAsync  -> Format.pp_print_string fmt "FAsync"
        | FGenerator  -> Format.pp_print_string fmt "FGenerator"
        | FAsyncGenerator  -> Format.pp_print_string fmt "FAsyncGenerator"
        | FCoroutine  -> Format.pp_print_string fmt "FCoroutine")
  [@ocaml.warning "-A"])

and show_fun_kind : fun_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_kind x

and pp_bop : Format.formatter -> bop -> Ppx_deriving_runtime.unit =
  let __0 () = pp_bop  in
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Plus  -> Format.pp_print_string fmt "Plus"
        | Minus  -> Format.pp_print_string fmt "Minus"
        | Star  -> Format.pp_print_string fmt "Star"
        | Slash  -> Format.pp_print_string fmt "Slash"
        | Eqeq  -> Format.pp_print_string fmt "Eqeq"
        | EQeqeq  -> Format.pp_print_string fmt "EQeqeq"
        | Starstar  -> Format.pp_print_string fmt "Starstar"
        | Diff  -> Format.pp_print_string fmt "Diff"
        | Diff2  -> Format.pp_print_string fmt "Diff2"
        | AMpamp  -> Format.pp_print_string fmt "AMpamp"
        | BArbar  -> Format.pp_print_string fmt "BArbar"
        | LogXor  -> Format.pp_print_string fmt "LogXor"
        | Lt  -> Format.pp_print_string fmt "Lt"
        | Lte  -> Format.pp_print_string fmt "Lte"
        | Gt  -> Format.pp_print_string fmt "Gt"
        | Gte  -> Format.pp_print_string fmt "Gte"
        | Dot  -> Format.pp_print_string fmt "Dot"
        | Amp  -> Format.pp_print_string fmt "Amp"
        | Bar  -> Format.pp_print_string fmt "Bar"
        | Ltlt  -> Format.pp_print_string fmt "Ltlt"
        | Gtgt  -> Format.pp_print_string fmt "Gtgt"
        | Percent  -> Format.pp_print_string fmt "Percent"
        | Xor  -> Format.pp_print_string fmt "Xor"
        | Cmp  -> Format.pp_print_string fmt "Cmp"
        | Eq a0 ->
            (Format.fprintf fmt "(@[<2>Eq@ ";
             ((function
               | None  -> Format.pp_print_string fmt "None"
               | Some x ->
                   (Format.pp_print_string fmt "(Some ";
                    ((__0 ()) fmt) x;
                    Format.pp_print_string fmt ")"))) a0;
             Format.fprintf fmt "@])"))
    [@ocaml.warning "-A"])

and show_bop : bop -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_bop x

and (pp_uop : Format.formatter -> uop -> Ppx_deriving_runtime.unit) =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | Utild  -> Format.pp_print_string fmt "Utild"
        | Unot  -> Format.pp_print_string fmt "Unot"
        | Uplus  -> Format.pp_print_string fmt "Uplus"
        | Uminus  -> Format.pp_print_string fmt "Uminus"
        | Uincr  -> Format.pp_print_string fmt "Uincr"
        | Udecr  -> Format.pp_print_string fmt "Udecr"
        | Upincr  -> Format.pp_print_string fmt "Upincr"
        | Updecr  -> Format.pp_print_string fmt "Updecr"
        | Uref  -> Format.pp_print_string fmt "Uref"
        | Usilence  -> Format.pp_print_string fmt "Usilence")
  [@ocaml.warning "-A"])

and show_uop : uop -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_uop x

include
  struct
    [@@@ocaml.warning "-4-26-27"]
    [@@@VISITORS.BEGIN ]
    class virtual ['self] iter_defs =
      object (self : 'self)
        inherit  [_] iter_defs_base
        method on_Cst_define env = ()
        method on_Cst_const env = ()
        method on_cst_kind env _visitors_this =
          match _visitors_this with
          | Cst_define  -> self#on_Cst_define env
          | Cst_const  -> self#on_Cst_const env
        method on_pos env _visitors_this = ()
        method on_id env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in ()
        method on_pstring env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in ()
        method on_SFlit env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in ()
        method on_SFclass_const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in ()
        method on_shape_field_name env _visitors_this =
          match _visitors_this with
          | SFlit _visitors_c0 -> self#on_SFlit env _visitors_c0
          | SFclass_const (_visitors_c0,_visitors_c1) ->
              self#on_SFclass_const env _visitors_c0 _visitors_c1
        method on_Covariant env = ()
        method on_Contravariant env = ()
        method on_Invariant env = ()
        method on_variance env _visitors_this =
          match _visitors_this with
          | Covariant  -> self#on_Covariant env
          | Contravariant  -> self#on_Contravariant env
          | Invariant  -> self#on_Invariant env
        method on_NSNamespace env = ()
        method on_NSClass env = ()
        method on_NSClassAndNamespace env = ()
        method on_NSFun env = ()
        method on_NSConst env = ()
        method on_ns_kind env _visitors_this =
          match _visitors_this with
          | NSNamespace  -> self#on_NSNamespace env
          | NSClass  -> self#on_NSClass env
          | NSClassAndNamespace  -> self#on_NSClassAndNamespace env
          | NSFun  -> self#on_NSFun env
          | NSConst  -> self#on_NSConst env
        method on_Constraint_as env = ()
        method on_Constraint_eq env = ()
        method on_Constraint_super env = ()
        method on_constraint_kind env _visitors_this =
          match _visitors_this with
          | Constraint_as  -> self#on_Constraint_as env
          | Constraint_eq  -> self#on_Constraint_eq env
          | Constraint_super  -> self#on_Constraint_super env
        method on_Cabstract env = ()
        method on_Cnormal env = ()
        method on_Cinterface env = ()
        method on_Ctrait env = ()
        method on_Cenum env = ()
        method on_class_kind env _visitors_this =
          match _visitors_this with
          | Cabstract  -> self#on_Cabstract env
          | Cnormal  -> self#on_Cnormal env
          | Cinterface  -> self#on_Cinterface env
          | Ctrait  -> self#on_Ctrait env
          | Cenum  -> self#on_Cenum env
        method on_MustExtend env = ()
        method on_MustImplement env = ()
        method on_trait_req_kind env _visitors_this =
          match _visitors_this with
          | MustExtend  -> self#on_MustExtend env
          | MustImplement  -> self#on_MustImplement env
        method on_Final env = ()
        method on_Static env = ()
        method on_Abstract env = ()
        method on_Private env = ()
        method on_Public env = ()
        method on_Protected env = ()
        method on_kind env _visitors_this =
          match _visitors_this with
          | Final  -> self#on_Final env
          | Static  -> self#on_Static env
          | Abstract  -> self#on_Abstract env
          | Private  -> self#on_Private env
          | Public  -> self#on_Public env
          | Protected  -> self#on_Protected env
        method on_Pinout env = ()
        method on_param_kind env _visitors_this =
          match _visitors_this with | Pinout  -> self#on_Pinout env
        method on_OG_nullthrows env = ()
        method on_OG_nullsafe env = ()
        method on_og_null_flavor env _visitors_this =
          match _visitors_this with
          | OG_nullthrows  -> self#on_OG_nullthrows env
          | OG_nullsafe  -> self#on_OG_nullsafe env
        method on_FSync env = ()
        method on_FAsync env = ()
        method on_FGenerator env = ()
        method on_FAsyncGenerator env = ()
        method on_FCoroutine env = ()
        method on_fun_kind env _visitors_this =
          match _visitors_this with
          | FSync  -> self#on_FSync env
          | FAsync  -> self#on_FAsync env
          | FGenerator  -> self#on_FGenerator env
          | FAsyncGenerator  -> self#on_FAsyncGenerator env
          | FCoroutine  -> self#on_FCoroutine env
        method on_Plus env = ()
        method on_Minus env = ()
        method on_Star env = ()
        method on_Slash env = ()
        method on_Eqeq env = ()
        method on_EQeqeq env = ()
        method on_Starstar env = ()
        method on_Diff env = ()
        method on_Diff2 env = ()
        method on_AMpamp env = ()
        method on_BArbar env = ()
        method on_LogXor env = ()
        method on_Lt env = ()
        method on_Lte env = ()
        method on_Gt env = ()
        method on_Gte env = ()
        method on_Dot env = ()
        method on_Amp env = ()
        method on_Bar env = ()
        method on_Ltlt env = ()
        method on_Gtgt env = ()
        method on_Percent env = ()
        method on_Xor env = ()
        method on_Cmp env = ()
        method on_Eq env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_bop env _visitors_c0  in
          ()
        method on_bop env _visitors_this =
          match _visitors_this with
          | Plus  -> self#on_Plus env
          | Minus  -> self#on_Minus env
          | Star  -> self#on_Star env
          | Slash  -> self#on_Slash env
          | Eqeq  -> self#on_Eqeq env
          | EQeqeq  -> self#on_EQeqeq env
          | Starstar  -> self#on_Starstar env
          | Diff  -> self#on_Diff env
          | Diff2  -> self#on_Diff2 env
          | AMpamp  -> self#on_AMpamp env
          | BArbar  -> self#on_BArbar env
          | LogXor  -> self#on_LogXor env
          | Lt  -> self#on_Lt env
          | Lte  -> self#on_Lte env
          | Gt  -> self#on_Gt env
          | Gte  -> self#on_Gte env
          | Dot  -> self#on_Dot env
          | Amp  -> self#on_Amp env
          | Bar  -> self#on_Bar env
          | Ltlt  -> self#on_Ltlt env
          | Gtgt  -> self#on_Gtgt env
          | Percent  -> self#on_Percent env
          | Xor  -> self#on_Xor env
          | Cmp  -> self#on_Cmp env
          | Eq _visitors_c0 -> self#on_Eq env _visitors_c0
        method on_Utild env = ()
        method on_Unot env = ()
        method on_Uplus env = ()
        method on_Uminus env = ()
        method on_Uincr env = ()
        method on_Udecr env = ()
        method on_Upincr env = ()
        method on_Updecr env = ()
        method on_Uref env = ()
        method on_Usilence env = ()
        method on_uop env _visitors_this =
          match _visitors_this with
          | Utild  -> self#on_Utild env
          | Unot  -> self#on_Unot env
          | Uplus  -> self#on_Uplus env
          | Uminus  -> self#on_Uminus env
          | Uincr  -> self#on_Uincr env
          | Udecr  -> self#on_Udecr env
          | Upincr  -> self#on_Upincr env
          | Updecr  -> self#on_Updecr env
          | Uref  -> self#on_Uref env
          | Usilence  -> self#on_Usilence env
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
        method on_Cst_define env _visitors_this =
          if true then _visitors_this else Cst_define
        method on_Cst_const env _visitors_this =
          if true then _visitors_this else Cst_const
        method on_cst_kind env _visitors_this =
          match _visitors_this with
          | Cst_define  as _visitors_this ->
              self#on_Cst_define env _visitors_this
          | Cst_const  as _visitors_this ->
              self#on_Cst_const env _visitors_this
        method on_pos env _visitors_this = _visitors_this
        method on_id env ((_visitors_c0,_visitors_c1) as _visitors_this) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_pstring env ((_visitors_c0,_visitors_c1) as _visitors_this)
          =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else (_visitors_r0, _visitors_r1)
        method on_SFlit env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else SFlit _visitors_r0
        method on_SFclass_const env _visitors_this _visitors_c0 _visitors_c1
          =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          if
            Pervasives.(&&) (Pervasives.(==) _visitors_c0 _visitors_r0)
              (Pervasives.(==) _visitors_c1 _visitors_r1)
          then _visitors_this
          else SFclass_const (_visitors_r0, _visitors_r1)
        method on_shape_field_name env _visitors_this =
          match _visitors_this with
          | SFlit _visitors_c0 as _visitors_this ->
              self#on_SFlit env _visitors_this _visitors_c0
          | SFclass_const (_visitors_c0,_visitors_c1) as _visitors_this ->
              self#on_SFclass_const env _visitors_this _visitors_c0
                _visitors_c1
        method on_Covariant env _visitors_this =
          if true then _visitors_this else Covariant
        method on_Contravariant env _visitors_this =
          if true then _visitors_this else Contravariant
        method on_Invariant env _visitors_this =
          if true then _visitors_this else Invariant
        method on_variance env _visitors_this =
          match _visitors_this with
          | Covariant  as _visitors_this ->
              self#on_Covariant env _visitors_this
          | Contravariant  as _visitors_this ->
              self#on_Contravariant env _visitors_this
          | Invariant  as _visitors_this ->
              self#on_Invariant env _visitors_this
        method on_NSNamespace env _visitors_this =
          if true then _visitors_this else NSNamespace
        method on_NSClass env _visitors_this =
          if true then _visitors_this else NSClass
        method on_NSClassAndNamespace env _visitors_this =
          if true then _visitors_this else NSClassAndNamespace
        method on_NSFun env _visitors_this =
          if true then _visitors_this else NSFun
        method on_NSConst env _visitors_this =
          if true then _visitors_this else NSConst
        method on_ns_kind env _visitors_this =
          match _visitors_this with
          | NSNamespace  as _visitors_this ->
              self#on_NSNamespace env _visitors_this
          | NSClass  as _visitors_this -> self#on_NSClass env _visitors_this
          | NSClassAndNamespace  as _visitors_this ->
              self#on_NSClassAndNamespace env _visitors_this
          | NSFun  as _visitors_this -> self#on_NSFun env _visitors_this
          | NSConst  as _visitors_this -> self#on_NSConst env _visitors_this
        method on_Constraint_as env _visitors_this =
          if true then _visitors_this else Constraint_as
        method on_Constraint_eq env _visitors_this =
          if true then _visitors_this else Constraint_eq
        method on_Constraint_super env _visitors_this =
          if true then _visitors_this else Constraint_super
        method on_constraint_kind env _visitors_this =
          match _visitors_this with
          | Constraint_as  as _visitors_this ->
              self#on_Constraint_as env _visitors_this
          | Constraint_eq  as _visitors_this ->
              self#on_Constraint_eq env _visitors_this
          | Constraint_super  as _visitors_this ->
              self#on_Constraint_super env _visitors_this
        method on_Cabstract env _visitors_this =
          if true then _visitors_this else Cabstract
        method on_Cnormal env _visitors_this =
          if true then _visitors_this else Cnormal
        method on_Cinterface env _visitors_this =
          if true then _visitors_this else Cinterface
        method on_Ctrait env _visitors_this =
          if true then _visitors_this else Ctrait
        method on_Cenum env _visitors_this =
          if true then _visitors_this else Cenum
        method on_class_kind env _visitors_this =
          match _visitors_this with
          | Cabstract  as _visitors_this ->
              self#on_Cabstract env _visitors_this
          | Cnormal  as _visitors_this -> self#on_Cnormal env _visitors_this
          | Cinterface  as _visitors_this ->
              self#on_Cinterface env _visitors_this
          | Ctrait  as _visitors_this -> self#on_Ctrait env _visitors_this
          | Cenum  as _visitors_this -> self#on_Cenum env _visitors_this
        method on_MustExtend env _visitors_this =
          if true then _visitors_this else MustExtend
        method on_MustImplement env _visitors_this =
          if true then _visitors_this else MustImplement
        method on_trait_req_kind env _visitors_this =
          match _visitors_this with
          | MustExtend  as _visitors_this ->
              self#on_MustExtend env _visitors_this
          | MustImplement  as _visitors_this ->
              self#on_MustImplement env _visitors_this
        method on_Final env _visitors_this =
          if true then _visitors_this else Final
        method on_Static env _visitors_this =
          if true then _visitors_this else Static
        method on_Abstract env _visitors_this =
          if true then _visitors_this else Abstract
        method on_Private env _visitors_this =
          if true then _visitors_this else Private
        method on_Public env _visitors_this =
          if true then _visitors_this else Public
        method on_Protected env _visitors_this =
          if true then _visitors_this else Protected
        method on_kind env _visitors_this =
          match _visitors_this with
          | Final  as _visitors_this -> self#on_Final env _visitors_this
          | Static  as _visitors_this -> self#on_Static env _visitors_this
          | Abstract  as _visitors_this ->
              self#on_Abstract env _visitors_this
          | Private  as _visitors_this -> self#on_Private env _visitors_this
          | Public  as _visitors_this -> self#on_Public env _visitors_this
          | Protected  as _visitors_this ->
              self#on_Protected env _visitors_this
        method on_Pinout env _visitors_this =
          if true then _visitors_this else Pinout
        method on_param_kind env _visitors_this =
          match _visitors_this with
          | Pinout  as _visitors_this -> self#on_Pinout env _visitors_this
        method on_OG_nullthrows env _visitors_this =
          if true then _visitors_this else OG_nullthrows
        method on_OG_nullsafe env _visitors_this =
          if true then _visitors_this else OG_nullsafe
        method on_og_null_flavor env _visitors_this =
          match _visitors_this with
          | OG_nullthrows  as _visitors_this ->
              self#on_OG_nullthrows env _visitors_this
          | OG_nullsafe  as _visitors_this ->
              self#on_OG_nullsafe env _visitors_this
        method on_FSync env _visitors_this =
          if true then _visitors_this else FSync
        method on_FAsync env _visitors_this =
          if true then _visitors_this else FAsync
        method on_FGenerator env _visitors_this =
          if true then _visitors_this else FGenerator
        method on_FAsyncGenerator env _visitors_this =
          if true then _visitors_this else FAsyncGenerator
        method on_FCoroutine env _visitors_this =
          if true then _visitors_this else FCoroutine
        method on_fun_kind env _visitors_this =
          match _visitors_this with
          | FSync  as _visitors_this -> self#on_FSync env _visitors_this
          | FAsync  as _visitors_this -> self#on_FAsync env _visitors_this
          | FGenerator  as _visitors_this ->
              self#on_FGenerator env _visitors_this
          | FAsyncGenerator  as _visitors_this ->
              self#on_FAsyncGenerator env _visitors_this
          | FCoroutine  as _visitors_this ->
              self#on_FCoroutine env _visitors_this
        method on_Plus env _visitors_this =
          if true then _visitors_this else Plus
        method on_Minus env _visitors_this =
          if true then _visitors_this else Minus
        method on_Star env _visitors_this =
          if true then _visitors_this else Star
        method on_Slash env _visitors_this =
          if true then _visitors_this else Slash
        method on_Eqeq env _visitors_this =
          if true then _visitors_this else Eqeq
        method on_EQeqeq env _visitors_this =
          if true then _visitors_this else EQeqeq
        method on_Starstar env _visitors_this =
          if true then _visitors_this else Starstar
        method on_Diff env _visitors_this =
          if true then _visitors_this else Diff
        method on_Diff2 env _visitors_this =
          if true then _visitors_this else Diff2
        method on_AMpamp env _visitors_this =
          if true then _visitors_this else AMpamp
        method on_BArbar env _visitors_this =
          if true then _visitors_this else BArbar
        method on_LogXor env _visitors_this =
          if true then _visitors_this else LogXor
        method on_Lt env _visitors_this = if true then _visitors_this else Lt
        method on_Lte env _visitors_this =
          if true then _visitors_this else Lte
        method on_Gt env _visitors_this = if true then _visitors_this else Gt
        method on_Gte env _visitors_this =
          if true then _visitors_this else Gte
        method on_Dot env _visitors_this =
          if true then _visitors_this else Dot
        method on_Amp env _visitors_this =
          if true then _visitors_this else Amp
        method on_Bar env _visitors_this =
          if true then _visitors_this else Bar
        method on_Ltlt env _visitors_this =
          if true then _visitors_this else Ltlt
        method on_Gtgt env _visitors_this =
          if true then _visitors_this else Gtgt
        method on_Percent env _visitors_this =
          if true then _visitors_this else Percent
        method on_Xor env _visitors_this =
          if true then _visitors_this else Xor
        method on_Cmp env _visitors_this =
          if true then _visitors_this else Cmp
        method on_Eq env _visitors_this _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_bop env _visitors_c0  in
          if Pervasives.(==) _visitors_c0 _visitors_r0
          then _visitors_this
          else Eq _visitors_r0
        method on_bop env _visitors_this =
          match _visitors_this with
          | Plus  as _visitors_this -> self#on_Plus env _visitors_this
          | Minus  as _visitors_this -> self#on_Minus env _visitors_this
          | Star  as _visitors_this -> self#on_Star env _visitors_this
          | Slash  as _visitors_this -> self#on_Slash env _visitors_this
          | Eqeq  as _visitors_this -> self#on_Eqeq env _visitors_this
          | EQeqeq  as _visitors_this -> self#on_EQeqeq env _visitors_this
          | Starstar  as _visitors_this ->
              self#on_Starstar env _visitors_this
          | Diff  as _visitors_this -> self#on_Diff env _visitors_this
          | Diff2  as _visitors_this -> self#on_Diff2 env _visitors_this
          | AMpamp  as _visitors_this -> self#on_AMpamp env _visitors_this
          | BArbar  as _visitors_this -> self#on_BArbar env _visitors_this
          | LogXor  as _visitors_this -> self#on_LogXor env _visitors_this
          | Lt  as _visitors_this -> self#on_Lt env _visitors_this
          | Lte  as _visitors_this -> self#on_Lte env _visitors_this
          | Gt  as _visitors_this -> self#on_Gt env _visitors_this
          | Gte  as _visitors_this -> self#on_Gte env _visitors_this
          | Dot  as _visitors_this -> self#on_Dot env _visitors_this
          | Amp  as _visitors_this -> self#on_Amp env _visitors_this
          | Bar  as _visitors_this -> self#on_Bar env _visitors_this
          | Ltlt  as _visitors_this -> self#on_Ltlt env _visitors_this
          | Gtgt  as _visitors_this -> self#on_Gtgt env _visitors_this
          | Percent  as _visitors_this -> self#on_Percent env _visitors_this
          | Xor  as _visitors_this -> self#on_Xor env _visitors_this
          | Cmp  as _visitors_this -> self#on_Cmp env _visitors_this
          | Eq _visitors_c0 as _visitors_this ->
              self#on_Eq env _visitors_this _visitors_c0
        method on_Utild env _visitors_this =
          if true then _visitors_this else Utild
        method on_Unot env _visitors_this =
          if true then _visitors_this else Unot
        method on_Uplus env _visitors_this =
          if true then _visitors_this else Uplus
        method on_Uminus env _visitors_this =
          if true then _visitors_this else Uminus
        method on_Uincr env _visitors_this =
          if true then _visitors_this else Uincr
        method on_Udecr env _visitors_this =
          if true then _visitors_this else Udecr
        method on_Upincr env _visitors_this =
          if true then _visitors_this else Upincr
        method on_Updecr env _visitors_this =
          if true then _visitors_this else Updecr
        method on_Uref env _visitors_this =
          if true then _visitors_this else Uref
        method on_Usilence env _visitors_this =
          if true then _visitors_this else Usilence
        method on_uop env _visitors_this =
          match _visitors_this with
          | Utild  as _visitors_this -> self#on_Utild env _visitors_this
          | Unot  as _visitors_this -> self#on_Unot env _visitors_this
          | Uplus  as _visitors_this -> self#on_Uplus env _visitors_this
          | Uminus  as _visitors_this -> self#on_Uminus env _visitors_this
          | Uincr  as _visitors_this -> self#on_Uincr env _visitors_this
          | Udecr  as _visitors_this -> self#on_Udecr env _visitors_this
          | Upincr  as _visitors_this -> self#on_Upincr env _visitors_this
          | Updecr  as _visitors_this -> self#on_Updecr env _visitors_this
          | Uref  as _visitors_this -> self#on_Uref env _visitors_this
          | Usilence  as _visitors_this ->
              self#on_Usilence env _visitors_this
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
        method on_Cst_define env = self#zero
        method on_Cst_const env = self#zero
        method on_cst_kind env _visitors_this =
          match _visitors_this with
          | Cst_define  -> self#on_Cst_define env
          | Cst_const  -> self#on_Cst_const env
        method on_pos env _visitors_this = self#zero
        method on_id env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_string env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_pstring env (_visitors_c0,_visitors_c1) =
          let _visitors_s0 = self#on_pos env _visitors_c0  in
          let _visitors_s1 = self#on_string env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_SFlit env _visitors_c0 =
          let _visitors_s0 = self#on_pstring env _visitors_c0  in
          _visitors_s0
        method on_SFclass_const env _visitors_c0 _visitors_c1 =
          let _visitors_s0 = self#on_id env _visitors_c0  in
          let _visitors_s1 = self#on_pstring env _visitors_c1  in
          self#plus _visitors_s0 _visitors_s1
        method on_shape_field_name env _visitors_this =
          match _visitors_this with
          | SFlit _visitors_c0 -> self#on_SFlit env _visitors_c0
          | SFclass_const (_visitors_c0,_visitors_c1) ->
              self#on_SFclass_const env _visitors_c0 _visitors_c1
        method on_Covariant env = self#zero
        method on_Contravariant env = self#zero
        method on_Invariant env = self#zero
        method on_variance env _visitors_this =
          match _visitors_this with
          | Covariant  -> self#on_Covariant env
          | Contravariant  -> self#on_Contravariant env
          | Invariant  -> self#on_Invariant env
        method on_NSNamespace env = self#zero
        method on_NSClass env = self#zero
        method on_NSClassAndNamespace env = self#zero
        method on_NSFun env = self#zero
        method on_NSConst env = self#zero
        method on_ns_kind env _visitors_this =
          match _visitors_this with
          | NSNamespace  -> self#on_NSNamespace env
          | NSClass  -> self#on_NSClass env
          | NSClassAndNamespace  -> self#on_NSClassAndNamespace env
          | NSFun  -> self#on_NSFun env
          | NSConst  -> self#on_NSConst env
        method on_Constraint_as env = self#zero
        method on_Constraint_eq env = self#zero
        method on_Constraint_super env = self#zero
        method on_constraint_kind env _visitors_this =
          match _visitors_this with
          | Constraint_as  -> self#on_Constraint_as env
          | Constraint_eq  -> self#on_Constraint_eq env
          | Constraint_super  -> self#on_Constraint_super env
        method on_Cabstract env = self#zero
        method on_Cnormal env = self#zero
        method on_Cinterface env = self#zero
        method on_Ctrait env = self#zero
        method on_Cenum env = self#zero
        method on_class_kind env _visitors_this =
          match _visitors_this with
          | Cabstract  -> self#on_Cabstract env
          | Cnormal  -> self#on_Cnormal env
          | Cinterface  -> self#on_Cinterface env
          | Ctrait  -> self#on_Ctrait env
          | Cenum  -> self#on_Cenum env
        method on_MustExtend env = self#zero
        method on_MustImplement env = self#zero
        method on_trait_req_kind env _visitors_this =
          match _visitors_this with
          | MustExtend  -> self#on_MustExtend env
          | MustImplement  -> self#on_MustImplement env
        method on_Final env = self#zero
        method on_Static env = self#zero
        method on_Abstract env = self#zero
        method on_Private env = self#zero
        method on_Public env = self#zero
        method on_Protected env = self#zero
        method on_kind env _visitors_this =
          match _visitors_this with
          | Final  -> self#on_Final env
          | Static  -> self#on_Static env
          | Abstract  -> self#on_Abstract env
          | Private  -> self#on_Private env
          | Public  -> self#on_Public env
          | Protected  -> self#on_Protected env
        method on_Pinout env = self#zero
        method on_param_kind env _visitors_this =
          match _visitors_this with | Pinout  -> self#on_Pinout env
        method on_OG_nullthrows env = self#zero
        method on_OG_nullsafe env = self#zero
        method on_og_null_flavor env _visitors_this =
          match _visitors_this with
          | OG_nullthrows  -> self#on_OG_nullthrows env
          | OG_nullsafe  -> self#on_OG_nullsafe env
        method on_FSync env = self#zero
        method on_FAsync env = self#zero
        method on_FGenerator env = self#zero
        method on_FAsyncGenerator env = self#zero
        method on_FCoroutine env = self#zero
        method on_fun_kind env _visitors_this =
          match _visitors_this with
          | FSync  -> self#on_FSync env
          | FAsync  -> self#on_FAsync env
          | FGenerator  -> self#on_FGenerator env
          | FAsyncGenerator  -> self#on_FAsyncGenerator env
          | FCoroutine  -> self#on_FCoroutine env
        method on_Plus env = self#zero
        method on_Minus env = self#zero
        method on_Star env = self#zero
        method on_Slash env = self#zero
        method on_Eqeq env = self#zero
        method on_EQeqeq env = self#zero
        method on_Starstar env = self#zero
        method on_Diff env = self#zero
        method on_Diff2 env = self#zero
        method on_AMpamp env = self#zero
        method on_BArbar env = self#zero
        method on_LogXor env = self#zero
        method on_Lt env = self#zero
        method on_Lte env = self#zero
        method on_Gt env = self#zero
        method on_Gte env = self#zero
        method on_Dot env = self#zero
        method on_Amp env = self#zero
        method on_Bar env = self#zero
        method on_Ltlt env = self#zero
        method on_Gtgt env = self#zero
        method on_Percent env = self#zero
        method on_Xor env = self#zero
        method on_Cmp env = self#zero
        method on_Eq env _visitors_c0 =
          let _visitors_s0 = self#on_option self#on_bop env _visitors_c0  in
          _visitors_s0
        method on_bop env _visitors_this =
          match _visitors_this with
          | Plus  -> self#on_Plus env
          | Minus  -> self#on_Minus env
          | Star  -> self#on_Star env
          | Slash  -> self#on_Slash env
          | Eqeq  -> self#on_Eqeq env
          | EQeqeq  -> self#on_EQeqeq env
          | Starstar  -> self#on_Starstar env
          | Diff  -> self#on_Diff env
          | Diff2  -> self#on_Diff2 env
          | AMpamp  -> self#on_AMpamp env
          | BArbar  -> self#on_BArbar env
          | LogXor  -> self#on_LogXor env
          | Lt  -> self#on_Lt env
          | Lte  -> self#on_Lte env
          | Gt  -> self#on_Gt env
          | Gte  -> self#on_Gte env
          | Dot  -> self#on_Dot env
          | Amp  -> self#on_Amp env
          | Bar  -> self#on_Bar env
          | Ltlt  -> self#on_Ltlt env
          | Gtgt  -> self#on_Gtgt env
          | Percent  -> self#on_Percent env
          | Xor  -> self#on_Xor env
          | Cmp  -> self#on_Cmp env
          | Eq _visitors_c0 -> self#on_Eq env _visitors_c0
        method on_Utild env = self#zero
        method on_Unot env = self#zero
        method on_Uplus env = self#zero
        method on_Uminus env = self#zero
        method on_Uincr env = self#zero
        method on_Udecr env = self#zero
        method on_Upincr env = self#zero
        method on_Updecr env = self#zero
        method on_Uref env = self#zero
        method on_Usilence env = self#zero
        method on_uop env _visitors_this =
          match _visitors_this with
          | Utild  -> self#on_Utild env
          | Unot  -> self#on_Unot env
          | Uplus  -> self#on_Uplus env
          | Uminus  -> self#on_Uminus env
          | Uincr  -> self#on_Uincr env
          | Udecr  -> self#on_Udecr env
          | Upincr  -> self#on_Upincr env
          | Updecr  -> self#on_Updecr env
          | Uref  -> self#on_Uref env
          | Usilence  -> self#on_Usilence env
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
        method on_Cst_define env = Cst_define
        method on_Cst_const env = Cst_const
        method on_cst_kind env _visitors_this =
          match _visitors_this with
          | Cst_define  -> self#on_Cst_define env
          | Cst_const  -> self#on_Cst_const env
        method on_pos env _visitors_this = _visitors_this
        method on_id env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_pstring env (_visitors_c0,_visitors_c1) =
          let _visitors_r0 = self#on_pos env _visitors_c0  in
          let _visitors_r1 = self#on_string env _visitors_c1  in
          (_visitors_r0, _visitors_r1)
        method on_SFlit env _visitors_c0 =
          let _visitors_r0 = self#on_pstring env _visitors_c0  in
          SFlit _visitors_r0
        method on_SFclass_const env _visitors_c0 _visitors_c1 =
          let _visitors_r0 = self#on_id env _visitors_c0  in
          let _visitors_r1 = self#on_pstring env _visitors_c1  in
          SFclass_const (_visitors_r0, _visitors_r1)
        method on_shape_field_name env _visitors_this =
          match _visitors_this with
          | SFlit _visitors_c0 -> self#on_SFlit env _visitors_c0
          | SFclass_const (_visitors_c0,_visitors_c1) ->
              self#on_SFclass_const env _visitors_c0 _visitors_c1
        method on_Covariant env = Covariant
        method on_Contravariant env = Contravariant
        method on_Invariant env = Invariant
        method on_variance env _visitors_this =
          match _visitors_this with
          | Covariant  -> self#on_Covariant env
          | Contravariant  -> self#on_Contravariant env
          | Invariant  -> self#on_Invariant env
        method on_NSNamespace env = NSNamespace
        method on_NSClass env = NSClass
        method on_NSClassAndNamespace env = NSClassAndNamespace
        method on_NSFun env = NSFun
        method on_NSConst env = NSConst
        method on_ns_kind env _visitors_this =
          match _visitors_this with
          | NSNamespace  -> self#on_NSNamespace env
          | NSClass  -> self#on_NSClass env
          | NSClassAndNamespace  -> self#on_NSClassAndNamespace env
          | NSFun  -> self#on_NSFun env
          | NSConst  -> self#on_NSConst env
        method on_Constraint_as env = Constraint_as
        method on_Constraint_eq env = Constraint_eq
        method on_Constraint_super env = Constraint_super
        method on_constraint_kind env _visitors_this =
          match _visitors_this with
          | Constraint_as  -> self#on_Constraint_as env
          | Constraint_eq  -> self#on_Constraint_eq env
          | Constraint_super  -> self#on_Constraint_super env
        method on_Cabstract env = Cabstract
        method on_Cnormal env = Cnormal
        method on_Cinterface env = Cinterface
        method on_Ctrait env = Ctrait
        method on_Cenum env = Cenum
        method on_class_kind env _visitors_this =
          match _visitors_this with
          | Cabstract  -> self#on_Cabstract env
          | Cnormal  -> self#on_Cnormal env
          | Cinterface  -> self#on_Cinterface env
          | Ctrait  -> self#on_Ctrait env
          | Cenum  -> self#on_Cenum env
        method on_MustExtend env = MustExtend
        method on_MustImplement env = MustImplement
        method on_trait_req_kind env _visitors_this =
          match _visitors_this with
          | MustExtend  -> self#on_MustExtend env
          | MustImplement  -> self#on_MustImplement env
        method on_Final env = Final
        method on_Static env = Static
        method on_Abstract env = Abstract
        method on_Private env = Private
        method on_Public env = Public
        method on_Protected env = Protected
        method on_kind env _visitors_this =
          match _visitors_this with
          | Final  -> self#on_Final env
          | Static  -> self#on_Static env
          | Abstract  -> self#on_Abstract env
          | Private  -> self#on_Private env
          | Public  -> self#on_Public env
          | Protected  -> self#on_Protected env
        method on_Pinout env = Pinout
        method on_param_kind env _visitors_this =
          match _visitors_this with | Pinout  -> self#on_Pinout env
        method on_OG_nullthrows env = OG_nullthrows
        method on_OG_nullsafe env = OG_nullsafe
        method on_og_null_flavor env _visitors_this =
          match _visitors_this with
          | OG_nullthrows  -> self#on_OG_nullthrows env
          | OG_nullsafe  -> self#on_OG_nullsafe env
        method on_FSync env = FSync
        method on_FAsync env = FAsync
        method on_FGenerator env = FGenerator
        method on_FAsyncGenerator env = FAsyncGenerator
        method on_FCoroutine env = FCoroutine
        method on_fun_kind env _visitors_this =
          match _visitors_this with
          | FSync  -> self#on_FSync env
          | FAsync  -> self#on_FAsync env
          | FGenerator  -> self#on_FGenerator env
          | FAsyncGenerator  -> self#on_FAsyncGenerator env
          | FCoroutine  -> self#on_FCoroutine env
        method on_Plus env = Plus
        method on_Minus env = Minus
        method on_Star env = Star
        method on_Slash env = Slash
        method on_Eqeq env = Eqeq
        method on_EQeqeq env = EQeqeq
        method on_Starstar env = Starstar
        method on_Diff env = Diff
        method on_Diff2 env = Diff2
        method on_AMpamp env = AMpamp
        method on_BArbar env = BArbar
        method on_LogXor env = LogXor
        method on_Lt env = Lt
        method on_Lte env = Lte
        method on_Gt env = Gt
        method on_Gte env = Gte
        method on_Dot env = Dot
        method on_Amp env = Amp
        method on_Bar env = Bar
        method on_Ltlt env = Ltlt
        method on_Gtgt env = Gtgt
        method on_Percent env = Percent
        method on_Xor env = Xor
        method on_Cmp env = Cmp
        method on_Eq env _visitors_c0 =
          let _visitors_r0 = self#on_option self#on_bop env _visitors_c0  in
          Eq _visitors_r0
        method on_bop env _visitors_this =
          match _visitors_this with
          | Plus  -> self#on_Plus env
          | Minus  -> self#on_Minus env
          | Star  -> self#on_Star env
          | Slash  -> self#on_Slash env
          | Eqeq  -> self#on_Eqeq env
          | EQeqeq  -> self#on_EQeqeq env
          | Starstar  -> self#on_Starstar env
          | Diff  -> self#on_Diff env
          | Diff2  -> self#on_Diff2 env
          | AMpamp  -> self#on_AMpamp env
          | BArbar  -> self#on_BArbar env
          | LogXor  -> self#on_LogXor env
          | Lt  -> self#on_Lt env
          | Lte  -> self#on_Lte env
          | Gt  -> self#on_Gt env
          | Gte  -> self#on_Gte env
          | Dot  -> self#on_Dot env
          | Amp  -> self#on_Amp env
          | Bar  -> self#on_Bar env
          | Ltlt  -> self#on_Ltlt env
          | Gtgt  -> self#on_Gtgt env
          | Percent  -> self#on_Percent env
          | Xor  -> self#on_Xor env
          | Cmp  -> self#on_Cmp env
          | Eq _visitors_c0 -> self#on_Eq env _visitors_c0
        method on_Utild env = Utild
        method on_Unot env = Unot
        method on_Uplus env = Uplus
        method on_Uminus env = Uminus
        method on_Uincr env = Uincr
        method on_Udecr env = Udecr
        method on_Upincr env = Upincr
        method on_Updecr env = Updecr
        method on_Uref env = Uref
        method on_Usilence env = Usilence
        method on_uop env _visitors_this =
          match _visitors_this with
          | Utild  -> self#on_Utild env
          | Unot  -> self#on_Unot env
          | Uplus  -> self#on_Uplus env
          | Uminus  -> self#on_Uminus env
          | Uincr  -> self#on_Uincr env
          | Udecr  -> self#on_Udecr env
          | Upincr  -> self#on_Upincr env
          | Updecr  -> self#on_Updecr env
          | Uref  -> self#on_Uref env
          | Usilence  -> self#on_Usilence env
      end
    [@@@VISITORS.END ]
  end
type fun_decl_kind =
  | FDeclAsync 
  | FDeclSync 
  | FDeclCoroutine [@@deriving show]
let rec (pp_fun_decl_kind :
          Format.formatter -> fun_decl_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | FDeclAsync  -> Format.pp_print_string fmt "FDeclAsync"
        | FDeclSync  -> Format.pp_print_string fmt "FDeclSync"
        | FDeclCoroutine  -> Format.pp_print_string fmt "FDeclCoroutine")
  [@ocaml.warning "-A"])

and show_fun_decl_kind : fun_decl_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_decl_kind x

let string_of_class_kind =
  function
  | Cabstract  -> "an abstract class"
  | Cnormal  -> "a class"
  | Cinterface  -> "an interface"
  | Ctrait  -> "a trait"
  | Cenum  -> "an enum" 
let string_of_kind =
  function
  | Final  -> "final"
  | Static  -> "static"
  | Abstract  -> "abstract"
  | Private  -> "private"
  | Public  -> "public"
  | Protected  -> "protected" 
let string_of_param_kind = function | Pinout  -> "inout" 
module ShapeField =
  struct
    type t = shape_field_name
    let compare x y =
      match (x, y) with
      | (SFlit _,SFclass_const _) -> (-1)
      | (SFclass_const _,SFlit _) -> 1
      | (SFlit (_,s1),SFlit (_,s2)) -> Pervasives.compare s1 s2
      | (SFclass_const ((_,s1),(_,s1')),SFclass_const ((_,s2),(_,s2'))) ->
          Pervasives.compare (s1, s1') (s2, s2')
      
  end
module ShapeMap =
  struct
    include (MyMap.Make)(ShapeField)
    let map_and_rekey m f1 f2 =
      fold (fun k  -> fun v  -> fun acc  -> add (f1 k) (f2 v) acc) m empty 
    let pp _ fmt _ = Format.pp_print_string fmt "[ShapeMap]" 
  end
