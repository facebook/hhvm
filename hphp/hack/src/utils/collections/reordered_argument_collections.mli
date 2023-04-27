(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Map_S = Reordered_argument_collections_sig.Map_S

module Reordered_argument_map (S : WrappedMap.S) :
  Map_S with type key = S.key with type 'a t = 'a S.t

module type Set_S = Reordered_argument_collections_sig.Set_S

module Reordered_argument_set (S : Set.S) :
  Set_S with type elt = S.elt with type t = S.t

module type SSet_S = Reordered_argument_collections_sig.SSet_S

module SSet :
  SSet_S
    with type elt = Reordered_argument_set(SSet).elt
    with type t = Reordered_argument_set(SSet).t

module type SMap_S = Reordered_argument_collections_sig.SMap_S

module SMap :
  SMap_S
    with type key = Reordered_argument_map(SMap).key
    with type 'a t = 'a Reordered_argument_map(SMap).t
