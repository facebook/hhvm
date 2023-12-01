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
    (child_name : string) : decl_error option =
  match (parent_kind, child_kind) with
  (* What is allowed *)
  | (Ast_defs.Cclass _, Ast_defs.Cclass _)
  | (Ast_defs.Ctrait, Ast_defs.Ctrait)
  | (Ast_defs.Cinterface, Ast_defs.Cinterface) ->
    None
  (* enums extend BuiltinEnum under the hood *)
  | (Ast_defs.Cclass k, (Ast_defs.Cenum | Ast_defs.Cenum_class _))
    when Ast_defs.is_abstract k ->
    None
  | (Ast_defs.Cenum_class _, Ast_defs.Cenum_class _) -> None
  | _ ->
    (* What is disallowed *)
    Some
      (Wrong_extend_kind
         {
           parent_pos;
           parent_kind;
           parent_name;
           pos = Pos_or_decl.unsafe_to_raw_pos child_pos (* TODO: T87242856 *);
           kind = child_kind;
           name = child_name;
         })

let check_use_kind
    (parent_pos : Pos_or_decl.t)
    (parent_kind : Ast_defs.classish_kind)
    (parent_name : string)
    (parent_is_module_level_trait : bool)
    (child_pos : Pos_or_decl.t)
    (child_kind : Ast_defs.classish_kind)
    (child_name : string)
    (child_is_module_level_trait : bool) : decl_error option =
  match (parent_kind, child_kind) with
  | (Ast_defs.Ctrait, Ast_defs.Ctrait)
    when child_is_module_level_trait && not parent_is_module_level_trait ->
    Some
      (Wrong_use_kind
         {
           parent_pos;
           parent_name;
           pos = Pos_or_decl.unsafe_to_raw_pos child_pos;
           name = child_name;
         })
  | _ -> None

(*****************************************************************************)
(* Functions used retrieve everything implemented in parent classes
 * The return values:
 * env: the new environment
 * parents: the name of all the parents and grand parents of the class this
 *          includes traits.
 * is_complete: true if all the parents live in Hack
 *)
(*****************************************************************************)

let member_heaps_enabled (ctx : Provider_context.t) : bool =
  let tco = Provider_context.get_tcopt ctx in
  TypecheckerOptions.(populate_member_heaps tco)

(**
 * Adds the traits/classes which are part of a class' hierarchy.
 *
 * Traits are tracked separately but merged into the parents list when
 * typechecking so that the class can access the trait members which are
 * declared as private/protected.
 *)
let add_grand_parents_or_traits
    (parent_pos : Pos_or_decl.t)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (acc :
      SSet.t * [ `Extends_pass | `Traits_pass | `Xhp_pass ] * decl_error list)
    (parent_type : Decl_defs.decl_class_type) : SSet.t * 'a * decl_error list =
  let (extends, pass, decl_errors) = acc in
  let class_pos = fst shallow_class.sc_name in
  let classish_kind = shallow_class.sc_kind in
  let class_name = snd shallow_class.sc_name in
  let class_is_module_level_trait =
    Attributes.mem
      SN.UserAttributes.uaModuleLevelTrait
      shallow_class.sc_user_attributes
  in
  let decl_errors =
    match pass with
    | `Extends_pass ->
      (match
         check_extend_kind
           parent_pos
           parent_type.dc_kind
           parent_type.dc_name
           class_pos
           classish_kind
           class_name
       with
      | Some err -> err :: decl_errors
      | None -> decl_errors)
    | `Traits_pass ->
      (match
         check_use_kind
           parent_pos
           parent_type.dc_kind
           parent_type.dc_name
           parent_type.dc_is_module_level_trait
           class_pos
           classish_kind
           class_name
           class_is_module_level_trait
       with
      | Some err -> err :: decl_errors
      | None -> decl_errors)
    | `Xhp_pass -> decl_errors
  in

  (* If we are crawling the xhp attribute deps, we need to merge their xhp deps
   * as well *)
  let parent_deps =
    if phys_equal pass `Xhp_pass then
      SSet.union parent_type.dc_extends parent_type.dc_xhp_attr_deps
    else
      parent_type.dc_extends
  in
  let extends' = SSet.union extends parent_deps in
  (extends', pass, decl_errors)

let get_class_parent_or_trait
    (env : Decl_env.env)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (parent_cache : Decl_store.class_entries SMap.t)
    ((parents, pass, decl_errors) :
      SSet.t * [ `Extends_pass | `Traits_pass | `Xhp_pass ] * decl_error list)
    (ty : Typing_defs.decl_phase Typing_defs.ty) : SSet.t * 'a * decl_error list
    =
  let (_, (parent_pos, parent), _) = Decl_utils.unwrap_class_type ty in
  (* If we already had this exact trait, we need to flag trait reuse *)
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
  | None -> (parents, pass, decl_errors)
  | Some parent_type ->
    let acc = (parents, pass, decl_errors) in
    add_grand_parents_or_traits parent_pos shallow_class acc parent_type

