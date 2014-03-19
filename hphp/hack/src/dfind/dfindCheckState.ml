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
(* Checks that the current state is clean, that is, that we have more
 * files than there actually are on the file system.
 * If this wasn't the case, there is something horribly wrong.
 *)
(*****************************************************************************)
open DfindEnv

let must_skip env path =
  match env.skip with
  | Some skip when Str.string_match skip path 0 -> true
  | _ -> false

let cannot_stat path =
  try ignore (Unix.stat path); false with _ -> true

let clean = ref true

let rec check_file stack env path =
  if must_skip env path then () else
  if SSet.mem path stack then () (* Cyclic link *) else
  if cannot_stat path then () else
  let stack = SSet.add path stack in
  let st = Unix.stat path in
  match st.Unix.st_kind with
  | Unix.S_LNK ->
      let link = Unix.readlink path in
      check_file stack env link
  | Unix.S_DIR ->
      begin 
        let dir_handle = Unix.opendir path in
        try 
          let files = DfindAddFile.get_files path dir_handle in
          (try Unix.closedir dir_handle with _ -> ());
          SSet.iter (check_file stack env) files;
        with e ->
          (try Unix.closedir dir_handle with _ -> ());
          raise e
      end
  | _ ->
      if TimeFiles.mem (Time.bot, path) env.files
      then ()
      else (clean := false; Printf.fprintf stderr "Missing file: %s\n" path)


let rec check env =
  SMap.iter begin fun dir _ ->
    check_file (SSet.empty) env dir
  end env.dirs
      
let check env =
  check env;
  if !clean then (Printf.printf "OK"; exit 0);
  exit 2
