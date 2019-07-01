(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Reordered_argument_collections

(* Known search providers *)
type search_provider =
  | CustomIndex
  | NoIndex
  | SqliteIndex
  | TrieIndex
[@@deriving show]

(* The context in which autocomplete is being performed *)
type autocomplete_type =
  | Acid
  | Acnew
  | Actype
  | Acclass_get
  | Acprop
  | Acshape_key
  | Actrait_only
[@@deriving show]

(* Convert a string to a provider *)
let provider_of_string (provider_str: string): search_provider =
  match provider_str with
  | "SqliteIndex" -> SqliteIndex
  | "NoIndex" -> NoIndex
  | "CustomIndex" -> CustomIndex
  | "TrieIndex" -> TrieIndex
  | _ -> TrieIndex
;;

(* Convert a string to a human readable description of the provider *)
let descriptive_name_of_provider (provider: search_provider): string =
  match provider with
  | CustomIndex -> "Custom symbol index"
  | NoIndex -> "Symbol index disabled"
  | SqliteIndex -> "Sqlite"
  | TrieIndex -> "SharedMem/Trie"
;;

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
  | Class of Ast_defs.class_kind option
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant
  | Namespace

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
  | SI_XHP
  | SI_Namespace
  [@@deriving show]

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
  | SI_XHP -> 10
  | SI_Namespace -> 11

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
  | 10 -> SI_XHP
  | 11 -> SI_Namespace
  | _ -> SI_Unknown

(* Convert an internal "kind" into an autocomplete result kind *)
let kind_to_result (kind: si_kind): search_result_type =
  match kind with
  | SI_Class -> Class (Some Ast_defs.Cnormal)
  | SI_Interface -> Class (Some Ast_defs.Cinterface)
  | SI_Enum -> Class (Some Ast_defs.Cenum)
  | SI_Trait -> Class (Some Ast_defs.Ctrait)
  | SI_Unknown -> Constant
  | SI_Mixed -> Constant
  | SI_Function -> Function
  | SI_Typedef -> Typedef
  | SI_GlobalConstant -> Constant
  | SI_XHP -> Class (Some Ast_defs.Cnormal)
  | SI_Namespace -> Namespace

(* Convert an autocomplete result "kind" into an internal kind *)
let result_to_kind (result: search_result_type): si_kind =
  match result with
  | Class (Some Ast_defs.Cnormal) -> SI_Class
  | Class (Some Ast_defs.Ctrait) -> SI_Trait
  | Class (Some Ast_defs.Cabstract) -> SI_Class
  | Class (Some Ast_defs.Cinterface) -> SI_Interface
  | Class (Some Ast_defs.Cenum) -> SI_Enum
  | Class (None) -> SI_Class
  | Constant -> SI_GlobalConstant
  | Function -> SI_Function
  | Typedef -> SI_Typedef
  | Namespace -> SI_Namespace
  | _ -> SI_Unknown

(* Internal representation of a single item stored by the symbol list *)
type si_item = {
  si_name: string;
  si_kind: si_kind;
  si_filehash: int64;
}

(* Determine the best "ty" string for an item *)
let to_ty_string (item: si_item): string =
  match item.si_kind with
  | SI_Class -> "class"
  | SI_Interface -> "interface"
  | SI_Enum -> "enum"
  | SI_Trait -> "trait"
  | SI_Unknown -> "unknown"
  | SI_Mixed -> "mixed"
  | SI_Function -> "function"
  | SI_Typedef -> "type alias"
  | SI_GlobalConstant -> "constant"
  | SI_XHP -> "XHP class"
  | SI_Namespace -> "namespace"
(* More complete representation of a symbol index item *)
type si_fullitem = {
  sif_name: string;
  sif_kind: si_kind;
  sif_filepath: string;
  sif_is_abstract: bool;
  sif_is_final: bool;
}

(* ACID represents a statement.  Everything other than interfaces are valid *)
let valid_for_acid (s: si_fullitem): bool =
  match s.sif_kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Interface -> false
  | _ -> true

(* ACTYPE represents a type definition that can be passed as a parameter *)
let valid_for_actype (s: si_fullitem): bool =
  match s.sif_kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Trait
  | SI_Function
  | SI_GlobalConstant -> false
  | _ -> true

(* ACNEW represents instantiation of an object, so cannot be abstract *)
let valid_for_acnew (s: si_fullitem): bool =
  match s.sif_kind with
  | SI_Class
  | SI_Typedef
  | SI_XHP -> (not s.sif_is_abstract)
  | _ -> false

(* Internal representation of a full list of results *)
type si_results =
  si_item list

(* Fully captured information from a scan of WWW *)
type si_capture =
  si_fullitem list

(* Which system notified us of a file changed? *)
type file_source =
  | SavedState
  | TypeChecker

(* Keep track of file hash tombstones *)
module Tombstone = struct
  type t = int64

  let compare = Pervasives.compare

  let to_string = Int64.to_string
end

module Tombstone_set = struct
  include Reordered_argument_set(Set.Make(Tombstone))
end

(* Information about one leaf in the namespace tree *)
type nss_node = {

  (* The name of just this leaf *)
  nss_name: string;

  (* The full name including all parent trunks above this leaf *)
  nss_full_namespace: string;

  (* A hashtable of all leaf elements below this branch *)
  nss_children: (string, nss_node) Hashtbl.t;
}

(* Context information for the current symbol index *)
type si_env = {
  sie_provider: search_provider;
  sie_quiet_mode: bool;
  sie_fuzzy_search_mode: bool ref;
  sie_log_timings: bool;

  (* LocalSearchService *)
  lss_fileinfos: FileInfo.t Relative_path.Map.t;
  lss_filenames: FileInfo.names Relative_path.Map.t;
  lss_tombstones: Tombstone_set.t;

  (* SqliteSearchService *)
  sql_symbolindex_db: Sqlite3.db option ref;
  sql_select_symbols_stmt: Sqlite3.stmt option ref;
  sql_select_symbols_by_kind_stmt: Sqlite3.stmt option ref;
  sql_select_acid_stmt: Sqlite3.stmt option ref;
  sql_select_acnew_stmt: Sqlite3.stmt option ref;
  sql_select_actype_stmt: Sqlite3.stmt option ref;
  sql_select_namespaces_stmt: Sqlite3.stmt option ref;

  (* NamespaceSearchService *)
  nss_root_namespace: nss_node;
}

(* Default provider with no functionality *)
let default_si_env = {
  sie_provider = NoIndex;
  sie_quiet_mode = false;
  sie_fuzzy_search_mode = ref false;
  sie_log_timings = false;

  (* LocalSearchService *)
  lss_fileinfos = Relative_path.Map.empty;
  lss_filenames = Relative_path.Map.empty;
  lss_tombstones = Tombstone_set.empty;

  (* SqliteSearchService *)
  sql_symbolindex_db = ref None;
  sql_select_symbols_stmt = ref None;
  sql_select_symbols_by_kind_stmt = ref None;
  sql_select_acid_stmt = ref None;
  sql_select_acnew_stmt = ref None;
  sql_select_actype_stmt = ref None;
  sql_select_namespaces_stmt = ref None;

  (* NamespaceSearchService *)
  nss_root_namespace = {
    nss_name = "\\";
    nss_full_namespace = "\\";
    nss_children = Hashtbl.create 0;
  };
}
