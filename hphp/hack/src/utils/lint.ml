(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

let (lint_list: Errors.t option ref) = ref None

let add code pos msg =
  if !Errors.is_hh_fixme pos code then () else begin
    let lint = Errors.make_error code [pos, msg] in
    match !lint_list with
    | Some lst -> lint_list := Some (lint :: lst)
    (* by default, we ignore lint errors *)
    | None -> ()
  end

module Codes = struct
  let lowercase_constant                    = 5001 (* DONT MODIFY!!!! *)
  let use_collection_literal                = 5002 (* DONT MODIFY!!!! *)

  (* Values 5501 - 5999 are reserved for FB-internal use *)

  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
end

let lowercase_constant pos cst =
  let lower = String.lowercase cst in
  add Codes.lowercase_constant pos
    (spf "Please use '%s' instead of '%s'" lower cst)

let use_collection_literal pos coll =
  let coll = strip_ns coll in
  add Codes.use_collection_literal pos
    (spf "Use `%s {...}` instead of `new %s(...)`" coll coll)

let do_ f =
  let list_copy = !lint_list in
  lint_list := Some [];
  let result = f () in
  let out = match !lint_list with
    | Some lst -> lst
    | None -> assert false in
  lint_list := list_copy;
  List.rev out, result
