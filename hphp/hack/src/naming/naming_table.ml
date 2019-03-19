(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = FileInfo.t Relative_path.Map.t
type fast = FileInfo.names Relative_path.Map.t
type saved_state_info = FileInfo.saved Relative_path.Map.t


(*****************************************************************************)
(* Forward naming table functions *)
(*****************************************************************************)


let combine a b = Relative_path.Map.union a b
let create a = a
let empty = Relative_path.Map.empty
let filter = Relative_path.Map.filter
let fold = Relative_path.Map.fold
let get_files = Relative_path.Map.keys
let get_file_info = Relative_path.Map.get
let get_file_info_unsafe = Relative_path.Map.find_unsafe
let has_file = Relative_path.Map.mem
let iter = Relative_path.Map.iter
let update a key data = Relative_path.Map.add a ~key ~data


(*****************************************************************************)
(* Conversion functions *)
(*****************************************************************************)


let from_saved saved =
  Relative_path.Map.fold saved ~init:Relative_path.Map.empty ~f:(fun fn saved acc ->
    let file_info = FileInfo.from_saved fn saved in
    Relative_path.Map.add acc fn file_info
  )

let to_saved a =
  Relative_path.Map.map a FileInfo.to_saved

let to_fast a =
  Relative_path.Map.map a FileInfo.simplify

let saved_to_fast saved =
  Relative_path.Map.map saved FileInfo.saved_to_names


(*****************************************************************************)
(* Reverse naming table functions *)
(*****************************************************************************)


(* The canon name (and assorted *Canon heaps) store the canonical name for a
   symbol, keyed off of the lowercase version of its name. We use the canon
   heaps to check for symbols which are redefined using different
   capitalizations so we can throw proper Hack errors for them. *)
let canon_name = String.lowercase_ascii
let canonize_set = SSet.map canon_name
let check_valid key pos =
  if FileInfo.get_pos_filename pos = Relative_path.default then begin
    Hh_logger.log
      ("WARNING: setting canonical position of %s to be in dummy file. If this \
      happens in incremental mode, things will likely break later.") key;
    Hh_logger.log "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100));
  end

module type ReverseNamingTable = sig
  type pos

  val add : string -> pos -> unit
  val get_pos : ?bypass_cache:bool -> string -> pos option
  val remove_batch : SSet.t -> unit

  val heap_string_of_key : string -> string
end

(* The Types module records both class names and typedefs since they live in the
* same namespace. That is, one cannot both define a class Foo and a typedef Foo
* (or FOO or fOo, due to case insensitivity). *)
type type_of_type =
  | TClass
  | TTypedef
module Types = struct
  type pos = FileInfo.pos * type_of_type

  module TypeCanonHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = string
      let prefix = Prefix.make()
      let description = "TypeCanon"
    end)

  module TypePosHeap = SharedMem.WithCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make ()
      let description = "TypePos"
    end)

  let add id type_info =
    if not @@ TypePosHeap.LocalChanges.has_local_changes () then check_valid id (fst type_info);
    TypeCanonHeap.add (canon_name id) id;
    TypePosHeap.write_around id type_info

  let get_pos ?(bypass_cache=false) id =
    if bypass_cache
    then TypePosHeap.get_no_cache id
    else TypePosHeap.get id

  let get_canon_name id =
    TypeCanonHeap.get id

  let remove_batch types =
    TypeCanonHeap.remove_batch (canonize_set types);
    TypePosHeap.remove_batch types

  let heap_string_of_key = TypePosHeap.string_of_key
end

module Funs = struct
  type pos = FileInfo.pos

  module FunCanonHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = string
      let prefix = Prefix.make()
      let description = "FunCanon"
    end)

  module FunPosHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make()
      let description = "FunPos"
    end)

  let add id pos =
    if not @@ FunPosHeap.LocalChanges.has_local_changes () then check_valid id pos;
    FunCanonHeap.add (canon_name id) id;
    FunPosHeap.add id pos

  let get_pos ?bypass_cache:(_=false) id =
    FunPosHeap.get id

  let get_canon_name name =
    FunCanonHeap.get name

  let remove_batch funs =
    FunCanonHeap.remove_batch (canonize_set funs);
    FunPosHeap.remove_batch funs

  let heap_string_of_key = FunPosHeap.string_of_key
end

module Consts = struct
  type pos = FileInfo.pos

  module ConstPosHeap = SharedMem.NoCache
    (SharedMem.ProfiledImmediate)
    (StringKey)
    (struct
      type t = pos
      let prefix = Prefix.make()
      let description = "ConstPos"
    end)

  let add id pos =
    if not @@ ConstPosHeap.LocalChanges.has_local_changes () then check_valid id pos;
    ConstPosHeap.add id pos

  let get_pos ?bypass_cache:(_=false) id =
    ConstPosHeap.get id

  let remove_batch consts =
    ConstPosHeap.remove_batch consts

  let heap_string_of_key = ConstPosHeap.string_of_key
end

let push_local_changes () =
  Types.TypePosHeap.LocalChanges.push_stack ();
  Types.TypeCanonHeap.LocalChanges.push_stack ();
  Funs.FunPosHeap.LocalChanges.push_stack ();
  Funs.FunCanonHeap.LocalChanges.push_stack ();
  Consts.ConstPosHeap.LocalChanges.push_stack ()

let pop_local_changes () =
  Types.TypePosHeap.LocalChanges.pop_stack ();
  Types.TypeCanonHeap.LocalChanges.pop_stack ();
  Funs.FunPosHeap.LocalChanges.pop_stack ();
  Funs.FunCanonHeap.LocalChanges.pop_stack ();
  Consts.ConstPosHeap.LocalChanges.pop_stack ()

let has_local_changes () =
  Types.TypePosHeap.LocalChanges.has_local_changes ()
  || Types.TypeCanonHeap.LocalChanges.has_local_changes ()
  || Funs.FunPosHeap.LocalChanges.has_local_changes ()
  || Funs.FunCanonHeap.LocalChanges.has_local_changes ()
  || Consts.ConstPosHeap.LocalChanges.has_local_changes ()
