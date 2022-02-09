(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to declare class types.
 * For each class we want to build a complete type, that is the type of
 * the methods defined in the class plus everything that was inherited.
 *)
(*****************************************************************************)
open Hh_prelude
open Decl_defs
open Aast
open Shallow_decl_defs
open Typing_defs
open Typing_deps
module Reason = Typing_reason
module Inst = Decl_instantiate
module Attrs = Typing_defs.Attributes
module SN = Naming_special_names

type class_entries = Decl_defs.decl_class_type * Decl_store.class_members option

type lazy_member_lookup_error =
  | LMLEShallowClassNotFound
  | LMLEMemberNotFound
[@@deriving show]

(*****************************************************************************)
(* Checking that the kind of a class is compatible with its parent
 * For example, a class cannot extend an interface, an interface cannot
 * extend a trait etc ...
 *)
(*****************************************************************************)

let check_extend_kind
    (parent_pos : Pos_or_decl.t)
    (parent_kind : Ast_defs.classish_kind)
    (parent_name : string)
    (child_pos : Pos_or_decl.t)
    (child_kind : Ast_defs.classish_kind)
    (child_name : string) : unit =
  match (parent_kind, child_kind) with
  (* What is allowed *)
  | (Ast_defs.Cclass _, Ast_defs.Cclass _)
  | (Ast_defs.Ctrait, Ast_defs.Ctrait)
  | (Ast_defs.Cinterface, Ast_defs.Cinterface) ->
    ()
  (* enums extend BuiltinEnum under the hood *)
  | (Ast_defs.Cclass k, (Ast_defs.Cenum | Ast_defs.Cenum_class _))
    when Ast_defs.is_abstract k ->
    ()
  | (Ast_defs.Cenum_class _, Ast_defs.Cenum_class _) -> ()
  | ( (Ast_defs.Cenum | Ast_defs.Cenum_class _),
      (Ast_defs.Cenum | Ast_defs.Cenum_class _) ) ->
    Errors.add_typing_error
      Typing_error.(
        primary
        @@ Primary.Wrong_extend_kind
             {
               parent_pos;
               parent_kind;
               parent_name;
               pos =
                 Pos_or_decl.unsafe_to_raw_pos child_pos (* TODO: T87242856 *);
               kind = child_kind;
               name = child_name;
             })
  | _ ->
    (* What is disallowed *)
    Errors.add_typing_error
      Typing_error.(
        primary
        @@ Primary.Wrong_extend_kind
             {
               parent_pos;
               parent_kind;
               parent_name;
               pos =
                 Pos_or_decl.unsafe_to_raw_pos child_pos (* TODO: T87242856 *);
               kind = child_kind;
               name = child_name;
             })

(*****************************************************************************)
(* Functions used retrieve everything implemented in parent classes
 * The return values:
 * env: the new environment
 * parents: the name of all the parents and grand parents of the class this
 *          includes traits.
 * is_complete: true if all the parents live in Hack
 *)
(*****************************************************************************)

let disallow_trait_reuse (env : Decl_env.env) : bool =
  TypecheckerOptions.disallow_trait_reuse (Decl_env.tcopt env)

let report_reused_trait
    (parent_type : Decl_defs.decl_class_type)
    (shallow_class : Shallow_decl_defs.shallow_class)
    trait_name =
  let (pos, class_name) =
    Positioned.unsafe_to_raw_positioned shallow_class.sc_name
  in
  Errors.add_typing_error
    Typing_error.(
      primary
      @@ Primary.Trait_reuse
           {
             parent_pos = parent_type.dc_pos;
             parent_name = parent_type.dc_name;
             pos;
             class_name;
             trait_name;
           })

(**
 * Verifies that a class never reuses the same trait throughout its hierarchy.
 *
 * Since Hack only has single inheritance and we already put up a warning for
 * cyclic class hierarchies, if there is any overlap between our extends and
 * our parents' extends, that overlap must be a trait.
 *
 * This does not hold for interfaces because they have multiple inheritance,
 * but interfaces cannot use traits in the first place.
 *
 * XHP attribute dependencies don't actually pull the trait into the class,
 * so we need to track them totally separately.
 *)
