(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

include Sys

module CE = Common_exns
module Str = Str_ext

open Utils

exception NotADirectory of string
let (=~) = Str.(=~)

let perms f =
  (Unix.stat f).Unix.st_perm

let is_file file =
  (Unix.stat file).Unix.st_kind = Unix.S_REG

let is_symlink file =
  (Unix.lstat file).Unix.st_kind = Unix.S_LNK

let copy_dir s d =
  if file_exists d
  then raise CE.Impossible
  else Unix.mkdir d (perms s)

let chop_dirsymbol = function
  | s when s =~ "\\(.*\\)/$" -> Str.matched_group 1 s
  | s -> s

let filename_without_leading_path prefix s =
  let prefix = chop_dirsymbol prefix in
  if s = prefix
  then "."
  else
    if s =~ ("^" ^ prefix ^ "/\\(.*\\)$")
    then Str.matched_group 1 s
    else raise CE.Impossible

let recursive_file_pairs src dest =
  let src = Path.make src in
  let escaped_src = Path.to_string src in
  List.map begin fun f ->
    let suffix = filename_without_leading_path escaped_src f in
    if suffix = "." then (f, dest) else (f, dest ^ "/" ^ suffix)
  end (Find.find [ src ])

let has_extension f ext =
  if ext.[0] <> '.'
  then failwith "I need an extension such as .c not just c";
  f =~ (".*\\" ^ ext ^ "$")

let set_extension f ext =
  if ext.[0] <> '.'
  then failwith "I need an extension such as .c not just c";
  if f =~ ("\\(.*\\)\\.[^\\.]+$")
  then (Str.matched_group 1 f) ^ ext
  else failwith "Regex match failure on extension"

let copy_file s d =
  if Sys.win32 then begin
    let st = Unix.stat s in
    ignore (command ("copy \"" ^ s ^ "\" \"" ^ d ^ "\""));
    (* `copy` does not preserve last access time (`atime`). *)
    Unix.(utimes d st.st_atime st.st_mtime)
  end else
    ignore (command ("cp -p \"" ^ s ^ "\" \"" ^ d ^ "\""))

let rec mkdir_p = function
  | "" -> raise CE.Impossible
  | d when not (file_exists d) ->
    mkdir_p (Filename.dirname d);
    Unix.mkdir d 0o770;
  | d when is_directory d -> ()
  | d -> raise (NotADirectory d)

let write_file str f =
  if file_exists f
  then raise (CE.FileExists f);
  let oc = open_out f in
  output_string oc str;
  close_out oc

let die errors =
  let oc = stderr in
  List.iter begin fun (name, msg) ->
    output_string oc (name ^ ":\n");
    output_string oc (msg ^ "\n");
  end errors;
  close_out oc;
  exit 2
