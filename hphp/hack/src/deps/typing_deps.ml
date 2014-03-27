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

(**********************************)
(* Handling dependencies *)
(**********************************)

module Dep = struct
  type variant =
    (* GConst is used for "global" constants, in other words,
     * constants that were introduced using "const X = ..." or
     * "define('X', ...)".
     *)
    | GConst of string

    (* Const is used to represent class constants. *)
    | Const of string * string

    | Class of string
    | Fun of string
    | FunName of string
    | CVar of string * string
    | SCVar of string * string
    | Method of string * string
    | SMethod of string * string
    | Cstr of string
    | Extends of string
    | Injectable

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

  let compare x y = x - y

end

(*****************************************************************************)
(* Module keeping track of what object depends on what. *)
(*****************************************************************************)
module Graph = Typing_graph

let trace = ref true

let add_idep root obj =
  if !trace
  then
    let root = 
      match root with
      | None -> assert false
      | Some x -> x
    in
    Graph.add (Dep.make obj) (Dep.make root)
  else ()

let get_ideps_from_hash x =
  Graph.get x

let get_ideps x =
  Graph.get (Dep.make x)

(* Gets ALL the dependencies ... hence the name *)
let get_bazooka x =
  match x with
  | Dep.GConst cid
  | Dep.Const (cid, _)
  | Dep.CVar (cid, _)
  | Dep.SCVar (cid, _)
  | Dep.Method (cid, _)
  | Dep.Cstr cid
  | Dep.SMethod (cid, _)
  | Dep.Extends cid
  | Dep.Class cid -> get_ideps (Dep.Class cid)
  | Dep.Fun fid -> get_ideps (Dep.Fun fid)
  | Dep.FunName fid -> get_ideps (Dep.FunName fid)
  | Dep.Injectable -> ISet.empty

(*****************************************************************************)
(* Module keeping track which files contain the toplevel definitions. *)
(*****************************************************************************)

let (ifiles: (int, SSet.t) Hashtbl.t) = Hashtbl.create 23

let get_files deps =
  ISet.fold begin fun dep acc ->
    try 
      let files = Hashtbl.find ifiles dep in
      SSet.union files acc
    with Not_found -> acc
  end deps SSet.empty

let update_files workers fast =
  SMap.iter begin fun filename info ->
    let {FileInfo.funs; classes; types; 
         consts = _ (* TODO probably a bug #3844332 *);
         comments = _;
        } = info in
    let funs = List.fold_left begin fun acc (_, fun_id) ->
      ISet.add (Dep.make (Dep.Fun fun_id)) acc
    end ISet.empty funs in
    let classes = List.fold_left begin fun acc (_, class_id) ->
      ISet.add (Dep.make (Dep.Class class_id)) acc
    end ISet.empty classes in
    let classes = List.fold_left begin fun acc (_, type_id) ->
      ISet.add (Dep.make (Dep.Class type_id)) acc
    end classes types in
    let defs = ISet.union funs classes in
    ISet.iter begin fun def ->
      let previous =
        try Hashtbl.find ifiles def with Not_found -> SSet.empty
      in
      Hashtbl.replace ifiles def (SSet.add filename previous)
    end defs
  end fast 