let check_no_duplicate_traits
    (parent_type : Decl_defs.decl_class_type)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (c_extends : SSet.t)
    (full_extends : SSet.t) : unit =
  let class_size = SSet.cardinal c_extends in
  let parents_size = SSet.cardinal parent_type.dc_extends in
  let full_size = SSet.cardinal full_extends in
  if class_size + parents_size > full_size then
    let duplicates = SSet.inter c_extends parent_type.dc_extends in
    SSet.iter (report_reused_trait parent_type shallow_class) duplicates

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are tracked separately but merged into the parents list when
 * typechecking so that the class can access the trait members which are
 * declared as private/protected.
 *)
let add_grand_parents_or_traits
    (no_trait_reuse : bool)
    (parent_pos : Pos_or_decl.t)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (acc : SSet.t * [> `Extends_pass | `Xhp_pass ])
    (parent_type : Decl_defs.decl_class_type) : SSet.t * 'a =
  let (extends, pass) = acc in
  let class_pos = fst shallow_class.sc_name in
  let classish_kind = shallow_class.sc_kind in
  let class_name = snd shallow_class.sc_name in
  if phys_equal pass `Extends_pass then
    check_extend_kind
      parent_pos
      parent_type.dc_kind
      parent_type.dc_name
      class_pos
      classish_kind
      class_name;

  (* If we are crawling the xhp attribute deps, we need to merge their xhp deps
   * as well *)
  let parent_deps =
    if phys_equal pass `Xhp_pass then
      SSet.union parent_type.dc_extends parent_type.dc_xhp_attr_deps
    else
      parent_type.dc_extends
  in
  let extends' = SSet.union extends parent_deps in
  (* Verify that merging the parent's extends did not introduce trait reuse *)
  if no_trait_reuse then
    check_no_duplicate_traits parent_type shallow_class extends extends';
  (extends', pass)

let get_class_parent_or_trait
    (env : Decl_env.env)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (parent_cache : Decl_store.class_entries SMap.t)
    ((parents, pass) : SSet.t * [> `Extends_pass | `Xhp_pass ])
    (ty : Typing_defs.decl_phase Typing_defs.ty) : SSet.t * 'a =
  (* See comment on check_no_duplicate_traits for reasoning here *)
  let no_trait_reuse =
    disallow_trait_reuse env
    && (not (phys_equal pass `Xhp_pass))
    && not Ast_defs.(is_c_interface shallow_class.sc_kind)
  in
  let (_, (parent_pos, parent), _) = Decl_utils.unwrap_class_type ty in
  (* If we already had this exact trait, we need to flag trait reuse *)
  let reused_trait = no_trait_reuse && SSet.mem parent parents in
  let parents = SSet.add parent parents in
  let parent_type =
    Decl_env.get_class_and_add_dep
      ~cache:parent_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      parent
  in
  match parent_type with
  | None -> (parents, pass)
  | Some parent_type ->
    (* The parent class lives in Hack, so we can report reused traits *)
    if reused_trait then report_reused_trait parent_type shallow_class parent;
    let acc = (parents, pass) in
    add_grand_parents_or_traits
      no_trait_reuse
      parent_pos
      shallow_class
      acc
      parent_type

let get_class_parents_and_traits
    (env : Decl_env.env)
    (shallow_class : Shallow_decl_defs.shallow_class)
    parent_cache : SSet.t * SSet.t =
  let parents = SSet.empty in
  (* extends parents *)
  let acc = (parents, `Extends_pass) in
  let (parents, _) =
    List.fold_left
      shallow_class.sc_extends
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (* traits *)
  let acc = (parents, `Traits_pass) in
  let (parents, _) =
    List.fold_left
      shallow_class.sc_uses
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = (SSet.empty, `Xhp_pass) in
  let (xhp_parents, _) =
    List.fold_left
      shallow_class.sc_xhp_attr_uses
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (parents, xhp_parents)

type class_env = {
  ctx: Provider_context.t;
  stack: SSet.t;
}

let check_if_cyclic (class_env : class_env) ((pos, cid) : Pos.t * string) : bool
    =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic then
    Errors.add_typing_error
      Typing_error.(primary @@ Primary.Cyclic_class_def { stack; pos });
  is_cyclic

let class_is_abstract (c : Shallow_decl_defs.shallow_class) : bool =
  match c.sc_kind with
  | Ast_defs.Cclass k -> Ast_defs.is_abstract k
  | Ast_defs.Cenum_class k -> Ast_defs.is_abstract k
  | Ast_defs.Cinterface
  | Ast_defs.Ctrait
  | Ast_defs.Cenum ->
    true

let synthesize_const_defaults c =
  let open Typing_defs in
  match c.cc_abstract with
  | CCAbstract true -> { c with cc_abstract = CCConcrete }
  | _ -> c

(* When all type constants have been inherited and declared, this step synthesizes
 * the defaults of abstract type constants into concrete type constants. *)
let synthesize_typeconst_defaults
    (k : string)
    (tc : Typing_defs.typeconst_type)
    ((typeconsts, consts) :
      Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t) :
    Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t =
  match tc.ttc_kind with
  | TCAbstract { atc_default = Some default; _ } ->
    let concrete =
      {
        tc with
        ttc_kind = TCConcrete { tc_type = default };
        ttc_concretized = true;
      }
    in
    let typeconsts = SMap.add k concrete typeconsts in
    (* OCaml 4.06 has an update method that makes this operation much more ergonomic *)
    let constant = SMap.find_opt k consts in
    let consts =
      Option.value_map constant ~default:consts ~f:(fun c ->
          SMap.add k { c with cc_abstract = CCConcrete } consts)
    in
    (typeconsts, consts)
  | _ -> (typeconsts, consts)

let get_sealed_whitelist (c : Shallow_decl_defs.shallow_class) : SSet.t option =
  match Attributes.find SN.UserAttributes.uaSealed c.sc_user_attributes with
  | None -> None
  | Some { ua_classname_params; _ } -> Some (SSet.of_list ua_classname_params)

let get_implements (env : Decl_env.env) parent_cache (ht : Typing_defs.decl_ty)
    : Typing_defs.decl_ty SMap.t =
  let (_r, (_p, c), paraml) = Decl_utils.unwrap_class_type ht in
  let class_ =
    Decl_env.get_class_and_add_dep
      ~cache:parent_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      c
  in
  match class_ with
  | None ->
    (* The class lives in PHP land *)
    SMap.singleton c ht
  | Some class_ ->
    let subst = Inst.make_subst class_.dc_tparams paraml in
    let sub_implements =
      SMap.map (fun ty -> Inst.instantiate subst ty) class_.dc_ancestors
    in
    SMap.add c ht sub_implements

let visibility
    (class_id : string)
    (module_ : Ast_defs.id option)
    (visibility : Aast_defs.visibility) : Typing_defs.ce_visibility =
  match visibility with
  | Public -> Vpublic
  | Protected -> Vprotected class_id
  | Private -> Vprivate class_id
  | Internal ->
    (match module_ with
    | Some m -> Vinternal m
    | None -> Vpublic)

let build_constructor_fun_elt
    ~(write_shmem : bool)
    ~(elt_origin : string)
    ~(method_ : Shallow_decl_defs.shallow_method) =
  let pos = fst method_.sm_name in
  let fe =
    {
      fe_module = None;
      fe_pos = pos;
      fe_internal = false;
      fe_deprecated = method_.sm_deprecated;
      fe_type = method_.sm_type;
      fe_php_std_lib = false;
      fe_support_dynamic_type = false;
    }
  in
  (if write_shmem then Decl_store.((get ()).add_constructor elt_origin fe));
  fe

let build_constructor
    ~(write_shmem : bool)
    ~(origin_class : Shallow_decl_defs.shallow_class)
    (method_ : Shallow_decl_defs.shallow_method) :
    (Decl_defs.element * Typing_defs.fun_elt option) option =
  let (_, class_name) = origin_class.sc_name in
  let vis =
    visibility class_name origin_class.sc_module method_.sm_visibility
  in
  let cstr =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:None
          ~final:(sm_final method_)
          ~abstract:(sm_abstract method_)
          ~lateinit:false
          ~const:false
          ~lsb:false
          ~synthesized:false
          ~superfluous_override:false
          ~dynamicallycallable:false
          ~readonly_prop:false
          ~support_dynamic_type:false
          ~needs_init:false;
      elt_visibility = vis;
      elt_origin = class_name;
      elt_deprecated = method_.sm_deprecated;
    }
  in
  let fe =
    build_constructor_fun_elt ~write_shmem ~elt_origin:class_name ~method_
  in
  Some (cstr, Some fe)

let constructor_decl_eager
    ~(sh : SharedMem.uses)
    ((parent_cstr, pconsist) :
      (Decl_defs.element * Typing_defs.fun_elt option) option
      * Typing_defs.consistent_kind)
    (class_ : Shallow_decl_defs.shallow_class) :
    (Decl_defs.element * Typing_defs.fun_elt option) option
    * Typing_defs.consistent_kind =
  let SharedMem.Uses = sh in
  (* constructors in children of class_ must be consistent? *)
  let cconsist =
    if class_.sc_final then
      FinalClass
    else if
      Attrs.mem
        SN.UserAttributes.uaConsistentConstruct
        class_.sc_user_attributes
    then
      ConsistentConstruct
    else
      Inconsistent
  in
  let cstr =
    match class_.sc_constructor with
    | None -> parent_cstr
    | Some method_ ->
      build_constructor ~write_shmem:true ~origin_class:class_ method_
  in
  (cstr, Decl_utils.coalesce_consistent pconsist cconsist)

let constructor_decl_lazy
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) ~(elt_origin : string) :
    (Typing_defs.fun_elt, lazy_member_lookup_error) result =
  let SharedMem.Uses = sh in
  match Shallow_classes_provider.get ctx elt_origin with
  | None -> Error LMLEShallowClassNotFound
  | Some class_ ->
    (match class_.sc_constructor with
    | None -> Error LMLEMemberNotFound
    | Some method_ ->
      Ok (build_constructor_fun_elt ~write_shmem:true ~elt_origin ~method_))

let class_const_fold
    (c : Shallow_decl_defs.shallow_class)
    (acc : Typing_defs.class_const SMap.t)
    (scc : Shallow_decl_defs.shallow_class_const) :
    Typing_defs.class_const SMap.t =
  let c_name = snd c.sc_name in
  let cc =
    {
      cc_synthesized = false;
      cc_abstract = scc.scc_abstract;
      cc_pos = fst scc.scc_name;
      cc_type = scc.scc_type;
      cc_origin = c_name;
      cc_refs = scc.scc_refs;
    }
  in
  let acc = SMap.add (snd scc.scc_name) cc acc in
  acc

(* Every class, interface, and trait implicitly defines a ::class to
 * allow accessing its fully qualified name as a string *)
let class_class_decl (class_id : Typing_defs.pos_id) : Typing_defs.class_const =
  let (pos, name) = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  let classname_ty =
    mk (reason, Tapply ((pos, SN.Classes.cClassname), [mk (reason, Tthis)]))
  in
  {
    cc_abstract = CCConcrete;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = classname_ty;
    cc_origin = name;
    cc_refs = [];
  }

let prop_decl
    ~(write_shmem : bool)
    (c : Shallow_decl_defs.shallow_class)
    (acc : (Decl_defs.element * Typing_defs.decl_ty option) SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) :
    (Decl_defs.element * Typing_defs.decl_ty option) SMap.t =
  let (sp_pos, sp_name) = sp.sp_name in
  let ty =
    match sp.sp_type with
    | None -> mk (Reason.Rwitness_from_decl sp_pos, Typing_defs.make_tany ())
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) c.sc_module sp.sp_visibility in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:sp.sp_xhp_attr
          ~final:true
          ~lsb:false
          ~synthesized:false
          ~superfluous_override:false
          ~const:(sp_const sp)
          ~lateinit:(sp_lateinit sp)
          ~abstract:(sp_abstract sp)
          ~dynamicallycallable:false
          ~readonly_prop:(sp_readonly sp)
          ~support_dynamic_type:false
          ~needs_init:(sp_needs_init sp);
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_deprecated = None;
    }
  in
  (if write_shmem then
    Decl_store.((get ()).add_prop (elt.elt_origin, sp_name) ty));
  let acc = SMap.add sp_name (elt, Some ty) acc in
  acc

