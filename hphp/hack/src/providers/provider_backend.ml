(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Decl_cache_entry = struct
  (* NOTE: we can't simply use a string as a key. In the case of a name
  conflict, we may put e.g. a function named 'foo' into the cache whose value is
  one type, and then later try to withdraw a class named 'foo' whose value is
  another type.

  The actual value type for [Class_decl] is a [Typing_classes_heap.Classes.t],
  but that module depends on this module, so we can't write it down or else we
  will cause a circular dependency. (It could probably be refactored to break
  the dependency.) We just use [Obj.t] instead, which is better than using
  [Obj.t] for all of the cases here.
  *)
  type _ t =
    | Fun_decl : string -> Typing_defs.fun_elt t
    | Class_decl : string -> Obj.t t
    | Record_decl : string -> Typing_defs.record_def_type t
    | Typedef_decl : string -> Typing_defs.typedef_type t
    | Gconst_decl : string -> (Typing_defs.decl_ty * Errors.t) t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value:_ = 1

  let key_to_log_string : type a. a key -> string =
   fun key ->
    match key with
    | Fun_decl s -> "FunDecl" ^ s
    | Class_decl s -> "ClassDecl" ^ s
    | Record_decl s -> "RecordDecl" ^ s
    | Typedef_decl s -> "TypedefDecl" ^ s
    | Gconst_decl s -> "GconstDecl" ^ s
end

module Decl_cache = Lru_cache.Cache (Decl_cache_entry)

module Shallow_decl_cache_entry = struct
  type _ t = Shallow_class_decl : string -> Shallow_decl_defs.shallow_class t

  type 'a key = 'a t

  type 'a value = 'a

  let get_size ~key:_ ~value =
    let words = Obj.reachable_words (Obj.repr value) in
    let bytes = words * Sys.word_size / 8 in
    bytes

  let key_to_log_string : type a. a key -> string =
   (fun (Shallow_class_decl key) -> "ClasssShallow" ^ key)
end

module Shallow_decl_cache = Lru_cache.Cache (Shallow_decl_cache_entry)

type fixme_map = Pos.t IMap.t IMap.t

module Fixme_store = struct
  type t = fixme_map Relative_path.Map.t ref

  let empty = ref Relative_path.Map.empty

  let get t filename = Relative_path.Map.find_opt !t filename

  let add t filename fixmes = t := Relative_path.Map.add !t filename fixmes

  let remove t filename = t := Relative_path.Map.remove !t filename

  let remove_batch t filenames =
    t :=
      Relative_path.Set.fold filenames ~init:!t ~f:(fun filename map ->
          Relative_path.Map.remove map filename)
end

module Fixmes = struct
  type t = {
    hh_fixmes: Fixme_store.t;
    decl_hh_fixmes: Fixme_store.t;
    disallowed_fixmes: Fixme_store.t;
  }
end

let empty_fixmes =
  Fixmes.
    {
      hh_fixmes = Fixme_store.empty;
      decl_hh_fixmes = Fixme_store.empty;
      disallowed_fixmes = Fixme_store.empty;
    }

type t =
  | Shared_memory
  | Local_memory of {
      decl_cache: Decl_cache.t;
      shallow_decl_cache: Shallow_decl_cache.t;
      fixmes: Fixmes.t;
    }
  | Decl_service of {
      decl: Decl_service_client.t;
      fixmes: Fixmes.t;
    }

let t_to_string (t : t) : string =
  match t with
  | Shared_memory -> "Shared_memory"
  | Local_memory _ -> "Local_memory"
  | Decl_service _ -> "Decl_service"

let backend_ref = ref Shared_memory

let set_shared_memory_backend () : unit = backend_ref := Shared_memory

let set_local_memory_backend
    ~(max_num_decls : int) ~(max_bytes_shallow_decls : int) : unit =
  backend_ref :=
    Local_memory
      {
        decl_cache = Decl_cache.make ~max_size:max_num_decls;
        shallow_decl_cache =
          Shallow_decl_cache.make ~max_size:max_bytes_shallow_decls;
        fixmes = empty_fixmes;
      }

let set_local_memory_backend_with_defaults () : unit =
  (* TODO(ljw): Figure out a good size. Some files read ~5k shallow
  decls to typecheck, at about 16k / shallow decl, fitting into 73Mb.
  I figure that 140Mb feels right. *)
  let max_bytes_shallow_decls = 140 * 1024 * 1024 in
  let max_num_decls = 5000 in
  set_local_memory_backend ~max_num_decls ~max_bytes_shallow_decls

let set_decl_service_backend (decl : Decl_service_client.t) : unit =
  backend_ref := Decl_service { decl; fixmes = empty_fixmes }

let get () : t = !backend_ref
