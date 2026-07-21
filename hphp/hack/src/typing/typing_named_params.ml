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
  missing_named_variadic: bool;
      (** True when the expected (super) function type has a named-variadic
          parameter but the actual (sub) function type does not. In that case
          the consumer of the expected type may pass arbitrary named args,
          which the actual function cannot absorb. *)
}

let find_names_mismatch
    ~(actual_ft : _ Typing_defs.fun_type)
    ~(expected_ft : _ Typing_defs.fun_type) : mismatch =
  (* Build the name → param map, EXCLUDING the named-variadic entry.
     A named-variadic represents "accepts any name of this element type",
     not a required named parameter, so it must not participate in
     missing/extra-name matching. *)
  let named_params_of (ft : _ Typing_defs.fun_type) =
    List.fold
      ~init:SMap.empty
      (Typing_defs.ft_params_without_named_variadic ft)
      ~f:(fun acc fp ->
        match Typing_defs.Named_params.name_of_named_param fp with
        | Some name -> SMap.add name fp acc
        | None -> acc)
  in
  let actual_named_params = named_params_of actual_ft in
  let expected_named_params = named_params_of expected_ft in
  let actual_has_named_variadic =
    Option.is_some (Typing_defs.ft_named_variadic_param actual_ft)
  in
  let expected_has_named_variadic =
    Option.is_some (Typing_defs.ft_named_variadic_param expected_ft)
  in
  let extra_names =
    SMap.fold
      (fun name actual_fp acc ->
        (* function(optional named bool $b): void   <:   function(): void *)
        let is_expected = SMap.mem name expected_named_params in
        let is_required = not (Typing_defs_core.get_fp_is_optional actual_fp) in
        if is_required && not is_expected then
          name :: acc
        else
          acc)
      actual_named_params
      []
  in
  let init =
    {
      extra_names;
      missing_names = [];
      too_optional_names = [];
      missing_named_variadic =
        expected_has_named_variadic && not actual_has_named_variadic;
    }
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
      | None ->
        (* If the actual function has a named-variadic, it absorbs any name
           the caller of the expected function might send. The element-type
           compatibility check happens separately during parameter subtyping. *)
        if actual_has_named_variadic then
          acc
        else
          { acc with missing_names = name :: acc.missing_names })
    expected_named_params
    init
