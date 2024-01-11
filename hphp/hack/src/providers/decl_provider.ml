(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Class = Typing_classes_heap.Api

type fun_key = string

type type_key = string

type gconst_key = string

type module_key = string

type fun_decl = Typing_defs.fun_elt

type class_decl = Typing_classes_heap.Api.t

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.const_decl

type module_decl = Typing_defs.module_def_type

(** This cache caches the result of full class computations
      (the class merged with all its inherited members.)  *)
module Cache =
  SharedMem.FreqCache
    (StringKey)
    (struct
      type t = Typing_classes_heap.class_t

      let description = "Decl_Typing_ClassType"
    end)
    (struct
      let capacity = 1000
    end)

let declare_folded_class (ctx : Provider_context.t) (name : type_key) :
    Decl_defs.decl_class_type * Decl_store.class_members option =
  let f () =
    match
      Errors.run_in_decl_mode (fun () ->
          Decl_folded_class.class_decl_if_missing ~sh:SharedMem.Uses ctx name)
    with
    | None -> Decl_defs.raise_decl_not_found None name
    | Some decl_and_members -> decl_and_members
  in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Local_memory local_memory
    when Disk.is_real_disk
         && Option.is_none
              (Provider_backend.Decl_cache.find
                 local_memory.Provider_backend.decl_cache
                 ~key:(Provider_backend.Decl_cache_entry.Class_decl name)) ->
    let open Provider_backend in
    Shallow_decl_cache.without_collections
      local_memory.shallow_decl_cache
      ~f:(fun () ->
        Decl_cache.without_collections local_memory.decl_cache ~f:(fun () ->
            let to_decl =
              Decl_provider_prefetch.
                {
                  types_to_fold = [name];
                  types_to_ty = [];
                  funs = [];
                  gconsts = [];
                  modules = [];
                }
            in
            let (_telemetry : Decl_provider_prefetch.telemetry) =
              Decl_provider_prefetch.prefetch_decls ~ctx ~local_memory to_decl
            in
            f ()))
  | _ -> f ()

let lookup_or_populate_class_cache class_name populate =
  match Cache.get class_name with
  | Some _ as result -> result
  | None -> begin
    match populate class_name with
    | None -> None
    | Some v as result ->
      Cache.add class_name v;
      result
  end

let get_class
    ?(tracing_info : Decl_counters.tracing_info option)
    (ctx : Provider_context.t)
    (class_name : type_key) : class_decl Decl_entry.t =
  Decl_counters.count_decl ?tracing_info Decl_counters.Class class_name
  @@ fun counter ->
  (* There are several possibilities:
     LOCAL BACKEND - the class_t is cached in the local backend.
     SHAREDMEM BACKEND - the class_t is cached in the worker-local 'Cache' heap.
       Note that in the case of eager, the class_t is really just a fairly simple
       derivation of the decl_class_type that lives in shmem.
     DECL BACKEND - the class_t is cached in the worker-local 'Cache' heap *)
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Decl_store.((get ()).get_class class_name)
          |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
    with
    | None -> Decl_entry.DoesNotExist
    | Some v -> Decl_entry.Found (counter, v, Some ctx)
  end
  | Provider_backend.Pessimised_shared_memory _ -> begin
    (* No pessimisation needs to be done here directly. All pessimisation is
     * done on the shallow classes within [Shallow_classes_provider] that the
     * [Typing_classes_heap.Api.t] returned here is constructed from
     * Crucially, we do not use the [Cache] here, which would contain
     * outdated member types once we update its members during
     * pessimisation. *)
    match Typing_classes_heap.get ctx class_name declare_folded_class with
    | None -> Decl_entry.DoesNotExist
    | Some v -> Decl_entry.Found (counter, v, Some ctx)
  end
  | Provider_backend.Shared_memory -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Typing_classes_heap.get ctx class_name declare_folded_class)
    with
    | None -> Decl_entry.DoesNotExist
    | Some v -> Decl_entry.Found (counter, v, Some ctx)
  end
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } -> begin
    match
      Typing_classes_heap.get_class_with_cache
        ctx
        class_name
        decl_cache
        declare_folded_class
    with
    | None -> Decl_entry.DoesNotExist
    | Some cls -> Decl_entry.Found (counter, cls, Some ctx)
  end
  | Provider_backend.Rust_provider_backend backend -> begin
    match
      lookup_or_populate_class_cache class_name (fun class_name ->
          Rust_provider_backend.Decl.get_folded_class
            backend
            (Naming_provider.rust_backend_ctx_proxy ctx)
            class_name
          |> Option.map ~f:Typing_classes_heap.make_eager_class_decl)
    with
    | None -> Decl_entry.DoesNotExist
    | Some v -> Decl_entry.Found (counter, v, Some ctx)
  end

