(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_defs

let ( let* ) = Option.Let_syntax.( let* )

let is_typedef ctx x =
  match Naming_provider.get_type_kind ctx x with
  | Some Naming_types.TTypedef -> true
  | _ -> false

type enf =
  (* The type is fully enforced *)
  | Enforced of decl_ty
  (* The type is not fully enforced, but is enforced at the given ty, if present *)
  | Unenforced of decl_ty option

type 'a class_or_typedef_result =
  | ClassResult of 'a
  | TypedefResult of Typing_defs.typedef_type

let get_class_or_typedef ctx x =
  if is_typedef ctx x then
    match Decl_provider_internals.get_typedef_without_pessimise ctx x with
    | None -> None
    | Some td -> Some (TypedefResult td)
  else
    match Shallow_classes_provider.get_shallow_class ctx x with
    | None -> None
    | Some cd -> Some (ClassResult cd)

let get_typeconst_type ctx c id =
  let open Shallow_decl_defs in
  (* Look for id directly defined in the given shallow class. Assume there is
     only one, since it is an error to have multiple definitions. *)
  let find_locally c =
    List.find_map
      ~f:(fun stc ->
        if String.equal id (snd stc.stc_name) then
          Some stc.stc_kind
        else
          None)
      c.sc_typeconsts
  in
  let class_type_to_shallow_class cty =
    match get_node cty with
    | Tapply ((_, class_name), _) ->
      Shallow_classes_provider.get_shallow_class ctx class_name
    | _ -> None
  in
  (* Find the first concrete definition of id in the list. If there is none, return
     the first abstract one, kept in found_abstract *)
  let rec find_in_list visited found_abstract ctys =
    match ctys with
    | [] -> found_abstract
    | cty :: ctys ->
      let c = class_type_to_shallow_class cty in
      (match c with
      | None -> find_in_list visited found_abstract ctys
      | Some c ->
        (match find_in_class visited c with
        | None -> find_in_list visited found_abstract ctys
        | Some (TCAbstract _) as new_abstract ->
          (match found_abstract with
          | None -> find_in_list visited new_abstract ctys
          | _ -> find_in_list visited found_abstract ctys)
        | Some (TCConcrete _ as res) -> Some res))
  (* Look for id in c, either locally, or inherited *)
  and find_in_class visited c =
    if SSet.mem (snd c.sc_name) visited then
      None
    else
      match find_locally c with
      | Some tc -> Some tc
      | None ->
        find_in_list
          (SSet.add (snd c.sc_name) visited)
          None
          (List.concat
             [
               c.sc_extends;
               c.sc_implements;
               c.sc_uses;
               c.sc_req_extends;
               c.sc_req_implements;
             ])
  in
  let* tc = find_in_class SSet.empty c in
  match tc with
  | TCAbstract abstract -> abstract.atc_as_constraint
  | TCConcrete concrete -> Some concrete.tc_type

let make_unenforced ty =
  match ty with
  | Enforced ty -> Unenforced (Some ty)
  | Unenforced _ -> ty

module VisitedSet = Caml.Set.Make (struct
  type t = string * string

  let compare (s1, r1) (s2, r2) =
    let c1 = String.compare s1 s2 in
    if c1 = 0 then
      String.compare r1 r2
    else
      c1
end)

module type ContextAccess = sig
  (** [t] is the type of the context that classes and typedefs can be found in *)
  type t

  (** [class_t] is the type that represents a class *)
  type class_t

  val get_tcopt : t -> TypecheckerOptions.t

  val get_class_or_typedef :
    t -> string -> class_t class_or_typedef_result option

  val get_typedef : t -> string -> typedef_type option

  val get_class : t -> string -> class_t option

  (** [get_typeconst ctx cls name] gets the definition of the type constant
      [name] from [cls] or ancestor if it exists. *)
  val get_typeconst_type : t -> class_t -> string -> decl_ty option

  val get_tparams : class_t -> decl_tparam list

  val get_name : class_t -> string

  (** [get_enum_type cls] returns the enumeration type if [cls] is an enum. *)
  val get_enum_type : class_t -> enum_type option
end

module type Enforcement = sig
  type ctx

  type class_t

  val get_enforcement :
    return_from_async:bool ->
    this_class:class_t option ->
    ctx ->
    Typing_defs.decl_ty ->
    enf

  val is_enforceable :
    return_from_async:bool ->
    this_class:class_t option ->
    ctx ->
    Typing_defs.decl_ty ->
    bool
end

module Enforce (ContextAccess : ContextAccess) :
  Enforcement
    with type ctx = ContextAccess.t
     and type class_t = ContextAccess.class_t = struct
  type ctx = ContextAccess.t

  type class_t = ContextAccess.class_t

  let add_to_visited visited cd ty =
    let name = ContextAccess.get_name cd in
    VisitedSet.add (name, ty) visited

  let mem_visited visited cd ty =
    let name = ContextAccess.get_name cd in
    VisitedSet.mem (name, ty) visited

  (* Find the class definition that ty represents, by expanding Taccess. Since
     expanding one Taccess might give us another type containing Taccess, we
     keep a set of visited type constant definitions and terminate if we reach
     a cycle. *)
  let rec get_owner ~this_class visited (ctx : ContextAccess.t) ty =
    match get_node ty with
    | Tthis -> this_class
    | Tapply ((_, name), _) -> ContextAccess.get_class ctx name
    | Taccess (ty, (_, tcname)) ->
      let* (visited, cd, ty) =
        get_owner_and_type ~this_class visited ctx ty tcname
      in
      get_owner ~this_class:(Some cd) visited ctx ty
    | _ -> None

  (* Lookup tcname in the class that ty represents. *)
  and get_owner_and_type ~this_class visited (ctx : ContextAccess.t) ty tcname =
    let* cd = get_owner ~this_class visited ctx ty in
    if mem_visited visited cd tcname then
      None
    else
      let* ty = ContextAccess.get_typeconst_type ctx cd tcname in
      Some (add_to_visited visited cd tcname, cd, ty)

  (* Resolve an access t::T to a type that is not an access *)
  let resolve_access ~this_class (ctx : ContextAccess.t) ty tcname =
    (* We need to maintain a visited set because there can be cycles between bounds on type constants *)
    let rec resolve_access ~this_class visited (ctx : ContextAccess.t) ty tcname
        =
      let* (visited, cd, ty) =
        get_owner_and_type ~this_class visited ctx ty tcname
      in
      match get_node ty with
      | Taccess (sub_ty, (_, tcname)) ->
        resolve_access ~this_class:(Some cd) visited ctx sub_ty tcname
      | _ -> Some ty
    in
    resolve_access ~this_class VisitedSet.empty ctx ty tcname

  let get_enforcement
      ~return_from_async ~this_class (ctx : ContextAccess.t) (ty : decl_ty) :
      enf =
    let tcopt = ContextAccess.get_tcopt ctx in
    let enable_sound_dynamic = TypecheckerOptions.enable_sound_dynamic tcopt in
    let tc_enforced =
      TypecheckerOptions.(
        experimental_feature_enabled
          tcopt
          experimental_consider_type_const_enforceable)
    in
    let ty_pos = get_pos ty in
    (* is_dynamic_enforceable controls whether the type dynamic is considered enforceable.
       It isn't at the top-level of a type, but is as an argument to a reified generic. *)
    let rec enforcement
        ~is_dynamic_enforceable (ctx : ContextAccess.t) visited ty =
      match get_node ty with
      | Tthis -> Unenforced None
      (* Look through supportdyn, just as we look through ~ *)
      | Tapply ((_, name), [ty])
        when String.equal name Naming_special_names.Classes.cSupportDyn
             && enable_sound_dynamic ->
        enforcement ~is_dynamic_enforceable ctx visited ty
      | Tapply ((_, name), tyl) ->
        (* Cyclic type definition error will be produced elsewhere *)
        if SSet.mem name visited then
          Unenforced None
        else begin
          (* The pessimised definition depends on the class or typedef being referenced,
             but we aren't adding any dependency edges here. It is therefore critical that
             we are sure thay are added elsewhere. Currently, that is when we revisit this type
             in Typing_enforceability when we are typechecking the function/property definition
             it is part of. *)
          match ContextAccess.get_class_or_typedef ctx name with
          | Some
              (TypedefResult
                { td_vis = Aast.(CaseType | Transparent); td_type; _ }) ->
            (* Expand type definition one step and compute its enforcement.
             * While case types are Tnewtype in the type system, at runtime
             * they are enforced transparently. TODO(dreeves) Case types
             * may need to be intersected with their cover types at most. *)
            enforcement
              ~is_dynamic_enforceable
              ctx
              (SSet.add name visited)
              td_type
          | Some
              (TypedefResult
                {
                  td_vis = Aast.Opaque;
                  td_pos;
                  td_type;
                  td_as_constraint;
                  td_tparams;
                  _;
                }) ->
            let transparent =
              Relative_path.equal
                (Pos_or_decl.filename ty_pos)
                (Pos_or_decl.filename td_pos)
            in
            let exp_ty =
              enforcement
                ~is_dynamic_enforceable
                ctx
                (SSet.add name visited)
                td_type
            in
            if not (Int.equal (List.length td_tparams) (List.length tyl)) then
              (* If there is an arity error then assume enforced because this is
                 * used to fake a Tany at localization time
              *)
              Enforced td_type
            else if transparent then
              (* Same as transparent case *)
              exp_ty
            else (
              (* Similar to enums, newtypes must not be pessimised to their
               * enforced types, otherwise for `newtype N = int;`, pessimising
               * `N` to `~N & int` tells the truth, but breaks N's opaqueness.
               * For `newtype N as O = P;` we want to allow `N` to override `O`,
               * but it is not safe to just use the constraint to get `~N & O`.
               * Consider `newtype N as IE = IE;` and `enum IE : int { A = 4; }`.
               * `N` will only be enforced at `int`, whereas `~N & IE` lies
               * that the value is definitely a member of `IE`. We could look at
               * the enforcement of `O`, but it is valid to have newtypes point
               * to enforced values but be constrained by unenforced ones, e.g.
               * `newtype N as G<int> = int; newtype G<T> as T = T;`. Looking at
               * `N`'s constraint's enforcement would take us to just `~N`. This
               * could not override a non-pessimised `int`, which is a shame
               * because we know N is an int.
               *
               * To compromise, if `newtype N as O = P;` is enforced at `Q`, we
               * pessimise `N` to `~N & (O | Q)`. This allows the value to flow
               * into an `O` if `Q` is a subtype of `O`, while making sure we
               * are intersecting only with something the runtime verifies i.e.
               * some type at least as large as `Q`.
               * *)
              match (exp_ty, td_as_constraint) with
              | ((Enforced ty | Unenforced (Some ty)), Some cstr) ->
                Unenforced
                  (Some (Typing_make_type.union (get_reason cstr) [cstr; ty]))
              | _ -> Unenforced None
            )
          | Some (TypedefResult { td_vis = Aast.OpaqueModule; _ }) ->
            Unenforced None
          | Some (ClassResult cls) ->
            (match ContextAccess.get_enum_type cls with
            | Some et ->
              (* for `enum E : int`, pessimising `E` to `~E & int` violates the
               * opacity of the enum, allowing the value to flow to a position
               * expecting `int`. We instead weaken our pessimised type to
               * `~E & arraykey`. For transparent enums `enum F : int as int`,
               * we pessimise to `~F & int`. *)
              let intersected_type =
                Option.value
                  ~default:(Typing_make_type.arraykey (get_reason et.te_base))
                  et.te_constraint
              in
              make_unenforced
                (enforcement
                   ~is_dynamic_enforceable
                   ctx
                   (SSet.add name visited)
                   intersected_type)
            | None ->
              List.Or_unequal_lengths.(
                (match
                   List.for_all2
                     tyl
                     (ContextAccess.get_tparams cls)
                     ~f:(fun targ tparam ->
                       match get_node targ with
                       | Tdynamic
                       (* We accept the inner type being dynamic regardless of reification *)
                       | Tlike _
                         when not enable_sound_dynamic ->
                         true
                       | _ ->
                         (match tparam.tp_reified with
                         | Aast.Erased -> false
                         | Aast.SoftReified -> false
                         | Aast.Reified ->
                           (match
                              enforcement
                                ~is_dynamic_enforceable:true
                                ctx
                                visited
                                targ
                            with
                           | Unenforced _ -> false
                           | Enforced _ -> true)))
                 with
                | Ok false
                | Unequal_lengths ->
                  Unenforced None
                | Ok true -> Enforced ty)))
          | None -> Unenforced None
        end
      | Tgeneric _ ->
        (* Previously we allowed dynamic ~> T when T is an __Enforceable generic,
         * that is, when it's valid on the RHS of an `is` or `as` expression.
         * However, `is` / `as` checks have different behavior than runtime checks
         * for `tuple`s and `shapes`s; `is` / `as` will shallow-ly check declared
         * fields but typehint enforcement only checks that we have the right
         * array type (`varray` for `tuple`, `darray` for `shape`). This means
         * it's unsound to allow this coercion.
         *
         * Additionally, higher kinded generics (i.e., with type arguments) cannot
         * be enforced at the moment; they are disallowed to have upper bounds.
         *)
        Unenforced None
      | Trefinement _ -> Unenforced None
      | Taccess (ty, (_, id)) when tc_enforced ->
        (match resolve_access ~this_class ctx ty id with
        | None -> Unenforced None
        | Some ty -> enforcement ~is_dynamic_enforceable ctx visited ty)
      | Taccess _ -> Unenforced None
      | Tlike ty when enable_sound_dynamic ->
        enforcement ~is_dynamic_enforceable ctx visited ty
      | Tlike _ -> Unenforced None
      | Tprim Aast.(Tvoid | Tnoreturn) -> Unenforced None
      | Tprim _ -> Enforced ty
      | Tany _ -> Enforced ty
      | Tnonnull -> Enforced ty
      | Tdynamic ->
        if (not enable_sound_dynamic) || is_dynamic_enforceable then
          Enforced ty
        else
          Unenforced None
      | Tfun _ -> Unenforced None
      | Ttuple _ -> Unenforced None
      | Tunion [] -> Enforced ty
      | Tunion _ -> Unenforced None
      | Tintersection _ -> Unenforced None
      | Tshape _ -> Unenforced None
      | Tmixed -> Enforced ty
      | Twildcard -> Unenforced None
      (* With no parameters, we enforce varray_or_darray just like array *)
      | Tvec_or_dict (_, el_ty) ->
        if is_any el_ty then
          Enforced ty
        else
          Unenforced None
      | Toption ty ->
        (match enforcement ~is_dynamic_enforceable ctx visited ty with
        | Enforced _ -> Enforced ty
        | Unenforced (Some ety) ->
          Unenforced (Some (mk (get_reason ty, Toption ety)))
        | Unenforced None -> Unenforced None)
      | Tnewtype (name, _, _) ->
        if SSet.mem name visited then
          Unenforced None
        else (
          match ContextAccess.get_typedef ctx name with
          | Some { td_vis = Aast.Opaque; td_type; _ } ->
            let exp_ty =
              enforcement
                ~is_dynamic_enforceable
                ctx
                (SSet.add name visited)
                td_type
            in
            make_unenforced exp_ty
          | Some { td_vis = Aast.OpaqueModule; _ } -> Unenforced None
          | _ -> failwith "should never happen"
        )
    in

    if return_from_async then
      match get_node ty with
      | Tapply ((_, name), [ty])
        when String.equal Naming_special_names.Classes.cAwaitable name ->
        enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty
      | _ -> enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty
    else
      enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty

  let is_enforceable
      ~return_from_async ~this_class (ctx : ContextAccess.t) (ty : decl_ty) =
    match get_enforcement ~return_from_async ~this_class ctx ty with
    | Enforced _ -> true
    | Unenforced _ -> false
end

module ShallowContextAccess :
  ContextAccess
    with type t = Provider_context.t
     and type class_t = Shallow_decl_defs.shallow_class = struct
  type t = Provider_context.t

  type class_t = Shallow_decl_defs.shallow_class

  let get_tcopt = Provider_context.get_tcopt

  let get_class_or_typedef = get_class_or_typedef

  let get_typedef = Decl_provider_internals.get_typedef_without_pessimise

  let get_class = Shallow_classes_provider.get_shallow_class

  let get_typeconst_type = get_typeconst_type

  let get_tparams sc = sc.Shallow_decl_defs.sc_tparams

  let get_name cd = snd cd.Shallow_decl_defs.sc_name

  let get_enum_type sc = sc.Shallow_decl_defs.sc_enum_type
end

module E = Enforce (ShallowContextAccess)
include E

let make_like_type ~reason ~intersect_with ~return_from_async ty =
  let like_and_intersect r ty =
    let like_ty = Typing_make_type.like r ty in
    match intersect_with with
    | None -> like_ty
    | Some enf_ty -> Typing_make_type.intersection r [enf_ty; like_ty]
  in
  let like_if_not_void ty =
    match get_node ty with
    | Tprim Aast.(Tvoid | Tnoreturn) -> ty
    | _ -> like_and_intersect reason ty
  in
  if return_from_async then
    match get_node ty with
    | Tapply ((pos, name), [ty])
      when String.equal Naming_special_names.Classes.cAwaitable name ->
      mk
        ( get_reason ty,
          Tapply ((pos, name), [like_and_intersect (get_reason ty) ty]) )
    | _ -> like_if_not_void ty
  else
    like_if_not_void ty

let make_supportdyn_type p r ty =
  mk (r, Tapply ((p, Naming_special_names.Classes.cSupportDyn), [ty]))

let supportdyn_mixed p r = make_supportdyn_type p r (mk (r, Tmixed))

let add_supportdyn_constraints p tparams =
  let r = Reason.Rwitness_from_decl p in
  List.map tparams ~f:(fun tparam ->
      if
        Naming_special_names.Coeffects.is_generated_generic (snd tparam.tp_name)
        || Typing_defs.Attributes.mem
             Naming_special_names.UserAttributes.uaNoAutoBound
             tparam.tp_user_attributes
      then
        tparam
      else
        {
          tparam with
          tp_constraints =
            (Ast_defs.Constraint_as, supportdyn_mixed p r)
            :: tparam.tp_constraints;
        })

let maybe_add_supportdyn_constraints ~this_class ctx p tparams =
  if Provider_context.implicit_sdt_for_class ctx this_class then
    add_supportdyn_constraints p tparams
  else
    tparams

let pessimise_type ~reason ~is_xhp_attr ~this_class ctx ty =
  if
    (not is_xhp_attr)
    && is_enforceable ~return_from_async:false ~this_class ctx ty
  then
    ty
  else
    make_like_type ~reason ~intersect_with:None ~return_from_async:false ty

let maybe_pessimise_type ~reason ~is_xhp_attr ~this_class ctx ty =
  if Provider_context.implicit_sdt_for_class ctx this_class then
    pessimise_type ~reason ~is_xhp_attr ~this_class ctx ty
  else
    ty

let update_return_ty ft ty =
  { ft with ft_ret = { et_type = ty; et_enforced = Unenforced } }

type fun_kind =
  | Function
  | Abstract_method
  | Concrete_method

(* We do not pessimise parameter types *except* for inout parameters,
 * which we pessimise regardless of enforcement, because override doesn't
 * preserve enforcement e.g. `inout int` might beoverridden by `inout C::MyTypeConstant`
 *)
let pessimise_param_type fp =
  match get_fp_mode fp with
  | FPnormal -> fp
  | FPinout ->
    {
      fp with
      fp_type =
        {
          et_enforced = Unenforced;
          et_type =
            make_like_type
              ~reason:(Reason.Rpessimised_inout fp.fp_pos)
              ~intersect_with:None
              ~return_from_async:false
              fp.fp_type.et_type;
        };
    }

(*
  How we pessimise a function depends on how the type is enforced, where the
  function is, and the experimental_always_pessimise_return option.

  The goal is to pessimise non-enforced return types to like types, while also
  avoiding hierarchy errors.

  If experimental_always_pessimise_return is set, the all methods should get
  like-types on their return to avoid hierarchy errors, but top-level functions should
  only be pessimised when the type is not enforceable.

  If experimental_always_pessimise_return is not set, we only pessimise the return of
  non-enforced types. Abstract methods do not have bodies to enforce anything, so we
  always pessimise their returns. For Concrete methods, if the type is only partially
  enforced, we pessimise and also intersect with the partially enforced type to
  avoid hierarchy errors.

*)
let pessimise_fun_type ~fun_kind ~this_class ~no_auto_likes ctx p ty =
  match get_node ty with
  | Tfun ft ->
    let ft =
      { ft with ft_tparams = add_supportdyn_constraints p ft.ft_tparams }
    in
    if no_auto_likes then
      mk (get_reason ty, Tfun ft)
    else
      let return_from_async = get_ft_async ft in
      let ret_ty = ft.ft_ret.et_type in
      let ft =
        { ft with ft_params = List.map ft.ft_params ~f:pessimise_param_type }
      in
      (* For the this type, although not enforced (because of generics), it's very
       * unlikely that we will get errors if we do not pessimise it.
       *)
      let optimistically_do_not_pessimise =
        match get_node ret_ty with
        | Tthis -> true
        | _ -> false
      in
      (match
         ( fun_kind,
           TypecheckerOptions.(
             experimental_feature_enabled
               (Provider_context.get_tcopt ctx)
               experimental_always_pessimise_return) )
       with
      | (Concrete_method, true)
      | (Abstract_method, _) ->
        mk
          ( get_reason ty,
            Tfun
              (update_return_ty
                 ft
                 (make_like_type
                    ~reason:(Reason.Rpessimised_return (get_pos ret_ty))
                    ~intersect_with:None
                    ~return_from_async
                    ret_ty)) )
      | _ ->
        if optimistically_do_not_pessimise then
          mk (get_reason ty, Tfun ft)
        else (
          match get_enforcement ~return_from_async ~this_class ctx ret_ty with
          | Enforced _ -> mk (get_reason ty, Tfun ft)
          | Unenforced enf_ty_opt ->
            (* For partially enforced type such as enums, we intersect with the
             * base type for concrete method return types in order to avoid
             * issues with hierarchies e.g. overriding a method that returns
             * the base type
             *)
            let intersect_with =
              match fun_kind with
              | Concrete_method -> enf_ty_opt
              | _ -> None
            in
            mk
              ( get_reason ty,
                Tfun
                  (update_return_ty
                     ft
                     (make_like_type
                        ~reason:(Reason.Rpessimised_return (get_pos ret_ty))
                        ~intersect_with
                        ~return_from_async
                        ret_ty)) )
        ))
  | _ -> ty
