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
(* Code for auto-completion *)
(*****************************************************************************)
open Utils

let complete_global nenv oc =
  let gname = !Autocomplete.auto_complete_for_global in
  let completion_type = !Autocomplete.argument_global_type in
  let gname = 
    String.sub gname 0 (String.length gname - Autocomplete.suffix_len)
  in
  let gname_size = String.length gname in
  let is_prefix x =
    gname_size <= String.length x &&
    String.sub x 0 gname_size = gname
  in
  let count = ref 0 in
  let print_match handler map =
    SMap.iter begin fun name _ ->
      if !count > 100 then raise Exit;
      if is_prefix name && handler name
      then (Printf.fprintf oc "%s\n" name; incr count)
      else ()
    end map;
  in
  try
    print_match (Typing_utils.should_complete_fun completion_type)
        nenv.Naming.ifuns;
    print_match (Typing_utils.should_complete_class completion_type)
        nenv.Naming.iclasses;
    flush oc
  with Exit ->
    flush oc

let restore() =
  Autocomplete.auto_complete := false;
  Silent.is_silent_mode := false;
  Autocomplete.auto_complete_for_global := "";
  Autocomplete.auto_complete_result := SMap.empty;
  Autocomplete.argument_global_type := None;
  ()
      
let setup() =
  Autocomplete.auto_complete := true;
  Silent.is_silent_mode := true;
  Autocomplete.auto_complete_for_global := "";
  Autocomplete.auto_complete_result := SMap.empty;
  Autocomplete.argument_global_type := None;
  ()

let auto_complete env content oc =
  let funs, classes = ServerIdeUtils.declare content in
  let nenv = env.ServerEnv.nenv in
  let dummy_pos = Pos.none, Ident.foo in
  let ifuns = 
    SSet.fold begin fun x acc -> SMap.add x dummy_pos acc
    end funs nenv.Naming.ifuns
  in
  let iclasses = 
    SSet.fold begin fun x acc -> 
      SMap.add x dummy_pos acc
    end classes nenv.Naming.iclasses
  in
  let nenv = 
    { nenv with Naming.ifuns = ifuns; Naming.iclasses = iclasses } in
  setup();
  ServerIdeUtils.fix_file_and_def content;
  let result =
    SMap.fold
      (fun _ res acc ->
        let name = res.Autocomplete.name in
        let pos = res.Autocomplete.pos in
        let ty = res.Autocomplete.ty in
        let pos_string = match pos with
          | None -> ""
          | Some p -> Pos.string p in
        let name = match ty with
          | None -> name
          | Some ty -> name^" "^ty in
        (name, pos_string) :: acc)
      !(Autocomplete.auto_complete_result)
      []
  in
  ServerIdeUtils.revive funs classes;
  Printf.printf "Auto-complete\n"; flush stdout;
  if !Autocomplete.auto_complete_for_global <> ""
  then complete_global nenv oc
  else List.iter begin fun (k, _) ->
    Printf.fprintf oc "%s\n" k; flush oc;
  end (List.rev result);
  restore();
  ()
