(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Metadata = struct
  (** When we enqueue "please direct-decl-parse this file" workitems to [Direct_decl_parser.Concurrent],
  we'll attach this metadata to them. When we get back the parsed file then we'll also get back
  the exact same metadata. That way, we'll remember what work we wanted to continue with now
  that we have the parsed file back. *)
  type t = {
    types_to_fold: string list;
        (** resolve any typedefs and prefetch all ancestors *)
    types_to_ty: string list;
        (** resolve any typedefs, but don't need to prefetch ancestors *)
  }

  let empty = { types_to_fold = []; types_to_ty = [] }
end

module Concurrent = Direct_decl_parser.Concurrent (Metadata)

type telemetry = {
  num_files_parsed: int;
  num_decls_parsed: int;
}

(** This represents what things we have to prefetch *)
type to_decl = {
  types_to_fold: string list;
      (** once we've prefetched them, we'll want to resolve any typedefs, and then
        prefetch all ancestors. *)
  types_to_ty: string list;
      (** for these types, we'll resolve typedefs, but we don't need to prefetch ancestors. *)
  funs: string list;  (** we want to prefetch these fun decls *)
  gconsts: string list;  (** we want to prefetch these gconst decls *)
  modules: string list;  (** we want to prefetch these module decls *)
}

let is_reserved_type_name name =
  let open Naming_special_names.Typehints in
  is_reserved_type_hint name
  || is_reserved_global_name name
  || is_reserved_hh_name name

type find_type_result =
  | Folded of Typing_class_types.class_t Provider_backend.Decl_cache_entry.value
  | Shallow of
      Shallow_decl_defs.shallow_class
      Provider_backend.Shallow_decl_cache_entry.value
  | Typedef of Typing_defs.typedef_type Provider_backend.Decl_cache_entry.value
  | Absent

(** Looks up whether the name is present in folded classes heap, failing that
  shallow classes heap, failing that typedef. It's called "find" to indicate
  that this function merely returns a decl that's already present; it never
  goes out and fetches the decl from disk. The [accept_folded] parameter
  says whether we should look for folded classes. *)
let find_type_name
    ~(local_memory : Provider_backend.local_memory)
    ?(accept_folded = true)
    (name : string) : find_type_result =
  let open Provider_backend in
  let Provider_backend.{ decl_cache; shallow_decl_cache; _ } = local_memory in
  let class_opt =
    if accept_folded then
      Decl_cache.find decl_cache ~key:(Decl_cache_entry.Class_decl name)
    else
      None
  in
  match class_opt with
  | Some class_ -> Folded class_
  | None -> begin
    match
      Shallow_decl_cache.find
        shallow_decl_cache
        ~key:(Shallow_decl_cache_entry.Shallow_class_decl name)
    with
    | Some shallow_class -> Shallow shallow_class
    | None -> begin
      match
        Decl_cache.find decl_cache ~key:(Decl_cache_entry.Typedef_decl name)
      with
      | Some typedef -> Typedef typedef
      | None -> Absent
    end
  end

(** This structure is used during [next_missing_types], so that if we've already
  done work on 'Foo' (either to add it to the list of things that must be direct-decl-parsed,
  or discovered that it already exists in the cache), then we don't need to do
  the same work a second time. *)
type visited = {
  v_fold: SSet.t;
      (** Which [types_to_fold] have we already finished work on? *)
  v_ty: SSet.t;  (** Which [types_to_ty] have we already finished work on? *)
}

(** This is the return type from [next_missing_types]. It says what further typenames
  must be obtained from a direct-decl-parse, and what further work we'll need to do
  on them. *)
type next_to_decl = {
  next_to_fold: string list;
      (** After we've direct-decl-parsed these typenames,
    we'll want to call [next_missing_type types_to_fold] on them. *)
  next_to_ty: string list;
      (** After we've direct-decl-parsed these typenames,
    we'll want to call [next_missing_types types_to_ty] on them. *)
}

(** Our challenge here is that we know we will want to fold a class, i.e.
  [Decl_provider.get_class]. And we want to concurrently "prefetch" all the
  shallow decls (classes, typedefs, ...) that will be needed to fold that class.
  But we can't know the full answer yet! All we can do is give an incremental
  answer, for instance "you wanted to fold class Foo, and I know that we'll therefore
  need to fold its parent Bar, but Bar is currently absent from cache." What this
  function does is return a list of all things it so far discovers that are absent
  from cache but will be needed. The caller will have to direct-decl-parse them,
  add them into the cache, and then call this function again to learn if anything
  further is missing.

  [Decl_provider.get_class "Foo"] needs to fold all parents of Foo. But it also calls
  [Decl_enforceability] on each return type of each method in Foo itself.
  This needs to fetch shallow decls mentioned in those return types. Or, if they're
  typedefs, it needs to resolve the typedef by fetching its shallow decl and then
  fetching whatever the typedef expands to.

  The parameters of this function: [to_fold] are the type names we know we wish to resolve
  any typedef aliases and then fold them and all their ancestors; [to_ty] are the type
  names which we know we wish to resolve any typedef aliases but have no need to fold
  ancestors.
  *)
let next_missing_types
    ~(local_memory : Provider_backend.local_memory)
    ~(to_fold : string list)
    ~(to_ty : string list)
    ~(visited : visited) : next_to_decl * visited =
  let rec do_type_name d (goal : [ `Fold | `Ty ]) name (acc, visited) =
    let d = d + 1 in
    match goal with
    | _ when is_reserved_type_name name -> (acc, visited)
    | `Fold when SSet.mem name visited.v_fold -> (acc, visited)
    | `Ty when SSet.mem name visited.v_ty -> (acc, visited)
    | `Ty -> begin
      let visited =
        { v_ty = SSet.add name visited.v_ty; v_fold = visited.v_fold }
      in
      match find_type_name ~local_memory ~accept_folded:false name with
      | Folded _ -> failwith "didn't ask for folded"
      | Shallow _ -> (acc, visited)
      | Typedef td -> do_ty d `Ty (acc, visited) td.Typing_defs.td_type
      | Absent -> ({ acc with next_to_ty = name :: acc.next_to_ty }, visited)
    end
    | `Fold -> begin
      let visited =
        {
          v_ty = SSet.add name visited.v_ty;
          v_fold = SSet.add name visited.v_fold;
        }
      in
      match find_type_name ~local_memory name with
      | Folded _ -> (acc, visited)
      | Shallow sc -> do_sc d `Fold sc (acc, visited)
      | Typedef td -> do_ty d `Fold (acc, visited) td.Typing_defs.td_type
      | Absent -> ({ acc with next_to_fold = name :: acc.next_to_fold }, visited)
    end
  and do_sc d (_goal : [ `Fold ]) sc (acc, visited) =
    let open Shallow_decl_defs in
    let d = d + 1 in
    let prop_ sp = (sp.sp_name, sp.sp_type, true) in
    let method_ sm =
      (sm.sm_name, sm.sm_type, not (MethodFlags.get_abstract sm.sm_flags))
    in
    match sc.sc_enum_type with
    | Some et ->
      let open Typing_defs in
      (* [enum class Name: te_base as te_constraint extends te_includes], and sc_extends
         is made of te_includes plus either \HH\BuiltinEnum or \HH\BuiltinAbstractEnumClass.
         Decl-folding needs most of those folded, except it only needs \HH\Builtin shallow. *)
      let ty = Option.value et.te_constraint ~default:et.te_base in
      let (acc, visited) = do_ty d `Fold (acc, visited) ty in
      List.fold et.te_includes ~init:(acc, visited) ~f:(do_ty d `Fold)
      |> do_sc_ancestors d `Ty sc.sc_extends
    | None ->
      (acc, visited)
      |> do_sc_ancestors d `Fold sc.sc_extends
      |> do_sc_ancestors d `Fold sc.sc_uses
      |> do_sc_ancestors d `Fold sc.sc_implements
      |> do_sc_ancestors d `Fold sc.sc_req_extends
      |> do_sc_ancestors d `Fold sc.sc_req_implements
      |> do_sc_ancestors d `Fold sc.sc_req_class
      |> do_sc_ancestors d `Fold sc.sc_xhp_attr_uses
      |> do_sc_members d `Ty (List.map sc.sc_props ~f:prop_)
      |> do_sc_members d `Ty (List.map sc.sc_sprops ~f:prop_)
      |> do_sc_members d `Ty (List.map sc.sc_methods ~f:method_)
      |> do_sc_members d `Ty (List.map sc.sc_static_methods ~f:method_)
  and do_sc_ancestors d (goal : [ `Fold | `Ty ]) ancestors (acc, visited) =
    List.fold ancestors ~init:(acc, visited) ~f:(fun (acc, visited) ty ->
        match Typing_defs.get_node ty with
        | Typing_defs.Tapply ((_pos, name), _args) ->
          do_type_name d goal name (acc, visited)
        | _ -> (acc, visited))
  and do_sc_members d (_goal : [ `Ty ]) members (acc, visited) =
    List.fold
      members
      ~init:(acc, visited)
      ~f:(fun (a, v) ((_, name), ty, check) ->
        let (a, v) =
          if check then
            do_ty d `Ty (a, v) ty
          else
            (a, v)
        in
        if String.equal name Naming_special_names.Members.__toString then
          do_type_name
            d
            `Fold
            Naming_special_names.Classes.cStringishObject
            (a, v)
        else
          (a, v))
  and do_ty d (goal : [ `Fold | `Ty ]) (acc, visited) ty =
    let open Typing_defs in
    let d = d + 1 in
    match Typing_defs.get_node ty with
    | Tthis
    | Tmixed
    | Tany _
    | Trefinement _
    | Twildcard
    | Tnonnull
    | Tdynamic
    | Tprim _
    | Tgeneric _
    | Tshape _
    | Tunion _
    | Tintersection _
    | Ttuple _ ->
      (acc, visited)
    | Toption ty
    | Tvec_or_dict (_, ty)
    | Tlike ty ->
      do_ty d goal (acc, visited) ty
    | Tfun { ft_ret = { et_type = ty; _ }; ft_flags; _ } -> begin
      match get_node ty with
      | Tapply ((_, name), [ty])
        when Typing_defs_flags.Fun.async ft_flags
             && String.equal name Naming_special_names.Classes.cAwaitable ->
        do_ty d goal (acc, visited) ty
      | _ -> do_ty d goal (acc, visited) ty
    end
    | Tnewtype (_name, tys, ty) ->
      let (acc, visited) = do_ty d goal (acc, visited) ty in
      List.fold tys ~init:(acc, visited) ~f:(do_ty d `Ty)
    | Tapply ((_, name), _tys) -> do_type_name d goal name (acc, visited)
    | Taccess _ -> (acc, visited)
  in
  let acc = { next_to_fold = []; next_to_ty = [] } in
  let (acc, visited) =
    List.fold to_fold ~init:(acc, visited) ~f:(fun (acc, visited) name ->
        do_type_name 0 `Fold name (acc, visited))
  in
  let (acc, visited) =
    List.fold to_ty ~init:(acc, visited) ~f:(fun (acc, visited) name ->
        do_type_name 0 `Ty name (acc, visited))
  in
  (acc, visited)

(** Given a set of toplevel symbol names [to_decl] which we have to decl,
  this function looks up the Naming_provider to consolidate them into a list
  of filenames that cover them all.
  * For names in [to_decl.funs] and the like, that's the end of the story:
    whoever direct-decl-parses that filename will get the decls they need.
  * For names in [to_decl.types_to_ty], then further work will be needed
    after the decls have been fetched, to dig through type aliases.
  * For names in [to_decl.types_to_fold], then the further work will
    be to dig through type aliases if necessary, then fold.

  This function, for each consolidated filename, also returns the
  [(to_decl.types_to_fold, to_decl.types_to_ty)] associated with
  that filename.

  Also, if the entry has file-content (e.g. due to being an IDE open file)
  then it returns that content. *)
let resolve_filenames_to_decl ctx (to_decl : to_decl) :
    (Relative_path.t * Metadata.t * Concurrent.content) list =
  let add_to_fold acc name path_opt =
    Option.value_map path_opt ~default:acc ~f:(fun path ->
        let Metadata.{ types_to_fold; types_to_ty } =
          Relative_path.Map.find_opt acc path
          |> Option.value ~default:Metadata.empty
        in
        Relative_path.Map.add
          acc
          ~key:path
          ~data:Metadata.{ types_to_fold = name :: types_to_fold; types_to_ty })
  in
  let add_to_ty acc name path_opt =
    Option.value_map path_opt ~default:acc ~f:(fun path ->
        let Metadata.{ types_to_fold; types_to_ty } =
          Relative_path.Map.find_opt acc path
          |> Option.value ~default:Metadata.empty
        in
        Relative_path.Map.add
          acc
          ~key:path
          ~data:Metadata.{ types_to_fold; types_to_ty = name :: types_to_ty })
  in
  let add acc path_opt =
    Option.value_map path_opt ~default:acc ~f:(fun path ->
        let data =
          Relative_path.Map.find_opt acc path
          |> Option.value ~default:Metadata.empty
        in
        Relative_path.Map.add acc ~key:path ~data)
  in
  let acc = Relative_path.Map.empty in
  let acc =
    List.fold to_decl.types_to_fold ~init:acc ~f:(fun acc name ->
        Naming_provider.get_type_path ctx name |> add_to_fold acc name)
  in
  let acc =
    List.fold to_decl.types_to_ty ~init:acc ~f:(fun acc name ->
        Naming_provider.get_type_path ctx name |> add_to_ty acc name)
  in
  let acc =
    List.fold to_decl.funs ~init:acc ~f:(fun acc name ->
        Naming_provider.get_fun_path ctx name |> add acc)
  in
  let acc =
    List.fold to_decl.gconsts ~init:acc ~f:(fun acc name ->
        Naming_provider.get_const_path ctx name |> add acc)
  in
  let acc =
    List.fold to_decl.modules ~init:acc ~f:(fun acc name ->
        Naming_provider.get_module_path ctx name |> add acc)
  in
  let entries = Provider_context.get_entries ctx in
  Relative_path.Map.bindings acc
  |> List.map ~f:(fun (file, tag) ->
         let content =
           Relative_path.Map.find_opt entries file
           |> Option.bind ~f:Provider_context.get_file_contents_if_present
         in
         (file, tag, content))

(** This is the main loop for prefetching decls. It repeats: (1) find the [next_missing_types]
  beyond those that are already in cache; (2) resolve them to filenames via Naming_provider;
  (3) dispatch those filenames to [Direct_decl_parse.Concurrent]; (4) block until the next
  available result from [Direct_decl_parse.Concurrent], and repeat from step 1, until no
  remaining types are missing. *)
let rec prefetch_loop
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(handle : Concurrent.handle)
    ~(to_decl : to_decl)
    ~(visited : visited)
    ~(telemetry : telemetry) : telemetry =
  let (next_to_decl, visited) =
    next_missing_types
      ~local_memory
      ~to_fold:to_decl.types_to_fold
      ~to_ty:to_decl.types_to_ty
      ~visited
  in
  let to_decl_by_path =
    resolve_filenames_to_decl
      ctx
      {
        to_decl with
        types_to_fold = next_to_decl.next_to_fold;
        types_to_ty = next_to_decl.next_to_ty;
      }
  in
  (* The follow concurrent call will stick new items into the "to_decl" queue,
     but we'll get back answers from either the [to_decl] in this iteration,
     or from any earlier [to_decl] from an earlier iteration. *)
  match
    Concurrent.enqueue_next_and_get_earlier_results handle to_decl_by_path
  with
  | None -> telemetry
  | Some
      ( earlier_file,
        Metadata.
          { types_to_fold = earlier_to_fold; types_to_ty = earlier_to_ty },
        earlier_decls ) ->
    (* We get back [Direct_decl_parser.parsed_file]. The function [Direct_decl_utils.cache_decls] actually
       expected a [parsed_file_with_hashes] which is almost identical except it also has a decl-hash.
       Since that hash gets ignored by [Direct_decl_utils.cache_decls] anyway, we're just going to
       stick in a dummy [Int64.zero]. *)
    let earlier_decls =
      List.map earlier_decls.Direct_decl_parser.pf_decls ~f:(fun (name, decl) ->
          (name, decl, Int64.zero))
    in
    Direct_decl_utils.cache_decls ctx earlier_file earlier_decls;
    (* Now that the decls are in cache, we're unblocked to revisit those decls and from
       them explore further what we need to continue prep. *)
    let visited =
      List.fold earlier_to_fold ~init:visited ~f:(fun v name ->
          { v with v_fold = SSet.remove name v.v_fold })
    in
    let visited =
      List.fold earlier_to_ty ~init:visited ~f:(fun v name ->
          { v with v_ty = SSet.remove name v.v_ty })
    in
    let to_decl =
      {
        types_to_fold = earlier_to_fold;
        types_to_ty = earlier_to_ty;
        funs = [];
        gconsts = [];
        modules = [];
      }
    in
    let telemetry =
      {
        num_files_parsed = telemetry.num_files_parsed + 1;
        num_decls_parsed =
          telemetry.num_decls_parsed + List.length earlier_decls;
      }
    in
    prefetch_loop ~ctx ~local_memory ~handle ~to_decl ~visited ~telemetry

(** This function incrementally fetches into [local_memory] caches at least all
  the direct-decl-parser results needed for [to_decl] and which aren't
  already in [local_memory] caches:
  * For [to_decl.funs,consts,modules] it places these named decls into [local_memory.decl_cache]
  * For [to_decl.types_to_shallow] if the name resolves to a classlike then it places
    that decl into [local_memory.shallow_decl_cache]; if it resolves to a typedef
    then it goes into [local_memory.decl_cache].
  * For [to_decl.types_to_fold] if the type name resolves to classlike
    then it anticipates what shallow decls and typedefs that [declare_folded_class]
    will need to fold it, and places them into [local_memory.shallow_decl_cache,decl_cache].
    If the type name resolves to a typedef, then it fetches that typedef decl
    into [local_memory.decl_cache], and recursively does "types_to_fold" for all
    named types in the expansion of the typedef.
  * Just like normal decl-fetching works, if ever we need part of the output of
    the direct-decl-parser for a file, then we take the opportunity to place
    all of the file's decls into [local_memory] caches.
  * This function is robust against missing decls

  What does "incrementally fetches" mean? Say we have [to_fold="D"]. We might
  fetch the shallow-decl for D, then learn that it extends C, then fetch
  the shallow-decl for C, then learn that it extends A and B and so fetch
  those shallow-decls too. In folding, we only learn what further things to
  fetch after we've already started. This function also is able to see which
  decls are already present in cache, and avoids fetching them again.

  This function aims to be fast. The slow part of fetching is direct-decl-parsing.
  This function uses multiple concurrent threads under the hood. For instance
  given [to_fold="C"], if we fetched C and discovered that C extends both A and B,
  then we can use two concurrent threads to decl-parse A and B.

  What does "anticipate what will be needed to fold a class" mean? Say we have
  [to_fold="D"]. The [declare_folded_class] algorithm will first transitively
  fold all of D's ancestors, and then (for pessimisation) it will shallow-fetch
  everything mentioned in return types.

  Why "at least all the decls"? The pessimisation step walks the return type
  of functions in a subtle way. This function instead just blindly grabs everything
  mentioned in the return type.

  Why "robust against missing decls"? This might arise in a few ways. First,
  if the user writes "class C extends B {}" but B isn't yet defined, then
  we simply won't fetch B. If the naming table thought that B was defined in b.php,
  but a disk change has happened (and the naming table hasn't yet been updated)
  then we'll parse the file if it exists, and either we'll find B or not.

  Sorry, our decl-caches are ugly. That's because [decl_cache] contains
  funs, typedefs, consts, modules (primitive things) and also folded-classes,
  while [shallow_decl_cache] contains shallow-classes (primitive). *)
let prefetch_decls ~ctx ~local_memory to_decl =
  let opts =
    Provider_context.get_popt ctx |> DeclParserOptions.from_parser_options
  in
  let root = Relative_path.path_of_prefix Relative_path.Root |> Path.make in
  let hhi = Relative_path.path_of_prefix Relative_path.Hhi |> Path.make in
  let tmp = Relative_path.path_of_prefix Relative_path.Tmp |> Path.make in
  let dummy = Relative_path.path_of_prefix Relative_path.Dummy |> Path.make in
  let handle = Concurrent.start ~opts ~root ~hhi ~tmp ~dummy in
  let telemetry = { num_decls_parsed = 0; num_files_parsed = 0 } in
  let visited = { v_ty = SSet.empty; v_fold = SSet.empty } in
  prefetch_loop ~ctx ~local_memory ~handle ~to_decl ~visited ~telemetry
