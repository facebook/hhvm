(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We put type definitions that we'll export into a separate module
to keep oxidized happy *)
type 'p pos =
  | Precomputed of 'p
  | Classish_end_of_body of string
  | Classish_start_of_body of string
[@@deriving eq, ord, show]

(** Positional information for a single class *)
type 'p classish_positions = {
  classish_start_of_body: 'p;
  classish_end_of_body: 'p;
}

(** Positional information for a collection of classes *)
type 'p t = 'p classish_positions SMap.t
