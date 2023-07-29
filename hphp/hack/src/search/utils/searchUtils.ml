(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Reordered_argument_collections
include SearchTypes

(* Known search providers *)
type search_provider =
  | CustomIndex
  | NoIndex
  | SqliteIndex
  | LocalIndex
[@@deriving show]

(* The context in which autocomplete is being performed *)
type autocomplete_type =
  | Acid
  | Acnew
  | Actype
  | Actrait_only
  | Ac_workspace_symbol (* Excludes namespaces; used for symbol search *)
[@@deriving eq, show]

(* Convert a string to a provider *)
let provider_of_string (provider_str : string) : search_provider =
  match provider_str with
  | "SqliteIndex" -> SqliteIndex
  | "NoIndex" -> NoIndex
  | "CustomIndex" -> CustomIndex
  | "LocalIndex" -> LocalIndex
  | _ -> SqliteIndex

(* Convert a string to a human readable description of the provider *)
let descriptive_name_of_provider (provider : search_provider) : string =
  match provider with
  | CustomIndex -> "Custom symbol index"
  | NoIndex -> "Symbol index disabled"
  | SqliteIndex -> "Sqlite"
  | LocalIndex -> "Local file index only"

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

let is_si_class = function
  | SI_Class -> true
  | _ -> false

let is_si_trait = function
  | SI_Trait -> true
  | _ -> false

(* Individual result object as known by the autocomplete system *)
type symbol = (Pos.absolute, si_kind) term

(* Used by some legacy APIs *)
type legacy_symbol = (FileInfo.pos, si_kind) term

(* Collected results as known by the autocomplete system *)
type result = symbol list

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
let kind_to_int (kind : si_kind) : int =
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
  | SI_ClassMethod -> 12
  | SI_Literal -> 13
  | SI_ClassConstant -> 14
  | SI_Property -> 15
  | SI_LocalVariable -> 16
  | SI_Keyword -> 17
  | SI_Constructor -> 18

(* Convert an integer back to an enum *)
let int_to_kind (kind_num : int) : si_kind =
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
  | 12 -> SI_ClassMethod
  | 13 -> SI_Literal
  | 14 -> SI_ClassConstant
  | 15 -> SI_Property
  | 16 -> SI_LocalVariable
  | 17 -> SI_Keyword
  | 18 -> SI_Constructor
  | _ -> SI_Unknown

type si_file =
  | SI_Filehash of string  (** string represent Int64 *)
  | SI_Path of Relative_path.t

(* Internal representation of a single item stored by the symbol list *)
type si_item = {
  si_name: string;
  si_kind: si_kind;
  si_file: si_file;
  si_fullname: string;
}

(* Determine the best "ty" string for an item *)
let kind_to_string (kind : si_kind) : string =
  match kind with
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
  | SI_ClassMethod -> "class method"
  | SI_Literal -> "literal"
  | SI_ClassConstant -> "class constant"
  | SI_Property -> "class property"
  | SI_LocalVariable -> "local variable"
  | SI_Keyword -> "keyword"
  | SI_Constructor -> "constructor"

(* Sigh, yet another string to enum conversion *)
let string_to_kind (type_ : string) : si_kind option =
  match type_ with
  | "class" -> Some SI_Class
  | "interface" -> Some SI_Interface
  | "enum" -> Some SI_Enum
  | "trait" -> Some SI_Trait
  | "unknown" -> Some SI_Unknown
  | "mixed" -> Some SI_Mixed
  | "function" -> Some SI_Function
  (* Compatibility with strings used by Hack Search Service as well as ty_string *)
  | "typedef"
  | "type alias" ->
    Some SI_Typedef
  | "constant" -> Some SI_GlobalConstant
  | "xhp" -> Some SI_XHP
  | "namespace" -> Some SI_Namespace
  | "class method" -> Some SI_ClassMethod
  | "literal" -> Some SI_Literal
  | "class constant" -> Some SI_ClassConstant
  | "property"
  | "class property" ->
    Some SI_Property
  | "local variable" -> Some SI_LocalVariable
  | "keyword" -> Some SI_Keyword
  | "constructor" -> Some SI_Constructor
  | _ -> None

(* More complete representation of a symbol index item *)
type si_fullitem = {
  (* NOTE: this is expected to have its leading backslash stripped. See [Utils.strip_ns] *)
  sif_name: string;
  sif_kind: si_kind;
  sif_filepath: string;
  sif_is_abstract: bool;
  sif_is_final: bool;
}

(* ACID represents a statement.  Everything other than interfaces are valid *)
let valid_for_acid (s : si_fullitem) : bool =
  match s.sif_kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Interface ->
    false
  | _ -> true

(* ACTYPE represents a type definition that can be passed as a parameter *)
let valid_for_actype (s : si_fullitem) : bool =
  match s.sif_kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Trait
  | SI_Function
  | SI_GlobalConstant ->
    false
  | _ -> true

(* ACNEW represents instantiation of an object, so cannot be abstract *)
let valid_for_acnew (s : si_fullitem) : bool =
  match s.sif_kind with
  | SI_Class
  | SI_Typedef
  | SI_XHP ->
    not s.sif_is_abstract
  | _ -> false

(* Internal representation of a full list of results *)
type si_results = si_item list

(* Fully captured information from a scan of WWW *)
type si_capture = si_fullitem list

(* Which system notified us of a file changed? *)
type file_source =
  | Init
  | TypeChecker

(* Keep track of file hash tombstones *)
module Tombstone = struct
  type t = int64

  let compare = Int64.compare

  let to_string = Int64.to_string
end

module Tombstone_set = struct
  include Reordered_argument_set (Set.Make (Tombstone))
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
  (*
   * Setting the "resolve" parameters to true slows down autocomplete
   * but increases precision for answers.
   *
   * sie_resolve_signatures: When a result appears in autocomplete,
   * look up the full declaration and include parameters.  Uses extra
   * memory.
   *
   * sie_resolve_positions: When a result appears in autocomplete,
   * look up the exact position of the symbol from the naming table.
   * Slows down autocomplete.
   *
   * sie_resolve_local_decl: When a file changes on disk, the local
   * search index digs through its decls and saves exact class
   * information.  Uses more memory and slows down processing of
   * changed files.
   *)
  sie_resolve_signatures: bool;
  sie_resolve_positions: bool;
  sie_resolve_local_decl: bool;
  (* LocalSearchService *)
  lss_fullitems: si_capture Relative_path.Map.t;
  lss_tombstones: Relative_path.Set.t;
      (** files that have been locally modified *)
  lss_tombstone_hashes: Tombstone_set.t;
      (** hashes of suffixes of files that have been locally modified -
      this only exists for compatibility with stores (sql, www.autocomplete)
      that use filehashes, and won't be needed once we move solely
      to stures that use paths (local, www.hack.light). *)
  (* SqliteSearchService *)
  sql_symbolindex_db: Sqlite3.db option ref;
  sql_select_symbols_stmt: Sqlite3.stmt option ref;
  sql_select_symbols_by_kind_stmt: Sqlite3.stmt option ref;
  sql_select_acid_stmt: Sqlite3.stmt option ref;
  sql_select_acnew_stmt: Sqlite3.stmt option ref;
  sql_select_actype_stmt: Sqlite3.stmt option ref;
  sql_select_namespaces_stmt: Sqlite3.stmt option ref;
  sql_select_namespaced_symbols_stmt: Sqlite3.stmt option ref;
  (* NamespaceSearchService *)
  nss_root_namespace: nss_node;
  (* CustomSearchService *)
  glean_reponame: string;
}

