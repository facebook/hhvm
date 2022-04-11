(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type size = int

module type Entry = Cache_sig.Entry

module Cache (Entry : Entry) :
  Cache_sig.Cache_intf
    with type 'a key := 'a Entry.key
     and type 'a value := 'a Entry.value
