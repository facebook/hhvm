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
open FileInfo

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
    | RequireExtends : string -> dependency variant
    | NotSubtype : string -> dependency variant
    | Const : string * string -> dependency variant
    | Constructor : string -> dependency variant
    | Prop : string * string -> dependency variant
    | SProp : string * string -> dependency variant
    | Method : string * string -> dependency variant
    | SMethod : string * string -> dependency variant
    | AllMembers : string -> dependency variant
    | GConstName : string -> 'a variant
    | Module : string -> 'a variant
    | Declares : 'a variant

  let dependency_of_variant : type a. a variant -> dependency variant = function
    | GConst s -> GConst s
    | GConstName s -> GConstName s
    | Type s -> Type s
    | Fun s -> Fun s
    | Module m -> Module m
    | Const (cls, s) -> Const (cls, s)
    | Prop (cls, s) -> Prop (cls, s)
    | SProp (cls, s) -> SProp (cls, s)
    | Method (cls, s) -> Method (cls, s)
    | SMethod (cls, s) -> SMethod (cls, s)
    | Constructor s -> Constructor s
    | AllMembers s -> AllMembers s
    | Extends s -> Extends s
    | RequireExtends s -> RequireExtends s
    | NotSubtype s -> NotSubtype s
    | Declares -> Declares

  (** NOTE: keep in sync with `typing_deps_hash.rs`. *)
  type dep_kind =
    | KGConst [@value 0]
    | KFun [@value 1]
    | KType [@value 2]
    | KExtends [@value 3]
    | KRequireExtends [@value 4]
    | KConst [@value 5]
    | KConstructor [@value 6]
    | KProp [@value 7]
    | KSProp [@value 8]
    | KMethod [@value 9]
    | KSMethod [@value 10]
    | KAllMembers [@value 11]
    | KGConstName [@value 12]
    | KModule [@value 13]
    | KDeclares [@value 14]
    | KNotSubtype [@value 15]
  [@@deriving enum]

  module Member = struct
    type t =
      | Method of string
      | SMethod of string
      | Prop of string
      | SProp of string
      | Constructor
      | Const of string
      | All
    [@@deriving show]

    let method_ name = Method name

    let smethod name = SMethod name

    let prop name = Prop name

    let sprop name = SProp name

    let constructor = Constructor

    let const name = Const name

    let all = All

    let to_dep_kind_and_name (member : t) : dep_kind * string =
      let default_name = "" in
      match member with
      | Const name -> (KConst, name)
      | Constructor -> (KConstructor, default_name)
      | Prop name -> (KProp, name)
      | SProp name -> (KSProp, name)
      | Method name -> (KMethod, name)
      | SMethod name -> (KSMethod, name)
      | All -> (KAllMembers, default_name)
  end

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

  let to_int hash = hash

  let ordinal_variant (type a) : a variant -> int = function
    | GConst _ -> 0
    | Fun _ -> 1
    | Type _ -> 2
    | Extends _ -> 3
    | RequireExtends _ -> 4
    | NotSubtype _ -> 15
    | Const _ -> 5
    | Constructor _ -> 6
    | Prop _ -> 7
    | SProp _ -> 8
    | Method _ -> 9
    | SMethod _ -> 10
    | AllMembers _ -> 11
    | GConstName _ -> 12
    | Module _ -> 13
    | Declares -> 14

  let compare_variant (type a) (v1 : a variant) (v2 : a variant) : int =
    match (v1, v2) with
    | (GConst x1, GConst x2)
    | (Fun x1, Fun x2)
    | (Type x1, Type x2)
    | (Extends x1, Extends x2)
    | (RequireExtends x1, RequireExtends x2)
    | (NotSubtype x1, NotSubtype x2)
    | (Constructor x1, Constructor x2)
    | (AllMembers x1, AllMembers x2)
    | (GConstName x1, GConstName x2) ->
      String.compare x1 x2
    | (Prop (c1, m1), Prop (c2, m2))
    | (SProp (c1, m1), SProp (c2, m2))
    | (Method (c1, m1), Method (c2, m2))
    | (SMethod (c1, m1), SMethod (c2, m2))
    | (Const (c1, m1), Const (c2, m2)) ->
      let res = String.compare c1 c2 in
      if Int.( <> ) res 0 then
        res
      else
        String.compare m1 m2
    | (Declares, Declares) -> 0
    | ( _,
        ( GConst _ | Fun _ | Type _ | Extends _ | RequireExtends _
        | NotSubtype _ | Const _ | Constructor _ | Prop _ | SProp _ | Method _
        | SMethod _ | AllMembers _ | GConstName _ | Module _ | Declares ) ) ->
      ordinal_variant v1 - ordinal_variant v2

  let dep_kind_of_variant : type a. a variant -> dep_kind = function
    | GConst _ -> KGConst
    | GConstName _ -> KGConstName
    | Const _ -> KConst
    | Type _ -> KType
    | Fun _ -> KFun
    | Prop _ -> KProp
    | SProp _ -> KSProp
    | Method _ -> KMethod
    | SMethod _ -> KSMethod
    | Constructor _ -> KConstructor
    | AllMembers _ -> KAllMembers
    | Extends _ -> KExtends
    | RequireExtends _ -> KRequireExtends
    | NotSubtype _ -> KNotSubtype
    | Module _ -> KModule
    | Declares -> KDeclares

  let make_member_dep_from_type_dep (type_dep : t) (member : Member.t) : t =
    let (dep_kind, member_name) = Member.to_dep_kind_and_name member in
    hash2 (dep_kind_to_enum dep_kind) type_dep member_name

  (** Return the 'Extends' dep for a class from its 'Type' dep  *)
  let extends_of_class class_dep = class_dep lxor 1

  (** Return the 'RequireExtends' dep for a class from its 'Type' dep  *)
  let require_extends_of_class class_dep =
    hash2 (dep_kind_to_enum KRequireExtends) class_dep ""

  (** Return the 'NotSubtype' dep for a class from its 'Type' dep  *)
  let not_subtype_of_class class_dep =
    hash2 (dep_kind_to_enum KNotSubtype) class_dep ""

  (* Keep in sync with the tags for `DepType` in `typing_deps_hash.rs`. *)
  let rec make : type a. a variant -> t = function
    (* Deps on defs *)
    | GConst name1 -> hash1 (dep_kind_to_enum KGConst) name1
    | Fun name1 -> hash1 (dep_kind_to_enum KFun) name1
    | GConstName name1 -> hash1 (dep_kind_to_enum KGConstName) name1
    | Module mname -> hash1 (dep_kind_to_enum KModule) mname
    | Type name1 -> hash1 (dep_kind_to_enum KType) name1
    | Extends name1 -> hash1 (dep_kind_to_enum KExtends) name1
    | RequireExtends name1 -> require_extends_of_class (make (Type name1))
    | NotSubtype name1 -> not_subtype_of_class (make (Type name1))
    (* Deps on members *)
    | Constructor name1 ->
      make_member_dep_from_type_dep (make (Type name1)) Member.Constructor
    | Const (name1, name2) ->
      make_member_dep_from_type_dep (make (Type name1)) (Member.Const name2)
    | Prop (name1, name2) ->
      make_member_dep_from_type_dep (make (Type name1)) (Member.Prop name2)
    | SProp (name1, name2) ->
      make_member_dep_from_type_dep (make (Type name1)) (Member.SProp name2)
    | Method (name1, name2) ->
      make_member_dep_from_type_dep (make (Type name1)) (Member.Method name2)
    | SMethod (name1, name2) ->
      make_member_dep_from_type_dep (make (Type name1)) (Member.SMethod name2)
    | AllMembers name1 ->
      make_member_dep_from_type_dep (make (Type name1)) Member.All
    | Declares -> hash1 (dep_kind_to_enum KDeclares) ""

  let is_class x = x land 1 = 1

  let extends_and_req_extends_of_class class_dep =
    (extends_of_class class_dep, require_extends_of_class class_dep)

  let compare = Int.compare

  let extract_name : type a. a variant -> string = function
    | Const (cls, s)
    | Prop (cls, s)
    | SProp (cls, s)
    | Method (cls, s)
    | SMethod (cls, s) ->
      spf "%s::%s" (Utils.strip_ns cls) s
    | GConst s
    | GConstName s
    | Type s
    | Fun s
    | Constructor s
    | AllMembers s
    | Extends s
    | RequireExtends s
    | NotSubtype s ->
      Utils.strip_ns s
    | Module m -> m
    | Declares -> "__declares__"

  let extract_root_name : type a. ?strip_namespace:bool -> a variant -> string =
   fun ?(strip_namespace = true) variant ->
    match variant with
    | GConst s
    | GConstName s
    | Constructor s
    | AllMembers s
    | Extends s
    | RequireExtends s
    | NotSubtype s
    | Module s
    | Type s
    | Fun s
    | Prop (s, _)
    | SProp (s, _)
    | Method (s, _)
    | SMethod (s, _)
    | Const (s, _) ->
      if strip_namespace then
        Utils.strip_ns s
      else
        s
    | Declares -> "__declares__"

  let extract_member_name : type a. a variant -> string option = function
    | GConst _
    | GConstName _
    | Constructor _
    | AllMembers _
    | Extends _
    | RequireExtends _
    | NotSubtype _
    | Module _
    | Type _
    | Fun _
    | Declares ->
      None
    | Const (_cls, s)
    | Prop (_cls, s)
    | SProp (_cls, s)
    | Method (_cls, s)
    | SMethod (_cls, s) ->
      Some s

  let to_decl_reference : type a. a variant -> Decl_reference.t = function
    | Type s
    | Const (s, _)
    | Extends s
    | RequireExtends s
    | NotSubtype s
    | AllMembers s
    | Constructor s
    | Prop (s, _)
    | SProp (s, _)
    | Method (s, _)
    | SMethod (s, _) ->
      Decl_reference.Type s
    | GConst s
    | GConstName s ->
      Decl_reference.GlobalConstant s
    | Fun s -> Decl_reference.Function s
    | Module m -> Decl_reference.Module m
    | Declares -> failwith "No Decl_reference.t for Declares Dep.variant"

  let to_debug_string = string_of_int

  let of_debug_string = int_of_string

  let to_hex_string = Printf.sprintf "0x%016x"

  let pp fmt dep = Format.fprintf fmt "%s" (to_hex_string dep)

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
      | RequireExtends _ -> "RequireExtends"
      | NotSubtype _ -> "NotSubtype"
      | Module _ -> "Module"
      | Declares -> "Declares"
    in
    match dep with
    | Declares -> prefix
    | _ -> prefix ^ " " ^ extract_name dep

  let pp_variant fmt variant =
    Format.fprintf fmt "%s" (variant_to_string variant)
