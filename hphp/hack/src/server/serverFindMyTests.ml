(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

type member = Method of string

let findrefs_member_of_member = function
  | Method m -> SearchTypes.Find_refs.Method m

let supported_test_framework_classes = SSet.of_list ["WWWTest"]

let strip_ns results = List.map results ~f:(fun (s, p) -> (Utils.strip_ns s, p))

let is_test_file ctx file_path file_nast =
  let is_test_class class_name =
    match Decl_provider.get_class ctx class_name with
    | Decl_entry.DoesNotExist ->
      failwith
        (Printf.sprintf
           "Internal error: Could not find class %s, even though we encountered it during traversal"
           class_name)
    | NotYetAvailable ->
      failwith
        (Printf.sprintf
           "Internal error: Class %s, marked as not yet available"
           class_name)
    | Found class_ ->
      let ancestors = Folded_class.all_ancestor_names class_ in
      List.exists ancestors ~f:(fun ancestor ->
          SSet.mem (Utils.strip_ns ancestor) supported_test_framework_classes)
  in

  match String.substr_index file_path ~pattern:"/__tests__/" with
  | None -> false
  | Some _ ->
    let classes_in_file =
      let visitor =
        object
          inherit [_] Aast.reduce

          method! on_class_ _ctx class_ =
            SSet.singleton (Ast_defs.get_id class_.c_name)

          method! on_method_ _ctx _ = SSet.empty

          method! on_fun_ _ctx _ = SSet.empty

          method plus = SSet.union

          method zero = SSet.empty
        end
      in
      visitor#on_program () file_nast
    in
    SSet.exists is_test_class classes_in_file

let search ctx target include_defs ~files genv =
  List.iter files ~f:(fun file ->
      Hh_logger.debug
        "ServerFindMyTests.search file %s"
        (Relative_path.to_absolute file));
  (* Get all the references to the provided target in the files *)
  let res =
    FindRefsService.find_references
      ctx
      genv.workers
      target
      include_defs
      files
      ~stream_file:None
  in
  strip_ns res

let search_member
    ctx
    (class_name : string)
    (member : member)
    ~(include_defs : bool)
    (genv : genv)
    (env : env) : (string * Pos.t) list =
  let dep_member_of member =
    let open Typing_deps.Dep.Member in
    match member with
    | Method n -> [method_ n; smethod n]
  in
  let fr_member = findrefs_member_of_member member in

  let class_name = Utils.add_ns class_name in
  let origin_class_name =
    FindRefsService.get_origin_class_name ctx class_name fr_member
  in

  let (descendant_class_files, member_use_files) =
    FindRefsService
    .get_files_for_descendants_and_dependents_of_members_in_descendants
      ctx
      ~class_name
      (dep_member_of member)
  in
  let descendant_classes =
    FindRefsService.find_child_classes_in_files
      ctx
      origin_class_name
      env.naming_table
      descendant_class_files
  in
  let class_and_descendants = SSet.add origin_class_name descendant_classes in
  let files =
    Relative_path.Set.union descendant_class_files member_use_files
    |> Relative_path.Set.elements
  in
  let target =
    FindRefsService.IMember
      (FindRefsService.Class_set class_and_descendants, fr_member)
  in
  search ctx target include_defs ~files genv

type symbol_def =
  | Method of {
      class_name: string;
      method_name: string;
    }
[@@deriving show]

(** A symbol we encounter during our BFS traversal *)
type symbol_node = {
  symbol_def: symbol_def;
  distance: int;
}

type error_msg = string

let full_name_of_symbol_def = function
  | Method { class_name; method_name } -> class_name ^ "::" ^ method_name

type result_entry = ServerCommandTypes.Find_my_tests.result_entry

(**
  This approach is awful performance-wise:
  FindRefsService gives us *positions* where a certain target symbol is referenced.
  But we need to find out the name of the enclosing definiton, so we can in turn find
  where its referenced from.

  Ideally, FindRefsService (or an alternative to it) would give us the enclosing SymbolDefinition
  directly, but that would require piping that information through IdentifySymbolService.
*)
let enclosing_def_of_symbol_occurrence
    (symbol_occurrence_pos : Pos.t) symbol_occurrence_nast : symbol_def option =
  let visitor =
    object
      inherit [_] Aast.reduce as super

      method! on_class_ _ctx class_ =
        let class_span = class_.Aast.c_span in
        if Pos.contains class_span symbol_occurrence_pos then
          let ctx = Some class_.Aast.c_name in
          super#on_class_ ctx class_
        else
          None

      method! on_method_ ctx m =
        let span = m.Aast.m_span in
        if Pos.contains span symbol_occurrence_pos then
          (* We are seeing a method, so on_class_ must have set the class_name *)
          let class_name = snd (Option.value_exn ctx) in
          let method_name = snd m.Aast.m_name in
          Some (Method { class_name; method_name })
        else
          None

      method zero = None

      method plus r1 r2 =
        match (r1, r2) with
        | (None, _) -> r2
        | (Some _, _) -> r1
    end
  in
  visitor#on_program None symbol_occurrence_nast

(**
  Find references to the given symbol.
  If the reference is from a test file, it's returned as part of the second list.
  Otherwise, the definition from where the reference happens is returned as part of the first list. *)
let get_references
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    symbol : (symbol_def list * string list, error_msg) Result.t =
  let search_results =
    match symbol with
    | Method { class_name; method_name } ->
      search_member
        ctx
        class_name
        (Method method_name)
        ~include_defs:false
        genv
        env
  in
  let resolve_found_position (acc_symbol_defs, acc_files) (_, referencing_pos) =
    let relative_path = Pos.filename referencing_pos in
    let path = Relative_path.suffix relative_path in

    (* We could implement all of our traversals without NASTs, but just CSTs and their
       Full_fidelity_positioned_syntax.parentage function.

       But those never seem to be cached, whereas the NAST may still be in cache after we just
       constructed the TAST in FindRefsService. *)
    let nast = Ast_provider.get_ast ~full:true ctx relative_path in

    if is_test_file ctx path nast then
      Result.Ok
        ( acc_symbol_defs,
          (Pos.to_absolute referencing_pos |> Pos.filename) :: acc_files )
    else
      match enclosing_def_of_symbol_occurrence referencing_pos nast with
      | Some referencing_def ->
        Result.Ok (referencing_def :: acc_symbol_defs, acc_files)
      | None ->
        Result.Error
          (Printf.sprintf
             "Could not find definition enclosing %s"
             (Pos.to_absolute referencing_pos |> Pos.show_absolute))
  in
  List.fold_result search_results ~init:([], []) ~f:resolve_found_position

let is_root_symbol_in_test_file ctx = function
  | Method { class_name; method_name = _ } ->
    (match Naming_provider.get_class_path ctx (Utils.add_ns class_name) with
    | None ->
      failwith
        (Printf.sprintf
           "Could not resolve class %s, even though it was given as part of a root"
           class_name)
    | Some relative_path ->
      let nast = Ast_provider.get_ast ~full:true ctx relative_path in
      let path = Relative_path.suffix relative_path in
      if is_test_file ctx path nast then
        Some path
      else
        None)

let search
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    max_distance
    (roots : symbol_def list) : result_entry list =
  let queue = Queue.create () in
  let seen_symbols = Hash_set.create (module String) in
  (* Maps test file paths to the minimum distance at which we have discovered them *)
  let test_files = Hashtbl.create (module String) in

  List.iter roots ~f:(fun root_symbol ->
      let full_name = full_name_of_symbol_def root_symbol in
      let seen = Result.is_error (Hash_set.strict_add seen_symbols full_name) in
      match is_root_symbol_in_test_file ctx root_symbol with
      | Some test_file -> ignore (Hashtbl.add test_files ~key:test_file ~data:0)
      | None ->
        if not seen then
          Queue.enqueue queue { distance = 0; symbol_def = root_symbol });

  let rec bfs () =
    match Queue.dequeue queue with
    | Some { distance; symbol_def } when distance < max_distance ->
      let new_distance = distance + 1 in
      (match get_references ~ctx ~genv ~env symbol_def with
      | Result.Ok (referencing_defs, referencing_test_files) ->
        List.iter referencing_defs ~f:(fun referencing_def ->
            let symbol_name = full_name_of_symbol_def referencing_def in
            match Hash_set.strict_add seen_symbols symbol_name with
            | Result.Ok () ->
              Queue.enqueue
                queue
                { distance = new_distance; symbol_def = referencing_def }
            | Result.Error _ -> (* Already seen, nothing to do *) ());
        List.iter referencing_test_files ~f:(fun referencing_test_file ->
            (* This does nothing if a mapping for the same file exists.
               Since we are doing a BFS traversal, we are guaranteed to maintain
               the invariant that the existing binding would have existing_distance <= new_distance *)
            ignore
              (Hashtbl.add
                 test_files
                 ~key:referencing_test_file
                 ~data:new_distance));
        bfs ()
      | Result.Error msg ->
        Hh_logger.log
          "Getting references for symbol %s failed: %s"
          (show_symbol_def symbol_def)
          msg)
    | _ -> ()
  in

  bfs ();

  Hashtbl.fold test_files ~init:[] ~f:(fun ~key ~data acc ->
      let open ServerCommandTypes.Find_my_tests in
      { file_path = key; distance = data } :: acc)

let go
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~(max_distance : int)
    (symbols : string list) : ServerCommandTypes.Find_my_tests.result =
  let open Result.Let_syntax in
  (match env.prechecked_files with
  | ServerEnv.Prechecked_files_disabled -> ()
  | _ ->
    (* Prechecked files influence what dependencies we are aware of, (see ServerFindRefs)
       We just choose not to support them, as they are being deprecated *)
    failwith
      "FindMyTests not supported by servers with prechecked optimisation enabled");

  let parse_method_def symbol =
    match Str.split (Str.regexp "::") symbol with
    | [class_name; method_name] ->
      Result.Ok (Method { class_name; method_name })
    | _ ->
      Result.Error
        "Invalid symbol format. Expected format: Class_name::method_name"
  in

  let* roots =
    List.fold_result symbols ~init:[] ~f:(fun acc symbol_name ->
        let* def = parse_method_def symbol_name in
        Result.Ok (def :: acc))
  in

  Result.Ok (search ~ctx ~genv ~env max_distance roots)
