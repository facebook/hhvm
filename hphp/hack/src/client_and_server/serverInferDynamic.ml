(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type target =
  | Target_function of string
  | Target_method of string * string

let parse_target (identifier : string) : target =
  match String.substr_index identifier ~pattern:"::" with
  | Some i ->
    let class_name = String.prefix identifier i in
    let method_name = String.drop_prefix identifier (i + 2) in
    let class_name =
      if String.is_prefix class_name ~prefix:"\\" then
        class_name
      else
        "\\" ^ class_name
    in
    Target_method (class_name, method_name)
  | None ->
    let name =
      if String.is_prefix identifier ~prefix:"\\" then
        identifier
      else
        "\\" ^ identifier
    in
    Target_function name

let solve_and_format ~as_data ~name env =
  let Equal = Tast_env.eq_typing_env in
  let inf_env = env.Typing_env_types.inference_env in
  let dynamic_locals =
    match env.Typing_env_types.fun_tast_info with
    | Some info -> info.Tast.dynamic_locals
    | None -> []
  in
  let solutions =
    Typing_shadow_solver.solve ~as_data env inf_env dynamic_locals
  in
  `Assoc
    [
      ("function", `String name);
      ( "solutions",
        `List
          (List.map solutions ~f:(Typing_shadow_solver.solution_to_json env)) );
    ]

let infer_function ~ctx ~tast ~as_data ~name =
  let Equal = Tast_env.eq_typing_env in
  List.find_map tast ~f:(fun def ->
      match def with
      | Aast.Fun { Aast.fd_name; _ } when String.equal (snd fd_name) name ->
        let env = Tast_env.def_env ctx def in
        Some (solve_and_format ~as_data ~name env)
      | _ -> None)

type 'a lookup_result =
  | Found of 'a
  | Class_not_in_tast
  | Method_not_found

let infer_method ~ctx ~tast ~as_data ~class_name ~method_name =
  let Equal = Tast_env.eq_typing_env in
  let found_class = ref false in
  let result =
    List.find_map tast ~f:(fun def ->
        match def with
        | Aast.Class { Aast.c_name; c_methods; _ }
          when String.equal (snd c_name) class_name ->
          found_class := true;
          let class_env = Tast_env.def_env ctx def in
          List.find_map c_methods ~f:(fun m ->
              if String.equal (snd m.Aast.m_name) method_name then
                let env = Tast_env.restore_method_env class_env m in
                let display_name = class_name ^ "::" ^ method_name in
                Some (solve_and_format ~as_data ~name:display_name env)
              else
                None)
        | _ -> None)
  in
  match result with
  | Some json -> Found json
  | None ->
    if !found_class then
      Method_not_found
    else
      Class_not_in_tast

let error_json msg = `Assoc [("error", `String msg)]

let go ~ctx ~identifier ~as_data : Yojson.Safe.t =
  let target = parse_target identifier in
  let path =
    match target with
    | Target_function name -> Naming_provider.get_fun_path ctx name
    | Target_method (class_name, _) ->
      Naming_provider.get_type_path ctx class_name
  in
  match path with
  | None ->
    let what =
      match target with
      | Target_function name -> Printf.sprintf "function %s" name
      | Target_method (class_name, _) -> Printf.sprintf "class %s" class_name
    in
    error_json (Printf.sprintf "Could not find %s in the naming table" what)
  | Some path ->
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in
    let tast = List.concat (Tast_with_dynamic.all tast) in
    (match target with
    | Target_function name ->
      (match infer_function ~ctx ~tast ~as_data ~name with
      | Some json -> json
      | None -> error_json (Printf.sprintf "Could not find function %s" name))
    | Target_method (class_name, method_name) ->
      (match infer_method ~ctx ~tast ~as_data ~class_name ~method_name with
      | Found json -> json
      | Method_not_found ->
        error_json
          (Printf.sprintf
             "Method %s not found in class %s"
             method_name
             class_name)
      | Class_not_in_tast ->
        error_json
          (Printf.sprintf
             "Class %s not found in TAST (file: %s)"
             class_name
             (Relative_path.suffix path))))

let infer_all_methods ~ctx:_ ~as_data ~class_env ~class_name ~methods =
  let Equal = Tast_env.eq_typing_env in
  List.filter_map methods ~f:(fun m ->
      let env = Tast_env.restore_method_env class_env m in
      let name = class_name ^ "::" ^ snd m.Aast.m_name in
      let dynamic_locals =
        match env.Typing_env_types.fun_tast_info with
        | Some info -> info.Tast.dynamic_locals
        | None -> []
      in
      if List.is_empty dynamic_locals then
        None
      else
        Some (solve_and_format ~as_data ~name env))

let go_ctx ~ctx ~entry ~as_data : Yojson.Safe.t =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let tast = List.concat (Tast_with_dynamic.all tast) in
  let results =
    List.concat_map tast ~f:(fun def ->
        match def with
        | Aast.Fun { Aast.fd_name; _ } ->
          let env = Tast_env.def_env ctx def in
          let Equal = Tast_env.eq_typing_env in
          let name = snd fd_name in
          let dynamic_locals =
            match env.Typing_env_types.fun_tast_info with
            | Some info -> info.Tast.dynamic_locals
            | None -> []
          in
          if List.is_empty dynamic_locals then
            []
          else
            [solve_and_format ~as_data ~name env]
        | Aast.Class { Aast.c_name; c_methods; _ } ->
          let class_env = Tast_env.def_env ctx def in
          let class_name = snd c_name in
          infer_all_methods
            ~ctx
            ~as_data
            ~class_env
            ~class_name
            ~methods:c_methods
        | _ -> [])
  in
  `List results
