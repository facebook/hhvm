(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* The local heap
 *)
(*****************************************************************************)
open Utils

type t = string list

(*****************************************************************************)
(* If there is no call to "make", it means that we are only dealing with
* one process, no need to create several data nodes, just use a local
* hashtable. (this is useful for hh_single_type_check for example).
*)
(*****************************************************************************)
let is_local = ref true
let (local_autocomplete: (string, string) Hashtbl.t) = Hashtbl.create 7
let (local_check: (string, string) Hashtbl.t) = Hashtbl.create 7
let (local_h: (string, string) Hashtbl.t ref) = ref local_autocomplete

let autocomplete_mode () =
  local_h := local_autocomplete

let check_mode () =
  local_h := local_check

let find_unsafe x =
  Hashtbl.find !local_h x

let get x = 
  try Some (find_unsafe x) with Not_found -> None
    
let get_batch ks =
  let acc = ref SMap.empty in
  SSet.iter begin fun x ->
    try acc := SMap.add x (Obj.magic (Hashtbl.find !local_h x)) !acc
    with Not_found -> ()
  end ks;
  !acc

let mem x = get x <> None

let add x data = 
  Hashtbl.replace !local_h x data

let remove x = 
  Hashtbl.remove !local_h x

let remove_batch x =
  SSet.iter begin fun x ->
    Hashtbl.remove !local_h x
  end x

(*****************************************************************************)
(* The signature of what we are actually going to expose to the user *)
(*****************************************************************************)        
module type S = sig
  type t

  val add: string -> t -> unit
  val get: string -> t option
  val get_old: string -> t option
  val find_unsafe: string -> t
  val get_batch: SSet.t -> t option SMap.t
  val remove: string -> unit
  val remove_batch: SSet.t -> unit
  val mem: string -> bool
  val invalidate_cache: unit -> unit
  val oldify: SSet.t -> unit
end


(*****************************************************************************)
(* NoCache means no caching, read and write directly *)
(*****************************************************************************)        
module type NoCache_type = functor (Value:Value.Type) -> S with type t = Value.t

module NoCache: NoCache_type = functor(Value:Value.Type) -> struct

  type t = Value.t

  let add x y =
    let x = Prefix.make_key Value.prefix x in
    add x (Obj.magic y)

  let find_unsafe x =
    let x = Prefix.make_key Value.prefix x in
    let data = find_unsafe x in
    (Obj.magic data)

  let get x =
    try Some (find_unsafe x) with Not_found -> None

  let oldify _ = raise Exit
  let get_old _ = None

  let remove x =
    let x = Prefix.make_key Value.prefix x in
    remove x

  let make_key_set xs =
    SSet.fold begin fun x acc -> 
      SSet.add (Prefix.make_key Value.prefix x) acc
    end xs SSet.empty
    
  let remove_batch xs = remove_batch (make_key_set xs)
  let get_batch xs = get_batch (make_key_set xs)

  let mem x =
    let x = Prefix.make_key Value.prefix x in
    try ignore (find_unsafe x); true with Not_found -> false

  let invalidate_cache () =
    (* Since there is no cache, there is not much to do here ... *)
    ()

end

(*****************************************************************************)
(* Same thing but with 4 layers of cache ... Useful for type-checking        *)
(*****************************************************************************)        
module WithCache = NoCache
