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

let is_test_file path =
  Option.is_some (String.substr_index path ~pattern:"/__tests__/")

let strip_ns results = List.map results ~f:(fun (s, p) -> (Utils.strip_ns s, p))

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
    (env : env) : string list =
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
  let search_result = search ctx target include_defs ~files genv in
  List.filter_map search_result ~f:(fun (_symbol, pos) ->
      let file = Pos.to_absolute pos |> Pos.filename in
      if is_test_file file then
        Some file
      else
        None)

let go
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
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
    | [class_name; method_name] -> Result.Ok (class_name, method_name)
    | _ ->
      Result.Error
        "Invalid symbol format. Expected format: Class_name::method_name"
  in

  let* result =
    List.fold_result symbols ~init:SSet.empty ~f:(fun acc symbol ->
        let* (class_name, method_name) = parse_method_def symbol in
        let result =
          search_member
            ctx
            class_name
            (Method method_name)
            ~include_defs:false
            genv
            env
        in

        Result.Ok (SSet.union acc (SSet.of_list result)))
  in
  Result.Ok (SSet.to_list result)