end

module DepMap = struct
  include WrappedMap.Make (Dep)

  let pp pp_data = make_pp Dep.pp pp_data

  let show pp_data x = Format.asprintf "%a" (pp pp_data) x
end

(***********************************************)
(* Dependency tracing                          *)
(***********************************************)

(** Whether or not to trace new dependency edges *)
let trace = ref true

(** List of callbacks, called when discovering dependency edges *)
let dependency_callbacks = Stdlib.Hashtbl.create 0

let add_dependency_callback ~name cb =
  Stdlib.Hashtbl.replace dependency_callbacks name cb

(** Set of dependencies used for the custom system

    The type `t` is an abstract type managed by `typing_deps.rs`.
    It is a pointer to an `HashTrieSet<Dep>`, a persistent Rust map *)
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

  let show s = Format.asprintf "%a" pp s
end

module DepHashKey = struct
  type t = Dep.t

  let compare = Int.compare

  let to_string t = string_of_int t
end

module VisitedSet = struct
  type t (* abstract type managed by Rust, RefCell<BTreeSet<Dep>> *)

  external hh_visited_set_make : unit -> t = "hh_visited_set_make"

  let make () : t = hh_visited_set_make ()
end

type dep_edge = {
  idependent: Dep.t;  (** The node depending on the dependency *)
  idependency: Dep.t;  (** The node the dependent depends upon *)
}

