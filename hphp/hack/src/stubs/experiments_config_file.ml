(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let update ~(file : string) ~(source : string option) ~(ttl : float) :
    (string, string) result =
  ignore (file, source, ttl);
  Ok "Experiments config update: nothing to do"

let get_primary_owner ~(logged_in_user : string) : string = logged_in_user
