(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Errors accumulator. *)
(*****************************************************************************)
open Json

type error = (Pos.t * string) list

type t = error list

let to_json (e : error) : json =
  let elts = List.map (fun (p, w) ->
                        let line, scol, ecol = Pos.info_pos p in
                        JAssoc [ "descr", JString w;
                                 "path",  JString p.Pos.pos_file;
                                 "line",  JInt line;
                                 "start", JInt scol;
                                 "end",   JInt ecol
                               ]
                      ) e
  in
  JAssoc [ "message", JList elts ]

let (error_list: t ref) = ref []

let add pos msg =
  error_list := [pos, msg] :: !error_list

let add_list l =
  assert (l <> []);
  error_list := l :: !error_list

let pmsg p s =
  Printf.sprintf "%s\n%s\n" (Pos.string p) s

let pmsg_l l =
  let l = List.map (fun (p, e) -> pmsg p e) l in
  List.fold_right (^) l ""

let to_string (e : error) : string =
  let buf = Buffer.create 50 in
  List.iter (fun (p, w) -> Buffer.add_string buf (pmsg p w)) e;
  Buffer.contents buf

(*****************************************************************************)
(* Try if errors. *)
(*****************************************************************************)

let try_ f1 f2 =
  let error_list_copy = !error_list in
  error_list := [];
  let result = f1 () in
  let errors = !error_list in
  error_list := error_list_copy;
  match List.rev errors with
  | [] -> result
  | l :: _ -> f2 l

(*****************************************************************************)
(* Do. *)
(*****************************************************************************)

let do_ f =
  let error_list_copy = !error_list in
  error_list := [];
  let result = f () in
  let out_errors = !error_list in
  error_list := error_list_copy;
  List.rev out_errors, result
