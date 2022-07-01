(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Cls = Decl_provider.Class
module Env = Typing_env

let get_enforcement (env : env) (ty : decl_ty) : Typing_defs.enforcement =
  let enable_sound_dynamic =
    TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
  in
  (* hack to avoid yet another flag, just for data gathering for pessimisation *)
  let mixed_nonnull_unenforced = TypecheckerOptions.like_casts env.genv.tcopt in
  let rec enforcement include_dynamic env visited ty =
    match get_node ty with
    | Tthis -> Unenforced
    (* Enums are only enforced at their underlying type, in contrast to as and is
     * tests which check for validity of values *)
    | Tapply ((_, name), _) when Env.is_enum env name -> Unenforced
    | Tapply ((_, name), tyl) ->
      (* Cyclic type definition error will be produced elsewhere *)
      if SSet.mem name visited then
        Enforced
      else begin
        match Env.get_class_or_typedef env name with
        | Some
            (Env.TypedefResult
              { td_vis = Aast.Transparent; td_tparams; td_type; _ }) ->
          (* So that the check does not collide with reified generics *)
          let env = Env.add_generic_parameters env td_tparams in
          enforcement include_dynamic env (SSet.add name visited) td_type
        | Some (Env.ClassResult tc) ->
          begin
            match tyl with
            | [] -> Enforced
            | targs ->
              List.Or_unequal_lengths.(
                begin
                  match
                    List.fold2
                      ~init:Enforced
                      targs
                      (Cls.tparams tc)
                      ~f:(fun acc targ tparam ->
                        match get_node targ with
                        | Tdynamic
                        (* We accept the inner type being dynamic regardless of reification *)
                        | Tlike _
                          when not enable_sound_dynamic ->
                          acc
                        | _ ->
                          (match tparam.tp_reified with
                          | Aast.Erased -> Unenforced
                          | Aast.SoftReified -> Unenforced
                          | Aast.Reified ->
                            (match acc with
                            | Enforced -> enforcement true env visited targ
                            | Unenforced -> Unenforced)))
                  with
                  | Ok new_acc -> new_acc
                  | Unequal_lengths -> Enforced
                end)
          end
        | _ -> Unenforced
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
      Unenforced
    | Trefinement _ -> Unenforced
    | Taccess _ -> Unenforced
    | Tlike ty when enable_sound_dynamic ->
      enforcement include_dynamic env visited ty
    | Tlike _ -> Unenforced
    | Tprim prim ->
      begin
        match prim with
        | Aast.Tvoid
        | Aast.Tnoreturn ->
          Unenforced
        | _ -> Enforced
      end
    | Tany _ -> Enforced
    | Terr -> Enforced
    | Tnonnull ->
      if mixed_nonnull_unenforced then
        Unenforced
      else
        Enforced
    | Tdynamic ->
      if (not enable_sound_dynamic) || include_dynamic then
        Enforced
      else
        Unenforced
    | Tfun _ -> Unenforced
    | Ttuple _ -> Unenforced
    | Tunion [] -> Enforced
    | Tunion _ -> Unenforced
    | Tintersection _ -> Unenforced
    | Tshape _ -> Unenforced
    | Tmixed ->
      if mixed_nonnull_unenforced then
        Unenforced
      else
        Enforced
    | Tvar _ -> Unenforced
    (* With no parameters, we enforce varray_or_darray just like array *)
    | Tvec_or_dict (_, ty) ->
      if is_any ty then
        Enforced
      else
        Unenforced
    | Toption ty -> enforcement include_dynamic env visited ty
  in
  enforcement false env SSet.empty ty

let is_enforceable (env : env) (ty : decl_ty) =
  match get_enforcement env ty with
  | Enforced -> true
  | Unenforced -> false

(* We don't trust that hhvm will enforce things consistent with the .hhi file,
   outside of hsl *)
let unenforced_hhi pos_or_decl =
  match Pos_or_decl.get_raw_pos_or_decl_reference pos_or_decl with
  | `Decl_ref _ -> false
  | `Raw p ->
    let path = Pos.filename p in
    Relative_path.(is_hhi (prefix path))
    &&
    let suffix = Relative_path.suffix path in
    not
      (String.is_prefix suffix ~prefix:"hsl_generated/"
      || String.is_prefix suffix ~prefix:"hsl/")

let get_enforced env ~explicitly_untrusted ty =
  if explicitly_untrusted || unenforced_hhi (get_pos ty) then
    Unenforced
  else
    get_enforcement env ty

let compute_enforced_ty env ?(explicitly_untrusted = false) (ty : decl_ty) =
  let et_enforced = get_enforced env ~explicitly_untrusted ty in
  { et_type = ty; et_enforced }

let compute_enforced_and_pessimize_ty
    env ?(explicitly_untrusted = false) (ty : decl_ty) =
  compute_enforced_ty env ~explicitly_untrusted ty

let handle_awaitable_return env ft_fun_kind (ft_ret : decl_possibly_enforced_ty)
    =
  let { et_type = return_type; _ } = ft_ret in
  match (ft_fun_kind, get_node return_type) with
  | (Ast_defs.FAsync, Tapply ((_, name), [inner_ty]))
    when String.equal name Naming_special_names.Classes.cAwaitable ->
    let { et_enforced; _ } = compute_enforced_ty env inner_ty in
    { et_type = return_type; et_enforced }
  | _ -> compute_enforced_and_pessimize_ty env return_type

let compute_enforced_and_pessimize_fun_type env (ft : decl_fun_type) =
  let { ft_params; ft_ret; _ } = ft in
  let ft_fun_kind = get_ft_fun_kind ft in
  let ft_ret = handle_awaitable_return env ft_fun_kind ft_ret in
  let ft_params =
    List.map
      ~f:(fun fp ->
        let { fp_type = { et_type; _ }; _ } = fp in
        let f =
          if equal_param_mode (get_fp_mode fp) FPinout then
            compute_enforced_and_pessimize_ty
          else
            compute_enforced_ty
        in
        let fp_type = f env et_type in
        { fp with fp_type })
      ft_params
  in
  { ft with ft_params; ft_ret }

let compute_enforced_ty = compute_enforced_ty ?explicitly_untrusted:None