let static_prop_decl
    ~(write_shmem : bool)
    (c : Shallow_decl_defs.shallow_class)
    (acc : (Decl_defs.element * Typing_defs.decl_ty option) SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) :
    (Decl_defs.element * Typing_defs.decl_ty option) SMap.t =
  let (sp_pos, sp_name) = sp.sp_name in
  let ty =
    match sp.sp_type with
    | None -> mk (Reason.Rwitness_from_decl sp_pos, Typing_defs.make_tany ())
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) c.sc_module sp.sp_visibility in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:sp.sp_xhp_attr
          ~final:true
          ~const:(sp_const sp)
          ~lateinit:(sp_lateinit sp)
          ~lsb:(sp_lsb sp)
          ~superfluous_override:false
          ~abstract:(sp_abstract sp)
          ~synthesized:false
          ~dynamicallycallable:false
          ~readonly_prop:(sp_readonly sp)
          ~support_dynamic_type:false
          ~needs_init:false;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_deprecated = None;
    }
  in
  (if write_shmem then
    Decl_store.((get ()).add_static_prop (elt.elt_origin, sp_name) ty));
  let acc = SMap.add sp_name (elt, Some ty) acc in
  acc

(* each concrete type constant T = <sometype> implicitly defines a
class constant with the same name which is TypeStructure<sometype> *)
let typeconst_structure
    (c : Shallow_decl_defs.shallow_class)
    (stc : Shallow_decl_defs.shallow_typeconst) : Typing_defs.class_const =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness_from_decl pos in
  let tsid = (pos, SN.FB.cTypeStructure) in
  let ts_ty =
    mk (r, Tapply (tsid, [mk (r, Taccess (mk (r, Tthis), stc.stc_name))]))
  in
  let abstract =
    match stc.stc_kind with
    | TCAbstract { atc_default = default; _ } ->
      CCAbstract (Option.is_some default)
    | _ -> CCConcrete
  in
  {
    cc_abstract = abstract;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = ts_ty;
    cc_origin = snd c.sc_name;
    cc_refs = [];
  }

