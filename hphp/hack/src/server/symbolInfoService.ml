(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ServerCommandTypes.Symbol_info_service

(* This module dumps all the symbol info(like fun-calls) in input files *)

let recheck_naming filename_l =
  List.iter filename_l (fun file ->
      Errors.ignore_ (fun () ->
          (* We only need to name to find references to locals *)
          List.iter (Ast_provider.get_ast file) (function
              | Aast.Fun f ->
                let _ = Naming.fun_ f in
                ()
              | Aast.Class c ->
                let _ = Naming.class_ c in
                ()
              | _ -> ())))

let helper tcopt acc filetuple_l =
  let filename_l = List.rev_map filetuple_l fst in
  recheck_naming filename_l;
  let tasts = ServerIdeUtils.recheck tcopt filetuple_l |> List.map ~f:snd in
  let fun_calls = SymbolFunCallService.find_fun_calls tasts in
  let symbol_types = SymbolTypeService.generate_types tasts in
  (fun_calls, symbol_types) :: acc

let parallel_helper workers filetuple_l tcopt =
  MultiWorker.call
    workers
    ~job:(helper tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers filetuple_l)

(* Format result from '(fun_calls * symbol_types) list' raw result into *)
(* 'fun_calls list, symbol_types list' and store in SymbolInfoService.result *)
let format_result raw_result =
  let result_list =
    List.fold_left
      raw_result
      ~f:
        begin
          fun acc bucket ->
          let (result1, result2) = acc in
          let (part1, part2) = bucket in
          (List.rev_append part1 result1, List.rev_append part2 result2)
        end
      ~init:([], [])
  in
  { fun_calls = fst result_list; symbol_types = snd result_list }

(* Entry Point *)
let go workers file_list env =
  (* Convert 'string list' into 'fileinfo list' *)
  let filetuple_l =
    List.fold_left
      file_list
      ~f:
        begin
          fun acc file_path ->
          let fn = Relative_path.create Relative_path.Root file_path in
          match Naming_table.get_file_info env.ServerEnv.naming_table fn with
          | Some fileinfo -> (fn, fileinfo) :: acc
          | None -> acc
        end
      ~init:[]
  in
  let tcopt = env.ServerEnv.tcopt in
  let raw_result =
    if List.length file_list < 10 then
      helper tcopt [] filetuple_l
    else
      parallel_helper workers filetuple_l tcopt
  in
  format_result raw_result
