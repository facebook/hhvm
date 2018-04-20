(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

let go env (filename, line, char) =
  let open Option.Monad_infix in
  let ServerEnv.{ tcopt; _ } = env in
  let relative_path = Relative_path.create_detect_prefix filename in
  File_heap.get_contents relative_path
  >>= begin fun contents ->
    let definitions =
      ServerIdentifyFunction.go contents line char tcopt
      |> List.filter_map ~f:(fun (_, def) -> def)
    in
    List.hd definitions
  end
  >>= ServerSymbolDefinition.get_definition_cst_node (ServerCommandTypes.FileName filename)
  >>= Docblock_finder.get_docblock