let typeconst_fold
    (c : Shallow_decl_defs.shallow_class)
    (acc : Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t)
    (stc : Shallow_decl_defs.shallow_typeconst) :
    Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t =
  let (typeconsts, consts) = acc in
  match c.sc_kind with
  | Ast_defs.Cenum_class _
  | Ast_defs.Cenum ->
    acc
  | Ast_defs.Ctrait
  | Ast_defs.Cinterface
  | Ast_defs.Cclass _ ->
    let name = snd stc.stc_name in
    let c_name = snd c.sc_name in
    let ts = typeconst_structure c stc in
    let consts = SMap.add name ts consts in
    let ptc_opt = SMap.find_opt name typeconsts in
    let enforceable =
      (* Without the positions, this is a simple OR, but this way allows us to
       * report the position of the <<__Enforceable>> attribute to the user *)
      if snd stc.stc_enforceable then
        stc.stc_enforceable
      else
        match ptc_opt with
        | Some ptc -> ptc.ttc_enforceable
        | None -> (Pos_or_decl.none, false)
    in
    let reifiable =
      if Option.is_some stc.stc_reifiable then
        stc.stc_reifiable
      else
        Option.bind ptc_opt ~f:(fun ptc -> ptc.ttc_reifiable)
    in
    let tc =
      {
        ttc_synthesized = false;
        ttc_name = stc.stc_name;
        ttc_kind = stc.stc_kind;
        ttc_origin = c_name;
        ttc_enforceable = enforceable;
        ttc_reifiable = reifiable;
        ttc_concretized = false;
        ttc_is_ctx = stc.stc_is_ctx;
      }
    in
    let typeconsts = SMap.add (snd stc.stc_name) tc typeconsts in
    (typeconsts, consts)