module DepEdgeSet = Stdlib.Set.Make (struct
  type t = dep_edge

  let compare x y =
    let d1 = Int.compare x.idependent y.idependent in
    if d1 = 0 then
      Int.compare x.idependency y.idependency
    else
      d1
end)

(** Graph management in the new system with custom file format. *)
module CustomGraph = struct
  (* from deps_rust_ffi.rs *)
  external hh_custom_dep_graph_register_custom_types : unit -> unit
    = "hh_custom_dep_graph_register_custom_types"

  (* from hh_shared.h *)
  external assert_master : unit -> unit = "hh_assert_master"

  let allow_reads_ref = ref false

  let allow_dependency_table_reads flag =
    assert_master ();
    let prev = !allow_reads_ref in
    allow_reads_ref := flag;
    prev

  external hh_custom_dep_graph_replace : Mode.t -> unit
    = "hh_custom_dep_graph_replace"
    [@@noalloc]

  external depgraph_has_edge : Mode.t -> Dep.t -> Dep.t -> bool
    = "hh_depgraph_has_edge"
    [@@noalloc]

  external get_ideps_from_hash : Mode.t -> Dep.t -> DepSet.t
    = "hh_custom_dep_graph_get_ideps_from_hash"

  external add_typing_deps : Mode.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_add_typing_deps"

  external add_extend_deps : Mode.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_add_extend_deps"

  external get_extend_deps :
    Mode.t -> VisitedSet.t -> Dep.t -> DepSet.t -> DepSet.t
    = "hh_custom_dep_graph_get_extend_deps"

  external get_member_fanout :
    Mode.t -> Dep.t -> Dep.dep_kind -> string -> DepSet.t -> DepSet.t
    = "hh_get_member_fanout"

  external get_not_subtype_fanout : Mode.t -> DepSet.t -> DepSet.t -> DepSet.t
    = "hh_get_not_subtype_fanout"

  external register_discovered_dep_edge : Dep.t -> Dep.t -> unit
    = "hh_custom_dep_graph_register_discovered_dep_edge"
    [@@noalloc]

  external remove_edge : Dep.t -> Dep.t -> unit
    = "hh_custom_dep_graph_remove_edge"
    [@@noalloc]

  external dep_graph_delta_num_edges : unit -> int
    = "hh_custom_dep_graph_dep_graph_delta_num_edges"
    [@@noalloc]

  external save_delta : string -> bool -> int = "hh_custom_dep_graph_save_delta"

  external load_delta : Mode.t -> string -> int
    = "hh_custom_dep_graph_load_delta"

  let add_all_deps mode x = x |> add_extend_deps mode |> add_typing_deps mode

  (** A batch of discovered dependency edges, of which some might
    already be in the dependency graph! *)
  let discovered_deps_batch : (dep_edge, unit) Hashtbl.t =
    (* There isn't really any reason why I choose Hashtbl over Set here. *)
    Hashtbl.create 1000

  (** A batch of dependency edges that are not yet in the dependency graph. *)
  let filtered_deps_batch : DepEdgeSet.t ref =
    (* We use a Set, because a Hashtbl is way too expensive to serialize/
       deserialize in OCaml. *)
    ref DepEdgeSet.empty

  (** Filter out the discovered dep edges which are already in the dep graph.
    Get [!filtered_deps_batch] to obtain the result. *)
  let filter_discovered_deps_batch mode =
    (* Empty discovered_deps_bach by checking for each edge whether it's already
     * in the dependency graph. If it is not, add it to the filtered deps batch. *)
    let s = !filtered_deps_batch in
    let s =
      Hashtbl.fold
        begin
          fun ({ idependent; idependency } as edge) () s ->
            if not (depgraph_has_edge mode idependent idependency) then
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

  let remove_edges : DepEdgeSet.t -> unit =
   fun s ->
    assert_master ();
    DepEdgeSet.iter
      begin
        (fun { idependent; idependency } -> remove_edge idependent idependency)
      end
      s

  let add_idep mode dependent dependency =
    let idependent = Dep.make dependent in
    let idependency = Dep.make dependency in
    if idependent = idependency then
      ()
    else (
      Stdlib.Hashtbl.iter
        (fun _ f -> f dependent dependency)
        dependency_callbacks;
      if !trace then begin
        Hashtbl.replace discovered_deps_batch { idependent; idependency } ();
        if Hashtbl.length discovered_deps_batch >= 1000 then
          filter_discovered_deps_batch mode
      end
    )

  let dump_current_edge_buffer ?deps_to_symbol_map () =
    let hash_to_string dep =
      match deps_to_symbol_map with
      | None -> Dep.to_hex_string dep
      | Some map ->
        (match DepMap.find_opt dep map with
        | None -> Dep.to_hex_string dep
        | Some symbol -> Dep.variant_to_string symbol)
    in
    Hashtbl.iter
      (fun { idependent; idependency } () ->
        Printf.printf
          "%s -> %s\n"
          (hash_to_string idependency)
          (hash_to_string idependent))
      discovered_deps_batch
end

module SaveHumanReadableDepMap : sig
  (** Add a dep to the current set of human readable deps. *)
  val add : Typing_deps_mode.t -> 'a Dep.variant * int -> unit

  (** Take the current set of human readable deps and writes them to disk.
    Reset the set of human readable deps to be empty.
    If [flush], flush the channel after the write. *)
  val export_to_disk : ?flush:bool -> Typing_deps_mode.t -> unit
end = struct
  let should_save mode =
    match mode with
    | SaveToDiskMode { human_readable_dep_map_dir = Some _; _ } -> true
    | _ -> false

  let human_readable_dep_map_channel_ref : Out_channel.t option ref = ref None

  let human_readable_dep_map_channel mode =
    match !human_readable_dep_map_channel_ref with
    | None ->
      let directory =
        match mode with
        | SaveToDiskMode { human_readable_dep_map_dir = Some d; _ } -> d
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

  let set_max_size = 20000

  (** The set of the seen (hashcode, dependency name)s, per worker.
    Use the name as well as the hashcode in case of hashcode conflicts. *)
  let seen_set_ref : (int * string, unit) Hashtbl.t option ref = ref None

  let seen_set () =
    match !seen_set_ref with
    | None ->
      let tbl = Hashtbl.create set_max_size in
      let () = seen_set_ref := Some tbl in
      tbl
    | Some tbl -> tbl

  (** Take the current set of human readable deps and writes them to disk.
    Reset the set of human readable deps to be empty.
    If [flush], flush the channel after the write. *)
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

  (** Add a dep to the current set of human readable deps. *)
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

module SaveCustomGraph : sig
  val add_idep :
    Mode.t -> Dep.dependent Dep.variant -> Dep.dependency Dep.variant -> unit

  (** Write to disk the dep edges which are not already in the depgraph. *)
  val filter_discovered_deps_batch : flush:bool -> Mode.t -> unit

  (** Move the source file to the worker's depgraph directory. *)
  val save_delta : Typing_deps_mode.t -> source:string -> int
end = struct
  (** [hh_save_custom_dep_graph_save_delta src dest_dir]
    moves the [src] file to the [dest_dir] directory. *)
  external hh_save_custom_dep_graph_save_delta : string -> string -> int
    = "hh_save_custom_dep_graph_save_delta"

  let discovered_deps_batch : (dep_edge, unit) Hashtbl.t = Hashtbl.create 1000

  let destination_file_handle_ref : Out_channel.t option ref = ref None

  let destination_filepath mode =
    match mode with
    | SaveToDiskMode { new_edges_dir; _ } ->
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

  let destination_dir mode =
    match mode with
    | SaveToDiskMode { new_edges_dir; _ } -> new_edges_dir
    | _ -> failwith "programming error: wrong mode"

  (** Write to disk the dep edges which are not already in the depgraph. *)
  let filter_discovered_deps_batch ~flush mode =
    let handle = destination_file_handle mode in
    Hashtbl.iter
      begin
        fun { idependent; idependency } () ->
          if not (CustomGraph.depgraph_has_edge mode idependent idependency)
          then begin
            (* To be kept in sync with typing_deps.rs::hh_custom_dep_graph_save_delta! *)

            (* Write dependency. *)
            for i = 0 to 6 do
              Out_channel.output_byte handle (idependency lsr (i * 8))
            done;
            (* Set a tag bit to indicate this is a dependency *)
            Out_channel.output_byte handle ((idependency lsr 56) + 128);

            (* Write dependent. *)
            for i = 0 to 7 do
              Out_channel.output_byte handle (idependent lsr (i * 8))
            done
          end
      end
      discovered_deps_batch;
    if flush then Out_channel.flush handle;
    Hashtbl.clear discovered_deps_batch

  let add_idep mode dependent dependency =
    let idependent = Dep.make dependent in
    let idependency = Dep.make dependency in
    if idependent = idependency then
      ()
    else (
      Stdlib.Hashtbl.iter
        (fun _ f -> f dependent dependency)
        dependency_callbacks;
      if !trace then begin
        Hashtbl.replace discovered_deps_batch { idependent; idependency } ();
        if Hashtbl.length discovered_deps_batch >= 1000 then
          filter_discovered_deps_batch ~flush:false mode
      end;
      let () = SaveHumanReadableDepMap.add mode (dependent, idependent) in
      SaveHumanReadableDepMap.add mode (dependency, idependency)
    )

  (** Move the source file to the worker's depgraph directory. *)
  let save_delta mode ~source =
    let dest = destination_dir mode in
    hh_save_custom_dep_graph_save_delta source dest
end

(** Registers Rust custom types with the OCaml runtime, supporting deserialization *)
let () = CustomGraph.hh_custom_dep_graph_register_custom_types ()

let deps_of_file_info (file_info : FileInfo.t) : Dep.t list =
  let {
    FileInfo.ids = { FileInfo.funs; classes; typedefs; consts; modules };
    comments = _;
    file_mode = _;
    position_free_decl_hash = _;
  } =
    file_info
  in
  let defs =
    List.fold_left
      consts
      ~f:
        begin
          (fun acc (id : FileInfo.id) -> Dep.make (Dep.GConst id.name) :: acc)
        end
      ~init:[]
  in
  let defs =
    List.fold_left
      funs
      ~f:
        begin
          (fun acc (id : FileInfo.id) -> Dep.make (Dep.Fun id.name) :: acc)
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      classes
      ~f:
        begin
          (fun acc (id : FileInfo.id) -> Dep.make (Dep.Type id.name) :: acc)
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      typedefs
      ~f:
        begin
          (fun acc (id : FileInfo.id) -> Dep.make (Dep.Type id.name) :: acc)
        end
      ~init:defs
  in
  let defs =
    List.fold_left
      modules
      ~f:
        begin
          (fun acc (id : FileInfo.id) -> Dep.make (Dep.Module id.name) :: acc)
        end
      ~init:defs
  in
  defs

module Telemetry = struct
  let depgraph_delta_num_edges mode =
    match mode with
    | InMemoryMode _ -> Some (CustomGraph.dep_graph_delta_num_edges ())
    | SaveToDiskMode _ -> None
end

type dep_edges = DepEdgeSet.t option

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
  | InMemoryMode _
  | SaveToDiskMode _ ->
    CustomGraph.allow_dependency_table_reads flag

let add_idep mode dependent dependency =
  match mode with
  | InMemoryMode _ -> CustomGraph.add_idep mode dependent dependency
  | SaveToDiskMode _ -> SaveCustomGraph.add_idep mode dependent dependency

let replace mode =
  match mode with
  | InMemoryMode _ -> CustomGraph.hh_custom_dep_graph_replace mode
  | _ -> ()

let dep_edges_make () : dep_edges = Some DepEdgeSet.empty

(** Depending on [mode], either return discovered edges
  which are not already in the dep graph
  or write those edges to disk. *)
let flush_ideps_batch mode : dep_edges =
  match mode with
  | InMemoryMode _ ->
    (* Make sure we don't miss any dependencies! *)
    CustomGraph.filter_discovered_deps_batch mode;
    let old_batch = !CustomGraph.filtered_deps_batch in
    CustomGraph.filtered_deps_batch := DepEdgeSet.empty;
    Some old_batch
  | SaveToDiskMode _ ->
    SaveCustomGraph.filter_discovered_deps_batch ~flush:true mode;
    SaveHumanReadableDepMap.export_to_disk ~flush:true mode;
    None

let merge_dep_edges (x : dep_edges) (y : dep_edges) : dep_edges =
  match (x, y) with
  | (Some x, Some y) -> Some (DepEdgeSet.union x y)
  | _ -> None

(** Register the provided dep edges in the dep table delta in [typing_deps.rs] *)
let register_discovered_dep_edges : dep_edges -> unit = function
  | None -> ()
  | Some batch -> CustomGraph.register_discovered_dep_edges batch

let remove_edges mode edges =
  match mode with
  | InMemoryMode _
  | SaveToDiskMode _ ->
    CustomGraph.remove_edges edges

let remove_declared_tags mode deps =
  let edges =
    DepSet.fold deps ~init:DepEdgeSet.empty ~f:(fun dep edges_acc ->
        let edge = { idependency = dep; idependent = Dep.make Dep.Declares } in
        DepEdgeSet.add edge edges_acc)
  in
  remove_edges mode edges

let save_discovered_edges mode ~dest ~reset_state_after_saving =
  match mode with
  | InMemoryMode _ -> CustomGraph.save_delta dest reset_state_after_saving
  | SaveToDiskMode _ ->
    failwith "save_discovered_edges not supported for SaveToDiskMode"

let load_discovered_edges mode source =
  match mode with
  | InMemoryMode _ -> CustomGraph.load_delta mode source
  | SaveToDiskMode _ -> SaveCustomGraph.save_delta mode ~source

let get_ideps_from_hash mode hash =
  match mode with
  | InMemoryMode _
  | SaveToDiskMode _ ->
    CustomGraph.get_ideps_from_hash mode hash

let get_ideps mode dependency = get_ideps_from_hash mode (Dep.make dependency)

let get_extend_deps ~mode ~visited ~source_class ~acc =
  CustomGraph.get_extend_deps mode visited source_class acc

let add_extend_deps mode acc = CustomGraph.add_extend_deps mode acc

let add_typing_deps mode acc = CustomGraph.add_typing_deps mode acc

let add_all_deps mode acc = CustomGraph.add_all_deps mode acc

let get_member_fanout mode ~class_dep member fanout_acc =
  let (member_dep_kind, member_name) = Dep.Member.to_dep_kind_and_name member in
  CustomGraph.get_member_fanout
    mode
    class_dep
    member_dep_kind
    member_name
    fanout_acc

let get_not_subtype_fanout mode ~descendant_deps fanout_acc =
  CustomGraph.get_not_subtype_fanout mode descendant_deps fanout_acc

let dump_current_edge_buffer_in_memory_mode =
  CustomGraph.dump_current_edge_buffer
