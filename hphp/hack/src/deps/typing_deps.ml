(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Format = Stdlib.Format
module Hashtbl = Stdlib.Hashtbl
module Mode = Typing_deps_mode
open Typing_deps_mode
open Utils

let worker_id : int option ref = ref None

(******************************************)
(* Handling dependencies and their hashes *)
(******************************************)
module Dep = struct
  type dependent

  type dependency

  (** NOTE: keep in sync with `typing_deps_hash.rs`. *)
  type _ variant =
    | GConst : string -> 'a variant
    | Fun : string -> 'a variant
    | Type : string -> 'a variant
    | Extends : string -> dependency variant
    | Const : string * string -> dependency variant
    | Constructor : string -> dependency variant
    | Prop : string * string -> dependency variant
    | SProp : string * string -> dependency variant
    | Method : string * string -> dependency variant
    | SMethod : string * string -> dependency variant
    | AllMembers : string -> dependency variant
    | GConstName : string -> 'a variant

  (** NOTE: keep in sync with `typing_deps_hash.rs`. *)
  type dep_kind =
    | KGConst [@value 0]
    | KFun [@value 1]
    | KType [@value 2]
    | KExtends [@value 3]
    | KConst [@value 5]
    | KConstructor [@value 6]
    | KProp [@value 7]
    | KSProp [@value 8]
    | KMethod [@value 9]
    | KSMethod [@value 10]
    | KAllMembers [@value 11]
    | KGConstName [@value 12]
  [@@deriving enum]

  external hash1 : int -> string -> int = "hash1_ocaml" [@@noalloc]

  external hash2 : int -> int -> string -> int = "hash2_ocaml" [@@noalloc]

  type t = int

  let to_int64 (t : t) : int64 =
    (* The [t] is 63 bits of data followed by a trailing bit 0.
       Since we're storing it in Dep.t, an ocaml int stored in two's complement, the first
       of those 63 bits is considered a sign bit. The trailing bit 0 is because ocaml reserves
       the trailing bit 0 to indicate that it's stored locally, not a reference. *)
    let a = Int64.of_int t in
    (* [a] is an Int64 version of that int, which unlike the built-in int is always stored
       as a reference on the heap and hence does not need to reserve a leading bit and hence
       uses all 64bits. The way two's complement works, when promoting from 63bit to 64bit,
       is that the extra new leading bit becomes a copy of the previous leading bit.
       e.g. decimal=-3, 7bit=111_1101, 8bit=1111_1101
       e.g. decimal=+3, 7bit=000_0011, 8bit=0000_0011
       If that's confusing, think of counting down from 2^bits, e.g.
       -1 is 2^7 - 1 = 111_1111, or equivalently 2^8 - 1 = 1111_1111
       -2 is 2^7 - 2 = 111_1110, or equivalently 2^8 - 1 = 1111_1110 *)
    let b = Int64.shift_right_logical (Int64.shift_left a 1) 1 in
    (* [b] has the top bit reset to 0. Thus, it's still the exact same leading
       bit 0 followed by 63 bits of data as the original Dep.t we started with.
       But whereas ocaml int and Dep.t might render this bit-pattern as a
       negative number, the Int64 will always render it a positive number.
       In particular, if we write this Int64 to a Sqlite.INT column then sqlite
       will show it as a positive integer. *)
    b

  (* Keep in sync with the tags for `DepType` in `typing_deps_hash.rs`. *)
  let make : type a. Mode.hash_mode -> a variant -> t =
   fun _ -> function
    | GConst name1 -> hash1 (dep_kind_to_enum KGConst) name1
    | Fun name1 -> hash1 (dep_kind_to_enum KFun) name1
    | Type name1 -> hash1 (dep_kind_to_enum KType) name1
    | Extends name1 -> hash1 (dep_kind_to_enum KExtends) name1
    | Const (name1, name2) ->
      hash2
        (dep_kind_to_enum KConst)
        (hash1 (dep_kind_to_enum KType) name1)
        name2
    | Constructor name1 -> hash1 (dep_kind_to_enum KConstructor) name1
    | Prop (name1, name2) ->
      hash2
        (dep_kind_to_enum KProp)
        (hash1 (dep_kind_to_enum KType) name1)
        name2
    | SProp (name1, name2) ->
      hash2
        (dep_kind_to_enum KSProp)
        (hash1 (dep_kind_to_enum KType) name1)
        name2
    | Method (name1, name2) ->
      hash2
        (dep_kind_to_enum KMethod)
        (hash1 (dep_kind_to_enum KType) name1)
        name2
    | SMethod (name1, name2) ->
      hash2
        (dep_kind_to_enum KSMethod)
        (hash1 (dep_kind_to_enum KType) name1)
        name2
    | AllMembers name1 -> hash1 (dep_kind_to_enum KAllMembers) name1
    | GConstName name1 -> hash1 (dep_kind_to_enum KGConstName) name1

  let make_dep_with_type_hash : Mode.hash_mode -> t -> string -> dep_kind -> t =
   fun _ type_hash member_name -> function
    | KGConst -> type_hash
    | KFun -> type_hash
    | KType -> type_hash
    | KExtends -> type_hash
    | KConst -> hash2 (dep_kind_to_enum KConst) type_hash member_name
    | KConstructor -> type_hash
    | KProp -> hash2 (dep_kind_to_enum KProp) type_hash member_name
    | KSProp -> hash2 (dep_kind_to_enum KSProp) type_hash member_name
    | KMethod -> hash2 (dep_kind_to_enum KMethod) type_hash member_name
    | KSMethod -> hash2 (dep_kind_to_enum KSMethod) type_hash member_name
    | KAllMembers -> type_hash
    | KGConstName -> type_hash

  let is_class x = x land 1 = 1

  let extends_of_class x = x lxor 1

  let compare = Int.compare

  let extract_name : type a. a variant -> string = function
    | GConst s -> Utils.strip_ns s
    | GConstName s -> Utils.strip_ns s
    | Const (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Type s -> Utils.strip_ns s
    | Fun s -> Utils.strip_ns s
    | Prop (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | SProp (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Method (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | SMethod (cls, s) -> spf "%s::%s" (Utils.strip_ns cls) s
    | Constructor s -> Utils.strip_ns s
    | AllMembers s -> Utils.strip_ns s
    | Extends s -> Utils.strip_ns s

  let to_decl_reference : type a. a variant -> Decl_reference.t = function
    | Type s -> Decl_reference.Type s
    | Const (s, _) -> Decl_reference.Type s
    | Extends s -> Decl_reference.Type s
    | AllMembers s -> Decl_reference.Type s
    | Constructor s -> Decl_reference.Type s
    | Prop (s, _) -> Decl_reference.Type s
    | SProp (s, _) -> Decl_reference.Type s
    | Method (s, _) -> Decl_reference.Type s
    | SMethod (s, _) -> Decl_reference.Type s
    | GConst s
    | GConstName s ->
      Decl_reference.GlobalConstant s
    | Fun s -> Decl_reference.Function s

  let to_debug_string = string_of_int

  let of_debug_string = int_of_string

  let to_hex_string = Printf.sprintf "0x%016x"

  let of_hex_string = int_of_string

  let variant_to_string : type a. a variant -> string =
   fun dep ->
    let prefix =
      match dep with
      | GConst _ -> "GConst"
      | GConstName _ -> "GConstName"
      | Const _ -> "Const"
      | Type _ -> "Type"
      | Fun _ -> "Fun"
      | Prop _ -> "Prop"
      | SProp _ -> "SProp"
      | Method _ -> "Method"
      | SMethod _ -> "SMethod"
      | Constructor _ -> "Constructor"
      | AllMembers _ -> "AllMembers"
      | Extends _ -> "Extends"
    in
    prefix ^ " " ^ extract_name dep
end

(***********************************************)
(* Dependency tracing                          *)
(***********************************************)

(** Whether or not to trace new dependency edges *)
let trace = ref true

(** List of callbacks, called when discovering dependency edges *)
let dependency_callbacks = Caml.Hashtbl.create 0

let add_dependency_callback cb_name cb =
  Caml.Hashtbl.replace dependency_callbacks cb_name cb

(** Set of dependencies used for the custom system

    The type `t` is an abstract type managed by `typing_deps.rs`.
    It is a pointer to an `OrdSet<Dep>`, a persistent Rust map *)
module DepSet = struct
  type t (* Abstract type *)

  type elt = Dep.t

  external make : unit -> t = "hh_dep_set_make"

  external singleton : elt -> t = "hh_dep_set_singleton"

  external add : t -> elt -> t = "hh_dep_set_add"

  external union : t -> t -> t = "hh_dep_set_union"

  external inter : t -> t -> t = "hh_dep_set_inter"

  external diff : t -> t -> t = "hh_dep_set_diff"

  external mem : t -> elt -> bool = "hh_dep_set_mem"

  external elements : t -> elt list = "hh_dep_set_elements"

  external cardinal : t -> int = "hh_dep_set_cardinal"

  external is_empty : t -> bool = "hh_dep_set_is_empty"

  external of_list : elt list -> t = "hh_dep_set_of_list"

  let iter s ~f = List.iter (elements s) ~f

  let fold : 'a. t -> init:'a -> f:(elt -> 'a -> 'a) -> 'a =
   fun s ~init ~f ->
    let l = elements s in
    List.fold l ~init ~f:(fun x acc -> f acc x)

  let pp fmt s =
    let open Format in
    pp_print_string fmt "{ ";
    iter s ~f:(fun x ->
        let str = Printf.sprintf "%x; " x in
        pp_print_string fmt str);
    pp_print_string fmt "}"
end

module DepHashKey = struct
  type t = Dep.t

  let compare = Int.compare

  let to_string t = string_of_int t
end

(* TODO(hverr): Clean up 32-bit, no longer necessary *)
module VisitedSet = struct
  type custom_t (* abstract type managed by Rust, RefCell<BTreeSet<Dep>> *)

  external hh_visited_set_make : unit -> custom_t = "hh_visited_set_make"

  type t = CustomVisitedSet of custom_t

  let make mode : t =
    match mode with
    | CustomMode _
    | SaveCustomMode _ ->
      CustomVisitedSet (hh_visited_set_make ())
end

(** Graph management in the new system with custom file format. *)
module CustomGraph = struct
  external hh_custom_dep_graph_register_custom_types : unit -> unit
    = "hh_custom_dep_graph_register_custom_types"

  external assert_master : unit -> unit = "assert_master"

  let allow_reads_ref = ref false

  let allow_dependency_table_reads flag =
    assert_master ();
    let prev = !allow_reads_ref in
    allow_reads_ref := flag;
    prev

  external hh_custom_dep_graph_has_edge : Mode.t -> Dep.t -> Dep.t -> bool
    = "hh_custom_dep_graph_has_edge"
    [@@noalloc]

  external get_ideps_from_hash : Mode.t -> Dep.t -> DepSet.t
    = "hh_custom_dep_graph_get_ideps_from_hash"

  external add_typing_deps : Mode.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_add_typing_deps"

  external add_extend_deps : Mode.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_add_extend_deps"

  external get_extend_deps :
    Mode.t -> VisitedSet.custom_t -> Dep.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_get_extend_deps"

  external register_discovered_dep_edge : Dep.t -> Dep.t -> unit
    = "hh_custom_dep_graph_register_discovered_dep_edge"
    [@@noalloc]

  external dep_graph_delta_num_edges : unit -> int
    = "hh_custom_dep_graph_dep_graph_delta_num_edges"
    [@@noalloc]

  external save_delta : string -> bool -> int = "hh_custom_dep_graph_save_delta"

  external load_delta : Mode.t -> string -> int
    = "hh_custom_dep_graph_load_delta"

  let add_all_deps mode x = x |> add_extend_deps mode |> add_typing_deps mode

  type dep_edge = {
    idependent: Dep.t;
    idependency: Dep.t;
  }

  module DepEdgeSet = Caml.Set.Make (struct
    type t = dep_edge

    let compare x y =
      let d1 = Int.compare x.idependent y.idependent in
      if d1 = 0 then
        Int.compare x.idependency y.idependency
      else
        d1
  end)

  (* A batch of discovered dependency edges, of which some might
   * already be in the dependency graph!
   *
   * There isn't really any reason why I choose Hashtbl over Set here. *)
  let discovered_deps_batch : (dep_edge, unit) Hashtbl.t = Hashtbl.create 1000

  (* A batch of dependency edges that are not yet in the dependency graph.
   * We use a Set, because a Hashtbl is way too expensive to serialize/
   * deserialize in OCaml. *)
  let filtered_deps_batch : DepEdgeSet.t ref = ref DepEdgeSet.empty

  let filter_discovered_deps_batch mode =
    (* Empty discovered_deps_bach by checking for each edge whether it's already
     * in the dependency graph. If it is not, add it to the filtered deps batch. *)
    let s = !filtered_deps_batch in
    let s =
      Hashtbl.fold
        begin
          fun ({ idependent; idependency } as edge) () s ->
          if not (hh_custom_dep_graph_has_edge mode idependent idependency) then
            DepEdgeSet.add edge s
          else
            s
        end
        discovered_deps_batch
        s
    in
    filtered_deps_batch := s;
    Hashtbl.clear discovered_deps_batch

  let register_discovered_dep_edges : DepEdgeSet.t -> unit =
   fun s ->
    assert_master ();
    DepEdgeSet.iter
      begin
        fun { idependent; idependency } ->
        register_discovered_dep_edge idependent idependency
      end
      s

  let add_idep mode dependent dependency =
    let idependent = Dep.make Hash64Bit dependent in
    let idependency = Dep.make Hash64Bit dependency in
    if idependent = idependency then
      ()
    else (
      Caml.Hashtbl.iter (fun _ f -> f dependent dependency) dependency_callbacks;
      if !trace then begin
        Hashtbl.replace discovered_deps_batch { idependent; idependency } ();
        if Hashtbl.length discovered_deps_batch >= 1000 then
          filter_discovered_deps_batch mode
      end
    )

  let idep_exists mode dependent dependency =
    let idependent = Dep.make Hash64Bit dependent in
    let idependency = Dep.make Hash64Bit dependency in
    hh_custom_dep_graph_has_edge mode idependent idependency
end

module SaveHumanReadableDepMap = struct
  let should_save mode =
    match mode with
    | SaveCustomMode { human_readable_dep_map_dir = Some _; _ } -> true
    | _ -> false

  let human_readable_dep_map_channel_ref : Out_channel.t option ref = ref None

  let human_readable_dep_map_channel mode =
    match !human_readable_dep_map_channel_ref with
    | None ->
      let directory =
        match mode with
        | SaveCustomMode { human_readable_dep_map_dir = Some d; _ } -> d
        | _ -> failwith "programming error: no human_readable_dep_map_dir"
      in
      let () =
        if (not (Sys.file_exists directory)) || not (Sys.is_directory directory)
        then
          Sys_utils.mkdir_p directory
      in
      let worker_id = Option.value_exn !worker_id in
      (* To avoid multiple processes interleaving writes to the same file, rely on
       * unique process id's to have each process write to separate files.
       * Use ~append:true to have each new process write to the previously created logs
       * if they happened to share the same process id.
       *)
      let filepath =
        Filename.concat
          directory
          (Printf.sprintf "human-readable-dep-map-%d.txt" worker_id)
      in
      let handle = Out_channel.create ~append:true ~perm:0o600 filepath in
      let () = human_readable_dep_map_channel_ref := Some handle in
      handle
    | Some h -> h

  (* For each process, keep a set of the seen (hashcode, dependency name)s to reduce logging duplicates.
   * Use the name as well as the hashcode in case of hashcode conflicts.
   *)
  let set_max_size = 20000

  let seen_set_ref : (int * string, unit) Hashtbl.t option ref = ref None

  let seen_set () =
    match !seen_set_ref with
    | None ->
      let tbl = Hashtbl.create set_max_size in
      let () = seen_set_ref := Some tbl in
      tbl
    | Some tbl -> tbl

  (* Takes the current set of human readable deps and writes them to disk
   * Resets the set of human readable deps to be empty
   * ~flush flushes the channel after the write *)
  let export_to_disk ?(flush = false) mode =
    if should_save mode then
      let ss = seen_set () in
      let out_channel = human_readable_dep_map_channel mode in
      let () =
        Hashtbl.iter
          (fun (hash, name) () ->
            Printf.fprintf out_channel "%u %s\n" hash name)
          ss
      in
      let () = Hashtbl.reset ss in
      if flush then Out_channel.flush out_channel

  (* Adds a dep to the current set of human readable deps *)
  let add mode (dep, hash) =
    if should_save mode then
      let ss = seen_set () in
      let name = Dep.variant_to_string dep in
      if Hashtbl.mem ss (hash, name) then
        ()
      else
        let () = Hashtbl.add ss (hash, name) () in
        if Hashtbl.length ss >= set_max_size then export_to_disk mode
end

module SaveCustomGraph = struct
  external hh_save_custom_dep_graph_save_delta : string -> string -> int
    = "hh_save_custom_dep_graph_save_delta"

  let discovered_deps_batch : (CustomGraph.dep_edge, unit) Hashtbl.t =
    Hashtbl.create 1000

  let destination_file_handle_ref : Out_channel.t option ref = ref None

  let destination_filepath mode =
    match mode with
    | SaveCustomMode { new_edges_dir; _ } ->
      let worker_id = Base.Option.value_exn !worker_id in
      Filename.concat
        new_edges_dir
        (Printf.sprintf "new-edges-worker-%d.bin" worker_id)
    | _ -> failwith "programming error: wrong mode"

  let destination_file_handle mode =
    match !destination_file_handle_ref with
    | Some handle -> handle
    | None ->
      let filepath = destination_filepath mode in
      let handle =
        Out_channel.create ~binary:true ~append:true ~perm:0o600 filepath
      in
      destination_file_handle_ref := Some handle;
      handle

  let filter_discovered_deps_batch ~flush mode =
    let handle = destination_file_handle mode in
    Hashtbl.iter
      begin
        fun CustomGraph.{ idependent; idependency } () ->
        if
          not
            (CustomGraph.hh_custom_dep_graph_has_edge
               mode
               idependent
               idependency)
        then begin
          (* To be kept in sync with typing_deps.rs::hh_custom_dep_graph_save_delta! *)
          (* output_binary_int outputs the lower 4-bytes only, regardless
           * of architecture. It outputs in big-endian format.*)
          Out_channel.output_binary_int handle (idependent lsr 32);
          Out_channel.output_binary_int handle idependent;
          Out_channel.output_binary_int handle (idependency lsr 32);
          Out_channel.output_binary_int handle idependency
        end
      end
      discovered_deps_batch;
    if flush then Out_channel.flush handle;
    Hashtbl.clear discovered_deps_batch

  let add_idep mode dependent dependency =
    let idependent = Dep.make Hash64Bit dependent in
    let idependency = Dep.make Hash64Bit dependency in
    if idependent = idependency then
      ()
    else (
      Caml.Hashtbl.iter (fun _ f -> f dependent dependency) dependency_callbacks;
      if !trace then begin
        Hashtbl.replace
          discovered_deps_batch
          CustomGraph.{ idependent; idependency }
          ();
        if Hashtbl.length discovered_deps_batch >= 1000 then
          filter_discovered_deps_batch ~flush:false mode
      end;
      let () = SaveHumanReadableDepMap.add mode (dependent, idependent) in
      SaveHumanReadableDepMap.add mode (dependency, idependency)
    )

  let save_delta mode ~source =
    let dest = destination_filepath mode in
    hh_save_custom_dep_graph_save_delta source dest
end

(** Registeres Rust custom types with the OCaml runtime, supporting deserialization *)
let () = CustomGraph.hh_custom_dep_graph_register_custom_types ()

let hash_mode = Typing_deps_mode.hash_mode

let deps_of_file_info (mode : Mode.t) (file_info : FileInfo.t) : Dep.t list =
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
    file_info
  in
  let hash_mode = Mode.hash_mode mode in
  let defs =
    List.fold_left
      consts
      ~f:
        begin
          fun acc (_, const_id, _) ->
          Dep.make hash_mode (Dep.GConst const_id) :: acc
        end
      ~init:[]
  in
  let defs =
    List.fold_left
      funs
      ~f:
        begin
          fun acc (_, fun_id, _) ->
          Dep.make hash_mode (Dep.Fun fun_id) :: acc
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      classes
      ~f:
        begin
          fun acc (_, class_id, _) ->
          Dep.make hash_mode (Dep.Type class_id) :: acc
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      record_defs
      ~f:
        begin
          fun acc (_, type_id, _) ->
          Dep.make hash_mode (Dep.Type type_id) :: acc
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      typedefs
      ~f:
        begin
          fun acc (_, type_id, _) ->
          Dep.make hash_mode (Dep.Type type_id) :: acc
        end
      ~init:defs
  in
  defs

module Telemetry = struct
  let depgraph_delta_num_edges mode =
    match mode with
    | CustomMode _ -> Some (CustomGraph.dep_graph_delta_num_edges ())
    | SaveCustomMode _ -> None
end

type dep_edge = CustomGraph.dep_edge

type dep_edges = CustomGraph.DepEdgeSet.t option

(** As part of few optimizations (prechecked files, interruptible typechecking), we
    allow the dependency table to get out of date (in order to be able to prioritize
    other work, like reporting errors in currently open file). This flag is there
    to avoid accidental reads of this stale data - anyone attempting to do so would
    either acknowledge that they don't care about accuracy (by setting this flag
    themselves), or plug in to a hh_server mechanic that will delay executing such
    command until dependency table is back up to date.
    *)
let allow_dependency_table_reads mode flag =
  match mode with
  | CustomMode _
  | SaveCustomMode _ ->
    CustomGraph.allow_dependency_table_reads flag

let add_idep mode dependent dependency =
  match mode with
  | CustomMode _ -> CustomGraph.add_idep mode dependent dependency
  | SaveCustomMode _ -> SaveCustomGraph.add_idep mode dependent dependency

let idep_exists mode dependent dependency =
  match mode with
  | CustomMode _ -> CustomGraph.idep_exists mode dependent dependency
  | _ -> false

let dep_edges_make () : dep_edges = Some CustomGraph.DepEdgeSet.empty

let flush_ideps_batch mode : dep_edges =
  match mode with
  | CustomMode _ ->
    (* Make sure we don't miss any dependencies! *)
    CustomGraph.filter_discovered_deps_batch mode;
    let old_batch = !CustomGraph.filtered_deps_batch in
    CustomGraph.filtered_deps_batch := CustomGraph.DepEdgeSet.empty;
    Some old_batch
  | SaveCustomMode _ ->
    SaveCustomGraph.filter_discovered_deps_batch ~flush:true mode;
    SaveHumanReadableDepMap.export_to_disk ~flush:true mode;
    None

let merge_dep_edges (x : dep_edges) (y : dep_edges) : dep_edges =
  match (x, y) with
  | (Some x, Some y) -> Some (CustomGraph.DepEdgeSet.union x y)
  | _ -> None

let register_discovered_dep_edges : dep_edges -> unit = function
  | None -> ()
  | Some batch -> CustomGraph.register_discovered_dep_edges batch

let save_discovered_edges mode ~dest ~reset_state_after_saving =
  match mode with
  | CustomMode _ -> CustomGraph.save_delta dest reset_state_after_saving
  | SaveCustomMode _ ->
    failwith "save_discovered_edges not supported for SaveCustomMode"

let load_discovered_edges mode source =
  match mode with
  | CustomMode _ -> CustomGraph.load_delta mode source
  | SaveCustomMode _ -> SaveCustomGraph.save_delta mode ~source

let get_ideps_from_hash mode hash =
  match mode with
  | CustomMode _
  | SaveCustomMode _ ->
    CustomGraph.get_ideps_from_hash mode hash

let get_ideps mode dependency =
  get_ideps_from_hash mode (Dep.make (hash_mode mode) dependency)

let get_extend_deps ~mode ~visited ~source_class ~acc =
  let open VisitedSet in
  match (visited, acc) with
  | (CustomVisitedSet visited, acc) ->
    CustomGraph.get_extend_deps mode visited source_class acc

let add_extend_deps mode acc = CustomGraph.add_extend_deps mode acc

let add_typing_deps mode acc = CustomGraph.add_typing_deps mode acc

let add_all_deps mode acc = CustomGraph.add_all_deps mode acc
