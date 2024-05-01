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

module ShallowContextAccess :
  Decl_enforceability.ContextAccess
    with type t = Provider_context.t
     and type class_t = Shallow_decl_defs.shallow_class = struct
  type t = Provider_context.t

  type class_t = Shallow_decl_defs.shallow_class

  let get_tcopt = Provider_context.get_tcopt

  let get_typedef = Decl_provider_internals.get_typedef_without_pessimise

  let get_class = Decl_provider_internals.get_shallow_class

  let get_class_or_typedef ctx x =
    if is_typedef ctx x then
      match get_typedef ctx x |> Decl_entry.to_option with
      | None -> None
      | Some td -> Some (Decl_enforceability.TypedefResult td)
    else
      match get_class ctx x with
      | None -> None
      | Some cd -> Some (Decl_enforceability.ClassResult cd)

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
      | Tapply ((_, class_name), _) -> get_class ctx class_name
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

  let get_tparams sc = sc.Shallow_decl_defs.sc_tparams

  let is_final sc = sc.Shallow_decl_defs.sc_final

  let get_name cd = snd cd.Shallow_decl_defs.sc_name

  let get_enum_type sc = sc.Shallow_decl_defs.sc_enum_type
end

include Decl_enforceability.Enforce (ShallowContextAccess)

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

type fun_kind =
  | Function
  | Abstract_method
  | Concrete_method

let update_return_ty ft ty = { ft with ft_ret = ty }

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
        make_like_type
          ~reason:(Reason.Rpessimised_inout fp.fp_pos)
          ~intersect_with:None
          ~return_from_async:false
          fp.fp_type;
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
      {
        ft with
        ft_tparams =
          Decl_enforceability.add_supportdyn_constraints p ft.ft_tparams;
      }
    in
    if no_auto_likes then
      mk (get_reason ty, Tfun ft)
    else
      let return_from_async = get_ft_async ft in
      let ret_ty = ft.ft_ret in
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
          | Decl_enforceability.Enforced _ -> mk (get_reason ty, Tfun ft)
          | Decl_enforceability.Unenforced enf_ty_opt ->
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
