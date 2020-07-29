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
  let prop_pol fmt (prop, pol) = fprintf fmt "%s=%a" prop pp pol in
  list pp_sep prop_pol fmt (SMap.bindings map)

let option pp fmt opt =
  match opt with
  | Some x -> fprintf fmt "%a" pp x
  | None -> fprintf fmt "None"

let show_policy = function
  | Pbot -> "PUBLIC"
  | Ptop -> "PRIVATE"
  | Ppurpose p -> p
  | Pfree_var (v, _s) -> v
  | Pbound_var n -> Printf.sprintf "<bound%d>" n

let policy fmt p = fprintf fmt "%s" (show_policy p)

let rec ptype fmt ty =
  let list' sep l =
    let pp_sep fmt () = fprintf fmt "%s@ " sep in
    fprintf fmt "(@[<hov2>%a@])" (list pp_sep ptype) l
  in
  let tparam fmt pty = ptype fmt pty in
  let tparams fmt = fprintf fmt "[@[<hov2>%a@]]" (list comma_sep tparam) in
  match ty with
  | Tprim p
  | Tgeneric p ->
    fprintf fmt "<%a>" policy p
  | Ttuple tl -> list' "," tl
  | Tunion [] -> fprintf fmt "nothing"
  | Tunion tl -> list' " |" tl
  | Tinter tl -> list' " &" tl
  | Tclass { c_name; c_self; c_lump; c_property_map; c_tparams } ->
    fprintf fmt "%s" c_name;
    if not @@ SMap.is_empty c_tparams then
      fprintf fmt "<@[<hov2>%a@]>" (smap comma_sep tparams) c_tparams;
    fprintf fmt "<@[<hov2>%a,@ %a" policy c_self policy c_lump;
    if not (SMap.is_empty c_property_map) then
      fprintf fmt ",@ %a" (smap comma_sep lazy_ptype) c_property_map;
    fprintf fmt "@]>"

and lazy_ptype fmt ty =
  if Lazy.is_val ty then
    ptype fmt (Lazy.force ty)
  else
    fprintf fmt "?thunk"

let fun_proto fmt fp =
  Option.iter ~f:(fprintf fmt "(this: %a)->" ptype) fp.fp_this;
  fprintf fmt "%s<%a>" fp.fp_name policy fp.fp_pc;
  fprintf fmt "(@[<hov>%a@])" (list comma_sep ptype) fp.fp_args;
  fprintf fmt ":@ %a" ptype fp.fp_ret

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
    | [`c (Chole fp)] -> fprintf fmt "@[<h>{%a}@]" fun_proto fp
    | l ->
      let pp = list comma_sep (fun fmt c -> aux b fmt [c]) in
      fprintf fmt "[@[<hov>%a@]]" pp l
  in
  (fun fmt c -> aux 0 fmt (conjuncts c))

let locals fmt env =
  let pp_lenv fmt lenv =
    pp_open_vbox fmt 0;
    fprintf
      fmt
      "@[<hov2>lvars:@ %a@]"
      (LMap.make_pp Local_id.pp ptype)
      lenv.le_vars;
    let policy_set fmt s = list comma_sep policy fmt (PCSet.elements s) in
    if not (PCSet.is_empty lenv.le_pc) then
      fprintf fmt "@,@[<hov2>pc: @[<hov>%a@]@]" policy_set lenv.le_pc;
    pp_close_box fmt ()
  in
  let pp_lenv_opt fmt = function
    | Some lenv -> pp_lenv fmt lenv
    | None -> fprintf fmt "<empty>"
  in
  pp_lenv_opt fmt (Env.get_lenv_opt env Typing_cont_key.Next)

let renv fmt renv =
  pp_open_vbox fmt 0;
  fprintf fmt "* @[<hov2>pc: @[<hov>%a@]@]" policy renv.re_gpc;
  fprintf fmt "@,* @[<hov2>This:@ @[<hov>%a@]@]" (option ptype) renv.re_this;
  fprintf fmt "@,* @[<hov2>Return:@ @[<hov>%a@]@]" ptype renv.re_ret;
  fprintf fmt "@,* @[<hov2>Exception: @[<hov>%a@]@]" ptype renv.re_exn;
  pp_close_box fmt ()

let env fmt env =
  pp_open_vbox fmt 0;
  fprintf fmt "@[<hov2>Deps:@ %a@]" SSet.pp env.e_deps;
  fprintf fmt "@,Locals:@,  %a@." locals env;
  let p = Logic.conjoin env.e_acc in
  fprintf fmt "@,Constraints:@,  @[<v>%a@]" prop p;
  pp_close_box fmt ()

let class_decl fmt { cd_policied_properties = props; _ } =
  let policied_property fmt { pp_name; pp_purpose; _ } =
    match pp_purpose with
    | Some purpose -> fprintf fmt "%s:%s" pp_name purpose
    | None -> fprintf fmt "%s" pp_name
  in
  let properties fmt = list comma_sep policied_property fmt in
  fprintf fmt "{ policied_props = [@[<hov>%a@]] }" properties props

let fun_decl fmt decl =
  let kind =
    match decl.fd_kind with
    | FDPublic -> "public"
    | FDCIPP -> "cipp"
    | FDInferFlows -> "infer"
  in
  fprintf fmt "{ kind = %s }" kind

let decl_env fmt de =
  let handle_class name decl =
    fprintf fmt "class %s: %a@ " name class_decl decl
  in
  let handle_fun name decl =
    fprintf fmt "function %s: %a@ " name fun_decl decl
  in
  fprintf fmt "Decls:@.  @[<v>";
  SMap.iter handle_class de.de_class;
  SMap.iter handle_fun de.de_fun;
  fprintf fmt "@]"

let violation fmt (l, r) =
  fprintf fmt "Data with policy %a appears in context %a." policy l policy r
