(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

module type S = sig
  type t

  val validate : ?env:Validation_env.t -> t -> t Validated.t * Validation_env.t
end
