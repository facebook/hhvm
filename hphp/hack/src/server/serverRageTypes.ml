(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** To make the rage output more useful, it's broken into rageItems. *)
type rageItem = {
  title: string;
  data: string;
}

type result = rageItem list
