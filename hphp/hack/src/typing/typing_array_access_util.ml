(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Aast
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type
module SN = Naming_special_names
open String.Replace_polymorphic_compare

(* Like widen_for_array_get, but for when constraint_can_index is on. *)
let widen_for_array_get_ci
    {
      ci_lhs_of_null_coalesce = lhs_of_null_coalesce;
      ci_index_expr = index_expr;
      ci_expr_pos = expr_pos;
      _;
    }
    env
    ty =
  Typing_log.(
    log_with_level env "typing" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos expr_pos)
          env
          [Log_head ("widen_for_array_get", [Log_type ("ty", ty)])]));
  match deref ty with
  (* The null type is valid only with a null-coalescing use of array get *)
  | (_, Tprim Tnull) when lhs_of_null_coalesce -> (env, Some ty)
  (* dynamic is valid for array get *)
  | (_, Tdynamic) -> (env, Some ty)
  (* All class-based containers, and keyset, vec and dict, are subtypes of
   * some instantiation of KeyedContainer
   *)
  | (r, Tclass ((_, cn), _, _))
    when cn = SN.Collections.cVector
         || cn = SN.Collections.cVec
         || cn = SN.Collections.cMap
         || cn = SN.Collections.cDict
         || cn = SN.Collections.cKeyset
         || cn = SN.Collections.cConstMap
         || cn = SN.Collections.cImmMap
         || cn = SN.Collections.cKeyedContainer
         || cn = SN.Collections.cAnyArray
         || cn = SN.Collections.cConstVector
         || cn = SN.Collections.cImmVector ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    let (env, index_ty) = Env.fresh_type_invariant env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    (env, Some ty)
  (* The same is true of PHP arrays *)
  | (r, Tvec_or_dict _) ->
    let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
    let (env, index_ty) = Env.fresh_type_invariant env expr_pos in
    let ty = MakeType.keyed_container r index_ty element_ty in
    (env, Some ty)
  (* For tuples, we just freshen the element types *)
  | (r, Ttuple { t_required; t_extra = Textra { t_optional; t_variadic } }) ->
  begin
    (* requires integer literal *)
    match index_expr with
    (* Should freshen type variables *)
    | (_, _, Int _) ->
      let (env, t_required) =
        List.map_env env t_required ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      let (env, t_optional) =
        List.map_env env t_optional ~f:(fun env _ty ->
            Env.fresh_type_invariant env expr_pos)
      in
      let (env, t_variadic) =
        if is_nothing t_variadic then
          (env, t_variadic)
        else
          Env.fresh_type_invariant env expr_pos
      in
      ( env,
        Some
          (mk
             ( r,
               Ttuple
                 { t_required; t_extra = Textra { t_optional; t_variadic } } ))
      )
    | _ -> (env, None)
  end
  (* Whatever the lower bound, construct an open, singleton shape type. *)
  | (r, Tshape { s_fields = fdm; s_origin = _; s_unknown_value = _ }) ->
    Typing_shapes.do_with_field_expr env index_expr ~with_error:(env, None)
    @@ fun field_name ->
    (match TShapeMap.find_opt field_name fdm with
    (* If field is in the lower bound but is optional, then no upper bound makes sense
     * unless this is a null-coalesce access *)
    | Some { sft_optional = true; sft_ty = _ } when not lhs_of_null_coalesce ->
      (env, None)
    | _ ->
      let (env, element_ty) = Env.fresh_type_invariant env expr_pos in
      let (env, rest_ty) = Env.fresh_type_invariant env expr_pos in
      let upper_fdm =
        TShapeMap.add
          field_name
          { sft_optional = lhs_of_null_coalesce; sft_ty = element_ty }
          TShapeMap.empty
      in
      let upper_shape_ty = MakeType.shape r rest_ty upper_fdm in
      (env, Some upper_shape_ty))
  | _ -> (env, None)
