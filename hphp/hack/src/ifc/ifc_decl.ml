(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Ifc_types
module Env = Ifc_env
module Logic = Ifc_logic
module Mapper = Ifc_mapper
module Lattice = Ifc_security_lattice
module L = Logic.Infix
module A = Aast
module T = Typing_defs

(* Everything done in this file should eventually be merged in Hack's
   regular decl phase. Right now it is more convenient to keep things
   simple and separate here. *)

exception FlowDecl of string

let fail s = raise (FlowDecl s)

let policied_id = "\\Policied"

let infer_flows_id = "\\InferFlows"

let exception_id = "\\Exception"

let out_of_bounds_exception_id = "\\OutOfBoundsException"

let vec_id = "\\HH\\vec"

let dict_id = "\\HH\\dict"

let keyset_id = "\\HH\\keyset"

let awaitable_id = "\\HH\\Awaitable"

let governed_id = "\\Governed"

let construct_id = "__construct"

let external_id = "\\External"

let callable_id = "\\CanCall"

let make_callable_name cls_name_opt name =
  match cls_name_opt with
  | None -> name
  | Some cls_name -> cls_name ^ "#" ^ name

let get_attr attr attrs =
  let is_attr a = String.equal (snd a.A.ua_name) attr in
  match List.filter ~f:is_attr attrs with
  | [] -> None
  | [a] -> Some a
  | _ -> fail ("multiple '" ^ attr ^ "' attributes found")

let callable_decl attrs args =
  let fd_kind =
    match get_attr governed_id attrs with
    | Some attr ->
      let policy =
        match attr.A.ua_params with
        | [] -> None
        | [((pos, _), A.String purpose)] ->
          Some (Lattice.parse_policy (PosSet.singleton pos) purpose)
        | _ -> fail "expected a string literal as governed by argument."
      in
      FDGovernedBy policy
    | None ->
      if Option.is_some (get_attr infer_flows_id attrs) then
        FDInferFlows
      else
        (* Eventually, make this (FDGovernedBy Pbot) *)
        FDInferFlows
  in
  let fd_args =
    let mk_arg_kind param id f def =
      match get_attr id param.A.param_user_attributes with
      | Some { A.ua_name = (pos, _); _ } -> f pos
      | None -> def
    in
    let f param =
      AKDefault
      |> mk_arg_kind param external_id (fun p -> AKExternal p)
      |> mk_arg_kind param callable_id (fun p -> AKCallable p)
    in
    List.map ~f args
  in
  { fd_kind; fd_args }

let fun_ { A.f_name = (_, name); f_user_attributes = attrs; f_params = args; _ }
    =
  (make_callable_name None name, callable_decl attrs args)

let meth
    class_name
    { A.m_name = (_, name); m_user_attributes = attrs; m_params = args; _ } =
  (make_callable_name (Some class_name) name, callable_decl attrs args)

let immediate_supers { A.c_uses; A.c_extends; _ } =
  let id_of_hint = function
    | (_, A.Happly (id, _)) -> snd id
    | _ -> fail "unexpected hint in inheritance hierarchy"
  in
  List.map ~f:id_of_hint (c_extends @ c_uses)

(* A property declared in a trait T can be redeclared in a class or a trait
 * inheriting from T. When this property is policied it will be inherited with a
 * (possibly) different policied annotation. We need to pick one.
 *
 * Our criteria for resolution is:
 * 1. If both declarations are unpolicied, it is not a policied property;
 * 2. If only one is policied, it is a policied property (possibly with a purpose);
 * 3. If both of them are policied and
 *   a. neither has a purpose, property is policied without a purpose
 *   b. only one has a purpose, property is policied with that purpose
 *   c. both have the same purpose, property is policied with that purpose
 *   d. have differing purposes, it is an error
 *
 * (1) and (2) are enforced automatically by the virtue of only keeping track
 * of policied properties. The following function enforces (3).
 *)
let resolve_duplicate_policied_properties policied_properties =
  let err_msg name purp1 purp2 =
    name ^ " has purpose " ^ purp1 ^ " and " ^ purp2 ^ " due to inheritance"
  in
  let prop_table = Caml.Hashtbl.create 10 in
  let go pprop =
    match Caml.Hashtbl.find_opt prop_table pprop.pp_name with
    | Some pprop' ->
      if not @@ String.equal pprop.pp_purpose pprop'.pp_purpose then
        fail @@ err_msg pprop.pp_name pprop.pp_purpose pprop'.pp_purpose
    | None -> Caml.Hashtbl.add prop_table pprop.pp_name pprop
  in
  List.iter ~f:go policied_properties;
  Caml.Hashtbl.fold (fun _ pprop acc -> pprop :: acc) prop_table []

let is_visible pp = not @@ A.equal_visibility A.Private pp.pp_visibility

let add_super class_decl_env class_decl_acc super =
  match SMap.find_opt super class_decl_env with
  | Some { cd_policied_properties } ->
    let super_props = List.filter ~f:is_visible cd_policied_properties in
    let props = super_props @ class_decl_acc.cd_policied_properties in
    { cd_policied_properties = props }
  | None -> fail @@ super ^ " wasn't found in the inheritance hierarchy"

let mk_policied_prop
    {
      A.cv_span = pp_pos;
      cv_id = (_, pp_name);
      cv_visibility = pp_visibility;
      cv_user_attributes = attrs;
      _;
    } =
  let find_policy attributes =
    match get_attr policied_id attributes with
    | None -> `No_policy
    | Some attr ->
      (match attr.A.ua_params with
      | [(_, A.String purpose)] -> `Policy purpose
      | _ -> fail "expected a string literal as a purpose argument")
  in
  match find_policy attrs with
  | `No_policy -> None
  | `Policy pp_purpose -> Some { pp_name; pp_visibility; pp_purpose; pp_pos }

