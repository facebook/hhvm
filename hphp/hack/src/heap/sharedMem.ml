(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils

(*****************************************************************************)
(* Initializes the shared memory. Must be called before forking. *)
(*****************************************************************************)
external init: unit -> unit = "hh_shared_init"

(*****************************************************************************)
(* The shared memory garbage collector. It must be called every time we
 * free data (cf hh_shared.c for the underlying C implementation).
 *)
(*****************************************************************************)
external collect: unit -> unit = "hh_collect"

(*****************************************************************************)
(* Must be called after the initialization of the hack server is over.
 * (cf serverInit.ml).
 *)
(*****************************************************************************)
external init_done: unit -> unit = "hh_call_after_init"

(*****************************************************************************)
(* Module returning the MD5 of the key. It's because the code in C land
 * expects this format. I prefer to make it an abstract type to make sure
 * we don't forget to give the MD5 instead of the key itself.
 *)
(*****************************************************************************)

module Key: sig

  (* The type of keys *)
  type t

  (* The type of old keys *)
  type old

  (* The md5 of an old or a new key *)
  type md5

  (* Creation/conversion primitives *)
  val make     : Prefix.t -> string -> t
  val make_old : Prefix.t -> string -> old
  val to_old   : t -> old
  val to_new   : old -> t

  (* Md5 primitives *)
  val md5     : t -> md5
  val md5_old : old -> md5

end = struct
  type t   = string
  type old = string
  type md5 = string

  (* The prefix we use for old keys. The prefix garantees that we never
   * mix old and new data, because a key can never start with the prefix
   * "old_", it always starts with a number (cf Prefix.make()).
   *)
  let old_prefix = "old_"

  let make = Prefix.make_key
  let make_old prefix x = old_prefix^Prefix.make_key prefix x

  let to_old x = old_prefix^x

  let to_new x = 
    let module S = String in
    S.sub x (S.length old_prefix) (S.length x - S.length old_prefix)

  let md5 = Digest.string
  let md5_old = Digest.string

end

(*****************************************************************************)
(* Functor for serialized values (to make sure we don't mix strings
 * with serialized strings).
 *)
(*****************************************************************************)

module Serial: functor(Value:Value.Type) -> sig
  type t

  val make : Value.t -> t
  val get  : t -> Value.t

end = functor(Value:Value.Type) -> struct
  type t = string

  let make x = Marshal.to_string x []
  let get x  = Marshal.from_string x 0
end


(*****************************************************************************)
(* Raw interface to shared memory (cf hh_shared.c for the underlying
 * representation).
 *)
(*****************************************************************************)
module Raw: functor(Value: sig type t end) -> sig

  val hh_shared_init : unit -> unit
  val hh_add         : Key.md5 -> Value.t -> unit
  val hh_mem         : Key.md5 -> bool
  val hh_get         : Key.md5 -> Value.t
  val hh_remove      : Key.md5 -> unit
  val hh_move        : Key.md5 -> Key.md5 -> unit

end = functor(Value: sig type t end) -> struct

  external hh_shared_init : unit -> unit               = "hh_shared_init"
  external hh_add         : Key.md5 -> Value.t -> unit = "hh_add"
  external hh_mem         : Key.md5 -> bool            = "hh_mem"
  external hh_get         : Key.md5 -> Value.t         = "hh_get"
  external hh_remove      : Key.md5 -> unit            = "hh_remove"
  external hh_move        : Key.md5 -> Key.md5 -> unit = "hh_move"
      
end

(*****************************************************************************)
(* Module used to access "new" values (as opposed to old ones).
 * There are several cases where we need to compare the old and the new
 * representation of objects (to determine what has changed).
 * The "old" representation is the value that was bound to that key in the
 * last round of type-checking.
 * Despite the fact that the same storage is used under the hood, it's good
 * to seperate the two interfaces to make sure we never mix old and new
 * values.
 *)
(*****************************************************************************)

module New: functor(Value: Value.Type) -> sig
  
  (* Adds a binding to the table, the table is left unchanged in the
   * key was already bound.
   *)
  val add         : Key.t -> Value.t -> unit

  val get         : Key.t -> Value.t option
  val find_unsafe : Key.t -> Value.t
  val remove      : Key.t -> unit
  val mem         : Key.t -> bool

  (* Binds the key to the old one.
   * If 'mykey' is bound to 'myvalue', oldifying 'mykey' makes 'mykey'
   * accessible to the "Old" module, in other words: "Old.mem mykey" returns
   * true and "New.mem mykey" returns false after oldifying.
   *)
  val oldify      : Key.t -> unit

end = functor(Value: Value.Type) -> struct

  module Data      = Serial(Value)
  module Raw = Raw(Data)

  let add key value = Raw.hh_add (Key.md5 key) (Data.make value)
  let mem key = Raw.hh_mem (Key.md5 key)

  let get key =
    let key = Key.md5 key in
    if Raw.hh_mem key
    then Some (Data.get (Raw.hh_get key))
    else None

  let find_unsafe key = 
    match get key with 
    | None -> raise Not_found
    | Some x -> x

  let remove key =
    let key = Key.md5 key in
    if Raw.hh_mem key
    then begin 
      Raw.hh_remove key;
      assert (not (Raw.hh_mem key));
    end
    else ()

  let oldify key =
    if mem key
    then
      let old_key = Key.to_old key in
      Raw.hh_move (Key.md5 key) (Key.md5_old old_key)
    else ()
end

(* Same as new, but for old values *)
module Old: functor(Value: Value.Type) -> sig

  val get         : Key.old -> Value.t option
  val find_unsafe : Key.old -> Value.t
  val remove      : Key.old -> unit
  val mem         : Key.old -> bool

  (* Takes an old value and moves it back to a "new" one 
   * (useful for auto-complete).
   *)
  val revive      : Key.old -> unit
end = functor(Value: Value.Type) -> struct

  module Data = Serial(Value)
  module Raw = Raw(Data)

  let get key =
    let key = Key.md5_old key in
    if Raw.hh_mem key
    then Some (Data.get (Raw.hh_get key))
    else None

  let mem key = Raw.hh_mem (Key.md5_old key)

  let remove key = 
    if mem key
    then Raw.hh_remove (Key.md5_old key)

  let find_unsafe key = 
    match get key with 
    | None -> raise Not_found
    | Some x -> x

  let revive key =
    if mem key
    then
      let new_key = Key.to_new key in
      let new_key = Key.md5 new_key in
      let old_key = Key.md5_old key in
      if Raw.hh_mem new_key
      then Raw.hh_remove new_key;
      Raw.hh_move old_key new_key
end

(*****************************************************************************)
(* The signature of what we are actually going to expose to the user *)
(*****************************************************************************)

module type S = sig
  type t

  val add              : string -> t -> unit
  val get              : string -> t option
  val get_old          : string -> t option
  val get_old_batch    : SSet.t -> t option SMap.t
  val remove_old_batch : SSet.t -> unit
  val find_unsafe      : string -> t
  val get_batch        : SSet.t -> t option SMap.t
  val remove_batch     : SSet.t -> unit
  val mem              : string -> bool
  val oldify_batch     : SSet.t -> unit
  val revive_batch     : SSet.t -> unit
end

(*****************************************************************************)
(* A functor returning an implementation of the S module without caching. *)
(*****************************************************************************)        

module NoCache = functor(Value:Value.Type) -> struct
  module New = New(Value)
  module Old = Old(Value)

  type t = Value.t

  let add x y = New.add (Key.make Value.prefix x) y
  let find_unsafe x = New.find_unsafe (Key.make Value.prefix x)

  let get x =
    try Some (find_unsafe x) with Not_found -> None

  let get_old x =
    let key = Key.make_old Value.prefix x in
    Old.get key

  let get_old_batch xs = 
    SSet.fold begin fun str_key acc ->
      let key = Key.make_old Value.prefix str_key in
      SMap.add str_key (Old.get key) acc
    end xs SMap.empty

  let remove_batch xs = 
    SSet.iter begin fun str_key ->
      let key = Key.make Value.prefix str_key in
      New.remove key
    end xs

  let oldify_batch xs =
    SSet.iter begin fun str_key ->
      let key = Key.make Value.prefix str_key in
      New.oldify key
    end xs

  let revive_batch xs =
    SSet.iter begin fun str_key ->
      let key = Key.make_old Value.prefix str_key in
      Old.revive key
    end xs

  let get_batch xs = 
    SSet.fold begin fun str_key acc ->
      let key = Key.make Value.prefix str_key in
      match New.get key with
      | None -> SMap.add str_key None acc
      | Some data -> SMap.add str_key (Some data) acc
    end xs SMap.empty

  let mem x = New.mem (Key.make Value.prefix x)

  let remove_old_batch xs =
    SSet.iter begin fun str_key ->
      let key = Key.make_old Value.prefix str_key in
      Old.remove key
    end xs

end

(*****************************************************************************)
(* All the cache are configured by a module of type ConfigType *)
(*****************************************************************************)

module type ConfigType = sig

(* The type of object we want to keep in cache *)
  type value         

(* The capacity of the cache *)
  val capacity : int

end

(*****************************************************************************)
(* All the caches are functors returning a module of the following signature
 *)
(*****************************************************************************)
      
module type CacheType = sig
  type value

  val add: Key.t -> value -> unit
  val get: Key.t -> value option
  val find: Key.t -> value
  val remove: Key.t -> unit
  val clear: unit -> unit
end


(*****************************************************************************)
(* Cache keeping the objects the most frequently used. *)
(*****************************************************************************)
      
module FreqCache = functor(Config:ConfigType) -> struct

  type value = Config.value

(* The cache itself *)
  let (cache: (Key.t, int ref * Config.value) Hashtbl.t)
      = Hashtbl.create (2 * Config.capacity)
  let size = ref 0

  let clear() = 
    Hashtbl.clear cache;
    size := 0
 

(* The collection function is called when we reach twice original capacity
 * in size. When the collection is triggered, we only keep the most recent
 * object.
 * So before collection: size = 2 * capacity
 * After collection: size = capacity (with the most recent objects)
 *)
  let collect() =
    if !size < 2 * Config.capacity then () else
    let l = ref [] in
    Hashtbl.iter begin fun key (freq, v) ->
      l := (key, !freq, v) :: !l
    end cache;
    Hashtbl.clear cache;
    l := List.sort (fun (_, x, _) (_, y, _) -> y - x) !l;
    let i = ref 0 in
    while !i < Config.capacity do
      match !l with
      | [] -> i := Config.capacity
      | (k, freq, v) :: rl ->
          Hashtbl.replace cache k (ref 0, v);
          l := rl;
          incr i;
    done;
    size := Config.capacity;
    ()

  let add x y =
    collect();
    try 
      let freq, y' = Hashtbl.find cache x in
      incr freq;
      if y' == y
      then ()
      else Hashtbl.replace cache x (freq, y)
    with Not_found ->
      incr size;
      let elt = ref 0, y in
      Hashtbl.replace cache x elt;
      ()

  let find x =
    let freq, value = Hashtbl.find cache x in
    incr freq;
    value

  let get x = try Some (find x) with Not_found -> None

  let remove x =
    if Hashtbl.mem cache x
    then decr size;
    Hashtbl.remove cache x
      
end

(*****************************************************************************)
(* An ordered cache keeps the most recently used objects *)
(*****************************************************************************)

module OrderedCache = functor(Config:ConfigType) -> struct
  
  type value = Config.value

  let iorder = ref 0
  let (cache: (Key.t, Config.value) Hashtbl.t) = 
    Hashtbl.create Config.capacity

  let queue = Queue.create()
  let size = ref 0

  let clear() =
    Hashtbl.clear cache;
    size := 0;
    Queue.clear queue;
    ()
      
  let rec add x y =
    if !size < Config.capacity
    then begin
      incr size;
      let () = Queue.push x queue in
      ()
    end
    else begin
      let elt = Queue.pop queue in
      Hashtbl.remove cache elt;
      Queue.push x queue;
      Hashtbl.replace cache x y
    end

  let find x = Hashtbl.find cache x
  let get x = try Some (find x) with Not_found -> None

  let remove x =
    try 
      if Hashtbl.mem cache x
      then decr size;
      Hashtbl.remove cache x;
    with Not_found -> ()

end

(*****************************************************************************)
(* Every time we create a new cache, a function that knows how to clear the
 * cache is registered in the "invalidate_callback_list" global.
 *)
(*****************************************************************************)

let invalidate_callback_list = ref []
let invalidate_caches () =
  List.iter begin fun callback -> callback() end !invalidate_callback_list

(*****************************************************************************)
(* A functor returning an implementation of the S module with caching.
 * We need to avoid constantly deserializing types, because it costs us too
 * much time. The caches keep a deserialized version of the types.
 *)
(*****************************************************************************)        

module WithCache = functor(Value:Value.Type) -> struct
  type t = Value.t

  module ConfValue = struct
    type value = Value.t
    let capacity = 1000
  end

  (* Young values cache *)
  module L1 = OrderedCache(ConfValue)
  (* Frequent values cache *)
  module L2 = FreqCache(ConfValue)

  module New = New(Value)
  module Old = Old(Value)

  module Direct = NoCache(Value)

  let add x y = 
    let x = Key.make Value.prefix x in
    L1.add x y;
    L2.add x y;
    New.add x y

  let force_get = function
    | Some x -> x
    | None -> raise Not_found
        
  let get x = 
    let x = Key.make Value.prefix x in
    match L1.get x with
    | None ->
        (match L2.get x with
        | None ->
            (match New.get x with
            | None ->
                None
            | Some v as result ->
                L1.add x v;
                L2.add x v;
                result
            )
        | Some v as result ->
            L1.add x v;
            result
        )
    | Some v as result ->
        L2.add x v;
        result

  (* We don't cache old objects, they are not accessed often enough. *)
  let get_old = Direct.get_old
  let get_old_batch = Direct.get_old_batch

  let find_unsafe x =
    match get x with
    | None -> raise Not_found
    | Some x -> x

  let mem x = 
    match get x with
    | None -> false
    | Some _ -> true
    
  let remove x = 
    let x = Key.make Value.prefix x in
    L1.remove x;
    L2.remove x;
    New.remove x

  let get_batch keys =
    SSet.fold begin fun key acc ->
      SMap.add key (get key) acc
    end keys SMap.empty

  let oldify_batch keys =
    Direct.oldify_batch keys;
    SSet.iter begin fun key ->
      let key = Key.make Value.prefix key in
      L1.remove key;
      L2.remove key;
    end keys

  let revive_batch = Direct.revive_batch

  let remove_batch xs = SSet.iter remove xs

  let () =
    invalidate_callback_list := begin fun () ->
      L1.clear();
      L2.clear();
    end :: !invalidate_callback_list

  let remove_old x = Old.remove (Key.make_old Value.prefix x)
  let remove_old_batch = SSet.iter remove_old

end

