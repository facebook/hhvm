(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open Typing_defs
module Cls = Folded_class

[@@@alert "-dependencies"]
(* No typing env here *)

type member_class =
  | Class_set of SSet.t
  | Subclasses_of of string

type action_internal =
  | IClass of string
  | IExplicitClass of string
  | IMember of member_class * member
  | IFunction of string
  | IGConst of string

let process_fun_id target_fun id =
  if String.equal target_fun (snd id) then
    Pos.Map.singleton (fst id) (snd id)
  else
    Pos.Map.empty

let check_if_extends_class ctx target_class_name class_name =
  let class_ = Decl_provider.get_class ctx class_name in
  match class_ with
  | Decl_entry.Found cls
    when Cls.has_ancestor cls target_class_name
         || Cls.requires_ancestor cls target_class_name ->
    true
  | _ -> false

let is_target_class ctx target_classes class_name =
  match target_classes with
  | Class_set s -> SSet.mem s class_name
  | Subclasses_of s ->
    String.equal s class_name || check_if_extends_class ctx s class_name

let process_member_id
    ctx target_classes target_member class_name mid ~is_method ~is_const =
  let member_name = snd mid in
  let is_target =
    match target_member with
    | Method target_name -> is_method && String.equal member_name target_name
    | Property target_name ->
      (not is_method)
      && (not is_const)
      && String.equal (String_utils.lstrip member_name "$") target_name
    | Class_const target_name ->
      is_const && String.equal member_name target_name
    | Typeconst _ -> false
  in
  if is_target && is_target_class ctx target_classes class_name then
    Pos.Map.singleton (fst mid) (class_name ^ "::" ^ snd mid)
  else
    Pos.Map.empty

let process_class_id
    target_class
    (pos, class_name)
    (class_id_type : SymbolOccurrence.class_id_type)
    (include_all_ci_types : bool) =
  if String.equal target_class class_name then
    match (include_all_ci_types, class_id_type) with
    | (false, SymbolOccurrence.Other) -> Pos.Map.empty
    | _ -> Pos.Map.singleton pos class_name
  else
    Pos.Map.empty

let process_taccess
    ctx target_classes target_typeconst (class_name, tconst_name, p) =
  if
    is_target_class ctx target_classes class_name
    && String.equal target_typeconst tconst_name
  then
    Pos.Map.singleton p (class_name ^ "::" ^ tconst_name)
  else
    Pos.Map.empty

let process_gconst_id target_gconst id =
  if String.equal target_gconst (snd id) then
    Pos.Map.singleton (fst id) (snd id)
  else
    Pos.Map.empty

let add_if_extends_class ctx target_class_name class_name acc =
  if check_if_extends_class ctx target_class_name class_name then
    SSet.add acc class_name
  else
    acc

let find_child_classes_in_files ctx target_class_name naming_table files =
  SharedMem.invalidate_local_caches ();
  Relative_path.Set.fold files ~init:SSet.empty ~f:(fun fn acc ->
      try
        let { FileInfo.ids = { FileInfo.classes; _ }; _ } =
          Naming_table.get_file_info_exn naming_table fn
        in
        List.fold_left classes ~init:acc ~f:(fun acc id ->
            add_if_extends_class ctx target_class_name id.FileInfo.name acc)
      with
      | Naming_table.File_info_not_found -> acc)

let get_origin_class_name ctx class_name member =
  let origin =
    match member with
    | Method method_name -> begin
      match Decl_provider.get_class ctx class_name with
      | Decl_entry.Found class_ ->
        let get_origin_class meth =
          match meth with
          | Some meth -> Some meth.ce_origin
          | None -> None
        in
        let origin = get_origin_class (Cls.get_method class_ method_name) in
        if Option.is_some origin then
          origin
        else
          get_origin_class (Cls.get_smethod class_ method_name)
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        None
    end
    | Property _
    | Class_const _
    | Typeconst _ ->
      None
  in
  Option.value origin ~default:class_name

let get_child_classes_files ctx class_name =
  match Naming_provider.get_type_kind ctx class_name with
  | Some Naming_types.TClass ->
    let deps_mode = Provider_context.get_deps_mode ctx in
    (* Find the files that contain classes that extend class_ *)
    let cid_hash = Typing_deps.(Dep.make (Dep.Type class_name)) in
    let extend_deps =
      Decl_compare.get_extend_deps
        deps_mode
        cid_hash
        (Typing_deps.DepSet.singleton cid_hash)
    in
    Naming_provider.get_files ctx extend_deps
  | _ -> Relative_path.Set.empty

let get_deps_set ctx classes =
  let deps_mode = Provider_context.get_deps_mode ctx in
  SSet.fold
    classes
    ~f:
      begin
        fun class_name acc ->
          match Naming_provider.get_type_path ctx class_name with
          | None -> acc
          | Some fn ->
            let dep = Typing_deps.Dep.Type class_name in
            let ideps = Typing_deps.get_ideps deps_mode dep in
            let files = Naming_provider.get_files ctx ideps in
            let files = Relative_path.Set.add files fn in
            Relative_path.Set.union files acc
      end
    ~init:Relative_path.Set.empty

let get_files_for_descendants_and_dependents_of_members_in_descendants
    ctx ~class_name members =
  let mode = Provider_context.get_deps_mode ctx in
  let class_dep =
    Typing_deps.DepSet.singleton
      (Typing_deps.Dep.make (Typing_deps.Dep.Type class_name))
  in
  let class_and_descendants_dep = Typing_deps.add_extend_deps mode class_dep in
  let dependents =
    Typing_deps.DepSet.fold
      class_and_descendants_dep
      ~init:(Typing_deps.DepSet.make ())
      ~f:(fun descendant_dep result ->
        List.fold members ~init:result ~f:(fun result member ->
            let member_dep =
              Typing_deps.Dep.make_member_dep_from_type_dep
                descendant_dep
                member
            in
            let member_fanout =
              Typing_deps.get_ideps_from_hash mode member_dep
            in
            Typing_deps.DepSet.union result member_fanout))
  in
  ( Naming_provider.get_files ctx class_and_descendants_dep,
    Naming_provider.get_files ctx dependents )

let get_deps_set_function ctx f_name =
  match Naming_provider.get_fun_path ctx f_name with
  | Some fn ->
    let deps_mode = Provider_context.get_deps_mode ctx in
    let dep = Typing_deps.Dep.Fun f_name in
    let ideps = Typing_deps.get_ideps deps_mode dep in
    let files = Naming_provider.get_files ctx ideps in
    Relative_path.Set.add files fn
  | None -> Relative_path.Set.empty

let get_deps_set_gconst ctx cst_name =
  match Naming_provider.get_const_path ctx cst_name with
  | Some fn ->
    let deps_mode = Provider_context.get_deps_mode ctx in
    let dep = Typing_deps.Dep.GConst cst_name in
    let ideps = Typing_deps.get_ideps deps_mode dep in
    let files = Naming_provider.get_files ctx ideps in
    Relative_path.Set.add files fn
  | None -> Relative_path.Set.empty

let fold_one_tast ctx target acc symbol =
  let module SO = SymbolOccurrence in
  let SO.{ type_; pos; name; _ } = symbol in
  Pos.Map.union acc
  @@
  match (target, type_) with
  | (IMember (classes, Typeconst tc), SO.Typeconst (c_name, tc_name)) ->
    process_taccess ctx classes tc (c_name, tc_name, pos)
  | (IMember (classes, member), SO.Method (SO.ClassName c_name, m_name))
  | (IMember (classes, member), SO.ClassConst (SO.ClassName c_name, m_name))
  | (IMember (classes, member), SO.Property (SO.ClassName c_name, m_name))
  | (IMember (classes, member), SO.XhpLiteralAttr (c_name, m_name)) ->
    let mid = (pos, m_name) in
    let is_method =
      match type_ with
      | SO.Method _ -> true
      | _ -> false
    in
    let is_const =
      match type_ with
      | SO.ClassConst _ -> true
      | _ -> false
    in
    process_member_id ctx classes member c_name mid ~is_method ~is_const
  | (IFunction fun_name, SO.Function) -> process_fun_id fun_name (pos, name)
  | (IClass c, SO.Class class_id_type) ->
    let include_all_ci_types = true in
    process_class_id c (pos, name) class_id_type include_all_ci_types
  | (IExplicitClass c, SO.Class class_id_type) ->
    let include_all_ci_types = false in
    process_class_id c (pos, name) class_id_type include_all_ci_types
  | (IGConst cst_name, SO.GConst) -> process_gconst_id cst_name (pos, name)
  | _ -> Pos.Map.empty

let should_cancel ~stream_file =
  match stream_file with
  | None -> false
  | Some stream_file -> not (Path.file_exists stream_file)

module Iter = struct
  exception Cancelled

  (** This datastructure is used for the [find_refs] function, which iterates
  over a list of files, finds references in them, and streams them. *)
  type acc = {
    last_cancellation_check: float;
        (** When did we last check for cancellation? *)
    results: (string * Pos.t) list list;
        (** Accumulates all results from [find_refs] *)
    to_stream_at_end_of_batch: (string * Pos.absolute) list list;
        (** Accumulates only those results that should be streamed at the end of [find_refs] *)
    streamed_so_far: int;
        (** How many individual references have been streamed so far *)
  }

  let init () =
    {
      last_cancellation_check = Unix.gettimeofday ();
      results = [];
      to_stream_at_end_of_batch = [];
      streamed_so_far = 0;
    }

  (** We'll check a few times a second whether we should cancel. *)
  let raise_if_should_cancel acc ~t_now ~stream_file =
    let last_cancellation_check =
      if Float.(t_now > acc.last_cancellation_check +. 0.2) then begin
        if should_cancel ~stream_file then raise Cancelled;
        t_now
      end else
        acc.last_cancellation_check
    in
    { acc with last_cancellation_check }

  (** Stream on a per-file basis for the first few results of a find-refs,
  but on a per-job basis for the rest. (Each job covers several hundred files).
  We do it this way because a find-refs might have 50k results, and it's too much
  for 30 processes to do 50k locks between them over the course of the find-refs. *)
  let stream_file ~per_file ~stream_fd ~t_now ~t_start acc =
    match stream_fd with
    | None -> acc
    | Some stream_fd ->
      let max_results_to_stream_by_file = 20 in
      let max_secs_to_stream_by_file = 10.0 in
      let should_stream_now =
        acc.streamed_so_far < max_results_to_stream_by_file
        && Float.(t_now < t_start +. max_secs_to_stream_by_file)
      in
      let abs_results =
        List.map per_file ~f:(fun (r, p) -> (r, Pos.to_absolute p))
      in
      if should_stream_now then
        let () =
          FindRefsWireFormat.Ide_stream.lock_and_append stream_fd abs_results
        in
        {
          acc with
          streamed_so_far = acc.streamed_so_far + List.length abs_results;
        }
      else
        {
          acc with
          to_stream_at_end_of_batch =
            abs_results :: acc.to_stream_at_end_of_batch;
        }

  (** This streams any results from the job that haven't already been done on
  a per-file basis earlier by [stream_file]. *)
  let stream_job ~stream_fd acc =
    match stream_fd with
    | None -> ()
    | Some stream_fd ->
      FindRefsWireFormat.Ide_stream.lock_and_append
        stream_fd
        (List.concat acc.to_stream_at_end_of_batch)
end

(** Either returns results from this batch (concatenated with [acc]), or [Error] if cancelled *)
let find_refs
    (ctx : Provider_context.t)
    (target : action_internal)
    (job_acc : ((string * Pos.t) list, unit) Result.t)
    (files : Relative_path.t list)
    ~(omit_declaration : bool)
    ~(stream_file : Path.t option)
    ~(t_start : float) : ((string * Pos.t) list, unit) Result.t =
  (* The helper function 'results_from_tast' takes a tast, looks at all
     use-sites in the tast e.g. "foo(1)" is a use-site of symbol foo,
     and returns a list of use-site-position along with the string name they used. *)
  let results_from_tast tast_opt =
    match tast_opt with
    | None -> []
    | Some tast ->
      let results_map =
        IdentifySymbolService.all_symbols ctx tast
        |> List.filter ~f:(fun symbol ->
               if omit_declaration then
                 Option.is_none symbol.SymbolOccurrence.is_declaration
               else
                 true)
        |> List.fold ~init:Pos.Map.empty ~f:(fold_one_tast ctx target)
      in
      let results_list =
        Pos.Map.fold (fun p str acc -> (str, p) :: acc) results_map []
      in
      results_list
  in

  (* Helper: [files] can legitimately refer to non-existent files, e.g.
     if they've been deleted since the depgraph was created.
     This is how we'll filter them out. *)
  let is_entry_valid entry =
    entry |> Provider_context.get_file_contents_if_present |> Option.is_some
  in

  (* Helper: given a path, obtains its tast *)
  let tast_of_file path =
    let (_ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    try
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      Some tast.Tast_with_dynamic.under_normal_assumptions
    with
    | _ when not (is_entry_valid entry) -> None
  in

  try
    let stream_fd =
      Option.map stream_file ~f:(fun stream_file ->
          let file = Path.to_string stream_file in
          try Unix.openfile file [Unix.O_WRONLY; Unix.O_APPEND] 0o666 with
          | Unix.Unix_error (Unix.ENOENT, _, _) -> raise Iter.Cancelled)
    in
    Utils.try_finally
      ~finally:(fun () -> Option.iter stream_fd ~f:Unix.close)
      ~f:(fun () ->
        let acc =
          List.fold files ~init:(Iter.init ()) ~f:(fun acc path ->
              let t_now = Unix.gettimeofday () in
              let acc = Iter.raise_if_should_cancel ~stream_file ~t_now acc in
              let per_file = results_from_tast (tast_of_file path) in
              let acc = Iter.{ acc with results = per_file :: acc.results } in
              Iter.stream_file ~per_file ~stream_fd ~t_now ~t_start acc)
        in
        Iter.stream_job ~stream_fd acc;
        let job_acc =
          match job_acc with
          | Ok job_acc -> job_acc
          | Error () -> raise Iter.Cancelled
        in
        Ok (List.rev_append job_acc (List.concat acc.Iter.results)))
  with
  | Iter.Cancelled -> Error ()

let find_refs_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(target : action_internal) : (string * Pos.Map.key) list =
  let symbols = IdentifySymbolService.all_symbols_ctx ~ctx ~entry in
  let results =
    symbols
    |> List.filter ~f:(fun symbol ->
           Option.is_none symbol.SymbolOccurrence.is_declaration)
    |> List.fold ~init:Pos.Map.empty ~f:(fold_one_tast ctx target)
  in
  Pos.Map.fold (fun p str acc -> (str, p) :: acc) results []

let parallel_find_refs
    workers
    files
    target
    ctx
    ~(omit_declaration : bool)
    ~(stream_file : Path.t option)
    ~(t_start : float) =
  MultiWorker.call
    workers
    ~job:(find_refs ctx target ~omit_declaration ~stream_file ~t_start)
    ~neutral:(Ok [])
    ~merge:(fun output acc ->
      match (output, acc) with
      | (Ok output, Ok acc) -> Ok (List.rev_append output acc)
      | _ -> Error ())
    ~next:
      (let next = MultiWorker.next workers files in
       (* We create the "next" function just once, now; it will dole
          out chunks of [files] each time it's asked, below. *)
       fun () ->
         if should_cancel ~stream_file then
           Bucket.Done
         else
           next ())

let get_definitions ctx action =
  List.map ~f:(fun (name, pos) ->
      (name, Naming_provider.resolve_position ctx pos))
  @@
  match action with
  | IMember (Class_set classes, Method method_name) ->
    SSet.fold classes ~init:[] ~f:(fun class_name acc ->
        match Decl_provider.get_class ctx class_name with
        | Decl_entry.Found class_ ->
          let add_meth get acc =
            match get method_name with
            | Some meth when String.equal meth.ce_origin (Cls.name class_) ->
              let pos = get_pos @@ Lazy.force meth.ce_type in
              (method_name, pos) :: acc
            | _ -> acc
          in
          let acc = add_meth (Cls.get_method class_) acc in
          let acc = add_meth (Cls.get_smethod class_) acc in
          acc
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          acc)
  | IMember (Class_set classes, Class_const class_const_name) ->
    SSet.fold classes ~init:[] ~f:(fun class_name acc ->
        match Decl_provider.get_class ctx class_name with
        | Decl_entry.Found class_ ->
          let add_class_const get acc =
            match get class_const_name with
            | Some class_const
              when String.equal class_const.cc_origin (Cls.name class_) ->
              let pos = class_const.cc_pos in
              (class_const_name, pos) :: acc
            | _ -> acc
          in
          let acc = add_class_const (Cls.get_const class_) acc in
          acc
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          acc)
  | IExplicitClass class_name
  | IClass class_name ->
    Option.value
      ~default:[]
      (Naming_provider.get_type_kind ctx class_name >>= function
       | Naming_types.TClass ->
         Decl_provider.get_class ctx class_name |> Decl_entry.to_option
         >>= fun class_ -> Some [(class_name, Cls.pos class_)]
       | Naming_types.TTypedef ->
         Decl_provider.get_typedef ctx class_name |> Decl_entry.to_option
         >>= fun type_ -> Some [(class_name, type_.td_pos)])
  | IFunction fun_name -> begin
    match Decl_provider.get_fun ctx fun_name with
    | Decl_entry.Found { fe_pos; _ } -> [(fun_name, fe_pos)]
    | _ -> []
  end
  | IGConst _
  | IMember (Subclasses_of _, _)
  | IMember (_, (Property _ | Typeconst _)) ->
    (* this code path is used only in ServerRename, we can update it at some
       later time *)
    []

let find_references ctx workers target include_defs files ~stream_file =
  let len = List.length files in
  Hh_logger.debug "find_references: %d files" len;
  let t_start = Unix.gettimeofday () in
  (* This "10" threshold is exercised in test/integration/findrefs_multiworker --
     if you change the threshold here, then please update the test. *)
  let results =
    if len < 10 then
      find_refs
        ctx
        target
        (Ok [])
        files
        ~omit_declaration:true
        ~stream_file
        ~t_start
    else
      parallel_find_refs
        workers
        files
        target
        ctx
        ~omit_declaration:true
        ~stream_file
        ~t_start
  in
  match results with
  | Error () ->
    Hh_logger.log "find-references: cancelled";
    []
  | Ok results ->
    let defs =
      if include_defs then
        get_definitions ctx target
      else
        []
    in
    Hh_logger.log
      "find-references: %d files -> %d results + %d defs(%b)"
      len
      (List.length results)
      (List.length defs)
      include_defs;
    List.rev_append defs results

let find_references_single_file ctx target file =
  let results =
    find_refs
      ctx
      target
      (Ok [])
      [file]
      ~omit_declaration:false
      ~stream_file:None
      ~t_start:(Unix.gettimeofday ())
  in
  match results with
  | Ok results -> results
  | Error () -> []

let get_dependent_files_function ctx _workers f_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_function ctx f_name

let get_dependent_files_gconst ctx _workers cst_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_gconst ctx cst_name

let get_dependent_files ctx _workers input_set =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set ctx input_set
