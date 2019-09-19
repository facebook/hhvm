(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type include_path =
  | Absolute of string (* /foo/bar/baz.php *)
  | SearchPathRelative of string (* foo/bar/baz.php *)
  | IncludeRootRelative of string * string (* $_SERVER['PHP_ROOT'] . "foo/bar/baz.php" *)
  | DocRootRelative of string
[@@deriving show]

module IncludePathSet = Set.Make (struct
  type nonrec t = include_path

  let compare = compare
end)

(* Data structure for keeping track of symbols (and includes) we encounter in
   the course of emitting bytecode for an AST. We split them into these four
   categories for the sake of HHVM, which has lookup function corresponding to
   each.
*)
type t = {
  includes: IncludePathSet.t;
  constants: SSet.t;
  functions: SSet.t;
  classes: SSet.t;
}

let resolve_to_doc_root_relative ?(include_roots = SMap.empty) = function
  | IncludeRootRelative (var, lit) as ip ->
    begin
      match SMap.find_opt var include_roots with
      | Some prefix ->
        let path = Filename.concat prefix lit in
        if Filename.is_relative path then
          DocRootRelative path
        else
          Absolute path
      (* This should probably never happen. *)
      | _ -> ip
    end
  | ip -> ip
