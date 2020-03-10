(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module PositionedSyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

type entry = {
  path: Relative_path.t;
  contents: string;
  mutable source_text: Full_fidelity_source_text.t option;
  mutable parser_return: Parser_return.t option;
  mutable ast_errors: Errors.t option;
  mutable cst: PositionedSyntaxTree.t option;
  mutable tast: Tast.program option;
  mutable tast_errors: Errors.t option;
  mutable symbols: Relative_path.t SymbolOccurrence.t list option;
}

type t = {
  popt: ParserOptions.t;
  tcopt: TypecheckerOptions.t;
  backend: Provider_backend.t;
  entries: entry Relative_path.Map.t;
}

let empty_for_tool ~popt ~tcopt ~backend =
  { popt; tcopt; backend; entries = Relative_path.Map.empty }

let empty_for_worker ~popt ~tcopt =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    entries = Relative_path.Map.empty;
  }

let empty_for_test ~popt ~tcopt =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    entries = Relative_path.Map.empty;
  }

let empty_for_debugging ~popt ~tcopt =
  {
    popt;
    tcopt;
    backend = Provider_backend.Shared_memory;
    entries = Relative_path.Map.empty;
  }

let map_tcopt (t : t) ~(f : TypecheckerOptions.t -> TypecheckerOptions.t) : t =
  { t with tcopt = f t.tcopt }

let ref_is_quarantined : bool ref = ref false

let is_quarantined () : bool = !ref_is_quarantined

let set_is_quarantined_internal () : unit =
  match !ref_is_quarantined with
  | true -> failwith "set_is_quarantined: was already quarantined"
  | false -> ref_is_quarantined := true

let unset_is_quarantined_internal () : unit =
  match !ref_is_quarantined with
  | true -> ref_is_quarantined := false
  | false -> failwith "unset_is_quarantined: wasn't already quarantined"

let get_telemetry (t : t) (telemetry : Telemetry.t) : Telemetry.t =
  let telemetry =
    telemetry
    |> Telemetry.string_
         ~key:"backend"
         ~value:(t.backend |> Provider_backend.t_to_string)
    |> Telemetry.object_ ~key:"SharedMem" ~value:(SharedMem.get_telemetry ())
    (* We get SharedMem telemetry for all providers, not just the SharedMem
  provider, just in case there are code paths which use SharedMem despite
  it not being the intended provider. *)
  in
  match t.backend with
  | Provider_backend.Local_memory
      { decl_cache; shallow_decl_cache; linearization_cache; _ } ->
    telemetry
    |> Telemetry.object_
         ~key:"decl_cache"
         ~value:(Provider_backend.Decl_cache.get_telemetry decl_cache)
    |> Telemetry.object_
         ~key:"shallow_decl_cache"
         ~value:
           (Provider_backend.Shallow_decl_cache.get_telemetry
              shallow_decl_cache)
    |> Telemetry.object_
         ~key:"linearization_cache"
         ~value:
           (Provider_backend.Linearization_cache.get_telemetry
              linearization_cache)
  | _ -> telemetry

let reset_telemetry (t : t) : unit =
  match t.backend with
  | Provider_backend.Local_memory { decl_cache; shallow_decl_cache; _ } ->
    Provider_backend.Decl_cache.reset_telemetry decl_cache;
    Provider_backend.Shallow_decl_cache.reset_telemetry shallow_decl_cache;
    ()
  | _ -> ()