let method_decl_acc
    ~(write_shmem : bool)
    ~(is_static : bool)
    (c : Shallow_decl_defs.shallow_class)
    ((acc, condition_types) :
      (Decl_defs.element * Typing_defs.fun_elt option) SMap.t * SSet.t)
    (m : Shallow_decl_defs.shallow_method) :
    (Decl_defs.element * Typing_defs.fun_elt option) SMap.t * SSet.t =
  (* If method doesn't override anything but has the <<__Override>> attribute, then
   * set the override flag in ce_flags and let typing emit an appropriate error *)
  let superfluous_override =
    sm_override m && not (SMap.mem (snd m.sm_name) acc)
  in
  let (pos, id) = m.sm_name in
  let vis =
    match (SMap.find_opt id acc, m.sm_visibility) with
    | (Some ({ elt_visibility = Vprotected _ as parent_vis; _ }, _), Protected)
      ->
      parent_vis
    | _ -> visibility (snd c.sc_name) c.sc_module m.sm_visibility
  in
  let support_dynamic_type = sm_support_dynamic_type m in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:None
          ~final:(sm_final m)
          ~abstract:(sm_abstract m)
          ~superfluous_override
          ~synthesized:false
          ~lsb:false
          ~const:false
          ~lateinit:false
          ~dynamicallycallable:(sm_dynamicallycallable m)
          ~readonly_prop:false
          ~support_dynamic_type
          ~needs_init:false;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_deprecated = m.sm_deprecated;
    }
  in
  let fe =
    {
      fe_module = None;
      fe_pos = pos;
      fe_internal = false;
      fe_deprecated = None;
      fe_type = m.sm_type;
      fe_php_std_lib = false;
      fe_support_dynamic_type = support_dynamic_type;
    }
  in
  (if write_shmem then
    if is_static then
      Decl_store.((get ()).add_static_method (elt.elt_origin, id) fe)
    else
      Decl_store.((get ()).add_method (elt.elt_origin, id) fe));
  let acc = SMap.add id (elt, Some fe) acc in
  (acc, condition_types)

