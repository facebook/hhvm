(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Verbosity : sig
  type t =
    | Low
    | High
end

type symbol_edge = {
  symbol_type: FileInfo.name_type;
  symbol_name: string;
  symbol_dep: Typing_deps.Dep.dependency Typing_deps.Dep.variant;
}

type changed_symbol = {
  symbol_edge: symbol_edge;
  num_outgoing_edges: int option;
  outgoing_files: Relative_path.Set.t option;
}

type explanation = {
  removed_symbols: changed_symbol list;
  added_symbols: changed_symbol list;
}

type result = {
  naming_table: Naming_table.t;
      (** The naming table resulting from re-parsing the changed files. *)
  fanout_dependents: Typing_deps.DepSet.t;
      (** The set of dependents in the fanout. *)
  fanout_files: Relative_path.Set.t;
      (** The list of files that are in the fanout of the changed files, based on
      how the symbols in those files changed. *)
  explanations: explanation Relative_path.Map.t;
      (** Explanations of why each changed file produced its fanout files. *)
  telemetry: Telemetry.t;  (** Telemetry. **)
}

val explanation_to_json : explanation -> Hh_json.json

val go :
  verbosity:Verbosity.t ->
  Provider_context.t ->
  Naming_table.t ->
  Path.Set.t ->
  result

val file_info_to_dep_set :
  verbosity:Verbosity.t ->
  FileInfo.t ->
  Typing_deps.DepSet.t * changed_symbol list