let maybe_pessimise_fun_decl ctx fun_decl =
  if Provider_context.implicit_sdt_for_fun ctx fun_decl then
    let no_auto_likes = Provider_context.no_auto_likes_for_fun fun_decl in
    Typing_defs.
      {
        fun_decl with
        fe_type =
          Decl_enforceability.(
            pessimise_fun_type
              ~fun_kind:Function
              ~this_class:None
              ~no_auto_likes
              ctx
              fun_decl.fe_pos
              fun_decl.fe_type);
      }
  else
    fun_decl

let maybe_pessimise_typedef_decl ctx typedef_decl =
  if
    TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx)
    && not
         (Typing_defs.Attributes.mem
            Naming_special_names.UserAttributes.uaNoAutoDynamic
            typedef_decl.Typing_defs.td_attributes)
  then
    (* TODO: deal with super constraint *)
    match typedef_decl.Typing_defs.td_as_constraint with
    | Some _ -> typedef_decl
    | None ->
      let open Typing_defs in
      let pos = typedef_decl.td_pos in
      {
        typedef_decl with
        td_as_constraint =
          Some
            (Decl_enforceability.supportdyn_mixed
               pos
               (Reason.Rwitness_from_decl pos));
      }
  else
    typedef_decl

let get_fun ?tracing_info ctx name =
  Decl_counters.count_decl Decl_counters.Fun ?tracing_info name
  @@ fun _counter ->
  Decl_provider_internals.get_fun_without_pessimise ctx name
  |> Decl_entry.map ~f:(maybe_pessimise_fun_decl ctx)

let get_shallow_class ctx name =
  Decl_provider_internals.get_shallow_class ctx name

let get_typedef ?tracing_info ctx name =
  Decl_counters.count_decl Decl_counters.Typedef ?tracing_info name
  @@ fun _counter ->
  Decl_provider_internals.get_typedef_without_pessimise ctx name
  |> Decl_entry.map ~f:(maybe_pessimise_typedef_decl ctx)

let get_gconst ?tracing_info ctx name =
  Decl_counters.count_decl Decl_counters.GConst ?tracing_info name
  @@ fun _counter -> Decl_provider_internals.get_gconst ctx name

let get_module ?tracing_info ctx name =
  Decl_counters.count_decl Decl_counters.Module_decl ?tracing_info name
  @@ fun _counter -> Decl_provider_internals.get_module ctx name

let get_overridden_method ctx ~class_name ~method_name ~is_static :
    Typing_defs.class_elt Decl_entry.t =
  Decl_entry.bind (get_class ctx class_name) @@ fun cls ->
  let get_class ctx x = Decl_entry.to_option (get_class ctx x) in
  Decl_entry.of_option_or_does_not_exist
  @@ Class.overridden_method cls ~method_name ~is_static ~get_class

(** This is a subtle function! If there is a winner defined for [name_type / name]
with that exact same capitalization, then it will return its position, otherwise
it will return None.

Example: [type t=int; type T=string;] where "t=int" is the winner.
* [get_pos_from_decl_of_winner Typedef "t"] --> Some
* [get_pos_from_decl_of_winner Typedef "T"] --> None
* [get_pos_from_decl_of_winner Class "T"] --> None

Example: [type tc=int; class tc {}] where "class tc" is the winner.
* [get_pos_from_decl_of_winner Class "tc"] --> Some
* [get_pos_from_decl_of_winner Typedef "tc"] --> None
* [get_pos_from_decl_of_winner Class "TC"] --> None

Why such a subtle function? We're trying to thread the needle between what's
efficient to obtain from the decl-provider without needing a costly
step of "obtain the canonical capitalization of this symbol", vs what's the
minimum correctness needed to support the function [is_this_def_the_winner].

Several branches of this function must first query Naming_provider
before it's safe to go on and query Decl_provider. This introduces
the possibility of disk races e.g. if the file has changed on disk but
we haven't yet gotten around to processing the file-change notification,
then Naming_provider will tell us that the symbol is in the file but
when Decl_provider comes to read it off disk then it's no longer there.

It's also vulnerable to a weird quirk with namespaces.
Consider "namespace N; namespace M {function f(){} }".
This gives Parsing[1002] "Cannot mix bracketed+unbracketed namespaces", but
naturally AST-parser and direct-decl-parser both still have to produce something for it.
AST claims it has symbol M\f, while direct-decl-parser claims N\M\f.
If the naming-table happened to give results from AST-parser
(e.g. as happens when Provider_context.entry provides overrides to the naming-table)
then we'll hit the same situation.

In both these cases, we return None from this function, indicating that we
couldn't find a decl position. It'd be better if naming and decl providers were
more tightly coupled, and had consistency-correctness, so that neither case
were possible. *)
let get_pos_from_decl_of_winner_FOR_TESTS_ONLY ctx name_type name : Pos.t option
    =
  let pos_opt =
    match name_type with
    | FileInfo.Fun ->
      if Naming_provider.get_fun_path ctx name |> Option.is_some then
        get_fun ctx name
        |> Decl_entry.to_option
        |> Option.map ~f:(fun { Typing_defs.fe_pos; _ } -> fe_pos)
      else
        None
    | FileInfo.Typedef ->
      if Naming_provider.get_typedef_path ctx name |> Option.is_some then
        get_typedef ctx name
        |> Decl_entry.to_option
        |> Option.map ~f:(fun { Typing_defs.td_pos; _ } -> td_pos)
      else
        None
    | FileInfo.Class ->
      if Naming_provider.get_class_path ctx name |> Option.is_some then
        get_class ctx name
        |> Decl_entry.to_option
        |> Option.map ~f:(fun cls -> Class.pos cls)
      else
        None
    | FileInfo.Const ->
      Option.map (get_gconst ctx name) ~f:(fun { Typing_defs.cd_pos; _ } ->
          cd_pos)
    | FileInfo.Module ->
      Option.map (get_module ctx name) ~f:(fun { Typing_defs.mdt_pos; _ } ->
          mdt_pos)
  in
  Option.map pos_opt ~f:Pos_or_decl.unsafe_to_raw_pos

