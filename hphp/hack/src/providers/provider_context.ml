(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type entry = {
  file_input: ServerCommandTypes.file_input;
  path: Relative_path.t;
  source_text: Full_fidelity_source_text.t;
  ast: Nast.program;
  mutable tast: Tast.program option;
  mutable errors: Errors.t option;
  mutable symbols: Relative_path.t SymbolOccurrence.t list option;
}

type t = {
  tcopt: TypecheckerOptions.t;
  entries: entry Relative_path.Map.t;
}

let empty ~tcopt = { tcopt; entries = Relative_path.Map.empty }

let global_context : t option ref = ref None

let get_file_input ~(ctx : t) ~(path : Relative_path.t) :
    ServerCommandTypes.file_input =
  match Relative_path.Map.find_opt ctx.entries path with
  | Some { file_input; _ } -> file_input
  | None -> ServerCommandTypes.FileName (Relative_path.to_absolute path)

let get_fileinfo ~(entry : entry) : FileInfo.t =
  let (funs, classes, record_defs, typedefs, consts) =
    Nast.get_defs entry.ast
  in
  {
    FileInfo.empty_t with
    FileInfo.funs;
    classes;
    record_defs;
    typedefs;
    consts;
  }

let get_global_context () : t option = !global_context

let set_global_context_internal (t : t) : unit =
  match !global_context with
  | Some _ ->
    failwith "set_global_context_internal: a global context is already set"
  | None -> global_context := Some t

let unset_global_context_internal () : unit =
  match !global_context with
  | Some _ -> global_context := None
  | None -> failwith "unset_global_context_internal: no global context is set"
