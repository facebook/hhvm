(* @generated from ast_defs.src.ml by hphp/hack/tools/ppx/ppx_gen. *)
(* SourceShasum<<c3b86d8a94b85907a7c5b36fde5615d34d5b4886>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2017, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
type cst_kind =
  | Cst_define 
  | Cst_const [@@deriving show]
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

type id = (Pos.t* string)[@@deriving show]
let rec pp_id : Format.formatter -> id -> Ppx_deriving_runtime.unit =
  let __0 () = Pos.pp  in
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

type pstring = (Pos.t* string)[@@deriving show]
let rec pp_pstring : Format.formatter -> pstring -> Ppx_deriving_runtime.unit
  =
  let __0 () = Pos.pp  in
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

type variance =
  | Covariant 
  | Contravariant 
  | Invariant [@@deriving show]
let rec (pp_variance :
          Format.formatter -> variance -> Ppx_deriving_runtime.unit)
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

type ns_kind =
  | NSNamespace 
  | NSClass 
  | NSClassAndNamespace 
  | NSFun 
  | NSConst [@@deriving show]
let rec (pp_ns_kind :
          Format.formatter -> ns_kind -> Ppx_deriving_runtime.unit)
  =
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

type constraint_kind =
  | Constraint_as 
  | Constraint_eq 
  | Constraint_super [@@deriving show]
let rec (pp_constraint_kind :
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

type class_kind =
  | Cabstract 
  | Cnormal 
  | Cinterface 
  | Ctrait 
  | Cenum [@@deriving show]
let rec (pp_class_kind :
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

type trait_req_kind =
  | MustExtend 
  | MustImplement [@@deriving show]
let rec (pp_trait_req_kind :
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

type kind =
  | Final 
  | Static 
  | Abstract 
  | Private 
  | Public 
  | Protected [@@deriving show]
let rec (pp_kind : Format.formatter -> kind -> Ppx_deriving_runtime.unit) =
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

type fun_decl_kind =
  | FDeclAsync 
  | FDeclSync [@@deriving show]
let rec (pp_fun_decl_kind :
          Format.formatter -> fun_decl_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | FDeclAsync  -> Format.pp_print_string fmt "FDeclAsync"
        | FDeclSync  -> Format.pp_print_string fmt "FDeclSync")
  [@ocaml.warning "-A"])

and show_fun_decl_kind : fun_decl_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_decl_kind x

type fun_kind =
  | FSync 
  | FAsync 
  | FGenerator 
  | FAsyncGenerator [@@deriving show]
let rec (pp_fun_kind :
          Format.formatter -> fun_kind -> Ppx_deriving_runtime.unit)
  =
  ((let open! Ppx_deriving_runtime in
      fun fmt  ->
        function
        | FSync  -> Format.pp_print_string fmt "FSync"
        | FAsync  -> Format.pp_print_string fmt "FAsync"
        | FGenerator  -> Format.pp_print_string fmt "FGenerator"
        | FAsyncGenerator  -> Format.pp_print_string fmt "FAsyncGenerator")
  [@ocaml.warning "-A"])

and show_fun_kind : fun_kind -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_fun_kind x

type shape_field_name =
  | SFlit of pstring 
  | SFclass_const of id* pstring [@@deriving show]
let rec pp_shape_field_name :
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

type bop =
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
  | Eq of bop option [@@deriving show]
let rec pp_bop : Format.formatter -> bop -> Ppx_deriving_runtime.unit =
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

type uop =
  | Utild 
  | Unot 
  | Uplus 
  | Uminus 
  | Uincr 
  | Udecr 
  | Upincr 
  | Updecr 
  | Uref 
  | Usplat 
  | Usilence [@@deriving show]
let rec (pp_uop : Format.formatter -> uop -> Ppx_deriving_runtime.unit) =
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
        | Usplat  -> Format.pp_print_string fmt "Usplat"
        | Usilence  -> Format.pp_print_string fmt "Usilence")
  [@ocaml.warning "-A"])

and show_uop : uop -> Ppx_deriving_runtime.string =
  fun x  -> Format.asprintf "%a" pp_uop x

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
    include MyMap.Make(ShapeField)
    let map_and_rekey m f1 f2 =
      fold (fun k  -> fun v  -> fun acc  -> add (f1 k) (f2 v) acc) m empty 
    let pp _ fmt _ = Format.pp_print_string fmt "[ShapeMap]" 
  end
