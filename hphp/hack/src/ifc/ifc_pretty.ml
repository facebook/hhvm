(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Format
open Ifc_types
module Env = Ifc_env
module Logic = Ifc_logic
module Utils = Ifc_utils

let comma_sep fmt () = fprintf fmt ",@ "

let blank_sep fmt () = fprintf fmt "@ "

let rec list pp_sep pp fmt = function
  | [] -> ()
  | [x] -> fprintf fmt "%a" pp x
  | x :: xs -> fprintf fmt "%a%a%a" pp x pp_sep () (list pp_sep pp) xs

let smap pp_sep pp fmt map =
  let map_as_list = SMap.fold (fun k v acc -> (k, v) :: acc) map [] in
  let prop_pol fmt (prop, pol) = fprintf fmt "%s -> %a" prop pp pol in
  list pp_sep prop_pol fmt map_as_list

let option pp fmt opt =
  match opt with
  | Some x -> fprintf fmt "%a" pp x
  | None -> fprintf fmt "None"

let show_policy = function
  | Pbot -> "Bot"
  | Ptop -> "Top"
  | Ppurpose p -> p
  | Pfree_var (v, _s) -> Printf.sprintf "v%d" v
  | Pbound_var n -> Printf.sprintf "<bound%d>" n

let policy fmt p = fprintf fmt "%s" (show_policy p)

let prop =
  let rec conjuncts = function
    | Cconj (Cflow (a, b), Cflow (c, d))
      when equal_policy a d && equal_policy b c ->
      [`q (a, b)]
    | Cflow (a, (Pbot as b))
    | Cflow ((Ptop as b), a) ->
      [`q (a, b)]
    | Cconj (cl, cr) -> conjuncts cl @ conjuncts cr
    | Ctrue -> []
    | c -> [`c c]
  in
  let bv =
    let rec f = function
      | [] -> ['a']
      | 'z' :: n -> 'a' :: f n
      | c :: n -> Char.(of_int_exn (1 + to_int c)) :: n
    in
    (fun i -> String.of_char_list (Utils.funpow i ~f ~init:[]))
  in
  let pp_policy b fmt = function
    | Pbound_var n -> fprintf fmt "%s" (bv (b - n))
    | p -> policy fmt p
  in
  let rec aux b fmt =
    let pol = pp_policy b in
    function
    | [] -> fprintf fmt "True"
    | [`q (p1, p2)] -> fprintf fmt "%a = %a" pol p1 pol p2
    | [`c (Cflow (p1, p2))] -> fprintf fmt "%a < %a" pol p1 pol p2
    | [`c (Cquant (q, n, c))] ->
      fprintf
        fmt
        "@[<hov2>%s @[<h>%a@].@ %a@]"
        (match q with
        | Qexists -> "exists"
        | Qforall -> "forall")
        (list blank_sep pp_print_string)
        (snd
           (Utils.funpow
              n
              ~f:(fun (i, l) -> (i + 1, l @ [bv i]))
              ~init:(b + 1, [])))
        (aux (b + n))
        (conjuncts c)
    | [`c (Ccond ((p, x), ct, ce))] ->
      fprintf fmt "@[<hov>if %a < %s@" pol p x;
      let cct = conjuncts ct in
      let cce = conjuncts ce in
      fprintf fmt "then %a@ else %a@]" (aux b) cct (aux b) cce
    | l ->
      let pp = list comma_sep (fun fmt c -> aux b fmt [c]) in
      fprintf fmt "[@[<hov>%a@]]" pp l
  in
  (fun fmt c -> aux 0 fmt (conjuncts c))

let rec ptype fmt ty =
  let list sep l =
    let pp_sep fmt () = fprintf fmt "%s@ " sep in
    fprintf fmt "(@[<hov2>%a@])" (list pp_sep ptype) l
  in
  match ty with
  | Tprim p -> fprintf fmt "<%a>" policy p
  | Ttuple tl -> list "," tl
  | Tunion tl -> list " |" tl
  | Tinter tl -> list " &" tl
  | Tclass { c_name; c_self; c_lump; c_property_map } ->
    fprintf
      fmt
      "%s<%a, %a, %a>"
      c_name
      policy
      c_self
      policy
      c_lump
      (smap comma_sep ptype)
      c_property_map

let locals fmt env =
  let pp_lenv fmt { le_vars } = LMap.make_pp Local_id.pp ptype fmt le_vars in
  let pp_lenv_opt fmt = function
    | Some lenv -> pp_lenv fmt lenv
    | None -> fprintf fmt "<empty>"
  in
  pp_lenv_opt fmt (Env.get_lenv_opt env Typing_cont_key.Next)

let renv fmt renv =
  fprintf fmt "@[<v>";
  fprintf fmt "* @[<hov2>pc: @[<hov>%a@]@]" policy (List.last_exn renv.re_gpc);
  fprintf fmt "@,* @[<hov2>This:@ @[<hov>%a@]@]" (option ptype) renv.re_this;
  fprintf fmt "@,* @[<hov2>Return:@ @[<hov>%a@]@]" ptype renv.re_ret;
  fprintf fmt "@]"

let env fmt env =
  fprintf fmt "@[<v>";
  fprintf fmt "@[<hov2>Locals:@ %a@]" locals env;
  let p = Logic.prop_conjoin (List.rev env.e_acc) in
  fprintf fmt "@,Constraints:@,  @[<v>%a@]" prop p;
  fprintf fmt "@]"

let policy_sig fmt { psig_policied_properties; psig_unpolicied_properties } =
  let property fmt (name, _ty) = fprintf fmt "%s" name in
  let properties fmt = list comma_sep property fmt in
  fprintf fmt "@[<v>";
  fprintf
    fmt
    "* @[<hov2>Policied properties:@ @[<hov>%a@]@]"
    properties
    psig_policied_properties;
  fprintf
    fmt
    "@,* @[<hov2>Unpolicied properties:@ @[<hov>%a@]@]"
    properties
    psig_unpolicied_properties;
  fprintf fmt "@]"

let policy_sig_env fmt map =
  let handle_class class_name psig =
    fprintf fmt "Policy signature for %s:@,%a@.@." class_name policy_sig psig
  in
  SMap.iter handle_class map