let class_ class_decl_env class_ =
  let { A.c_name = (_, name); c_vars = properties; _ } = class_ in

  (* Class decl using the immediately available information of the base class *)
  let cd_policied_properties = List.filter_map ~f:mk_policied_prop properties in
  let base_class_decl = { cd_policied_properties } in

  (* Class decl extended with inherited policied properties *)
  let supers = immediate_supers class_ in
  let class_decl =
    let f = add_super class_decl_env in
    List.fold ~f ~init:base_class_decl supers
  in
  let cd_policied_properties =
    resolve_duplicate_policied_properties class_decl.cd_policied_properties
    |> List.sort ~compare:(fun p1 p2 -> String.compare p1.pp_name p2.pp_name)
  in
  let class_decl = { cd_policied_properties } in

  (* Function declarations out of methods *)
  let fun_decls = List.map ~f:(meth name) class_.A.c_methods in

  (name, class_decl, fun_decls)

let magic_class_decls =
  SMap.of_list
    [
      (vec_id, { cd_policied_properties = [] });
      (exception_id, { cd_policied_properties = [] });
    ]

let topsort_classes classes =
  (* Record the class hierarchy *)
  let dependency_table = Caml.Hashtbl.create 10 in
  let id_of_hint = function
    | (_, A.Happly (id, _)) -> snd id
    | _ -> fail "unexpected hint in inheritance hierarchy"
  in
  let go ({ A.c_name; A.c_extends; A.c_uses; _ } as class_) =
    let supers = List.map ~f:id_of_hint (c_extends @ c_uses) in
    Caml.Hashtbl.add dependency_table (snd c_name) (class_, supers, false)
  in
  List.iter ~f:go classes;

  (* Put classes, traits, and interfaces in topological order *)
  let schedule = ref [] in
  let rec process id =
    match Caml.Hashtbl.find_opt dependency_table id with
    | Some (class_, dependencies, is_visited) ->
      if not is_visited then begin
        Caml.Hashtbl.replace dependency_table id (class_, dependencies, true);
        List.iter ~f:process dependencies;
        schedule := class_ :: !schedule
      end
    | None ->
      (* If it's a magic builtin, then it has no dependencies, so do nothing *)
      if not @@ SMap.mem id magic_class_decls then
        fail @@ id ^ " is missing entity in the inheritance hierarchy"
  in
  List.iter ~f:process (List.map ~f:(fun c -> snd c.A.c_name) classes);

  List.rev !schedule

(* Removes all the auxiliary info needed only during declaration analysis. *)
let collect_sigs defs =
  (* Prepare class and function definitions *)
  let pick = function
    | A.Class class_ -> `Fst class_
    | A.Fun fun_ -> `Snd fun_
    | _ -> `Trd ()
  in
  let (classes, funs, _) = List.partition3_map ~f:pick defs in
  let classes = topsort_classes classes in

  (* Process and accumulate function decls *)
  let fun_decls = SMap.of_list (List.map ~f:fun_ funs) in

  (* Process and accumulate class decls *)
  let init = { de_class = magic_class_decls; de_fun = fun_decls } in
  let add_class_decl { de_class; de_fun } cls =
    let (class_name, class_decl, meth_decls) = class_ de_class cls in
    let de_class = SMap.add class_name class_decl de_class in
    let de_fun = SMap.union (SMap.of_list meth_decls) de_fun in
    { de_class; de_fun }
  in
  List.fold ~f:add_class_decl ~init classes

let property_policy { de_class; _ } cname pname =
  Option.(
    SMap.find_opt cname de_class >>= fun cls ->
    List.find
      ~f:(fun p -> String.equal p.pp_name pname)
      cls.cd_policied_properties
    >>= fun p ->
    return
      (Ifc_security_lattice.parse_policy
         (PosSet.singleton p.pp_pos)
         p.pp_purpose))

(* Builds the type scheme for a callable *)
let make_callable_scheme renv pol fp args =
  let renv = { renv with re_scope = Scope.alloc () } in
  let policy =
    match pol with
    | Some policy -> policy
    | None -> Env.new_policy_var renv "implicit"
  in
  let rec set_policy p pty = Mapper.ptype (set_policy p) (const p) pty in
  let (acc, args) =
    let f acc ty = function
      | AKDefault -> (acc, set_policy policy ty)
      | AKExternal pos ->
        let ext = Env.new_policy_var renv "external" in
        (L.(ext < policy) ~pos acc, set_policy ext ty)
      | AKCallable pos ->
        let c = Env.new_policy_var renv "callable" in
        (L.(policy < c) ~pos acc, set_policy c ty)
    in
    List.map2_env [] ~f fp.fp_type.f_args args
  in
  let fp' =
    {
      fp_name = fp.fp_name;
      fp_this = Option.map ~f:(set_policy policy) fp.fp_this;
      fp_type =
        {
          f_pc = policy;
          f_args = args;
          f_ret = set_policy policy fp.fp_type.f_ret;
          f_exn = set_policy policy fp.fp_type.f_exn;
          f_self = pbot;
        };
    }
  in
  Fscheme (renv.re_scope, fp', Logic.conjoin acc)
