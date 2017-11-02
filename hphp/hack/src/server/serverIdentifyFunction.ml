(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(* Identifying a symbol can be a first step to another operation. For example,
 * you can identify symbol and then highlight other "equal" symbols.
 * get_occurrence_and_map is useful for such application, because ~f function
 * will execute in the same environment that the symbol was identified -
 * content ASTs and defs will still be available in shared memory for the
 * subsequent operation. *)
let get_occurrence_and_map tcopt content line char ~f =
  let result = ref [] in
  IdentifySymbolService.attach_hooks result line char;
  ServerIdeUtils.declare_and_check content ~f:begin fun path file_info ->
    IdentifySymbolService.detach_hooks ();
    f path file_info !result
  end tcopt

let get_occurrence content line char =
  get_occurrence_and_map content line char ~f:(fun _ _ x -> x)

let go content line char (tcopt : TypecheckerOptions.t) =
  (* Order symbols from innermost to outermost *)
  let by_nesting x y =
    if Pos.contains x.SymbolOccurrence.pos y.SymbolOccurrence.pos
    then
      if Pos.contains y.SymbolOccurrence.pos x.SymbolOccurrence.pos
      then 0
      else 1
    else -1
  in

  let rec take_best_suggestions l = match l with
    | (first :: rest) ->
      (* Check if we should stop finding suggestions. For example, in
       "foo($bar)" it's not useful to look outside the local variable "$bar". *)
      let stop = match first.SymbolOccurrence.type_ with
        | SymbolOccurrence.LocalVar -> true
        | SymbolOccurrence.Method _ -> true
        | _ -> false
      in
      if stop then
        (* We're stopping here, but also include the other suggestions for
           this span. *)
        first :: List.take_while rest ~f:(fun x -> by_nesting first x == 0)
      else first :: take_best_suggestions rest
    | [] -> []
  in

  get_occurrence_and_map tcopt content line char ~f:(fun path _ symbols ->
  let symbols = take_best_suggestions (List.sort by_nesting symbols) in
  let (ast, _) = Parser_heap.ParserHeap.find_unsafe path in
    List.map symbols ~f:(fun x ->
      let symbol_definition = ServerSymbolDefinition.go tcopt ast x in
      x, symbol_definition)
      )

let go_absolute content line char tcopt =
  List.map (go content line char tcopt) begin fun (x, y) ->
    SymbolOccurrence.to_absolute x, Option.map y SymbolDefinition.to_absolute
  end