let rec declare_class_and_parents
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (shallow_class : Shallow_decl_defs.shallow_class) : Decl_store.class_entries
    =
  let (_, name) = shallow_class.sc_name in
  let class_env = { class_env with stack = SSet.add name class_env.stack } in
  let (errors, (tc, member_heaps_values)) =
    Errors.do_ (fun () ->
        let parents = class_parents_decl ~sh class_env shallow_class in
        class_decl ~sh class_env.ctx shallow_class ~parents)
  in
  let class_ = { tc with dc_decl_errors = Some errors } in
  (class_, Some member_heaps_values)

and class_parents_decl
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (c : Shallow_decl_defs.shallow_class) : Decl_store.class_entries SMap.t =
  let class_type_decl acc class_ty =
    match get_node class_ty with
    | Tapply ((pos, class_name), _) ->
      if
        check_if_cyclic class_env (Pos_or_decl.unsafe_to_raw_pos pos, class_name)
      then
        acc
      else (
        match class_decl_if_missing ~sh class_env class_name with
        | None -> acc
        | Some decls -> SMap.add class_name decls acc
      )
    | _ -> acc
  in
  let acc = SMap.empty in
  let acc = List.fold c.sc_extends ~f:class_type_decl ~init:acc in
  let acc = List.fold c.sc_implements ~f:class_type_decl ~init:acc in
  let acc = List.fold c.sc_uses ~f:class_type_decl ~init:acc in
  let acc = List.fold c.sc_xhp_attr_uses ~f:class_type_decl ~init:acc in
  let acc = List.fold c.sc_req_extends ~f:class_type_decl ~init:acc in
  let acc = List.fold c.sc_req_implements ~f:class_type_decl ~init:acc in
  let enum_includes =
    Aast.enum_includes_map ~f:(fun et -> et.te_includes) c.sc_enum_type
  in
  let acc = List.fold enum_includes ~f:class_type_decl ~init:acc in
  acc

and class_decl_if_missing
    ~(sh : SharedMem.uses) (class_env : class_env) (class_name : string) :
    Decl_store.class_entries option =
  match Decl_store.((get ()).get_class class_name) with
  | Some decl -> Some (decl, None)
  | None ->
    (match Shallow_classes_provider.get class_env.ctx class_name with
    | None -> None
    | Some shallow_class ->
      let fn =
        Pos.filename (fst shallow_class.sc_name |> Pos_or_decl.unsafe_to_raw_pos)
      in
      Errors.run_in_context fn Errors.Decl @@ fun () ->
      let ((class_, _) as result) =
        declare_class_and_parents ~sh class_env shallow_class
      in
      Decl_store.((get ()).add_class class_name class_);
      Some result)

