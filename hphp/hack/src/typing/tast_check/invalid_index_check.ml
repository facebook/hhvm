(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module TCO = TypecheckerOptions
module MakeType = Typing_make_type
module SN = Naming_special_names
open String.Replace_polymorphic_compare

let should_enforce env = TCO.disallow_invalid_arraykey (Env.get_tcopt env)

(* For new-inference, types of keys in collection types may not be resolved
 * until TAST check time. For this reason, we replicate some of the checks that
 * are applied in Typing_array_access.array_get here. Specifically, those
 * checks that involve the key type from a dictionary-like container, checked
 * against the type of the index expression. For example,
 * the container might have a type dict<k,t> and the index expression might
 * have a type k'. We want k' <: k.
 *)
let rec array_get ~array_pos ~expr_pos ~index_pos env array_ty index_ty =
  let type_index env ty_have ty_expect reason =
    let t_env = Env.tast_env_as_typing_env env in
    match
      Typing_coercion.try_coerce t_env ty_have (MakeType.unenforced ty_expect)
    with
    | Some _ -> ()
    | None ->
      if
        ( Env.can_subtype env ty_have (MakeType.dynamic (get_reason ty_have))
        (* Terrible heuristic to agree with legacy: if we inferred `nothing` for
         * the key type of the array, just let it pass *)
        || Env.can_subtype
             env
             ty_expect
             (MakeType.nothing (get_reason ty_expect)) )
        (* If the key is not even an arraykey, we've already produced an error *)
        || (not (Env.can_subtype env ty_have (MakeType.arraykey Reason.Rnone)))
           && should_enforce env
      then
        ()
      else
        let (_, ty_have) = Env.expand_type env ty_have in
        let (_, ty_expect) = Env.expand_type env ty_expect in
        let ty_expect_str = Env.print_error_ty env ty_expect in
        let ty_have_str = Env.print_error_ty env ty_have in
        Errors.index_type_mismatch
          (expr_pos, Reason.string_of_ureason reason)
          ( Typing_reason.to_string
              ("This is " ^ ty_expect_str)
              (get_reason ty_expect)
          @ Typing_reason.to_string
              ("It is incompatible with " ^ ty_have_str)
              (get_reason ty_have) )
  in
  let (_, ety) = Env.expand_type env array_ty in
  match get_node ety with
  | Tunion tyl ->
    List.iter tyl ~f:(fun ty ->
        array_get ~array_pos ~expr_pos ~index_pos env ty index_ty)
  | Tdarray (key_ty, _) -> type_index env index_ty key_ty Reason.index_array
  | Tclass ((_, cn), _, key_ty :: _)
    when cn = SN.Collections.cMap
         || cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cAnyArray
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cKeyset ->
    type_index env index_ty key_ty (Reason.index_class cn)
  | Toption array_ty ->
    array_get ~array_pos ~expr_pos ~index_pos env array_ty index_ty
  | Tnonnull
  | Tprim _
  | Tfun _
  | Tvar _
  | Tclass _
  | Ttuple _
  | Tshape _
  | Tobject
  | Terr
  | Tdynamic
  | Tany _
  | Tvarray _
  | Tvarray_or_darray _
  | Tnewtype _
  | Tdependent _
  | _ ->
    ()

let index_visitor =
  object (this)
    inherit [_] Tast_visitor.iter_with_state as super

    (* Distinguish between lvalue and rvalue array indexing.
     * Suppose $x : dict<string,int>
     * We want to check rvalue e.g. $y = $x[3], or $z[$x[3]] = 5
     * But not lvalue e.g. $x[3] = 5 or list ($x[3], $w) = e;
     *)
    method! on_expr (env, is_lvalue) (((p, _), expr) as e) =
      match expr with
      | Array_get ((((p1, ty1), _) as e1), Some (((p2, ty2), _) as e2)) ->
        if not is_lvalue then
          array_get ~array_pos:p1 ~expr_pos:p ~index_pos:p2 env ty1 ty2;
        this#on_expr (env, false) e1;
        this#on_expr (env, false) e2
      | Binop (Ast_defs.Eq _, e1, e2) ->
        this#on_expr (env, true) e1;
        this#on_expr (env, false) e2
      | List el -> List.iter ~f:(this#on_expr (env, is_lvalue)) el
      | _ -> super#on_expr (env, is_lvalue) e
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env = index_visitor#on_fun_def (env, false)

    method! at_method_ env = index_visitor#on_method_ (env, false)
  end
