(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val pass :
  ( Naming_phase_env.t,
    Naming_phase_error.err Naming_phase_error.Free_monoid.t )
  Naming_phase_pass.t
