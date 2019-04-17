(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Known search providers *)
type search_provider =
  | AllLocalIndex
  (*
   * This is called the GleanApiIndex rather than GleanIndex since it
   * actually calls a thrift API for all answers.  All other providers
   * get their answers locally.
   *)
  | GleanApiIndex
  | GrepIndex
  | NoIndex
  | RipGrepIndex
  | SqliteIndex
  | TrieIndex

(* Shared Search code between Fuzzy and Trie based searches *)
module type Searchable = sig
  type t
  val fuzzy_types : t list
  val compare_result_type : t -> t -> int
end

(* The results we'll return to the client *)
type ('a, 'b) term = {
  name: string;
  pos: 'a;
  result_type: 'b;
}

let to_absolute t = { t with pos = Pos.to_absolute t.pos }

(* This is the result type as known by the autocomplete system *)
type search_result_type =
  | Class of Ast.class_kind option
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant

(* Individual result object as known by the autocomplete system *)
type symbol = (Pos.absolute, search_result_type) term

(* Used by some legacy APIs *)
type legacy_symbol = (FileInfo.pos, search_result_type) term

(* Collected results as known by the autocomplete system *)
type result = symbol list


(*
 * Needed to distinguish between two types of typechecker updates
 * to the local symbol table. When we no longer need to get local
 * symbols from the saved-state, this can be deleted.
 *)
type info =
  | Full of FileInfo.t
  | Fast of FileInfo.names

(* Flattened enum that contains one element for each type of symbol *)
type si_kind =
  | SI_Class
  | SI_Interface
  | SI_Enum
  | SI_Trait
  | SI_Unknown
  | SI_Mixed
  | SI_Function
  | SI_Typedef
  | SI_GlobalConstant

(*
 * Convert the enum to an integer whose value will not change accidentally.
 *
 * Although [@@deriving] would be a very ocaml-ish way to solve this same
 * problem, we would run the risk that anyone who edits the code to reorder
 * list of si_kinds, perhaps by adding something new in its correct
 * alphabetic order, would cause unexpected autocomplete behavior due to
 * mismatches between the [@@deriving] ordinal values and the integers
 * stored in sqlite.
 *)
let kind_to_int (kind: si_kind): int =
  match kind with
  | SI_Class -> 1
  | SI_Interface -> 2
  | SI_Enum -> 3
  | SI_Trait -> 4
  | SI_Unknown -> 5
  | SI_Mixed -> 6
  | SI_Function -> 7
  | SI_Typedef -> 8
  | SI_GlobalConstant -> 9

(* Convert an integer back to an enum *)
let int_to_kind (kind_num: int): si_kind =
  match kind_num with
  | 1 -> SI_Class
  | 2 -> SI_Interface
  | 3 -> SI_Enum
  | 4 -> SI_Trait
  | 5 -> SI_Unknown
  | 6 -> SI_Mixed
  | 7 -> SI_Function
  | 8 -> SI_Typedef
  | 9 -> SI_GlobalConstant
  | _ -> SI_Unknown

(* Convert an internal "kind" into an autocomplete result kind *)
let kind_to_result (kind: si_kind): search_result_type =
  match kind with
  | SI_Class -> Class (Some Ast.Cnormal)
  | SI_Interface -> Class (Some Ast.Cinterface)
  | SI_Enum -> Class (Some Ast.Cenum)
  | SI_Trait -> Class (Some Ast.Ctrait)
  | SI_Unknown -> Constant
  | SI_Mixed -> Constant
  | SI_Function -> Function
  | SI_Typedef -> Typedef
  | SI_GlobalConstant -> Constant

(* Convert an autocomplete result "kind" into an internal kind *)
let result_to_kind (result: search_result_type): si_kind =
  match result with
  | Class (Some Ast.Cnormal) -> SI_Class
  | Class (Some Ast.Ctrait) -> SI_Trait
  | Class (Some Ast.Cabstract) -> SI_Class
  | Class (Some Ast.Cinterface) -> SI_Interface
  | Class (Some Ast.Cenum) -> SI_Enum
  | Class (None) -> SI_Class
  | Constant -> SI_GlobalConstant
  | Function -> SI_Function
  | Typedef -> SI_Typedef
  | _ -> SI_Unknown

(* Internal representation of a single item stored by the symbol list *)
type si_item = {
  si_name: string;
  si_kind: si_kind;
}

(* Internal representation of a full list of results *)
type si_results =
  si_item list
