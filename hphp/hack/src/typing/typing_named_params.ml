(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type mismatch = {
  extra_names: string list;
  missing_names: string list;
  too_optional_names: string list;
}

let find_names_mismatch
    ~(actual_ft : _ Typing_defs.fun_type)
    ~(expected_ft : _ Typing_defs.fun_type) : mismatch =
  let named_params_of (ft : _ Typing_defs.fun_type) =
    List.fold ~init:SMap.empty ft.ft_params ~f:(fun acc fp ->
        match Typing_defs.Named_params.name_of_named_param fp with
        | Some name -> SMap.add name fp acc
        | None -> acc)
  in
  let actual_named_params = named_params_of actual_ft in
  let expected_named_params = named_params_of expected_ft in
  let extra_names =
    SMap.fold
      (fun name _actual_fp acc ->
        if not (SMap.mem name expected_named_params) then
          name :: acc
        else
          acc)
      actual_named_params
      []
  in
  SMap.fold
    (fun name expected_fp acc ->
      match SMap.find_opt name actual_named_params with
      | Some actual_fp ->
        let expected_is_optional =
          Typing_defs_core.get_fp_is_optional expected_fp
        in
        let actual_is_required =
          not (Typing_defs_core.get_fp_is_optional actual_fp)
        in
        if expected_is_optional && actual_is_required then
          { acc with too_optional_names = name :: acc.too_optional_names }
        else
          acc
      | None -> { acc with missing_names = name :: acc.missing_names })
    expected_named_params
    { extra_names; missing_names = []; too_optional_names = [] }
