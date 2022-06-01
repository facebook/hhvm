(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t

external make_ffi :
  root:string -> hhi_root:string -> tmp:string -> ParserOptions.t -> t
  = "hh_rust_provider_backend_make"

let make popt =
  make_ffi
    ~root:Relative_path.(path_of_prefix Root)
    ~hhi_root:Relative_path.(path_of_prefix Hhi)
    ~tmp:Relative_path.(path_of_prefix Tmp)
    popt

external push_local_changes_ffi : t -> unit
  = "hh_rust_provider_backend_push_local_changes"

external pop_local_changes_ffi : t -> unit
  = "hh_rust_provider_backend_pop_local_changes"

module Decl = struct
  module type Store = sig
    type key

    type value

    val get : t -> key -> value option

    val clear_cache : unit -> unit
  end

  module StoreWithLocalCache
      (Key : SharedMem.Key)
      (Value : SharedMem.Value) (Ffi : sig
        val get : t -> Key.t -> Value.t option

        (* val remove_batch : t -> Key.t list -> unit *)
      end) : Store with type key = Key.t and type value = Value.t = struct
    type key = Key.t

    type value = Value.t

    module Cache =
      SharedMem.FreqCache (Key) (Value)
        (struct
          let capacity = 1000
        end)

    let get t key =
      match Cache.get key with
      | Some _ as value_opt -> value_opt
      | None ->
        let value_opt = Ffi.get t key in
        (match value_opt with
        | Some value -> Cache.add key value
        | None -> ());
        value_opt

    (* let remove_batch backend keys =
       Cache.remove_batch keys;
       Ffi.remove_batch backend keys *)

    let clear_cache = Cache.clear
  end

  module Funs =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.fun_decl

        let description = "Rust_Decl_Fun"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.fun_decl option
          = "hh_rust_provider_backend_get_fun"
      end)

  module ShallowClasses =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.class_decl

        let description = "Rust_Decl_ShallowClass"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.class_decl option
          = "hh_rust_provider_backend_get_shallow_class"
      end)

  module Typedefs =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.typedef_decl

        let description = "Rust_Decl_Typedef"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.typedef_decl option
          = "hh_rust_provider_backend_get_typedef"
      end)

  module GConsts =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.const_decl

        let description = "Rust_Decl_GConst"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.const_decl option
          = "hh_rust_provider_backend_get_gconst"
      end)

  module Modules =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Shallow_decl_defs.module_decl

        let description = "Rust_Decl_Module"
      end)
      (struct
        external get : t -> string -> Shallow_decl_defs.module_decl option
          = "hh_rust_provider_backend_get_module"
      end)

  module ClassEltKey = struct
    type t = string * string

    let compare (cls1, elt1) (cls2, elt2) =
      let r = String.compare cls1 cls2 in
      if not (Core_kernel.Int.equal r 0) then
        r
      else
        String.compare elt1 elt2

    let to_string (cls, elt) = cls ^ "::" ^ elt
  end

  module Props =
    StoreWithLocalCache
      (ClassEltKey)
      (struct
        type t = Typing_defs.decl_ty

        let description = "Rust_Decl_Prop"
      end)
      (struct
        external get : t -> string * string -> Typing_defs.decl_ty option
          = "hh_rust_provider_backend_get_prop"
      end)

  module StaticProps =
    StoreWithLocalCache
      (ClassEltKey)
      (struct
        type t = Typing_defs.decl_ty

        let description = "Rust_Decl_StaticProp"
      end)
      (struct
        external get : t -> string * string -> Typing_defs.decl_ty option
          = "hh_rust_provider_backend_get_static_prop"
      end)

  let build_fun_elt fe_type =
    Typing_defs.
      {
        fe_module = None;
        fe_pos = Typing_defs.get_pos fe_type;
        fe_internal = false;
        fe_deprecated = None;
        fe_type;
        fe_php_std_lib = false;
        fe_support_dynamic_type = false;
      }

  module Methods =
    StoreWithLocalCache
      (ClassEltKey)
      (struct
        type t = Typing_defs.fun_elt

        let description = "Rust_Decl_Method"
      end)
      (struct
        external get_ffi : t -> string * string -> Typing_defs.decl_ty option
          = "hh_rust_provider_backend_get_method"

        let get t name = get_ffi t name |> Option.map ~f:build_fun_elt
      end)

  module StaticMethods =
    StoreWithLocalCache
      (ClassEltKey)
      (struct
        type t = Typing_defs.fun_elt

        let description = "Rust_Decl_StaticMethod"
      end)
      (struct
        external get_ffi : t -> string * string -> Typing_defs.decl_ty option
          = "hh_rust_provider_backend_get_static_method"

        let get t name = get_ffi t name |> Option.map ~f:build_fun_elt
      end)

  module Constructors =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Typing_defs.fun_elt

        let description = "Rust_Decl_Constructor"
      end)
      (struct
        external get_ffi : t -> string -> Typing_defs.decl_ty option
          = "hh_rust_provider_backend_get_constructor"

        let get t name = get_ffi t name |> Option.map ~f:build_fun_elt
      end)

  module FoldedClasses =
    StoreWithLocalCache
      (StringKey)
      (struct
        type t = Decl_defs.decl_class_type

        let description = "Rust_Decl_FoldedClass"
      end)
      (struct
        external get : t -> string -> Decl_defs.decl_class_type option
          = "hh_rust_provider_backend_get_folded_class"
      end)

  external direct_decl_parse_and_cache :
    t ->
    DeclParserOptions.t ->
    Relative_path.t ->
    string ->
    Direct_decl_parser.parsed_file_with_hashes
    = "hh_rust_provider_backend_direct_decl_parse_and_cache"

  external add_shallow_decls :
    t -> (string * Shallow_decl_defs.decl) list -> unit
    = "hh_rust_provider_backend_add_shallow_decls"

  let get_fun = Funs.get

  let get_shallow_class = ShallowClasses.get

  let get_typedef = Typedefs.get

  let get_gconst = GConsts.get

  let get_module = Modules.get

  let get_folded_class = FoldedClasses.get

  external declare_folded_class : t -> string -> unit
    = "hh_rust_provider_backend_declare_folded_class"
