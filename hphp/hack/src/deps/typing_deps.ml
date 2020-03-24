(*
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
module Dep = struct
  type dependent

  type dependency

  type _ variant =
    | GConst : string -> 'a variant
    | GConstName : string -> 'a variant
    | Const : (string * string) -> dependency variant
    | AllMembers : string -> dependency variant
    | Class : string -> 'a variant
    | RecordDef : string -> 'a variant
    | Fun : string -> 'a variant
    | FunName : string -> 'a variant
    | Prop : (string * string) -> dependency variant
    | SProp : (string * string) -> dependency variant
    | Method : (string * string) -> dependency variant
    | SMethod : (string * string) -> dependency variant
    | Cstr : string -> dependency variant
    | Extends : string -> dependency variant

  (** Explicit [equals] function since [=] doesn't work on the [variant] GADT. *)
  let variant_equals : type a b. a variant -> b variant -> bool =
   fun lhs rhs ->
    match (lhs, rhs) with
    | (GConst lhs, GConst rhs) -> lhs = rhs
    | (Class lhs, Class rhs) -> lhs = rhs
    | (RecordDef lhs, RecordDef rhs) -> lhs = rhs
    | (Fun lhs, Fun rhs) -> lhs = rhs
    | (Const lhs, Const rhs) -> lhs = rhs
    | (Prop lhs, Prop rhs) -> lhs = rhs
    | (SProp lhs, SProp rhs) -> lhs = rhs
    | (Method lhs, Method rhs) -> lhs = rhs
    | (SMethod lhs, SMethod rhs) -> lhs = rhs
    | (GConstName lhs, GConstName rhs) -> lhs = rhs
    | (FunName lhs, FunName rhs) -> lhs = rhs
    | (AllMembers lhs, AllMembers rhs) -> lhs = rhs
    | (Extends lhs, Extends rhs) -> lhs = rhs
    | (GConst _, _) -> false
    | (Class _, _) -> false
    | (RecordDef _, _) -> false
    | (Fun _, _) -> false
    | (Cstr _, _) -> false
    | (Const _, _) -> false
    | (Prop _, _) -> false
    | (SProp _, _) -> false
    | (Method _, _) -> false
    | (SMethod _, _) -> false
    | (GConstName _, _) -> false
    | (FunName _, _) -> false
    | (AllMembers _, _) -> false
    | (Extends _, _) -> false

  type t = int

  (* 30 bits for the hash and 1 bit to determine if something is a class *)
  let mask = (1 lsl 30) - 1

  let make : type a. a variant -> t = function
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

  let compare = ( - )

  let extract_name : type a. a variant -> string = function
    | GConst s -> Utils.strip_ns s
    | GConstName s -> Utils.strip_ns s
    | Const (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Class s -> Utils.strip_ns s
    | RecordDef s -> Utils.strip_ns s
    | Fun s -> Utils.strip_ns s
    | FunName s -> Utils.strip_ns s
    | Prop (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | SProp (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Method (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | SMethod (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Cstr s -> Utils.strip_ns s
    | AllMembers s -> Utils.strip_ns s
    | Extends s -> Utils.strip_ns s

  let to_debug_string = string_of_int

  let of_debug_string = int_of_string

  let variant_to_string : type a. a variant -> string =
   fun dep ->
    let prefix =
      match dep with
      | GConst _ -> "GConst"
      | GConstName _ -> "GConstName"
      | Const _ -> "Const"
      | Class _ -> "Class"
      | RecordDef _ -> "RecordDef"
      | Fun _ -> "Fun"
      | FunName _ -> "FunName"
      | Prop _ -> "Prop"
      | SProp _ -> "SProp"
      | Method _ -> "Method"
      | SMethod _ -> "SMethod"
      | Cstr _ -> "Cstr"
      | AllMembers _ -> "AllMembers"
      | Extends _ -> "Extends"
    in
    prefix ^ " " ^ extract_name dep
end

module DepSet = struct
  include Reordered_argument_set (Set.Make (Dep))

  let pp = make_pp (fun fmt -> Format.fprintf fmt "%d")
end

(****************************************************************************)
(* Module for a compact graph. *)
(* Please consult hh_shared.c for the underlying representation. *)
(****************************************************************************)
module Graph = struct
  external hh_add_dep : int -> unit = "hh_add_dep"

  external hh_get_dep : int -> int list = "hh_get_dep"

  external hh_get_dep_sqlite : int -> int list = "hh_get_dep_sqlite"

  external hh_allow_dependency_table_reads : bool -> bool
    = "hh_allow_dependency_table_reads"

  external hh_assert_allow_dependency_table_reads : unit -> unit
    = "hh_assert_allow_dependency_table_reads"

  let hh_add_dep x = WorkerCancel.with_worker_exit (fun () -> hh_add_dep x)

  let hh_get_dep x = WorkerCancel.with_worker_exit (fun () -> hh_get_dep x)

  let add x y = hh_add_dep ((x lsl 31) lor y)

  let get x =
    hh_assert_allow_dependency_table_reads ();
    let deps = DepSet.empty in
    let deps = List.fold_left ~init:deps ~f:DepSet.add (hh_get_dep x) in
    let deps = List.fold_left ~init:deps ~f:DepSet.add (hh_get_dep_sqlite x) in
    deps
end

(*****************************************************************************)
(* Module keeping track of what object depends on what. *)
(*****************************************************************************)

let allow_dependency_table_reads = Graph.hh_allow_dependency_table_reads

let get_ideps_from_hash x =
  (* FIXME: It shouldn't be necessary to add x to the result set here. We do so
     because historically, we added self-referential edges to the dependency
     graph (e.g., Class "\Foo" -> Class "\Foo"). We no longer do this in order
     to save memory, but we aren't yet confident that these edges were not
     relied upon anywhere. *)
  DepSet.add (Graph.get x) x

let get_ideps x = get_ideps_from_hash (Dep.make x)

let trace = ref true

let dependency_callbacks = Caml.Hashtbl.create 0

let add_dependency_callback cb_name cb =
  Caml.Hashtbl.replace dependency_callbacks cb_name cb

let add_idep
    (root : Dep.dependent Dep.variant) (obj : Dep.dependency Dep.variant) =
  if Dep.variant_equals root obj then
    ()
  else (
    Caml.Hashtbl.iter (fun _ f -> f root obj) dependency_callbacks;
    if !trace then Graph.add (Dep.make obj) (Dep.make root)
  )

(*****************************************************************************)
(* Module keeping track which files contain the toplevel definitions. *)
(*****************************************************************************)

let (ifiles : (Dep.t, Relative_path.Set.t) Hashtbl.t ref) =
  ref (Hashtbl.create 23)

let get_files deps =
  DepSet.fold
    ~f:
      begin
        fun dep acc ->
        try
          let files = Hashtbl.find !ifiles dep in
          Relative_path.Set.union files acc
        with Not_found -> acc
      end
    deps
    ~init:Relative_path.Set.empty

let update_file filename info =
  (* TODO: Figure out if we need GConstName and FunName as well here *)
  let {
    FileInfo.funs;
    classes;
    record_defs;
    typedefs;
    consts;
    comments = _;
    file_mode = _;
    hash = _;
  } =
    info
  in
  let consts =
    List.fold_left
      consts
      ~f:
        begin
          fun acc (_, const_id) ->
          DepSet.add acc (Dep.make (Dep.GConst const_id))
        end
      ~init:DepSet.empty
  in
  let funs =
    List.fold_left
      funs
      ~f:
        begin
          fun acc (_, fun_id) ->
          DepSet.add acc (Dep.make (Dep.Fun fun_id))
        end
      ~init:DepSet.empty
  in
  let classes =
    List.fold_left
      classes
      ~f:
        begin
          fun acc (_, class_id) ->
          DepSet.add acc (Dep.make (Dep.Class class_id))
        end
      ~init:DepSet.empty
  in
  let classes =
    List.fold_left
      record_defs
      ~f:
        begin
          fun acc (_, type_id) ->
          DepSet.add acc (Dep.make (Dep.Class type_id))
        end
      ~init:classes
  in
  let classes =
    List.fold_left
      typedefs
      ~f:
        begin
          fun acc (_, type_id) ->
          DepSet.add acc (Dep.make (Dep.Class type_id))
        end
      ~init:classes
  in
  let defs = DepSet.union funs classes in
  let defs = DepSet.union defs consts in
  DepSet.iter
    ~f:
      begin
        fun def ->
        let previous =
          try Hashtbl.find !ifiles def
          with Not_found -> Relative_path.Set.empty
        in
        Hashtbl.replace !ifiles def (Relative_path.Set.add previous filename)
      end
    defs

let rec get_extend_deps ~visited ~source_class ~acc =
  if DepSet.mem !visited source_class then
    acc
  else (
    visited := DepSet.add !visited source_class;
    let cid_hash = Dep.extends_of_class source_class in
    let ideps = get_ideps_from_hash cid_hash in
    DepSet.fold
      ~f:
        begin
          fun obj acc ->
          if Dep.is_class obj then
            let acc = DepSet.add acc obj in
            get_extend_deps visited obj acc
          else
            acc
        end
      ideps
      ~init:acc
  )

let add_extend_deps deps =
  let trace = ref DepSet.empty in
  DepSet.fold deps ~init:deps ~f:(fun dep acc ->
      if not @@ Dep.is_class dep then
        acc
      else
        get_extend_deps trace dep acc)

let add_typing_deps deps =
  DepSet.fold deps ~init:deps ~f:(fun dep acc ->
      DepSet.union (get_ideps_from_hash dep) acc)

let add_all_deps x = x |> add_extend_deps |> add_typing_deps
