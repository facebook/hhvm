(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes.Symbol_info_service

let recheck_naming ctx filename_l =
  List.iter filename_l ~f:(fun file ->
      Errors.ignore_ (fun () ->
          (* We only need to name to find references to locals *)
          List.iter (Ast_provider.get_ast ctx file) ~f:(function
              | Aast.Fun f ->
                let _ = Naming.fun_def ctx f in
                ()
              | Aast.Class c ->
                let _ = Naming.class_ ctx c in
                ()
              | _ -> ())))

let helper ctx acc filename_l =
  let filename_l = List.rev filename_l in
  recheck_naming ctx filename_l;
  let tasts =
    List.map filename_l ~f:(fun path ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast.tast; _ } =
          Tast_provider.compute_tast_unquarantined ~ctx ~entry
        in
        tast)
  in
  let fun_calls = SymbolFunCallService.find_fun_calls ctx tasts in
  let symbol_types = SymbolTypeService.generate_types ctx tasts in
  (fun_calls, symbol_types) :: acc

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
