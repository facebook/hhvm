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

let newline_sep fmt () = fprintf fmt "@,"

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
  | Pbot _ -> "PUBLIC"
  | Ptop _ -> "PRIVATE"
  | Ppurpose (_, p) -> p
  | Pfree_var (v, _s) -> v
  | Pbound_var n -> Printf.sprintf "<bound%d>" n

let policy fmt p = fprintf fmt "%s" (show_policy p)

let array_kind fmt = function
  | Avec -> fprintf fmt "vec"
  | Adict -> fprintf fmt "dict"
  | Akeyset -> fprintf fmt "keyset"

let rec ptype fmt ty =
  let list' sep l =
    let pp_sep fmt () = fprintf fmt "%s@ " sep in
    fprintf fmt "(@[<hov2>%a@])" (list pp_sep ptype) l
  in
  match ty with
  | Tprim p
  | Tgeneric p ->
    fprintf fmt "<%a>" policy p
  | Ttuple tl -> list' "," tl
  | Tunion [] -> fprintf fmt "nothing"
  | Tunion tl -> list' " |" tl
  | Tinter tl -> list' " &" tl
  | Tclass { c_name; c_self; c_lump } ->
    fprintf fmt "%s<@[<hov2>%a,@ %a@]>" c_name policy c_self policy c_lump
  | Tfun fn -> fun_ fmt fn
  | Tcow_array { a_key; a_value; a_length; a_kind } ->
    fprintf fmt "%a" array_kind a_kind;
    fprintf fmt "<%a => %a; |%a|>" ptype a_key ptype a_value policy a_length

(* Format: <pc, self>(arg1, arg2, ...): ret [exn] *)
and fun_ fmt fn =
  fprintf fmt "<%a, %a>" policy fn.f_pc policy fn.f_self;
  fprintf fmt "(@[<hov>%a@])" (list comma_sep ptype) fn.f_args;
  fprintf fmt ":@ %a [%a]" ptype fn.f_ret ptype fn.f_exn

let fun_proto fmt fp =
  Option.iter ~f:(fprintf fmt "(this: %a)->" ptype) fp.fp_this;
  fprintf fmt "%s%a" fp.fp_name fun_ fp.fp_type

let prop =
  let is_symmetric = function
    | (Cflow (_, a, b), Cflow (_, c, d)) -> equal_policy a d && equal_policy b c
    | _ -> false
  in
  let equate a b =
    if Policy.compare a b <= 0 then
      `q (a, b)
    else
      `q (b, a)
  in
  let rec conjuncts = function
    | Cconj ((Cflow (_, a, b) as f1), Cconj ((Cflow _ as f2), prop))
      when is_symmetric (f1, f2) ->
      equate a b :: conjuncts prop
    | Cconj ((Cflow (_, a, b) as f1), (Cflow _ as f2)) when is_symmetric (f1, f2)
      ->
      [equate a b]
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
    | [`c (Cflow (_, p1, p2))] -> fprintf fmt "%a < %a" pol p1 pol p2
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
    | [`c (Ccond ((_, p, x), ct, ce))] ->
      fprintf fmt "@[<hov>if %a < %s@" pol p x;
      let cct = conjuncts ct in
      let cce = conjuncts ce in
      fprintf fmt "then %a@ else %a@]" (aux b) cct (aux b) cce
    | [`c (Chole (_, fp))] -> fprintf fmt "@[<h>{%a}@]" fun_proto fp
    | l ->
      let pp = list comma_sep (fun fmt c -> aux b fmt [c]) in
      fprintf fmt "@[<hov>%a@]" pp l
  in
  (fun fmt c -> aux 0 fmt (conjuncts c))

let cont fmt k =
  pp_open_vbox fmt 0;
  fprintf fmt "@[<hov2>%a@]" (LMap.make_pp Local_id.pp ptype) k.k_vars;
  let policy_set fmt s = list comma_sep policy fmt (PSet.elements s) in
  if not (PSet.is_empty k.k_pc) then
    fprintf fmt "@,@[<hov2>pc: @[<hov>%a@]@]" policy_set k.k_pc;
  pp_close_box fmt ()

let renv fmt renv =
  pp_open_vbox fmt 0;
  fprintf fmt "* @[<hov2>pc: @[<hov>%a@]@]" policy renv.re_gpc;
  fprintf fmt "@,* @[<hov2>This:@ @[<hov>%a@]@]" (option ptype) renv.re_this;
  fprintf fmt "@,* @[<hov2>Return:@ @[<hov>%a@]@]" ptype renv.re_ret;
  fprintf fmt "@,* @[<hov2>Exception: @[<hov>%a@]@]" ptype renv.re_exn;
  pp_close_box fmt ()

let env fmt env =
  let rec flatten = function
    | Cconj (p1, p2) -> flatten p1 @ flatten p2
    | prop -> [prop]
  in
  let rec group_by_line =
    let has_same_pos p1 p2 =
      Option.equal Pos.equal (unique_pos_of_prop p1) (unique_pos_of_prop p2)
    in
    function
    | [] -> []
    | prop :: props ->
      let (group, rest) = List.partition_tf ~f:(has_same_pos prop) props in
      let pos_opt = unique_pos_of_prop prop in
      let prop = Logic.conjoin (prop :: group) in
      (pos_opt, prop) :: group_by_line rest
  in
  let group fmt (pos_opt, p) =
    match pos_opt with
    | Some pos -> fprintf fmt "@[<hov2>%a@ %a@]" Pos.pp pos prop p
    | None -> fprintf fmt "@[<hov2>[no pos]@ %a@]" prop p
  in
  let groups = list newline_sep group in

  pp_open_vbox fmt 0;
  fprintf fmt "@[<hov2>Deps:@ %a@]" SSet.pp (Env.get_deps env);
  let props = List.concat_map ~f:flatten (Env.get_constraints env) in
  let prop_groups =
    let compare (pos1, _) (pos2, _) = Option.compare Pos.compare pos1 pos2 in
    group_by_line props |> List.sort ~compare
  in
  fprintf fmt "@,Constraints:@,  @[<v>%a@]" groups prop_groups;
  pp_close_box fmt ()

let class_decl fmt { cd_policied_properties = props; _ } =
  let policied_property fmt { pp_name; pp_purpose; _ } =
    fprintf fmt "%s:%s" pp_name pp_purpose
  in
  let properties fmt = list comma_sep policied_property fmt in
  fprintf fmt "{ policied_props = [@[<hov>%a@]] }" properties props

let fun_decl fmt decl =
  let kind =
    match decl.fd_kind with
    | FDGovernedBy (Some policy) -> show_policy policy
    | FDGovernedBy None -> "implicit"
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

let implicit_violation fmt (l, r) =
  fprintf
    fmt
    "Data with an implicit policy is leaked by %a in context %a."
    policy
    l
    policy
    r

let security_lattice fmt lattice =
  let flow fmt (l, r) = fprintf fmt "%a < %a" policy l policy r in
  let flows = FlowSet.elements lattice in
  fprintf fmt "{%a}" (list comma_sep flow) flows
