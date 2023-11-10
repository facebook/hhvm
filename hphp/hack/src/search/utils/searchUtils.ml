(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Reordered_argument_collections
open SearchTypes

type mock_on_find =
  query_text:string ->
  context:SearchTypes.autocomplete_type ->
  kind_filter:SearchTypes.si_kind option ->
  SearchTypes.si_item list

(* Known search providers *)
type search_provider =
  | CustomIndex
  | NoIndex
  | MockIndex of { mock_on_find: mock_on_find [@opaque] }
      (** used in testing and debugging *)
  | LocalIndex
[@@deriving show]

(* Convert a string to a provider *)
let provider_of_string (provider_str : string) : search_provider =
  match provider_str with
  | "NoIndex" -> NoIndex
  | "MockIndex" -> failwith ("unsupported provider " ^ provider_str)
  | "CustomIndex" -> CustomIndex
  | "LocalIndex" -> LocalIndex
  | _ -> failwith ("invalid search-provider: " ^ provider_str)

(* Convert a string to a human readable description of the provider *)
let descriptive_name_of_provider (provider : search_provider) : string =
  match provider with
  | CustomIndex -> "Custom symbol index"
  | NoIndex -> "Symbol index disabled"
  | MockIndex _ -> "Mock index"
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
  sie_namespace_map: (string * string) list;
  (* MockIndex *)
  mock_on_find: mock_on_find;
  (* LocalSearchService *)
  lss_fullitems: si_capture Relative_path.Map.t;
  lss_tombstones: Relative_path.Set.t;
      (** files that have been locally modified *)
  lss_tombstone_hashes: Tombstone_set.t;
      (** hashes of suffixes of files that have been locally modified -
      this only exists for compatibility with stores (sql, www.autocomplete)
      that use filehashes, and won't be needed once we move solely
      to stures that use paths (local, www.hack.light). *)
  (* CustomSearchService *)
  glean_reponame: string;
  glean_handle: Glean.handle option;
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
    sie_namespace_map = [];
    (* MockIndex *)
    mock_on_find = (fun ~query_text:_ ~context:_ ~kind_filter:_ -> []);
    (* LocalSearchService *)
    lss_fullitems = Relative_path.Map.empty;
    lss_tombstones = Relative_path.Set.empty;
    lss_tombstone_hashes = Tombstone_set.empty;
    (* CustomSearchService *)
    glean_reponame = "";
    glean_handle = None;
  }

(* Default provider, but no logging *)
let quiet_si_env =
  { default_si_env with sie_quiet_mode = true; sie_log_timings = false }
