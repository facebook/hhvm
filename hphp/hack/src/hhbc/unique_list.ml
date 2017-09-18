(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module WithValue(Value: Set.OrderedType) = struct
  module ValueSet = Set.Make (Value)
  type t = {
    unique_set : ValueSet.t;
    unique_list : Value.t list
  }

  let empty = { unique_set = ValueSet.empty; unique_list = [] }

  let add uniq_set item =
    if ValueSet.mem item uniq_set.unique_set then
      uniq_set
    else
      {
        unique_set = (ValueSet.add item uniq_set.unique_set);
        unique_list = item :: uniq_set.unique_list
      }

  let items uniq_set =
    List.rev uniq_set.unique_list

  let items_set uniq_set =
    uniq_set.unique_set

  let remove item uniq_set =
    if ValueSet.mem item uniq_set.unique_set then
      {
        unique_set = (ValueSet.remove item uniq_set.unique_set);
        unique_list = List.filter (fun a -> a <> item) uniq_set.unique_list
      }
    else
      uniq_set

  let diff s1 s2 =
    ValueSet.fold remove s2.unique_set s1

  let cardinal uniq_set = ValueSet.cardinal uniq_set.unique_set
end
