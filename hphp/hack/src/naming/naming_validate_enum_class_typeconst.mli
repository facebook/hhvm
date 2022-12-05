(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Raises and error when an enum class with type constants is encountered.

   We're still in the middle of developing type-constants for enum classes,
   so we gate them carefully for now:
   - They must use the feature flag `type_constants_in_enum_class` AND
   - be in a selected list of directories.

   For internal testing, we provide a global "enable" flag to just
  enable them. This is off by default except in hh_single_type_check.
*)
val pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t
