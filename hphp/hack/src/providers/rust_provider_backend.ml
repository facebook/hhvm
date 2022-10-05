(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t

external register_custom_types : unit -> unit
  = "hh_rust_provider_backend_register_custom_types"

let () = register_custom_types ()

external make_ffi :
  root:string -> hhi_root:string -> tmp:string -> ParserOptions.t -> t
  = "hh_rust_provider_backend_make"

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

        val remove_batch : t -> Key.t list -> unit
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

    let remove_batch backend (keys : Key.t list) =
      List.iter ~f:Cache.remove keys;
      Ffi.remove_batch backend keys
      [@@ocaml.warning "-32"]
    (* Currently an unused value. *)

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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_funs_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_shallow_classes_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_typedefs_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_gconsts_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_modules_batch"
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

        external remove_batch : t -> (string * string) list -> unit
          = "hh_rust_provider_backend_remove_props_batch"
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

        external remove_batch : t -> (string * string) list -> unit
          = "hh_rust_provider_backend_remove_static_props_batch"
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

        external remove_batch : t -> (string * string) list -> unit
          = "hh_rust_provider_backend_remove_methods_batch"
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

        external remove_batch : t -> (string * string) list -> unit
          = "hh_rust_provider_backend_remove_static_methods_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_constructors_batch"
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

        external remove_batch : t -> string list -> unit
          = "hh_rust_provider_backend_remove_folded_classes_batch"
      end)

  let decl_store t =
    let noop_add _ _ = () in
    let noop () = () in
    Decl_store.
      {
        add_prop = noop_add;
        get_prop = Props.get t;
        add_static_prop = noop_add;
        get_static_prop = StaticProps.get t;
        add_method = noop_add;
        get_method = Methods.get t;
        add_static_method = noop_add;
        get_static_method = StaticMethods.get t;
        add_constructor = noop_add;
        get_constructor = Constructors.get t;
        add_class = noop_add;
        get_class = FoldedClasses.get t;
        add_fun = noop_add;
        get_fun = Funs.get t;
        add_typedef = noop_add;
        get_typedef = Typedefs.get t;
        add_gconst = noop_add;
        get_gconst = GConsts.get t;
        add_module = noop_add;
        get_module = Modules.get t;
        pop_local_changes = noop;
        push_local_changes = noop;
      }

  let did_set_decl_store = ref false

  let set_decl_store t =
    if not !did_set_decl_store then (
      did_set_decl_store := true;
      Decl_store.set (decl_store t)
    )

  external direct_decl_parse_and_cache :
    t -> Relative_path.t -> string -> Direct_decl_parser.parsed_file_with_hashes
    = "hh_rust_provider_backend_direct_decl_parse_and_cache"

  let direct_decl_parse_and_cache t =
    set_decl_store t;
    direct_decl_parse_and_cache t

  external add_shallow_decls :
    t -> (string * Shallow_decl_defs.decl) list -> unit
    = "hh_rust_provider_backend_add_shallow_decls"

  let add_shallow_decls t =
    set_decl_store t;
    add_shallow_decls t

  let get_fun t =
    set_decl_store t;
    Funs.get t

  let get_shallow_class t =
    set_decl_store t;
    ShallowClasses.get t

  let get_typedef t =
    set_decl_store t;
    Typedefs.get t

  let get_gconst t =
    set_decl_store t;
    GConsts.get t

  let get_module t =
    set_decl_store t;
    Modules.get t

  let get_folded_class t =
    set_decl_store t;
    FoldedClasses.get t

  external declare_folded_class : t -> string -> unit
    = "hh_rust_provider_backend_declare_folded_class"

  let declare_folded_class t =
    set_decl_store t;
    declare_folded_class t
end

let make popt =
  let backend =
    make_ffi
      ~root:Relative_path.(path_of_prefix Root)
      ~hhi_root:Relative_path.(path_of_prefix Hhi)
      ~tmp:Relative_path.(path_of_prefix Tmp)
      popt
  in
  Decl.set_decl_store backend;
  backend

let set backend = Decl.set_decl_store backend

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
