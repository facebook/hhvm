(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* Identifying a symbol can be a first step to another operation. For example,
 * you can identify symbol and then highlight other "equal" symbols.
 * get_occurrence_and_map is useful for such application, because ~f function
 * will execute in the same environment that the symbol was identified -
 * content ASTs and defs will still be available in shared memory for the
 * subsequent operation. *)
let get_occurrence_and_map tcopt content line char ~f =
  ServerIdeUtils.declare_and_check
    content
    ~f:
      begin
        fun path file_info tast ->
        let result = IdentifySymbolService.go tast line char in
        f path file_info result
      end
    tcopt

(* Order symbols from innermost to outermost *)
let by_nesting x y =
  if Pos.contains x.SymbolOccurrence.pos y.SymbolOccurrence.pos then
    if Pos.contains y.SymbolOccurrence.pos x.SymbolOccurrence.pos then
      0
    else
      1
  else
    -1

let rec take_best_suggestions l =
  match l with
  | first :: rest ->
    (* Check if we should stop finding suggestions. For example, in
     "foo($bar)" it's not useful to look outside the local variable "$bar". *)
    let stop =
      match first.SymbolOccurrence.type_ with
      | SymbolOccurrence.LocalVar -> true
      | SymbolOccurrence.Method _ -> true
      | _ -> false
    in
    if stop then
      (* We're stopping here, but also include the other suggestions for
         this span. *)
      first :: List.take_while rest ~f:(fun x -> by_nesting first x = 0)
    else
      first :: take_best_suggestions rest
  | [] -> []

(** NOTE: the paths of any positions within any returned `SymbolOccurrence` or
    `SymbolDefinition` objects will be the empty string (`""`) if the symbol is
    located in the passed in content buffer. *)
let go content line char (tcopt : TypecheckerOptions.t) =
  get_occurrence_and_map tcopt content line char ~f:(fun path _ symbols ->
      let symbols = take_best_suggestions (List.sort by_nesting symbols) in
      let ast = Some (Ast_provider.get_ast path) in
      let result =
        List.map symbols ~f:(fun x ->
            let symbol_definition = ServerSymbolDefinition.go ast x in
            (x, symbol_definition))
      in
      result)

(** NOTE: the paths of any positions within any returned `SymbolOccurrence` or
    `SymbolDefinition` objects will be the empty string (`""`) if the symbol is
    located in the passed in content buffer. *)
let go_absolute content line char tcopt =
  List.map (go content line char tcopt) (fun (x, y) ->
      ( SymbolOccurrence.to_absolute x,
        Option.map y SymbolDefinition.to_absolute ))

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) =
  let symbols =
    IdentifySymbolService.go
      (Provider_utils.compute_tast ~ctx ~entry)
      line
      column
  in
  let symbols = take_best_suggestions (List.sort by_nesting symbols) in
  List.map symbols ~f:(fun symbol ->
      let symbol_definition =
        ServerSymbolDefinition.go (Some entry.Provider_context.ast) symbol
      in
      (symbol, symbol_definition))

let go_ctx_absolute
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) :
    (string SymbolOccurrence.t * string SymbolDefinition.t option) list =
  go_ctx ~ctx ~entry ~line ~column
  |> List.map ~f:(fun (occurrence, definition) ->
         let occurrence = SymbolOccurrence.to_absolute occurrence in
         let definition =
           Option.map ~f:SymbolDefinition.to_absolute definition
         in
         (occurrence, definition))
