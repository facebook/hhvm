(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs_core
open Typing_env_types
module Env = Typing_env
module SN = Naming_special_names

let check_local_capability (mk_required : env -> env * locl_ty) callback env =
  (* gate the check behavior on coeffects TC option *)
  if TypecheckerOptions.local_coeffects (Env.get_tcopt env) then
    let available = Env.get_local env Typing_coeffects.local_capability_id in
    let (env, required) = mk_required env in
    Typing_subtype.sub_type_or_fail env available required (fun () ->
        callback available required)
  else
    env

let enforce_local_capability
    ?((* Run-time enforced ops must have the default as it's unfixmeable *)
    err_code = Error_codes.Typing.err_code Error_codes.Typing.OpCoeffects)
    (mk_required : env -> env * locl_ty)
    (op : string)
    (op_pos : Pos.t)
    env =
  check_local_capability
    mk_required
    (fun available required ->
      Errors.op_coeffect_error
        ~locally_available:(Typing_print.coeffects env available)
        ~available_pos:(Typing_defs.get_pos available)
        ~required:(Typing_print.coeffects env required)
        ~err_code
        op
        op_pos)
    env

module Capabilities = struct
  module Reason = Typing_reason
  include SN.Capabilities

  let mk special_name env =
    let r = Reason.Rnone in
    Typing_make_type.apply r (Reason.to_pos r, special_name) []
    |> Typing_phase.localize_with_self ~ignore_errors:true env
end

let enforce_static_property_access =
  enforce_local_capability
    Capabilities.(mk accessStaticVariable)
    "Static property access"

let enforce_io =
  enforce_local_capability Capabilities.(mk io) "`echo` or `print` builtin"

let enforce_rx_is_enabled =
  enforce_local_capability Capabilities.(mk rx) ("`" ^ SN.Rx.is_enabled ^ "`")

let enforce_awaitable_immediately_awaited =
  enforce_local_capability
    Capabilities.(mk accessStaticVariable)
    "Not immediately `await`ing `Awaitable`-typed values"

let enforce_enum_class_variant =
  enforce_local_capability
    Capabilities.(mk writeProperty)
    "Accessing an enum class constant"

(* basic local mutability checks *)
let rec is_byval_collection_or_string_or_any_type env ty =
  let check ty =
    let open Aast in
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Toption inner -> is_byval_collection_or_string_or_any_type env inner
    | Tclass ((_, x), _, _) ->
      String.equal x SN.Collections.cVec
      || String.equal x SN.Collections.cDict
      || String.equal x SN.Collections.cKeyset
    | Tvarray _
    | Tdarray _
    | Tvarray_or_darray _
    | Tvec_or_dict _
    | Ttuple _
    | Tshape _
    | Tprim Tstring
    | Tdynamic
    | Tany _ ->
      true
    | Tunion tyl ->
      List.for_all tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tintersection tyl ->
      List.exists tyl ~f:(is_byval_collection_or_string_or_any_type env)
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      (* FIXME we should probably look at the upper bounds here. *)
      false
    | Terr
    | Tnonnull
    | Tprim _
    | Tobject
    | Tfun _
    | Tvar _
    | Taccess _ ->
      false
    | Tunapplied_alias _ ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
  in
  let (_, tl) = Typing_utils.get_concrete_supertypes env ty in
  List.for_all tl ~f:check

let mutating_this_in_ctor env e : bool =
  match snd e with
  | Aast.This when Typing_env.fun_is_constructor env -> true
  | _ -> false

let rec is_valid_mutable_subscript_expression_target env v =
  let open Aast in
  match v with
  | ((_, ty), Lvar _) -> is_byval_collection_or_string_or_any_type env ty
  | ((_, ty), Array_get (e, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && is_valid_mutable_subscript_expression_target env e
  | ((_, ty), Obj_get (e, _, _, _)) ->
    is_byval_collection_or_string_or_any_type env ty
    && ( is_valid_mutable_subscript_expression_target env e
       || mutating_this_in_ctor env e )
  | _ -> false

let is_valid_append_target _env ty =
  (* TODO(coeffects) move this to typing.ml and completely redesign
     the line below (as it was in Tast_check) doesn't do the right thing *)
  (* let (_env, ty) = Env.expand_type _env ty in *)
  match get_node ty with
  | Tclass ((_, n), _, []) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
    && String.( <> ) n SN.Collections.cMap
  | Tclass ((_, n), _, [_]) ->
    String.( <> ) n SN.Collections.cVector
    && String.( <> ) n SN.Collections.cSet
  | Tclass ((_, n), _, [_; _]) -> String.( <> ) n SN.Collections.cMap
  | _ -> true

let check_assignment_or_unset_target
    ~is_assignment
    ?append_pos_opt
    env
    te1
    capability_available
    capability_required =
  let fail =
    Errors.op_coeffect_error
      ~locally_available:(Typing_print.coeffects env capability_available)
      ~available_pos:(Typing_defs.get_pos capability_available)
      ~required:(Typing_print.coeffects env capability_required)
      ~err_code:(Error_codes.Typing.err_code Error_codes.Typing.OpCoeffects)
  in
  let ((p, _), _) = te1 in
  let open Aast in
  match snd te1 with
  | Obj_get (e1, _, _, _) when mutating_this_in_ctor env e1 -> ()
  | Array_get (((_, ty1), _), i)
    when is_assignment && not (is_valid_append_target env ty1) ->
    let is_append = Option.is_none i in
    let msg_prefix =
      if is_append then
        "Appending to a Hack Collection object"
      else
        "Assigning to an element of a Hack Collection object via `[]`"
    in
    fail msg_prefix (Option.value append_pos_opt ~default:p)
  | Array_get (e1, _) when is_valid_mutable_subscript_expression_target env e1
    ->
    ()
  (* we already report errors about statics in rx context, no need to do it twice *)
  | Class_get _ -> ()
  | Obj_get _
  | Array_get _ ->
    if is_assignment then
      fail
        ( "This object's property is being mutated (used as an lvalue)"
        ^ "\nSetting non-mutable object properties" )
        p
    else
      fail "Non-mutable argument for `unset`" p
  | _ -> ()

(* END logic from Typed AST check in basic_reactivity_check *)

let check_assignment env ((append_pos_opt, _), te_) =
  let open Ast_defs in
  match te_ with
  | Aast.Unop ((Uincr | Udecr | Upincr | Updecr), te1)
  | Aast.Binop (Eq _, te1, _) ->
    check_local_capability
      Capabilities.(mk writeProperty)
      (check_assignment_or_unset_target
         ~is_assignment:true
         ~append_pos_opt
         env
         te1)
      env
  | _ -> env

let check_unset_target env tel =
  check_local_capability
    Capabilities.(mk writeProperty)
    (fun available required ->
      List.iter tel ~f:(fun te ->
          check_assignment_or_unset_target
            ~is_assignment:false
            env
            te
            available
            required))
    env