(* Default provider with no functionality *)
let default_si_env =
  {
    sie_provider = NoIndex;
    sie_quiet_mode = false;
    sie_fuzzy_search_mode = ref false;
    sie_log_timings = false;
    sie_resolve_signatures = false;
    sie_resolve_positions = false;
    sie_resolve_local_decl = false;
    (* LocalSearchService *)
    lss_fullitems = Relative_path.Map.empty;
    lss_tombstones = Relative_path.Set.empty;
    lss_tombstone_hashes = Tombstone_set.empty;
    (* SqliteSearchService *)
    sql_symbolindex_db = ref None;
    sql_select_symbols_stmt = ref None;
    sql_select_symbols_by_kind_stmt = ref None;
    sql_select_acid_stmt = ref None;
    sql_select_acnew_stmt = ref None;
    sql_select_actype_stmt = ref None;
    sql_select_namespaces_stmt = ref None;
    sql_select_namespaced_symbols_stmt = ref None;
    (* NamespaceSearchService *)
    nss_root_namespace =
      {
        nss_name = "\\";
        nss_full_namespace = "\\";
        nss_children = Hashtbl.create 0;
      };
    (* CustomSearchService *)
    glean_reponame = "";
  }

(* Default provider, but no logging *)
let quiet_si_env =
  { default_si_env with sie_quiet_mode = true; sie_log_timings = false }