and class_decl
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class)
    ~(parents : Decl_store.class_entries SMap.t) :
    Decl_defs.decl_class_type * Decl_store.class_members =
  let is_abstract = class_is_abstract c in
  let const = Attrs.mem SN.UserAttributes.uaConst c.sc_user_attributes in
  let internal = Attrs.mem SN.UserAttributes.uaInternal c.sc_user_attributes in
  let (_p, cls_name) = c.sc_name in
  let class_dep = Dep.Type cls_name in
  let env = { Decl_env.mode = c.sc_mode; droot = Some class_dep; ctx } in
  let inherited = Decl_inherit.make env c ~cache:parents in
  let props = inherited.Decl_inherit.ih_props in
  let props =
    List.fold_left ~f:(prop_decl ~write_shmem:true c) ~init:props c.sc_props
  in
  let inherited_methods = inherited.Decl_inherit.ih_methods in
  let (methods, condition_types) =
    List.fold_left
      ~f:(method_decl_acc ~write_shmem:true ~is_static:false c)
      ~init:(inherited_methods, SSet.empty)
      c.sc_methods
  in
  let consts = inherited.Decl_inherit.ih_consts in
  let consts =
    List.fold_left ~f:(class_const_fold c) ~init:consts c.sc_consts
  in
  let consts = SMap.add SN.Members.mClass (class_class_decl c.sc_name) consts in
  let typeconsts = inherited.Decl_inherit.ih_typeconsts in
  let (typeconsts, consts) =
    List.fold_left
      c.sc_typeconsts
      ~f:(typeconst_fold c)
      ~init:(typeconsts, consts)
  in
  let (typeconsts, consts) =
    if Ast_defs.is_c_normal c.sc_kind then
      let consts = SMap.map synthesize_const_defaults consts in
      SMap.fold synthesize_typeconst_defaults typeconsts (typeconsts, consts)
    else
      (typeconsts, consts)
  in
  let sclass_var = static_prop_decl ~write_shmem:true c in
  let inherited_static_props = inherited.Decl_inherit.ih_sprops in
  let static_props =
    List.fold_left c.sc_sprops ~f:sclass_var ~init:inherited_static_props
  in
  let inherited_static_methods = inherited.Decl_inherit.ih_smethods in
  let (static_methods, condition_types) =
    List.fold_left
      c.sc_static_methods
      ~f:(method_decl_acc ~write_shmem:true ~is_static:true c)
      ~init:(inherited_static_methods, condition_types)
  in
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl_eager ~sh parent_cstr c in
  let has_concrete_cstr =
    match fst cstr with
    | None -> false
    | Some (elt, _) when get_elt_abstract elt -> false
    | _ -> true
  in
  let impl = c.sc_extends @ c.sc_implements @ c.sc_uses in
  let (impl, parents) =
    match
      List.find c.sc_methods ~f:(fun sm ->
          String.equal (snd sm.sm_name) SN.Members.__toString)
    with
    | Some { sm_name = (pos, _); _ }
      when String.( <> ) cls_name SN.Classes.cStringishObject ->
      (* HHVM implicitly adds StringishObject interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      (* Declare StringishObject and parents if not already declared *)
      let class_env = { ctx; stack = SSet.empty } in
      let parents =
        (* Ensure stringishObject is declared. *)
        match
          class_decl_if_missing ~sh class_env SN.Classes.cStringishObject
        with
        | None -> parents
        | Some stringish_cls ->
          SMap.add SN.Classes.cStringishObject stringish_cls parents
      in
      let ty =
        mk (Reason.Rhint pos, Tapply ((pos, SN.Classes.cStringishObject), []))
      in
      (ty :: impl, parents)
    | _ -> (impl, parents)
  in
  let impl = List.map impl ~f:(get_implements env parents) in
  let impl = List.fold_right impl ~f:(SMap.fold SMap.add) ~init:SMap.empty in
  let (extends, xhp_attr_deps) = get_class_parents_and_traits env c parents in
  let (req_ancestors, req_ancestors_extends) =
    Decl_requirements.get_class_requirements env parents c
  in
  let enum = c.sc_enum_type in
  let enum_inner_ty = SMap.find_opt SN.FB.tInner typeconsts in
  let is_enum_class = Ast_defs.is_c_enum_class c.sc_kind in
  let consts =
    Decl_enum.rewrite_class
      c.sc_name
      ~is_enum_class
      enum
      Option.(
        enum_inner_ty >>= fun t ->
        (* TODO(T88552052) can make logic more explicit now, enum members appear to
         * only need abstract without default and concrete type consts *)
        match t.ttc_kind with
        | TCConcrete { tc_type } -> Some tc_type
        | TCAbstract { atc_default; _ } -> atc_default)
      ~get_ancestor:(fun x -> SMap.find_opt x impl)
      consts
  in
  let has_own_cstr = has_concrete_cstr && Option.is_some c.sc_constructor in
  let deferred_members =
    let get_class_add_dep decl_env cls =
      Decl_env.get_class_and_add_dep
        ~cache:parents
        ~shmem_fallback:false
        ~fallback:Decl_env.no_fallback
        decl_env
        cls
    in
    Decl_init_check.nonprivate_deferred_init_props
      ~has_own_cstr
      ~get_class_add_dep
      env
      c
  in
  let sealed_whitelist = get_sealed_whitelist c in
  let tc =
    {
      dc_final = c.sc_final;
      dc_const = const;
      dc_internal = internal;
      dc_abstract = is_abstract;
      dc_need_init = has_concrete_cstr;
      dc_deferred_init_members = deferred_members;
      dc_kind = c.sc_kind;
      dc_is_xhp = c.sc_is_xhp;
      dc_has_xhp_keyword = c.sc_has_xhp_keyword;
      dc_module = c.sc_module;
      dc_name = snd c.sc_name;
      dc_pos = fst c.sc_name;
      dc_tparams = c.sc_tparams;
      dc_where_constraints = c.sc_where_constraints;
      dc_substs = inherited.Decl_inherit.ih_substs;
      dc_consts = consts;
      dc_typeconsts = typeconsts;
      dc_props = SMap.map fst props;
      dc_sprops = SMap.map fst static_props;
      dc_methods = SMap.map fst methods;
      dc_smethods = SMap.map fst static_methods;
      dc_construct = Tuple.T2.map_fst ~f:(Option.map ~f:fst) cstr;
      dc_ancestors = impl;
      dc_support_dynamic_type = c.sc_support_dynamic_type;
      dc_extends = extends;
      dc_sealed_whitelist = sealed_whitelist;
      dc_xhp_attr_deps = xhp_attr_deps;
      dc_xhp_enum_values = c.sc_xhp_enum_values;
      dc_req_ancestors = req_ancestors;
      dc_req_ancestors_extends = req_ancestors_extends;
      dc_enum_type = enum;
      dc_decl_errors = None;
      dc_condition_types = condition_types;
    }
  in
  let member_heaps_values =
    {
      Decl_store.m_static_properties = SMap.filter_map snd static_props;
      m_properties = SMap.filter_map snd props;
      m_static_methods = SMap.filter_map snd static_methods;
      m_methods = SMap.filter_map snd methods;
      m_constructor = Option.(cstr |> fst >>= snd);
    }
  in
  SMap.iter
    begin
      fun x _ ->
      Typing_deps.add_idep
        (Provider_context.get_deps_mode ctx)
        (Dep.Type cls_name)
        (Dep.Type x)
    end
    impl;
  (tc, member_heaps_values)

let class_decl_if_missing
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (class_name : string) :
    Decl_store.class_entries option =
  match Decl_store.((get ()).get_class class_name) with
  | Some class_ -> Some (class_, None)
  | None ->
    (* Class elements are in memory if and only if the class itself is there.
     * Exiting before class declaration is ready would break this invariant *)
    WorkerCancel.with_no_cancellations @@ fun () ->
    class_decl_if_missing ~sh { ctx; stack = SSet.empty } class_name
