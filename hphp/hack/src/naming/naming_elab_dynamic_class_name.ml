(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = unit

  let empty = ()
end

let is_dynamic = function
  | Aast.(CIparent | CIself | CIstatic | CI _ | CIexpr (_, _, (Lvar _ | This)))
    ->
    false
  | _ -> true

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    (* TODO[mjt] we have to override `on_expr` since we need the top-level
       expression position in our error cases *)
    method! on_expr env (ex, pos, expr_) =
      let res =
        let open Aast in
        match expr_ with
        | Aast.(New ((_, ci_pos, ci), targs, exprs, expr_opt, ann))
          when is_dynamic ci ->
          let err =
            Err.naming @@ Naming_error.Dynamic_new_in_strict_mode ci_pos
          in
          let class_id = ((), pos, Aast.CI (ci_pos, SN.Classes.cUnknown)) in
          let expr_ = Aast.New (class_id, targs, exprs, expr_opt, ann) in
          Ok (expr_, err)
        (* TODO[mjt] can we decompose this case further? This is considering
           both the class_id and the class_get_expr *)
        | Class_get ((_, _, ci), CGstring _, _prop_or_meth)
          when not @@ is_dynamic ci ->
          Ok (expr_, self#zero)
        | Class_get ((_, _, ci), CGexpr (_, cg_expr_pos, _), Ast_defs.Is_method)
          when not @@ is_dynamic ci ->
          let err =
            Err.naming @@ Naming_error.Dynamic_method_access cg_expr_pos
          in
          Ok (expr_, err)
        | Class_get
            ( (_, _, CIexpr (_, ci_pos, _)),
              (CGstring _ | CGexpr (_, _, (Lvar _ | This))),
              _prop_or_meth ) ->
          let err =
            Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode ci_pos
          in
          Error err
        | Class_get
            ((_, _, CIexpr (_, ci_pos, _)), CGexpr (_, cg_pos, _), _prop_or_meth)
          ->
          let err1 =
            Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode ci_pos
          in
          (* TODO[mjt] this seems like the wrong error? Shouldn't this be
             `Dynamic_method_access` as in the case above? *)
          let err2 =
            Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode cg_pos
          in
          Error (self#plus err1 err2)
        | Class_get (_, Aast.CGexpr (_, cg_pos, _), _prop_or_meth) ->
          let err =
            Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode cg_pos
          in
          Error err
        | Aast.(FunctionPointer (FP_class_const ((_, _, ci), _), _))
          when is_dynamic ci ->
          (* TODO[mjt] report error in strict mode *)
          Error self#zero
        | Aast.Class_const ((_, _, ci), _) when is_dynamic ci ->
          (* TODO[mjt] report error in strict mode *)
          Error self#zero
        | _ -> Ok (expr_, self#zero)
      in
      (* Where we are replacing the entire expression, there is no need
         to proceed with the recursion *)
      match res with
      | Error err -> ((ex, pos, Err.invalid_expr_ pos), err)
      | Ok (expr_, err) ->
        let (expr_, super_err) = super#on_expr_ env expr_ in
        ((ex, pos, expr_), self#plus err super_err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