end

let push_local_changes t =
  Decl.Funs.clear_cache ();
  Decl.ShallowClasses.clear_cache ();
  Decl.Typedefs.clear_cache ();
  Decl.GConsts.clear_cache ();
  Decl.Modules.clear_cache ();
  Decl.Props.clear_cache ();
  Decl.StaticProps.clear_cache ();
  Decl.Methods.clear_cache ();
  Decl.StaticMethods.clear_cache ();
  Decl.Constructors.clear_cache ();
  Decl.FoldedClasses.clear_cache ();
  push_local_changes_ffi t

let pop_local_changes t =
  Decl.Funs.clear_cache ();
  Decl.ShallowClasses.clear_cache ();
  Decl.Typedefs.clear_cache ();
  Decl.GConsts.clear_cache ();
  Decl.Modules.clear_cache ();
  Decl.Props.clear_cache ();
  Decl.StaticProps.clear_cache ();
  Decl.Methods.clear_cache ();
  Decl.StaticMethods.clear_cache ();
  Decl.Constructors.clear_cache ();
  Decl.FoldedClasses.clear_cache ();
  pop_local_changes_ffi t

module File = struct
  type file_type =
    | Disk of string
    | Ide of string

  external get : t -> Relative_path.t -> file_type option
    = "hh_rust_provider_backend_file_provider_get"

  external get_contents : t -> Relative_path.t -> string
    = "hh_rust_provider_backend_file_provider_get_contents"

  external provide_file_for_tests : t -> Relative_path.t -> string -> unit
    = "hh_rust_provider_backend_file_provider_provide_file_for_tests"

  external provide_file_for_ide : t -> Relative_path.t -> string -> unit
    = "hh_rust_provider_backend_file_provider_provide_file_for_ide"

  external provide_file_hint : t -> Relative_path.t -> file_type -> unit
    = "hh_rust_provider_backend_file_provider_provide_file_hint"

  external remove_batch : t -> Relative_path.Set.t -> unit
    = "hh_rust_provider_backend_file_provider_remove_batch"
end

module Naming = struct
  module type ReverseNamingTable = sig
    type pos

    val add : t -> string -> pos -> unit

    val get_pos : t -> string -> pos option

    val remove_batch : t -> string list -> unit
  end

  module Types = struct
    type pos = FileInfo.pos * Naming_types.kind_of_type

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_types_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_types_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_types_remove_batch"

    external get_canon_name : t -> string -> string option
      = "hh_rust_provider_backend_naming_types_get_canon_name"
  end

  module Funs = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_funs_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_funs_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_funs_remove_batch"

    external get_canon_name : t -> string -> string option
      = "hh_rust_provider_backend_naming_funs_get_canon_name"
  end

  module Consts = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_consts_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_consts_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_consts_remove_batch"
  end

  module Modules = struct
    type pos = FileInfo.pos

    external add : t -> string -> pos -> unit
      = "hh_rust_provider_backend_naming_modules_add"

    external get_pos : t -> string -> pos option
      = "hh_rust_provider_backend_naming_modules_get_pos"

    external remove_batch : t -> string list -> unit
      = "hh_rust_provider_backend_naming_modules_remove_batch"
  end

  external get_db_path_ffi : t -> string option
    = "hh_rust_provider_backend_naming_get_db_path"

  let get_db_path t =
    get_db_path_ffi t |> Option.map ~f:(fun path -> Naming_sqlite.Db_path path)

  external set_db_path_ffi : t -> string -> unit
    = "hh_rust_provider_backend_naming_set_db_path"

  let set_db_path t (Naming_sqlite.Db_path path) = set_db_path_ffi t path

  external get_filenames_by_hash :
    t -> Typing_deps.DepSet.t -> Relative_path.Set.t
    = "hh_rust_provider_backend_naming_get_filenames_by_hash"
end
