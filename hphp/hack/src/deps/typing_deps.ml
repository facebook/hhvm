(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_core
open Reordered_argument_collections
open Utils

(**********************************)
(* Handling dependencies *)
(**********************************)
type debug_trace_type = Bazooka | Full | No_trace
module Dep = struct
  type variant =
    (* GConst is used for "global" constants, in other words,
     * constants that were introduced using "const X = ..." or
     * "define('X', ...)".
     *)
    | GConst of string
    | GConstName of string

    (* Const is used to represent class constants. *)
    | Const of string * string
    (* There is a dependency on all members of a class *)
    | AllMembers of string
    | Class of string
    | Fun of string
    | FunName of string
    | Prop of string * string
    | SProp of string * string
    | Method of string * string
    | SMethod of string * string
    | Cstr of string
    | Extends of string

  type t = int

  (* 30 bits for the hash and 1 bit to determine if something is a class *)
  let mask = 1 lsl 30 - 1

  let make = function
    | Class class_name ->
        let h = Hashtbl.hash class_name land mask in
        let h = h lsl 1 in
        let h = h lor 1 in
        h
    | Extends class_name ->
        let h = Hashtbl.hash class_name land mask in
        let h = h lsl 1 in
        h
    | variant ->
        let h = Hashtbl.hash variant land mask in
        let h = h lsl 1 in
        h

  let is_class x = x land 1 = 1
  let extends_of_class x = x lxor 1

  let compare = (-)

  let to_string = function
    | GConst s -> "GConst "^s
    | GConstName s -> "GConstName "^s
    | Const (cls, s) -> spf "Const %s::%s" cls s
    | Class s -> "Class "^s
    | Fun s -> "Fun "^s
    | FunName s -> "FunName "^s
    | Prop (cls, s) -> spf "Prop %s::%s" cls s
    | SProp (cls, s) -> spf "SProp %s::%s" cls s
    | Method (cls, s) -> spf "Method %s::%s" cls s
    | SMethod (cls, s) -> spf "SMethod %s::%s" cls s
    | Cstr s -> "Cstr "^s
    | AllMembers s -> "AllMembers "^s
    | Extends s -> "Extends "^s

end

module DepSet = Reordered_argument_set(Set.Make (Dep))

(****************************************************************************)
(* Module for a compact graph. *)
(* Please consult hh_shared.c for the underlying representation. *)
(****************************************************************************)
module Graph = struct
  external hh_add_dep: int -> unit     = "hh_add_dep"
  external hh_get_dep: int -> int list = "hh_get_dep"
  external hh_get_dep_sqlite: int -> int list = "hh_get_dep_sqlite"
  external hh_allow_dependency_table_reads : bool -> bool
    = "hh_allow_dependency_table_reads"
  external hh_assert_allow_dependency_table_reads : unit -> unit
    = "hh_assert_allow_dependency_table_reads"

  let hh_add_dep x =
    WorkerCancel.with_worker_exit (fun () -> hh_add_dep x)

  let hh_get_dep x =
    WorkerCancel.with_worker_exit (fun () -> hh_get_dep x)

  let add x y = hh_add_dep ((x lsl 31) lor y)

  let union_deps l1 l2 = List.dedup (List.append l1 l2)

  let get x =
    hh_assert_allow_dependency_table_reads ();
    let l = union_deps (hh_get_dep x) (hh_get_dep_sqlite x) in
    List.fold_left l ~f:begin fun acc node ->
      DepSet.add acc node
    end ~init:DepSet.empty
end

(*****************************************************************************)
(* Module keeping track of what object depends on what. *)
(*****************************************************************************)

let allow_dependency_table_reads = Graph.hh_allow_dependency_table_reads

let get_ideps_from_hash x =
  Graph.get x

let get_ideps x =
  Graph.get (Dep.make x)

let to_bazooka x =
  match x with
  | Dep.AllMembers cid
  | Dep.Const (cid, _)
  | Dep.Prop (cid, _)
  | Dep.SProp (cid, _)
  | Dep.Method (cid, _)
  | Dep.Cstr cid
  | Dep.SMethod (cid, _)
  | Dep.Extends cid
  | Dep.Class cid -> Dep.Class cid
  | x -> x

let simplify x =
  let x = to_bazooka x in
  (* Get rid of FunName and GConstName *)
  match x with
  | Dep.FunName f -> Dep.Fun f
  | Dep.GConstName g -> Dep.GConst g
  | _ -> x

(* Gets ALL the dependencies ... hence the name *)
let get_bazooka x =
  get_ideps (to_bazooka x)

let trace = ref true
(* Instead of actually recording the dependencies in shared memory, we record
 * string representations of them for printing out *)
let debug_trace = ref No_trace
let dbg_dep_set = HashSet.create 0

let add_idep root obj =
  if !trace then Graph.add (Dep.make obj) (Dep.make root);
  (* Note: this is the inverse of what is actually stored in the shared
   * memory table. I find it easier to read "X depends on Y" instead of
   * "Y is a dependent of X" *)
  match !debug_trace with
  | Full ->
    HashSet.add dbg_dep_set
      ((Dep.to_string root) ^ " -> " ^ (Dep.to_string obj))
  | Bazooka ->
    let root = simplify root in
    let obj = simplify obj in
    if root = obj then () else
    HashSet.add dbg_dep_set
      ((Dep.to_string root) ^ " -> " ^ (Dep.to_string obj))
  | No_trace -> ()

let print_string_hash_set set =
  let xs = HashSet.fold (fun x xs -> x :: xs) set [] in
  let xs = List.sort String.compare xs in
  List.iter xs print_endline

let dump_debug_deps () = print_string_hash_set dbg_dep_set



(*****************************************************************************)
(* Module keeping track which files contain the toplevel definitions. *)
(*****************************************************************************)

let (ifiles: (Dep.t, Relative_path.Set.t) Hashtbl.t ref) = ref (Hashtbl.create 23)

let get_files deps =
  DepSet.fold ~f:begin fun dep acc ->
    try
      let files = Hashtbl.find !ifiles dep in
      Relative_path.Set.union files acc
    with Not_found -> acc
  end deps ~init:Relative_path.Set.empty

let update_files fileInfo =
  (* TODO: Figure out if we need GConstName and FunName as well here *)
  Relative_path.Map.iter fileInfo begin fun filename info ->
    let {FileInfo.funs; classes; typedefs;
         consts;
         comments = _;
         file_mode = _;
         hash = _;
        } = info in
    let consts = List.fold_left consts ~f: begin fun acc (_, const_id) ->
      DepSet.add acc (Dep.make (Dep.GConst const_id))
    end ~init:DepSet.empty in
    let funs = List.fold_left funs ~f:begin fun acc (_, fun_id) ->
      DepSet.add acc (Dep.make (Dep.Fun fun_id))
    end ~init:DepSet.empty in
    let classes = List.fold_left classes ~f:begin fun acc (_, class_id) ->
      DepSet.add acc (Dep.make (Dep.Class class_id))
    end ~init:DepSet.empty in
    let classes = List.fold_left typedefs ~f:begin fun acc (_, type_id) ->
      DepSet.add acc (Dep.make (Dep.Class type_id))
    end ~init:classes in
    let defs = DepSet.union funs classes in
    let defs = DepSet.union defs consts in
    DepSet.iter ~f:begin fun def ->
      let previous =
        try Hashtbl.find !ifiles def with Not_found -> Relative_path.Set.empty
      in
      Hashtbl.replace !ifiles def (Relative_path.Set.add previous filename)
    end defs
  end

let rec get_extend_deps ~visited ~source_class ~acc =
  if DepSet.mem !visited source_class
  then acc
  else begin
    visited := DepSet.add !visited source_class;
    let cid_hash = Dep.extends_of_class source_class in
    let ideps = get_ideps_from_hash cid_hash in
    DepSet.fold ~f:begin fun obj acc ->
      if Dep.is_class obj
      then
        let acc = DepSet.add acc obj in
        get_extend_deps visited obj acc
      else acc
    end ideps ~init:acc
  end

let add_extend_deps deps =
  let trace = ref DepSet.empty in
  DepSet.fold deps
    ~init:deps
    ~f:begin fun dep acc ->
      if not @@ Dep.is_class dep then acc else
      get_extend_deps trace dep acc
    end

let add_typing_deps deps =
  DepSet.fold deps
    ~init:deps
    ~f:begin fun dep acc ->
      DepSet.union (get_ideps_from_hash dep) acc
    end

let add_all_deps x =  x |> add_extend_deps |> add_typing_deps
