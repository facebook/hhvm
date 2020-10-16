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

(*****************************************************************************)
(* Checking that the kind of a class is compatible with its parent
 * For example, a class cannot extend an interface, an interface cannot
 * extend a trait etc ...
 *)
(*****************************************************************************)

let check_extend_kind
    (parent_pos : Pos.t)
    (parent_kind : Ast_defs.class_kind)
    (parent_name : string)
    (parent_is_enum_class : bool)
    (child_pos : Pos.t)
    (child_kind : Ast_defs.class_kind)
    (child_name : string)
    (child_is_enum_class : bool) : unit =
  match (parent_kind, child_kind) with
  (* What is allowed *)
  | ( (Ast_defs.Cabstract | Ast_defs.Cnormal),
      (Ast_defs.Cabstract | Ast_defs.Cnormal) )
  | (Ast_defs.Cabstract, Ast_defs.Cenum)
  (* enums extend BuiltinEnum under the hood *)
  | (Ast_defs.Ctrait, Ast_defs.Ctrait)
  | (Ast_defs.Cinterface, Ast_defs.Cinterface) ->
    ()
  | (Ast_defs.Cenum, Ast_defs.Cenum) ->
    if parent_is_enum_class && child_is_enum_class then
      ()
    else
      Errors.wrong_extend_kind
        ~parent_pos
        ~parent_kind
        ~parent_name
        ~parent_is_enum_class
        ~child_pos
        ~child_kind
        ~child_name
        ~child_is_enum_class
  | _ ->
    (* What is disallowed *)
    Errors.wrong_extend_kind
      ~parent_pos
      ~parent_kind
      ~parent_name
      ~parent_is_enum_class
      ~child_pos
      ~child_kind
      ~child_name
      ~child_is_enum_class

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
    (shallow_class : Shallow_decl_defs.shallow_class) : string -> unit =
  Errors.trait_reuse
    parent_type.dc_pos
    parent_type.dc_name
    shallow_class.sc_name

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
    (parent_pos : Pos.t)
    (shallow_class : Shallow_decl_defs.shallow_class)
    (acc : SSet.t * bool * [> `Extends_pass | `Xhp_pass ])
    (parent_type : Decl_defs.decl_class_type)
    (parent_enum_type : enum_type option) : SSet.t * bool * 'a =
  let (extends, is_complete, pass) = acc in
  let class_pos = fst shallow_class.sc_name in
  let class_kind = shallow_class.sc_kind in
  let class_name = snd shallow_class.sc_name in
  let class_enum_type = shallow_class.sc_enum_type in
  let parent_is_enum_class = is_enum_class parent_enum_type in
  let class_is_enum_class = is_enum_class class_enum_type in
  if phys_equal pass `Extends_pass then
    check_extend_kind
      parent_pos
      parent_type.dc_kind
      parent_type.dc_name
      parent_is_enum_class
      class_pos
      class_kind
      class_name
      class_is_enum_class;

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
  (extends', parent_type.dc_members_fully_known && is_complete, pass)

let get_class_parent_or_trait
    (env : Decl_env.env)
    (shallow_class : Shallow_decl_defs.shallow_class)
    ((parents, is_complete, pass) :
      SSet.t * bool * [> `Extends_pass | `Xhp_pass ])
    (ty : Typing_defs.decl_phase Typing_defs.ty) : SSet.t * bool * 'a =
  (* See comment on check_no_duplicate_traits for reasoning here *)
  let no_trait_reuse =
    disallow_trait_reuse env
    && (not (phys_equal pass `Xhp_pass))
    && not Ast_defs.(equal_class_kind shallow_class.sc_kind Cinterface)
  in
  let (_, (parent_pos, parent), _) = Decl_utils.unwrap_class_type ty in
  (* If we already had this exact trait, we need to flag trait reuse *)
  let reused_trait = no_trait_reuse && SSet.mem parent parents in
  let parents = SSet.add parent parents in
  let parent_type = Decl_env.get_class_dep env parent in
  match parent_type with
  | None ->
    (* The class lives in PHP *)
    (parents, false, pass)
  | Some parent_type ->
    (* The parent class lives in Hack, so we can report reused traits *)
    if reused_trait then report_reused_trait parent_type shallow_class parent;
    let acc = (parents, is_complete, pass) in
    add_grand_parents_or_traits
      no_trait_reuse
      parent_pos
      shallow_class
      acc
      parent_type
      parent_type.dc_enum_type

