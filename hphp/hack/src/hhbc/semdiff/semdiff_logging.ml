(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Default logging level is 2 *)
let verbosity_level = ref 2

(* Default: similarity is shown *)
let hide_sim = ref false

(* Default: size is shown *)
let hide_size = ref false

(* Default: distance is shown *)
let hide_dist = ref false

(* Default: assumed and todo info is shown *)
let hide_assm = ref false

let print ~level ?(newline = true) ?(channel = stdout) c s =
  if !verbosity_level >= level
  then Tty.cprintf ~out_channel:channel c "%s%s" s (if newline then "\n" else "")
  else ()

let print_default ?(level = 0) ?(color = Tty.White) ?(newline = true) =
  print ~level ~newline (Tty.Normal color)

let print_newline ~level =
  print_default ~level ""

let error ~level ?(newline = true) c s =
  print ~level ~newline ~channel:stderr c s

let debug ?(newline = true) c s =
  print ~level:2 ~newline ~channel:stdout c s

let trace ?(newline = true) c s =
    print ~level:3 ~newline ~channel:stdout c s

type tagged_string =
 | Add of string
 | Del of string
 | Def of string

let print_tagged_string ?(out_channel=stdout) ts =
 match ts with
  | Add s -> Tty.cprintf ~out_channel (Tty.Normal Tty.Green) "+ %s\n" s
  | Del s -> Tty.cprintf (Tty.Normal Tty.Red) "- %s\n" s
  | Def s -> Tty.cprintf (Tty.Normal Tty.White) "%s\n" s

let print_edit_sequence ~level ?(out_channel = stdout) edit_seq =
 if !verbosity_level >= level
 then List.iter (print_tagged_string ~out_channel) edit_seq
 else ()

let print_set_json ?(level = 0) ?(trailing_coma = true) ~name s =
  print_default ~level @@ Printf.sprintf "  \"%s\":[" name;
  let list_string = String.concat ",\n"
    @@ List.map (fun s -> "    \"" ^ s ^ "\"") @@ SSet.elements s in
  print_default ~level list_string;
  print_default ~level @@
    Printf.sprintf "  ]%s" (if trailing_coma then "," else "")
