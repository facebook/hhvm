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
module SN = Naming_special_names

(* Everything done in this file should eventually be merged in Hack's
   regular decl phase. Right now it is more convenient to keep things
   simple and separate here. *)

exception FlowDecl of string

let fail s = raise (FlowDecl s)

let policied_id = SN.UserAttributes.uaPolicied

let infer_flows_id = SN.UserAttributes.uaInferFlows

let exception_id = "\\Exception"

let out_of_bounds_exception_id = "\\OutOfBoundsException"

let vec_id = "\\HH\\vec"

let dict_id = "\\HH\\dict"

let keyset_id = "\\HH\\keyset"

let awaitable_id = "\\HH\\Awaitable"

let construct_id = SN.Members.__construct

let external_id = SN.UserAttributes.uaExternal

let callable_id = SN.UserAttributes.uaCanCall

let convert_ifc_fun_decl pos (tfd : T.ifc_fun_decl) : fun_decl_kind =
  match tfd with
  | T.FDInferFlows -> FDInferFlows
  | T.FDPolicied None -> FDPolicied None
  | T.FDPolicied (Some purpose) ->
    FDPolicied (Some (Lattice.parse_policy (PosSet.singleton pos) purpose))

let get_method_from_provider ~static ctx class_name method_name =
  match Decl_provider.get_class ctx class_name with
  | None -> None
  | Some cls when static -> Decl_provider.Class.get_smethod cls method_name
  | Some cls when String.equal method_name construct_id ->
    let (construct_opt, _) = Typing_classes_heap.Api.construct cls in
    construct_opt
  | Some cls -> Decl_provider.Class.get_method cls method_name

let convert_fun_type ctx fun_ty =
  let open Typing_defs in
  let resolve = Naming_provider.resolve_position ctx in
  let pos = get_pos fun_ty |> resolve in
  let fty = get_node fun_ty in
  match fty with
  | Tfun { ft_params; ft_ifc_decl; _ } ->
    let fd_kind = convert_ifc_fun_decl pos ft_ifc_decl in
    let mk_arg fp =
      let pos = resolve fp.fp_pos in
      match (T.get_fp_ifc_can_call fp, T.get_fp_ifc_external fp) with
      | (_, true) -> AKExternal pos
      | (true, _) -> AKCallable pos
      | (false, false) -> AKDefault
    in
    let fd_args = List.map ft_params ~f:mk_arg in
    { fd_kind; fd_args }
  | _ -> fail "Expected a Tfun type from function declaration"

(* Grab a function from the decl heap and convert it into a fun decl*)
let get_fun (ctx : Provider_context.t) (fun_name : string) : fun_decl option =
  let open Typing_defs in
  match Decl_provider.get_fun ctx fun_name with
  | None -> None
  | Some { fe_type; _ } -> Some (convert_fun_type ctx fe_type)

(* Grab a method from the decl heap and convert it into a fun_decl *)
let get_method
    (ctx : Provider_context.t) (class_name : string) (method_name : string) :
    fun_decl option =
  let open Typing_defs in
  match get_method_from_provider ~static:false ctx class_name method_name with
  (* The default constructor for classes is public and takes no arguments *)
  | None when String.equal method_name construct_id ->
    let default_kind =
      convert_ifc_fun_decl Pos.none Typing_defs.default_ifc_fun_decl
    in
    Some { fd_kind = default_kind; fd_args = [] }
  | None -> None
  | Some { ce_type = (lazy fun_type); _ } ->
    Some (convert_fun_type ctx fun_type)

let get_static_method
    (ctx : Provider_context.t) (class_name : string) (method_name : string) :
    fun_decl option =
  let open Typing_defs in
  match get_method_from_provider ~static:true ctx class_name method_name with
  | None -> None
  | Some { ce_type = (lazy fun_type); _ } ->
    Some (convert_fun_type ctx fun_type)

(* Grab any callable from the decl heap *)
let get_callable_decl (ctx : Provider_context.t) (callable_name : callable_name)
    : fun_decl option =
  match callable_name with
  | Method (cls, name) -> get_method ctx cls name
  | StaticMethod (cls, name) -> get_static_method ctx cls name
  | Function name -> get_fun ctx name

let callable_name_to_string = function
  | StaticMethod (cls, name)
  | Method (cls, name) ->
    cls ^ "#" ^ name
  | Function name -> name

let make_callable_name ~is_static cls_name_opt name =
  match cls_name_opt with
  | None -> Function name
  | Some cls_name when is_static -> StaticMethod (cls_name, name)
  | Some cls_name -> Method (cls_name, name)

(* Grab an attribute from a list of attrs. Only used for policy properties *)
let get_attr attr attrs =
  let is_attr a = String.equal (snd a.A.ua_name) attr in
  match List.filter ~f:is_attr attrs with
  | [] -> None
  | [a] -> Some a
  | _ -> fail ("multiple '" ^ attr ^ "' attributes found")

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
  | None ->
    (* Must be a builtin. Assume that builtins don't have policied properties *)
    class_decl_acc

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

  (name, class_decl)

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
      (* Must be a builtin. Assume that builtins don't have policied properties *)
      ()
  in
  List.iter ~f:process (List.map ~f:(fun c -> snd c.A.c_name) classes);

  List.rev !schedule

(* Removes all the auxiliary info needed only during declaration analysis. *)
let collect_sigs defs =
  let pick = function
    | A.Class class_ -> Some class_
    | _ -> None
  in
  let classes = List.filter_map ~f:pick defs |> topsort_classes in
  (* Process and accumulate class decls *)
  let init = { de_class = SMap.empty } in
  let add_class_decl { de_class } cls =
    let (class_name, class_decl) = class_ de_class cls in
    let de_class = SMap.add class_name class_decl de_class in
    { de_class }
  in
  List.fold ~f:add_class_decl ~init classes

let property_policy { de_class } cname pname =
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

    (* The length of arguments in the function type may not match that of
       argument kinds which is derived from the decl. Since all default
       arguments are public, we simply truncate the list. *)
    let fp_args = fp.fp_type.f_args in
    List.map2_env [] ~f fp_args (List.take args @@ List.length fp_args)
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
