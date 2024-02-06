(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let mk_result env pos ty_actual ty_expected =
  TastHolesService.(
    Set.singleton
      {
        actual_ty_string = Tast_env.print_ty env ty_actual;
        actual_ty_json =
          Hh_json.json_to_string @@ Tast_env.ty_to_json env ty_actual;
        expected_ty_string = Tast_env.print_ty env ty_expected;
        expected_ty_json =
          Hh_json.json_to_string @@ Tast_env.ty_to_json env ty_expected;
        pos;
      })

let filter_source hole_filter src =
  match (hole_filter, src) with
  | (ServerCommandTypes.Tast_hole.Any, _) -> true
  | (ServerCommandTypes.Tast_hole.Typing, Aast.Typing) -> true
  | (ServerCommandTypes.Tast_hole.Cast, Aast.(UnsafeCast _ | EnforcedCast _)) ->
    true
  | _ -> false

let visitor hole_filter =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method private zero = TastHolesService.Set.empty

    method private plus t1 t2 = TastHolesService.Set.union t1 t2

    method! on_Hole env ((_, pos, _) as expr) from_ty to_ty src =
      let acc = super#on_Hole env expr from_ty to_ty src in
      if filter_source hole_filter src then
        self#plus acc (mk_result env pos from_ty to_ty)
      else
        acc
  end

let tast_holes ctx tast hole_src_opt =
  TastHolesService.Set.elements ((visitor hole_src_opt)#go ctx tast)

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(hole_filter : ServerCommandTypes.Tast_hole.filter) =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  tast_holes ctx tast.Tast_with_dynamic.under_normal_assumptions hole_filter
