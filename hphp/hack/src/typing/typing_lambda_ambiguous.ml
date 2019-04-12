(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module T = Tast
module Env = Typing_env
module Reason = Typing_reason
module FL = FeatureLogging

let log_anonymous env =
  let found = ref false in
  Env.iter_anonymous env begin fun pos ftys ->
    let n = List.length ftys in
    if n > 0 then found := true;
    if GlobalOptions.tco_language_feature_logging (Env.get_tcopt env)
    then Measure.sample (FL.Lambda.unknown_params_with_uses n) 1.0;
    if TypecheckerOptions.disallow_ambiguous_lambda (Env.get_tcopt env) && n > 0
    then Errors.ambiguous_lambda pos
      (List.map ftys (fun fty ->
        (Reason.to_pos (fst fty), Typing_print.full_strip_ns env fty)))
    else ()
  end;
  !found

let make_suggest_ty tys =
  (Reason.Rnone, Tabstract(AKnewtype ("HackSuggest", tys), None))

let visitor = object
  inherit Tast_visitor.endo as super
  method! on_expr env x =
    let on_fun pos id f =
      let ftys = Tast_env.get_anonymous_lambda_types env id in
      if List.is_empty ftys
      then None
      else
      let newty = make_suggest_ty ftys in
      (* If we have a single type, propagate types onto parameters *)
      let newty, f = match ftys with
        | [(_, Tfun { ft_params; _ })] ->
          let rec add_types tfun_params efun_params =
            match tfun_params, efun_params with
            | _, [] -> []
            | { fp_type; _ } :: tfun_params,
              ({ Tast.param_hint = None; Tast.param_annotation = (pos, _); _ } as param)::params ->
              { param with Tast.param_annotation = (pos, make_suggest_ty [fp_type]) }
                :: add_types tfun_params params
            | _ :: tfun_params, param :: params ->
              param :: add_types tfun_params params
            | [], _ -> efun_params
            in
          let f_params = add_types ft_params f.Tast.f_params in
          (pos, newty), {f with Tast.f_params = f_params }
        | _ -> (pos, newty), f in
      Some (newty, f) in
    let result =
      match x with
      | (pos, (_p, Tanon(_anon_arity, id))), Tast.Efun (f, ids) ->
        begin match on_fun pos id f with
        | None -> x
        | Some (newty, f) -> newty, Tast.Efun (f, ids)
        end
      | (pos, (_p, Tanon(_anon_arity, id))), Tast.Lfun (f, ids) ->
        begin match on_fun pos id f with
        | None -> x
        | Some (newty, f) -> newty, Tast.Lfun (f, ids)
        end
      | _ ->
        x
    in
    super#on_expr env result
end

(* Add HackSuggest types to legacy lambdas *)
let suggest_fun_def env tast =
  let found_anon = log_anonymous env in
  if found_anon
  then
    let tastenv = Tast_env.typing_env_as_tast_env env in
    visitor#on_fun_ tastenv tast
  else tast

let suggest_method_def env tast =
  let found_anon = log_anonymous env in
  if found_anon
  then
    let tastenv = Tast_env.typing_env_as_tast_env env in
    visitor#on_method_ tastenv tast
  else tast
