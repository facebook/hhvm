(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-40"]

module Eval = Custom_error_eval
module Ty = Typing_defs_core
module V = Validated

let mk_ty ty_ = Ty.mk (Typing_reason.Rnone, ty_)

(* Pattern match over a `Violated_constraint` error matching exactly the tparam
   name for which the constraint is violated *)
let%test_unit "patt string exactly" =
  let open Patt_error in
  let open Patt_locl_ty in
  let name = "Tviolated" in
  let pod_none = Pos_or_decl.none in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
        ty_sup = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in
  (* Matches [Apply_reasons] error with any callback, applied to
     [Violated_constraint] secondary error with contrained tparam `Tviolated` *)
  let patt =
    Apply_reasons
      ( Any_reasons_callback,
        Violated_constraint (Name (Patt_string.Exactly name), Any, Any) )
  in
  let error_message = Error_message.{ message = [Lit "Ok"] } in
  let custom_err = Custom_error.{ name; patt; error_message } in
  let custom_config = Custom_error_config.Config [custom_err] in
  let open Core in
  [%test_eq: (string, Eval.Value.t) Either.t list list]
    (Eval.eval custom_config ~err)
    [[Either.First "Ok"]]

(* Pattern match over a `Violated_constraint` error matching exactly the tparam
   name for which the constraint is violated and binding that name to a variable
   for use in the error message *)
let%test_unit "patt string exactly bound" =
  let open Patt_error in
  let open Patt_locl_ty in
  let name = "Tviolated" in
  let pod_none = Pos_or_decl.none in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
        ty_sup = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in
  (* As above but bind the tparam name to `x` *)
  let patt =
    Apply_reasons
      ( Any_reasons_callback,
        Violated_constraint
          (As { lbl = "x"; patt = Patt_string.Exactly name }, Any, Any) )
  in
  let error_message = Error_message.{ message = [Lit "Ok:"; Name_var "x"] } in
  let custom_err = Custom_error.{ name; patt; error_message } in
  let custom_config = Custom_error_config.Config [custom_err] in
  let open Core in
  [%test_eq: (string, Eval.Value.t) Either.t list list]
    (Eval.eval custom_config ~err)
    [Either.[First "Ok:"; Second (Eval.Value.Name (pod_none, name))]]

(* Pattern match over the [tysub] contained in a `Violated_constraint` error;
   the type match requires an exact match on the class name and for it to
   have exactly one type param. The type param must be a shape containing
   a field name "a" which can be of any type *)
let%test_unit "patt tysub" =
  let open Patt_error in
  let open Patt_locl_ty in
  let param_name = "Tviolated" in
  let class_name = "Classy" in
  let pod_none = Pos_or_decl.none in
  let shp =
    Ty.(
      Tshape
        ( Missing_origin,
          mk_ty Tdynamic,
          TShapeMap.of_list
            [
              ( TSFlit_str (pod_none, "c"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ( TSFlit_str (pod_none, "a"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ( TSFlit_str (pod_none, "b"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
            ] ))
  in
  let ty_locl_sub =
    Ty.Tclass ((pod_none, class_name), Ty.nonexact, [mk_ty shp])
  in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, param_name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.Rnone, ty_locl_sub));
        ty_sup = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in

  (* Matches a class with exactly one parameter where that param has a shape
     type with one field named 'a' which can have any type *)
  let patt_ty_sub =
    Patt_locl_ty.Apply
      {
        patt_name = Patt_name.Name (Patt_string.Exactly class_name);
        patt_params =
          Cons
            {
              patt_hd =
                Shape
                  (Fld
                     {
                       patt_fld =
                         {
                           lbl = StrLbl "a";
                           optional = false;
                           patt = As { lbl = "x"; patt = Any };
                         };
                       patt_rest = Open;
                     });
              patt_tl = Nil;
            };
      }
  in
  (* Match the subtype in our error message *)
  let patt =
    Apply_reasons
      ( Any_reasons_callback,
        Violated_constraint (Patt_name.Wildcard, patt_ty_sub, Any) )
  in
  let error_message = Error_message.{ message = [Lit "Ok:"; Ty_var "x"] } in
  let custom_err = Custom_error.{ name = "patt tysub"; patt; error_message } in
  let custom_config = Custom_error_config.Config [custom_err] in
  let open Core in
  [%test_eq: (string, Eval.Value.t) Either.t list list]
    (Eval.eval custom_config ~err)
    [Either.[First "Ok:"; Second (Eval.Value.Ty (mk_ty Ty.Tdynamic))]]

(* Pattern match over the [tysub] contained in a `Violated_constraint` error;
   the type match requires an exact match on the class name and for it to
   have exactly one type param. The type parameter may _either_ be an
   arraykey (which we bind to `x`) or a shape with a field named `a` (whose
   type we bind to `x`) *)
let%test_unit "patt tysub or pattern" =
  let open Patt_error in
  let open Patt_locl_ty in
  let param_name = "Tviolated" in
  let class_name = "Classy" in
  let pod_none = Pos_or_decl.none in
  let shp =
    Ty.(
      Tshape
        ( Missing_origin,
          mk_ty Tdynamic,
          TShapeMap.of_list
            [
              ( TSFlit_str (pod_none, "c"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ( TSFlit_str (pod_none, "a"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ( TSFlit_str (pod_none, "b"),
                { sft_optional = false; sft_ty = mk_ty Tdynamic } );
            ] ))
  in
  let ty_locl_sub =
    Ty.Tclass ((pod_none, class_name), Ty.nonexact, [mk_ty shp])
  in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, param_name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.Rnone, ty_locl_sub));
        ty_sup = LoclType (Ty.mk (Typing_reason.Rnone, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in

  (* Matches a class with exactly one parameter where that param has a shape
     type with one field named 'a' which can have any type *)
  let patt_ty_sub =
    Patt_locl_ty.Apply
      {
        patt_name = Patt_name.Name (Patt_string.Exactly class_name);
        patt_params =
          Cons
            {
              patt_hd =
                Or
                  {
                    patt_fst = As { lbl = "x"; patt = Prim Arraykey };
                    patt_snd =
                      Shape
                        (Fld
                           {
                             patt_fld =
                               {
                                 lbl = StrLbl "a";
                                 optional = false;
                                 patt = As { lbl = "x"; patt = Any };
                               };
                             patt_rest = Open;
                           });
                  };
              patt_tl = Nil;
            };
      }
  in
  (* Match the subtype in our error message *)
  let patt =
    Apply_reasons
      ( Any_reasons_callback,
        Violated_constraint (Patt_name.Wildcard, patt_ty_sub, Any) )
  in
  let error_message = Error_message.{ message = [Lit "Ok:"; Ty_var "x"] } in
  let custom_err = Custom_error.{ name = "patt tysub"; patt; error_message } in
  let custom_config = Custom_error_config.Config [custom_err] in
  let open Core in
  [%test_eq: (string, Eval.Value.t) Either.t list list]
    (Eval.eval custom_config ~err)
    [Either.[First "Ok:"; Second (Eval.Value.Ty (mk_ty Ty.Tdynamic))]]

let%test_unit "validate ty, unbound right" =
  let patt_fst = Patt_locl_ty.(As { lbl = "x"; patt = Dynamic }) in
  let patt_snd = Patt_locl_ty.Nonnull in
  let patt_snd_invalid =
    Patt_locl_ty.(Invalid ([Validation_err.Unbound "x"], patt_snd))
  in
  let patt = Patt_locl_ty.Or { patt_fst; patt_snd } in
  let patt_invalid =
    Patt_locl_ty.Or { patt_fst; patt_snd = patt_snd_invalid }
  in
  let open Core in
  [%test_eq: Patt_locl_ty.t V.t]
    (fst @@ Patt_locl_ty.validate patt)
    (V.Invalid patt_invalid)

let%test_unit "validate ty, unbound left" =
  let patt_snd = Patt_locl_ty.(As { lbl = "x"; patt = Dynamic }) in
  let patt_fst = Patt_locl_ty.Nonnull in
  let patt_fst_invalid =
    Patt_locl_ty.(Invalid ([Validation_err.Unbound "x"], patt_fst))
  in
  let patt = Patt_locl_ty.Or { patt_fst; patt_snd } in
  let patt_invalid =
    Patt_locl_ty.Or { patt_fst = patt_fst_invalid; patt_snd }
  in
  let open Core in
  [%test_eq: Patt_locl_ty.t V.t]
    (fst @@ Patt_locl_ty.validate patt)
    (V.Invalid patt_invalid)

let%test_unit "validate ty, shadow" =
  let patt_name = Patt_name.As { lbl = "x"; patt = Exactly "Classy" } in
  let patt_hd = Patt_locl_ty.As { lbl = "x"; patt = Dynamic } in
  let patt_hd_invalid =
    Patt_locl_ty.Invalid ([Validation_err.Shadowed "x"], patt_hd)
  in
  let patt_tl = Patt_locl_ty.Nil in
  let patt =
    Patt_locl_ty.(Apply { patt_name; patt_params = Cons { patt_hd; patt_tl } })
  in
  let patt_invalid =
    Patt_locl_ty.(
      Apply
        { patt_name; patt_params = Cons { patt_hd = patt_hd_invalid; patt_tl } })
  in
  let open Core in
  [%test_eq: Patt_locl_ty.t V.t]
    (fst @@ Patt_locl_ty.validate patt)
    (V.Invalid patt_invalid)

let%test_unit "validate ty, mismatch" =
  let patt_name = Patt_name.As { lbl = "x"; patt = Exactly "Classy" } in
  let patt_hd = Patt_locl_ty.Dynamic in
  let patt_tl = Patt_locl_ty.Nil in
  let patt_fst =
    Patt_locl_ty.(Apply { patt_name; patt_params = Cons { patt_hd; patt_tl } })
  in
  let patt_snd = Patt_locl_ty.As { lbl = "x"; patt = Dynamic } in
  let patt_snd_invalid =
    Patt_locl_ty.(Invalid ([Validation_err.Mismatch (Name, Ty)], patt_snd))
  in
  let patt = Patt_locl_ty.Or { patt_fst; patt_snd } in
  let patt_invalid =
    Patt_locl_ty.Or { patt_fst; patt_snd = patt_snd_invalid }
  in
  let open Core in
  [%test_eq: Patt_locl_ty.t V.t]
    (fst @@ Patt_locl_ty.validate patt)
    (V.Invalid patt_invalid)