let get_class_parents_and_traits
    (env : Decl_env.env)
    (shallow_class : Shallow_decl_defs.shallow_class)
    parent_cache
    decl_errors : SSet.t * SSet.t * decl_error list =
  let parents = SSet.empty in
  (* extends parents *)
  let acc = (parents, `Extends_pass, decl_errors) in
  let (parents, _, decl_errors) =
    List.fold_left
      shallow_class.sc_extends
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (* traits *)
  let acc = (parents, `Traits_pass, decl_errors) in
  let (parents, _, decl_errors) =
    List.fold_left
      shallow_class.sc_uses
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = (SSet.empty, `Xhp_pass, decl_errors) in
  let (xhp_parents, _, decl_errors) =
    List.fold_left
      shallow_class.sc_xhp_attr_uses
      ~f:(get_class_parent_or_trait env shallow_class parent_cache)
      ~init:acc
  in
  (parents, xhp_parents, decl_errors)

type class_env = {
  ctx: Provider_context.t;
  stack: SSet.t;
}

let check_if_cyclic (class_env : class_env) ((pos, cid) : Pos.t * string) :
    decl_error option =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic then
    Some (Cyclic_class_def { stack; pos })
  else
    None

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
  | Some { ua_params; _ } ->
    let cn_params =
      List.filter_map
        ~f:(function
          | Classname cn -> Some cn
          | _ -> None)
        ua_params
    in
    Some (SSet.of_list cn_params)

let get_instantiated_ancestors_and_self
    (env : Decl_env.env) parent_cache (ht : Typing_defs.decl_ty) :
    Typing_defs.decl_ty SMap.t =
  let (_r, (_p, class_name), paraml) = Decl_utils.unwrap_class_type ht in
  let class_ =
    Decl_env.get_class_and_add_dep
      ~cache:parent_cache
      ~shmem_fallback:false
      ~fallback:Decl_env.no_fallback
      env
      class_name
  in
  match class_ with
  | None ->
    (* The class lives in PHP land *)
    SMap.singleton class_name ht
  | Some class_ ->
    let subst = Inst.make_subst class_.dc_tparams paraml in
    let instantiated_ancestors =
      SMap.map (fun ty -> Inst.instantiate subst ty) class_.dc_ancestors
    in
    SMap.add class_name ht instantiated_ancestors

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
    | Some m -> Vinternal (snd m)
    | None -> Vpublic)

let build_constructor_fun_elt
    ~(ctx : Provider_context.t)
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
      fe_no_auto_dynamic = false;
      fe_no_auto_likes = false;
    }
  in
  (if member_heaps_enabled ctx then
    Decl_store.((get ()).add_constructor elt_origin fe));
  fe

let build_constructor
    ~(ctx : Provider_context.t)
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
          ~needs_init:false
          ~safe_global_variable:false;
      elt_visibility = vis;
      elt_origin = class_name;
      elt_deprecated = method_.sm_deprecated;
      elt_sort_text = method_.sm_sort_text;
    }
  in
  let fe = build_constructor_fun_elt ~ctx ~elt_origin:class_name ~method_ in
  Some (cstr, Some fe)

let constructor_decl_eager
    ~(sh : SharedMem.uses)
    ~(ctx : Provider_context.t)
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
    | Some method_ -> build_constructor ~ctx ~origin_class:class_ method_
  in
  (cstr, Decl_utils.coalesce_consistent pconsist cconsist)

let constructor_decl_lazy
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) ~(elt_origin : string) :
    (Typing_defs.fun_elt, lazy_member_lookup_error) result =
  let SharedMem.Uses = sh in
  match Decl_provider_internals.get_shallow_class ctx elt_origin with
  | None -> Error LMLEShallowClassNotFound
  | Some class_ ->
    (match class_.sc_constructor with
    | None -> Error LMLEMemberNotFound
    | Some method_ -> Ok (build_constructor_fun_elt ~ctx ~elt_origin ~method_))

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

let build_prop_sprop_ty
    ~(ctx : Provider_context.t)
    ~(this_class : shallow_class option)
    ~(is_static : bool)
    ~(elt_origin : string)
    (sp : Shallow_decl_defs.shallow_prop) : Typing_defs.decl_ty =
  let (_sp_pos, sp_name) = sp.sp_name in
  let is_xhp_attr = is_some sp.sp_xhp_attr in
  let no_auto_likes = PropFlags.get_no_auto_likes sp.sp_flags in
  let ty =
    if no_auto_likes then
      sp.sp_type
    else
      Decl_enforceability.maybe_pessimise_type
        ~reason:(Reason.Rpessimised_prop (get_pos sp.sp_type))
        ~is_xhp_attr
        ~this_class
        ctx
        sp.sp_type
  in
  (if member_heaps_enabled ctx then
    if is_static then
      Decl_store.((get ()).add_static_prop (elt_origin, sp_name) ty)
    else
      Decl_store.((get ()).add_prop (elt_origin, sp_name) ty));
  ty

let prop_decl_eager
    ~(ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class)
    (acc : (Decl_defs.element * Typing_defs.decl_ty option) SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) :
    (Decl_defs.element * Typing_defs.decl_ty option) SMap.t =
  let elt_origin = snd c.sc_name in
  let ty =
    build_prop_sprop_ty
      ~ctx
      ~this_class:(Some c)
      ~is_static:false
      ~elt_origin
      sp
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
          ~needs_init:(sp_needs_init sp)
          ~safe_global_variable:false;
      elt_visibility = vis;
      elt_origin;
      elt_deprecated = None;
      elt_sort_text = None;
    }
  in
  let acc = SMap.add (snd sp.sp_name) (elt, Some ty) acc in
  acc

let prop_decl_lazy
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    ~(elt_origin : string)
    ~(sp_name : string) : (Typing_defs.decl_ty, lazy_member_lookup_error) result
    =
  let SharedMem.Uses = sh in
  match Decl_provider_internals.get_shallow_class ctx elt_origin with
  | None -> Error LMLEShallowClassNotFound
  | Some class_ ->
    (match
       List.find class_.sc_props ~f:(fun prop ->
           String.equal (snd prop.sp_name) sp_name)
     with
    | None -> Error LMLEMemberNotFound
    | Some sp ->
      Ok
        (build_prop_sprop_ty
           ~ctx
           ~this_class:(Some class_)
           ~is_static:false
           ~elt_origin
           sp))

let static_prop_decl_eager
    ~(ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class)
    (acc : (Decl_defs.element * Typing_defs.decl_ty option) SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) :
    (Decl_defs.element * Typing_defs.decl_ty option) SMap.t =
  let elt_origin = snd c.sc_name in
  let ty =
    build_prop_sprop_ty ~ctx ~this_class:(Some c) ~is_static:true ~elt_origin sp
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
          ~needs_init:false
          ~safe_global_variable:(sp_safe_global_variable sp);
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_deprecated = None;
      elt_sort_text = None;
    }
  in
  let acc = SMap.add (snd sp.sp_name) (elt, Some ty) acc in
  acc

let static_prop_decl_lazy
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    ~(elt_origin : string)
    ~(sp_name : string) : (Typing_defs.decl_ty, lazy_member_lookup_error) result
    =
  let SharedMem.Uses = sh in
  match Decl_provider_internals.get_shallow_class ctx elt_origin with
  | None -> Error LMLEShallowClassNotFound
  | Some class_ ->
    (match
       List.find class_.sc_sprops ~f:(fun prop ->
           String.equal (snd prop.sp_name) sp_name)
     with
    | None -> Error LMLEMemberNotFound
    | Some sp ->
      Ok
        (build_prop_sprop_ty
           ~ctx
           ~this_class:(Some class_)
           ~is_static:true
           ~elt_origin
           sp))

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
    | TCConcrete _ -> CCConcrete
  in
  {
    cc_abstract = abstract;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = ts_ty;
    cc_origin = snd c.sc_name;
    cc_refs = [];
  }

let maybe_add_supportdyn_bound ctx p kind =
  if TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx) then
    match kind with
    | TCAbstract { atc_as_constraint = None; atc_super_constraint; atc_default }
      ->
      TCAbstract
        {
          atc_as_constraint =
            Some
              (Decl_enforceability.supportdyn_mixed
                 p
                 (Typing_defs.Reason.Rwitness_from_decl p));
          atc_super_constraint;
          atc_default;
        }
    | TCAbstract _
    | TCConcrete _ ->
      kind
  else
    kind

let typeconst_fold
    (ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class)
    (acc : Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t)
    (stc : Shallow_decl_defs.shallow_typeconst) :
    Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t =
  let (typeconsts, consts) = acc in
  match c.sc_kind with
  | Ast_defs.Cenum -> acc
  | Ast_defs.Cenum_class _
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
        ttc_kind =
          (if stc.stc_is_ctx then
            stc.stc_kind
          else
            maybe_add_supportdyn_bound ctx (fst stc.stc_name) stc.stc_kind);
        ttc_origin = c_name;
        ttc_enforceable = enforceable;
        ttc_reifiable = reifiable;
        ttc_concretized = false;
        ttc_is_ctx = stc.stc_is_ctx;
      }
    in
    let typeconsts = SMap.add (snd stc.stc_name) tc typeconsts in
    (typeconsts, consts)

let build_method_fun_elt
    ~(ctx : Provider_context.t)
    ~(this_class : shallow_class option)
    ~(is_static : bool)
    ~(elt_origin : string)
    (m : Shallow_decl_defs.shallow_method) : Typing_defs.fun_elt =
  let (pos, id) = m.sm_name in
  let support_dynamic_type = sm_support_dynamic_type m in
  let fe_no_auto_dynamic =
    Typing_defs.Attributes.mem
      Naming_special_names.UserAttributes.uaNoAutoDynamic
      m.Shallow_decl_defs.sm_attributes
  in
  let fe_no_auto_likes =
    Typing_defs.Attributes.mem
      Naming_special_names.UserAttributes.uaNoAutoLikes
      m.Shallow_decl_defs.sm_attributes
  in
  let fe =
    {
      fe_module = None;
      fe_pos = pos;
      fe_internal = false;
      fe_deprecated = None;
      fe_type =
        (if
         (not fe_no_auto_dynamic)
         && Provider_context.implicit_sdt_for_class ctx this_class
        then
          Decl_enforceability.(
            pessimise_fun_type
              ~fun_kind:
                (if MethodFlags.get_abstract m.sm_flags then
                  Abstract_method
                else
                  Concrete_method)
              ~this_class
              ~no_auto_likes:fe_no_auto_likes
              ctx
              pos
              m.sm_type)
        else
          m.sm_type);
      fe_php_std_lib = false;
      fe_support_dynamic_type = support_dynamic_type;
      fe_no_auto_dynamic;
      fe_no_auto_likes;
    }
  in
  (if member_heaps_enabled ctx then
    if is_static then
      Decl_store.((get ()).add_static_method (elt_origin, id) fe)
    else
      Decl_store.((get ()).add_method (elt_origin, id) fe));
  fe

let method_decl_eager
    ~(ctx : Provider_context.t)
    ~(is_static : bool)
    (c : Shallow_decl_defs.shallow_class)
    (acc : (Decl_defs.element * Typing_defs.fun_elt option) SMap.t)
    (m : Shallow_decl_defs.shallow_method) :
    (Decl_defs.element * Typing_defs.fun_elt option) SMap.t =
  (* If method doesn't override anything but has the <<__Override>> attribute, then
   * set the override flag in ce_flags and let typing emit an appropriate error *)
  let superfluous_override =
    sm_override m && not (SMap.mem (snd m.sm_name) acc)
  in
  let (_pos, id) = m.sm_name in
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
          ~needs_init:false
          ~safe_global_variable:false;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_deprecated = m.sm_deprecated;
      elt_sort_text = m.sm_sort_text;
    }
  in
  let fe =
    build_method_fun_elt
      ~ctx
      ~this_class:(Some c)
      ~is_static
      ~elt_origin:elt.elt_origin
      m
  in
  let acc = SMap.add id (elt, Some fe) acc in
  acc

let method_decl_lazy
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    ~(is_static : bool)
    ~(elt_origin : string)
    ~(sm_name : string) : (Typing_defs.fun_elt, lazy_member_lookup_error) result
    =
  let SharedMem.Uses = sh in
  match Decl_provider_internals.get_shallow_class ctx elt_origin with
  | None -> Error LMLEShallowClassNotFound
  | Some class_ ->
    let methods =
      if is_static then
        class_.sc_static_methods
      else
        class_.sc_methods
    in
    (match
       List.find methods ~f:(fun m -> String.equal (snd m.sm_name) sm_name)
     with
    | None -> Error LMLEMemberNotFound
    | Some sm ->
      Ok
        (build_method_fun_elt
           ~this_class:(Some class_)
           ~ctx
           ~is_static
           ~elt_origin
           sm))

let rec declare_class_and_parents
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (shallow_class : Shallow_decl_defs.shallow_class) : Decl_store.class_entries
    =
  let (_, name) = shallow_class.sc_name in
  let class_env = { class_env with stack = SSet.add name class_env.stack } in
  let (class_, member_heaps_values) =
    let (parents, errors) = class_parents_decl ~sh class_env shallow_class in
    class_decl ~sh class_env.ctx shallow_class ~parents errors
  in
  (class_, Some member_heaps_values)

and class_parents_decl
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (c : Shallow_decl_defs.shallow_class) :
    Decl_store.class_entries SMap.t * decl_error list =
  let class_type_decl (parents, errs) class_ty =
    match get_node class_ty with
    | Tapply ((pos, class_name), _) ->
      (match
         check_if_cyclic
           class_env
           (Pos_or_decl.unsafe_to_raw_pos pos, class_name)
       with
      | Some err -> (parents, err :: errs)
      | None ->
        (match class_decl_if_missing ~sh class_env class_name with
        | None -> (parents, errs)
        | Some decls -> (SMap.add class_name decls parents, errs)))
    | _ -> (parents, errs)
  in
  let acc = (SMap.empty, []) in
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
    (match
       Decl_provider_internals.get_shallow_class class_env.ctx class_name
     with
    | None -> None
    | Some shallow_class ->
      let ((class_, _) as result) =
        declare_class_and_parents ~sh class_env shallow_class
      in
      Decl_store.((get ()).add_class class_name class_);
      Some result)

and class_decl
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class)
    ~(parents : Decl_store.class_entries SMap.t)
    (decl_errors : decl_error list) :
    Decl_defs.decl_class_type * Decl_store.class_members =
  let is_abstract = class_is_abstract c in
  let const = Attrs.mem SN.UserAttributes.uaConst c.sc_user_attributes in
  (* Support both attribute and keyword for now, until typechecker changes are made *)
  let internal = c.sc_internal in
  let (p, cls_name) = c.sc_name in
  let class_dep = Dep.Type cls_name in
  let env =
    {
      Decl_env.mode = c.sc_mode;
      droot = Some class_dep;
      droot_member = None;
      ctx;
    }
  in
  let inherited = Decl_inherit.make env c ~cache:parents in
  let props = inherited.Decl_inherit.ih_props in
  let props =
    List.fold_left ~f:(prop_decl_eager ~ctx c) ~init:props c.sc_props
  in
  let inherited_methods = inherited.Decl_inherit.ih_methods in
  let methods =
    List.fold_left
      ~f:(method_decl_eager ~ctx ~is_static:false c)
      ~init:inherited_methods
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
      ~f:(typeconst_fold ctx c)
      ~init:(typeconsts, consts)
  in
  let (typeconsts, consts) =
    if Ast_defs.is_c_concrete c.sc_kind then
      let consts = SMap.map synthesize_const_defaults consts in
      SMap.fold synthesize_typeconst_defaults typeconsts (typeconsts, consts)
    else
      (typeconsts, consts)
  in
  let sclass_var = static_prop_decl_eager ~ctx c in
  let inherited_static_props = inherited.Decl_inherit.ih_sprops in
  let static_props =
    List.fold_left c.sc_sprops ~f:sclass_var ~init:inherited_static_props
  in
  let inherited_static_methods = inherited.Decl_inherit.ih_smethods in
  let static_methods =
    List.fold_left
      c.sc_static_methods
      ~f:(method_decl_eager ~ctx ~is_static:true c)
      ~init:inherited_static_methods
  in
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl_eager ~sh ~ctx parent_cstr c in
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
  let impl =
    List.map impl ~f:(get_instantiated_ancestors_and_self env parents)
  in
  let impl =
    List.fold
      impl
      ~f:(SMap.union ~combine:(fun _ ty1 _ty2 -> Some ty1))
      ~init:SMap.empty
  in
  let (extends, xhp_attr_deps, decl_errors) =
    get_class_parents_and_traits env c parents decl_errors
  in
  let (req_ancestors, req_ancestors_extends, req_class_ancestors) =
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
  let dc_tparams =
    Decl_enforceability.maybe_add_supportdyn_constraints
      ~this_class:(Some c)
      ctx
      p
      c.sc_tparams
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
      dc_is_module_level_trait =
        Attributes.mem SN.UserAttributes.uaModuleLevelTrait c.sc_user_attributes;
      dc_name = snd c.sc_name;
      dc_pos = fst c.sc_name;
      dc_tparams;
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
      dc_support_dynamic_type =
        c.sc_support_dynamic_type
        || inherited.Decl_inherit.ih_support_dynamic_type
           && TypecheckerOptions.implicit_inherit_sdt
                (Provider_context.get_tcopt ctx)
        || Attributes.mem
             SN.UserAttributes.uaDynamicallyReferenced
             c.sc_user_attributes;
      dc_extends = extends;
      dc_sealed_whitelist = sealed_whitelist;
      dc_xhp_attr_deps = xhp_attr_deps;
      dc_xhp_enum_values = c.sc_xhp_enum_values;
      dc_xhp_marked_empty = c.sc_xhp_marked_empty;
      dc_req_ancestors = req_ancestors;
      dc_req_ancestors_extends = req_ancestors_extends;
      dc_req_class_ancestors = req_class_ancestors;
      dc_enum_type = enum;
      dc_decl_errors = decl_errors;
      dc_docs_url = c.sc_docs_url;
    }
  in
  let filter_snd map = SMap.filter_map (fun _k v -> snd v) map in
  let member_heaps_values =
    {
      Decl_store.m_static_properties = filter_snd static_props;
      m_properties = filter_snd props;
      m_static_methods = filter_snd static_methods;
      m_methods = filter_snd methods;
      m_constructor = Option.(cstr |> fst >>= snd);
    }
  in
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
