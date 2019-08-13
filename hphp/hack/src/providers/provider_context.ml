(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type entry = {
  file_input: ServerCommandTypes.file_input;
  path: Relative_path.t;
  ast: Nast.program;
  tast: Tast.program;
}

type t = entry Relative_path.Map.t

let empty = Relative_path.Map.empty

let get_file_input
    ~(ctx: t)
    ~(path: Relative_path.t)
    : ServerCommandTypes.file_input =
  match Relative_path.Map.get ctx path with
  | Some { file_input; _ } ->
    file_input
  | None ->
    ServerCommandTypes.FileName (Relative_path.to_absolute path)

let get_fileinfo ~(entry: entry): FileInfo.t =
  let (funs, classes, typedefs, consts) = Nast.get_defs entry.ast in
  { FileInfo.empty_t with
    FileInfo.funs;
    classes;
    typedefs;
    consts;
  }