type winner =
  | Winner
  | Loser_to of Pos.t
  | Not_found

let is_this_def_the_winner ctx name_type (pos, name) =
  match
    (get_pos_from_decl_of_winner_FOR_TESTS_ONLY ctx name_type name, name_type)
  with
  | (Some winner_pos, _) when Pos.overlaps pos winner_pos ->
    (* There is a winner decl for [name_type name], the exact same name_type and capitalization
       as we provided, and its position overlaps. Therefore [name_type / name / pos] is the winner!
       The winner for [name_type / name] has the exact same capitalization and name_type,
       We use "overlaps" to allow flexibility for whether the
       pos associated with the AST refers to the pos of the identifier token or the pos of the whole
       thing, and likewise the pos associated the decl. *)
    Winner
  | (Some winner_pos, _) ->
    (* There is a winner decl for [name_type name], the exact same name_type and capitalization
       as we provided, but it is at a different position. Therefore we are the loser. *)
    Loser_to winner_pos
  | (None, FileInfo.(Const | Module)) ->
    (* There is no winner decl for [name_type name]. These name-types are case-sensitive,
       so we don't need to look further. *)
    Not_found
  | (None, FileInfo.Fun) -> begin
    (* If there wasn't a winner decl for [Fun name], then maybe there is for a different
       capitalization of [Name]? Note: this codepath results in either [Not_found] or [Loser_to],
       neither of which occur in a program that typechecks clean, so it's okay if they're a little slow.
       It technically has one path which returns [Winner] but we believe and assert that it will never arise. *)
    match Naming_provider.get_fun_canon_name ctx name with
    | None -> Not_found
    | Some cname -> begin
      match get_pos_from_decl_of_winner_FOR_TESTS_ONLY ctx name_type cname with
      | None -> Not_found
      | Some winner_pos when Pos.overlaps pos winner_pos ->
        HackEventLogger.invariant_violation_bug
          "caller provided wrong capitalization of fun name (unnecessarily slow path; should avoid)"
          ~pos:(Pos.to_relative_string pos |> Pos.string)
          ~data:(Printf.sprintf "name=%s cname=%s" name cname);
        Winner
      | Some winner_pos -> Loser_to winner_pos
    end
  end
  | (None, FileInfo.(Class | Typedef)) ->
    (* If there wasn't a winner decl for [name_type name], then maybe there is a winning
       decl for a different capitalization of [name]? or for the other [name_type]?
       Note: this codepath results in either [Not_found] or [Loser_to], niether of which
       occur in a program that typechecks clean, so it's okay if they're a little slow.
       It technically has one path which returns [Winner] but we believe and assert that it will never arise. *)
    (match Naming_provider.get_type_canon_name ctx name with
    | None -> Not_found
    | Some cname ->
      let winner_pos_opt =
        Option.first_some
          (get_pos_from_decl_of_winner_FOR_TESTS_ONLY ctx FileInfo.Class cname)
          (get_pos_from_decl_of_winner_FOR_TESTS_ONLY
             ctx
             FileInfo.Typedef
             cname)
      in
      (match winner_pos_opt with
      | None -> Not_found
      | Some winner_pos when Pos.overlaps pos winner_pos ->
        HackEventLogger.decl_consistency_bug
          "caller provided wrong capitalization of type name (unnecessarily slow path; should avoid)"
          ~pos:(Pos.to_relative_string pos |> Pos.string)
          ~data:(Printf.sprintf "name=%s cname=%s" name cname);
        Winner
      | Some winner_pos -> Loser_to winner_pos))

let local_changes_push_sharedmem_stack () =
  Shallow_classes_heap.Classes.LocalChanges.push_stack ();
  Decl_store.((get ()).push_local_changes ())

let local_changes_pop_sharedmem_stack () =
  Shallow_classes_heap.Classes.LocalChanges.pop_stack ();
  Decl_store.((get ()).pop_local_changes ())

let declare_folded_class_in_file_FOR_TESTS_ONLY ctx cid =
  fst (declare_folded_class ctx cid)