let get_class_parents_and_traits
    (env : Decl_env.env) (shallow_class : Shallow_decl_defs.shallow_class) :
    SSet.t * SSet.t * bool =
  let parents = SSet.empty in
  let is_complete = true in
  (* extends parents *)
  let acc = (parents, is_complete, `Extends_pass) in
  let (parents, is_complete, _) =
    List.fold_left
      shallow_class.sc_extends
      ~f:(get_class_parent_or_trait env shallow_class)
      ~init:acc
  in
  (* traits *)
  let acc = (parents, is_complete, `Traits_pass) in
  let (parents, is_complete, _) =
    List.fold_left
      shallow_class.sc_uses
      ~f:(get_class_parent_or_trait env shallow_class)
      ~init:acc
  in
  (* XHP classes whose attributes were imported via "attribute :foo;" syntax *)
  let acc = (SSet.empty, is_complete, `Xhp_pass) in
  let (xhp_parents, is_complete, _) =
    List.fold_left
      shallow_class.sc_xhp_attr_uses
      ~f:(get_class_parent_or_trait env shallow_class)
      ~init:acc
  in
  (parents, xhp_parents, is_complete)

type class_env = {
  ctx: Provider_context.t;
  stack: SSet.t;
}

let check_if_cyclic (class_env : class_env) ((pos, cid) : Pos.t * string) : bool
    =
  let stack = class_env.stack in
  let is_cyclic = SSet.mem cid stack in
  if is_cyclic then Errors.cyclic_class_def stack pos;
  is_cyclic

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let pu_enum_fold
    origin
    (acc : Typing_defs.pu_enum_type SMap.t)
    (spu : Shallow_decl_defs.shallow_pu_enum) : Typing_defs.pu_enum_type SMap.t
    =
  let spu_name = snd spu.spu_name in
  let tpu =
    match SMap.find_opt spu_name acc with
    | None -> Decl_to_typing.shallow_pu_enum_to_pu_enum_type origin spu
    | Some tpu ->
      let origin = { pu_class = origin; pu_enum = spu_name } in
      {
        tpu_name = spu.spu_name;
        tpu_is_final = spu.spu_is_final;
        tpu_case_types =
          List.fold_left
            spu.spu_case_types
            ~init:tpu.tpu_case_types
            ~f:(fun acc tp ->
              let sid = snd tp.tp_name in
              SMap.add sid (origin, tp) acc);
        tpu_case_values =
          List.fold_left
            spu.spu_case_values
            ~init:tpu.tpu_case_values
            ~f:(fun acc (name, dty) ->
              SMap.add (snd name) (origin, name, dty) acc);
        tpu_members =
          List.fold_left spu.spu_members ~init:tpu.tpu_members ~f:(fun acc sm ->
              let tpum_types =
                match SMap.find_opt (snd sm.spum_atom) acc with
                | None -> SMap.empty
                | Some tm -> tm.tpum_types
              in
              let tpum_exprs =
                match SMap.find_opt (snd sm.spum_atom) acc with
                | None -> SMap.empty
                | Some tm -> tm.tpum_exprs
              in
              let tpum_types =
                List.fold_left
                  sm.spum_types
                  ~init:tpum_types
                  ~f:(fun acc (sid, declty) ->
                    let k = snd sid in
                    SMap.add k (origin, sid, declty) acc)
              in
              let tpum_exprs =
                List.fold_left sm.spum_exprs ~init:tpum_exprs ~f:(fun acc k ->
                    SMap.add (snd k) (origin, k) acc)
              in
              SMap.add
                (snd sm.spum_atom)
                {
                  tpum_atom = sm.spum_atom;
                  tpum_origin = origin;
                  tpum_types;
                  tpum_exprs;
                }
                acc);
      }
  in
  SMap.add (snd spu.spu_name) tpu acc

let rec class_decl_if_missing
    ~(sh : SharedMem.uses) (class_env : class_env) (c : Nast.class_) :
    (string * Decl_defs.decl_class_type) option =
  let ((_, cid) as c_name) = c.c_name in
  if check_if_cyclic class_env c_name then
    None
  else if shallow_decl_enabled class_env.ctx then
    (* This function is often called for its side effect of ensuring that the
         class is declared. When shallow-decl is enabled, we still want this
         side effect (for use cases like on-the-fly declaring entire files in
         Decl_redecl_service for incremental typechecking), but since we are not
         producing a folded class declaration, there is nothing we can return.
         This is a code smell--we should use a function with a different
         signature when we only want this side effect. *)
    let (_ : shallow_class) =
      Shallow_classes_provider.decl class_env.ctx ~use_cache:true c
    in
    None
  else
    match Decl_heap.Classes.get cid with
    | Some class_ -> Some (cid, class_)
    | None ->
      (* Class elements are in memory if and only if the class itself is there.
       * Exiting before class declaration is ready would break this invariant *)
      WorkerCancel.with_no_cancellations @@ fun () ->
      let class_ = class_naming_and_decl ~sh class_env cid c in
      Some class_

and class_naming_and_decl
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (cid : string)
    (c : Nast.class_) : string * Decl_defs.decl_class_type =
  let class_env = { class_env with stack = SSet.add cid class_env.stack } in
  let shallow_class =
    Shallow_classes_provider.decl class_env.ctx ~use_cache:false c
  in
  let (errors, tc) =
    Errors.do_ (fun () ->
        class_parents_decl ~sh class_env shallow_class;
        class_decl ~sh class_env.ctx shallow_class)
  in
  let name = snd shallow_class.sc_name in
  let class_ = { tc with dc_decl_errors = Some errors } in
  Decl_heap.Classes.add name class_;
  (name, class_)

and class_parents_decl
    ~(sh : SharedMem.uses)
    (class_env : class_env)
    (c : Shallow_decl_defs.shallow_class) : unit =
  let class_type class_ =
    let (_ : Decl_defs.decl_class_type option) =
      class_type_decl ~sh class_env class_
    in
    ()
  in
  List.iter c.sc_extends class_type;
  List.iter c.sc_implements class_type;
  List.iter c.sc_uses class_type;
  List.iter c.sc_xhp_attr_uses class_type;
  List.iter c.sc_req_extends class_type;
  List.iter c.sc_req_implements class_type;
  let enum_includes =
    Aast.enum_includes_map ~f:(fun et -> et.te_includes) c.sc_enum_type
  in
  List.iter enum_includes class_type;
  ()

and is_disposable_type (env : Decl_env.env) (hint : Typing_defs.decl_ty) : bool
    =
  match get_node hint with
  | Tapply ((_, c), _) ->
    begin
      match Decl_env.get_class_dep env c with
      | None -> false
      | Some c -> c.dc_is_disposable
    end
  | _ -> false

and class_type_decl
    ~(sh : SharedMem.uses) (class_env : class_env) (hint : Typing_defs.decl_ty)
    : Decl_defs.decl_class_type option =
  match get_node hint with
  | Tapply ((_, cid), _) ->
    begin
      match Naming_provider.get_class_path class_env.ctx cid with
      | Some fn when not (Decl_heap.Classes.mem cid) ->
        (* We are supposed to redeclare the class *)
        let class_opt = Ast_provider.find_class_in_file class_env.ctx fn cid in
        Errors.run_in_context fn Errors.Decl (fun () ->
            Option.Monad_infix.(
              class_opt >>= class_decl_if_missing ~sh class_env >>| snd))
      | _ -> None
    end
  | _ ->
    (* This class lives in PHP land *)
    None

and class_is_abstract (c : Shallow_decl_defs.shallow_class) : bool =
  match c.sc_kind with
  | Ast_defs.Cabstract
  | Ast_defs.Cinterface
  | Ast_defs.Ctrait
  | Ast_defs.Cenum ->
    true
  | _ -> false

(* When all type constants have been inherited and declared, this step synthesizes
 * the defaults of abstract type constants into concrete type constants. *)
and synthesize_defaults
    (k : string)
    (tc : Typing_defs.typeconst_type)
    ((typeconsts, consts) :
      Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t) :
    Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t =
  match tc.ttc_abstract with
  | TCAbstract (Some default) ->
    let concrete =
      {
        tc with
        ttc_abstract = TCConcrete;
        ttc_constraint = None;
        ttc_type = Some default;
      }
    in
    let typeconsts = SMap.add k concrete typeconsts in
    (* OCaml 4.06 has an update method that makes this operation much more ergonomic *)
    let constant = SMap.find_opt k consts in
    let consts =
      Option.value_map constant ~default:consts ~f:(fun c ->
          SMap.add k { c with cc_abstract = false } consts)
    in
    (typeconsts, consts)
  | _ -> (typeconsts, consts)

and class_decl
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    (c : Shallow_decl_defs.shallow_class) : Decl_defs.decl_class_type =
  let is_abstract = class_is_abstract c in
  let const = Attrs.mem SN.UserAttributes.uaConst c.sc_user_attributes in
  let (_p, cls_name) = c.sc_name in
  let class_dep = Dep.Class cls_name in
  let env = { Decl_env.mode = c.sc_mode; droot = Some class_dep; ctx } in
  let inherited = Decl_inherit.make env c in
  let props = inherited.Decl_inherit.ih_props in
  let props =
    List.fold_left ~f:(prop_decl ~write_shmem:true c) ~init:props c.sc_props
  in
  let m = inherited.Decl_inherit.ih_methods in
  let (m, condition_types) =
    List.fold_left
      ~f:(method_decl_acc ~write_shmem:true ~is_static:false c)
      ~init:(m, SSet.empty)
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
    if Ast_defs.(equal_class_kind c.sc_kind Cnormal) then
      SMap.fold synthesize_defaults typeconsts (typeconsts, consts)
    else
      (typeconsts, consts)
  in
  let pu_enums = inherited.Decl_inherit.ih_pu_enums in
  let pu_enums =
    List.fold_left c.sc_pu_enums ~f:(pu_enum_fold cls_name) ~init:pu_enums
  in
  let sclass_var = static_prop_decl ~write_shmem:true c in
  let sprops = inherited.Decl_inherit.ih_sprops in
  let sprops = List.fold_left c.sc_sprops ~f:sclass_var ~init:sprops in
  let sm = inherited.Decl_inherit.ih_smethods in
  let (sm, condition_types) =
    List.fold_left
      c.sc_static_methods
      ~f:(method_decl_acc ~write_shmem:true ~is_static:true c)
      ~init:(sm, condition_types)
  in
  let parent_cstr = inherited.Decl_inherit.ih_cstr in
  let cstr = constructor_decl ~sh parent_cstr c in
  let has_concrete_cstr =
    match fst cstr with
    | None -> false
    | Some elt when get_elt_abstract elt -> false
    | _ -> true
  in
  let impl = c.sc_extends @ c.sc_implements @ c.sc_uses in
  let impl =
    match
      List.find c.sc_methods ~f:(fun sm ->
          String.equal (snd sm.sm_name) SN.Members.__toString)
    with
    | Some { sm_name = (pos, _); _ }
      when String.( <> ) cls_name SN.Classes.cStringish ->
      (* HHVM implicitly adds Stringish interface for every class/iface/trait
       * with a __toString method; "string" also implements this interface *)
      (* Declare Stringish and parents if not already declared *)
      let class_env = { ctx; stack = SSet.empty } in
      let ty =
        mk (Reason.Rhint pos, Tapply ((pos, SN.Classes.cStringish), []))
      in
      let (_ : Decl_defs.decl_class_type option) =
        class_type_decl ~sh class_env ty
      in
      ty :: impl
    | _ -> impl
  in
  let impl = List.map impl (get_implements env) in
  let impl = List.fold_right impl ~f:(SMap.fold SMap.add) ~init:SMap.empty in
  let (extends, xhp_attr_deps, ext_strict) =
    get_class_parents_and_traits env c
  in
  let (req_ancestors, req_ancestors_extends) =
    Decl_requirements.get_class_requirements env c
  in
  (* Interfaces IDisposable and IAsyncDisposable are *disposable types*, as
   * are any classes that implement either of these interfaces, directly or
   * indirectly. Also treat any trait that *requires* extension or
   * implementation of a disposable class as disposable itself.
   *)
  let is_disposable_class_name cls_name =
    String.equal cls_name SN.Classes.cIDisposable
    || String.equal cls_name SN.Classes.cIAsyncDisposable
  in
  let is_disposable =
    is_disposable_class_name cls_name
    || SMap.exists (fun n _ -> is_disposable_class_name n) impl
    || List.exists
         (c.sc_req_extends @ c.sc_req_implements)
         (is_disposable_type env)
  in
  (* If this class is disposable then we require that any extended class or
   * trait that is used, is also disposable, in order that escape analysis
   * has been applied on the $this parameter.
   *)
  let ext_strict =
    List.fold_left c.sc_uses ~f:(trait_exists env) ~init:ext_strict
  in
  let enum = c.sc_enum_type in
  let enum_inner_ty = SMap.find_opt SN.FB.tInner typeconsts in
  let consts =
    Decl_enum.rewrite_class
      c.sc_name
      enum
      Option.(enum_inner_ty >>= fun t -> t.ttc_type)
      (fun x -> SMap.find_opt x impl)
      consts
  in
  let has_own_cstr = has_concrete_cstr && Option.is_some c.sc_constructor in
  let deferred_members =
    if shallow_decl_enabled ctx then
      SSet.empty
    else
      snd (Decl_init_check.class_ ~has_own_cstr env c)
  in
  let sealed_whitelist = get_sealed_whitelist c in
  let tc =
    {
      dc_final = c.sc_final;
      dc_const = const;
      dc_abstract = is_abstract;
      dc_need_init = has_concrete_cstr;
      dc_deferred_init_members = deferred_members;
      dc_members_fully_known = ext_strict;
      dc_kind = c.sc_kind;
      dc_is_xhp = c.sc_is_xhp;
      dc_has_xhp_keyword = c.sc_has_xhp_keyword;
      dc_is_disposable = is_disposable;
      dc_name = snd c.sc_name;
      dc_pos = fst c.sc_name;
      dc_tparams = c.sc_tparams;
      dc_where_constraints = c.sc_where_constraints;
      dc_substs = inherited.Decl_inherit.ih_substs;
      dc_consts = consts;
      dc_typeconsts = typeconsts;
      dc_pu_enums = pu_enums;
      dc_props = props;
      dc_sprops = sprops;
      dc_methods = m;
      dc_smethods = sm;
      dc_construct = cstr;
      dc_ancestors = impl;
      dc_extends = extends;
      dc_sealed_whitelist = sealed_whitelist;
      dc_xhp_attr_deps = xhp_attr_deps;
      dc_req_ancestors = req_ancestors;
      dc_req_ancestors_extends = req_ancestors_extends;
      dc_enum_type = enum;
      dc_decl_errors = None;
      dc_condition_types = condition_types;
    }
  in
  SMap.iter
    begin
      fun x _ ->
      Typing_deps.add_idep class_dep (Dep.Class x)
    end
    impl;
  tc

and get_sealed_whitelist (c : Shallow_decl_defs.shallow_class) : SSet.t option =
  match Attributes.find SN.UserAttributes.uaSealed c.sc_user_attributes with
  | None -> None
  | Some { ua_classname_params; _ } -> Some (SSet.of_list ua_classname_params)

and get_implements (env : Decl_env.env) (ht : Typing_defs.decl_ty) :
    Typing_defs.decl_ty SMap.t =
  let (_r, (_p, c), paraml) = Decl_utils.unwrap_class_type ht in
  let class_ = Decl_env.get_class_dep env c in
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

and trait_exists (env : Decl_env.env) (acc : bool) (trait : Typing_defs.decl_ty)
    : bool =
  match get_node trait with
  | Tapply ((_, trait), _) ->
    let class_ = Decl_env.get_class_dep env trait in
    (match class_ with
    | None -> false
    | Some _class -> acc)
  | _ -> false

and constructor_decl
    ~(sh : SharedMem.uses)
    ((pcstr, pconsist) : Decl_defs.element option * Typing_defs.consistent_kind)
    (class_ : Shallow_decl_defs.shallow_class) :
    Decl_defs.element option * Typing_defs.consistent_kind =
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
    | None -> pcstr
    | Some method_ -> build_constructor ~write_shmem:true class_ method_
  in
  (cstr, Decl_utils.coalesce_consistent pconsist cconsist)

and build_constructor
    ~(write_shmem : bool)
    (class_ : Shallow_decl_defs.shallow_class)
    (method_ : Shallow_decl_defs.shallow_method) : Decl_defs.element option =
  let (_, class_name) = class_.sc_name in
  let vis = visibility class_name method_.sm_visibility in
  let pos = fst method_.sm_name in
  let cstr =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:None
          ~final:method_.sm_final
          ~abstract:method_.sm_abstract
          ~lateinit:false
          ~const:false
          ~lsb:false
          ~memoizelsb:false
          ~synthesized:false
          ~override:false
          ~dynamicallycallable:false;
      elt_visibility = vis;
      elt_origin = class_name;
      elt_reactivity = None;
      elt_deprecated = method_.sm_deprecated;
    }
  in
  let fe =
    {
      fe_pos = pos;
      fe_deprecated = method_.sm_deprecated;
      fe_type = method_.sm_type;
      fe_php_std_lib = false;
    }
  in
  if write_shmem then Decl_heap.Constructors.add class_name fe;
  Some cstr

and class_const_fold
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
    }
  in
  let acc = SMap.add (snd scc.scc_name) cc acc in
  acc

(* Every class, interface, and trait implicitly defines a ::class to
 * allow accessing its fully qualified name as a string *)
and class_class_decl (class_id : Ast_defs.id) : Typing_defs.class_const =
  let (pos, name) = class_id in
  let reason = Reason.Rclass_class (pos, name) in
  let classname_ty =
    mk (reason, Tapply ((pos, SN.Classes.cClassname), [mk (reason, Tthis)]))
  in
  {
    cc_abstract = false;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = classname_ty;
    cc_origin = name;
  }

and prop_decl
    ~(write_shmem : bool)
    (c : Shallow_decl_defs.shallow_class)
    (acc : Decl_defs.element SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) : Decl_defs.element SMap.t =
  let (sp_pos, sp_name) = sp.sp_name in
  let ty =
    match sp.sp_type with
    | None -> mk (Reason.Rwitness sp_pos, Typing_defs.make_tany ())
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) sp.sp_visibility in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:sp.sp_xhp_attr
          ~final:true
          ~lsb:false
          ~synthesized:false
          ~override:false
          ~memoizelsb:false
          ~const:sp.sp_const
          ~lateinit:sp.sp_lateinit
          ~abstract:sp.sp_abstract
          ~dynamicallycallable:false;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_reactivity = None;
      elt_deprecated = None;
    }
  in
  if write_shmem then Decl_heap.Props.add (elt.elt_origin, sp_name) ty;
  let acc = SMap.add sp_name elt acc in
  acc

and static_prop_decl
    ~(write_shmem : bool)
    (c : Shallow_decl_defs.shallow_class)
    (acc : Decl_defs.element SMap.t)
    (sp : Shallow_decl_defs.shallow_prop) : Decl_defs.element SMap.t =
  let (sp_pos, sp_name) = sp.sp_name in
  let ty =
    match sp.sp_type with
    | None -> mk (Reason.Rwitness sp_pos, Typing_defs.make_tany ())
    | Some ty' -> ty'
  in
  let vis = visibility (snd c.sc_name) sp.sp_visibility in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:sp.sp_xhp_attr
          ~final:true
          ~const:sp.sp_const
          ~lateinit:sp.sp_lateinit
          ~lsb:sp.sp_lsb
          ~override:false
          ~memoizelsb:false
          ~abstract:sp.sp_abstract
          ~synthesized:false
          ~dynamicallycallable:false;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_reactivity = None;
      elt_deprecated = None;
    }
  in
  if write_shmem then Decl_heap.StaticProps.add (elt.elt_origin, sp_name) ty;
  let acc = SMap.add sp_name elt acc in
  acc

and visibility (cid : string) (visibility : Aast_defs.visibility) :
    Typing_defs.ce_visibility =
  match visibility with
  | Public -> Vpublic
  | Protected -> Vprotected cid
  | Private -> Vprivate cid

(* each concrete type constant T = <sometype> implicitly defines a
class constant with the same name which is TypeStructure<sometype> *)
and typeconst_structure
    (c : Shallow_decl_defs.shallow_class)
    (stc : Shallow_decl_defs.shallow_typeconst) : Typing_defs.class_const =
  let pos = fst stc.stc_name in
  let r = Reason.Rwitness pos in
  let tsid = (pos, SN.FB.cTypeStructure) in
  let ts_ty =
    mk (r, Tapply (tsid, [mk (r, Taccess (mk (r, Tthis), [stc.stc_name]))]))
  in
  let abstract =
    match stc.stc_abstract with
    | TCAbstract _ -> true
    | _ -> false
  in
  {
    cc_abstract = abstract;
    cc_pos = pos;
    cc_synthesized = true;
    cc_type = ts_ty;
    cc_origin = snd c.sc_name;
  }

and typeconst_fold
    (c : Shallow_decl_defs.shallow_class)
    (acc : Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t)
    (stc : Shallow_decl_defs.shallow_typeconst) :
    Typing_defs.typeconst_type SMap.t * Typing_defs.class_const SMap.t =
  let (typeconsts, consts) = acc in
  match c.sc_kind with
  | Ast_defs.Ctrait
  | Ast_defs.Cenum ->
    acc
  | Ast_defs.Cinterface
  | Ast_defs.Cabstract
  | Ast_defs.Cnormal ->
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
        | None -> (Pos.none, false)
    in
    let reifiable =
      if Option.is_some stc.stc_reifiable then
        stc.stc_reifiable
      else
        Option.bind ptc_opt (fun ptc -> ptc.ttc_reifiable)
    in
    let tc =
      {
        ttc_abstract = stc.stc_abstract;
        ttc_name = stc.stc_name;
        ttc_constraint = stc.stc_constraint;
        ttc_type = stc.stc_type;
        ttc_origin = c_name;
        ttc_enforceable = enforceable;
        ttc_reifiable = reifiable;
      }
    in
    let typeconsts = SMap.add (snd stc.stc_name) tc typeconsts in
    (typeconsts, consts)

and method_decl_acc
    ~(write_shmem : bool)
    ~(is_static : bool)
    (c : Shallow_decl_defs.shallow_class)
    ((acc, condition_types) : Decl_defs.element SMap.t * SSet.t)
    (m : Shallow_decl_defs.shallow_method) : Decl_defs.element SMap.t * SSet.t =
  (* If method doesn't override anything but has the <<__Override>> attribute, then
   * set the override flag in ce_flags and let typing emit an appropriate error *)
  let check_override = m.sm_override && not (SMap.mem (snd m.sm_name) acc) in
  let (pos, id) = m.sm_name in
  let get_reactivity t =
    match get_node t with
    | Tfun { ft_reactive; _ } -> ft_reactive
    | _ -> Local None
  in
  let condition_types =
    match get_reactivity m.sm_type with
    | Pure (Some ty)
    | Reactive (Some ty)
    | Shallow (Some ty)
    | Local (Some ty) ->
      begin
        match get_node ty with
        | Tapply ((_, cls), []) -> SSet.add cls condition_types
        | _ -> condition_types
      end
    | _ -> condition_types
  in
  let vis =
    match (SMap.find_opt id acc, m.sm_visibility) with
    | (Some { elt_visibility = Vprotected _ as parent_vis; _ }, Protected) ->
      parent_vis
    | _ -> visibility (snd c.sc_name) m.sm_visibility
  in
  let elt =
    {
      elt_flags =
        make_ce_flags
          ~xhp_attr:None
          ~final:m.sm_final
          ~abstract:m.sm_abstract
          ~override:check_override
          ~synthesized:false
          ~lsb:false
          ~memoizelsb:false
          ~const:false
          ~lateinit:false
          ~dynamicallycallable:m.sm_dynamicallycallable;
      elt_visibility = vis;
      elt_origin = snd c.sc_name;
      elt_reactivity = m.sm_reactivity;
      elt_deprecated = m.sm_deprecated;
    }
  in
  let fe =
    {
      fe_pos = pos;
      fe_deprecated = None;
      fe_type = m.sm_type;
      fe_php_std_lib = false;
    }
  in
  if write_shmem then
    if is_static then
      Decl_heap.StaticMethods.add (elt.elt_origin, id) fe
    else
      Decl_heap.Methods.add (elt.elt_origin, id) fe;
  let acc = SMap.add id elt acc in
  (acc, condition_types)
