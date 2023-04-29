(*
 * Copyright (c) 2018-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *)

type t = {
  exn: exn;
  backtrace: Printexc.raw_backtrace;
}

(* In ocaml, backtraces (the path that the exception bubbled up after being thrown) are stored as
 * global state and NOT with the exception itself. This means the only safe place to ever read the
 * backtrace is immediately after the exception is caught in the `with` block of a `try...with`.
 *
 * Proper use of this module is something like
 *
 *  try
 *    ...
 *  with exn ->
 *    let e = Exception.wrap exn in (* DO THIS FIRST!!! *)
 *    my_fun e; (* If this code throws internally it will overwrite the global backtrace *)
 *    Exception.reraise e
 *)

let wrap exn =
  let backtrace = Printexc.get_raw_backtrace () in
  { exn; backtrace }

(* The inverse of `wrap`, returns the wrapped `exn`. You might use this to pattern
   match on the raw exception or print it, but should not reraise it since it
   will not include the correct backtrace; use `reraise` or `to_exn` instead. *)
let unwrap { exn; backtrace = _ } = exn

let reraise { exn; backtrace } = Printexc.raise_with_backtrace exn backtrace

(* Converts back to an `exn` with the right backtrace. Generally, avoid this in favor of
   the helpers in this module, like `to_string` and `get_backtrace_string`. *)
let to_exn t =
  try reraise t with
  | exn -> exn

(* Like `wrap`, but for the unusual case where you want to create an `Exception`
   for an un-raised `exn`, capturing its stack trace. If you've caught an exception,
   you should use `wrap` instead, since it already has a stack trace. *)
let wrap_unraised ?(frames = 100) exn =
  let frames =
    if Printexc.backtrace_status () then
      frames
    else
      0
  in
  let backtrace = Printexc.get_callstack frames in
  { exn; backtrace }

let get_ctor_string { exn; backtrace = _ } = Printexc.to_string exn

let register_printer printer = Printexc.register_printer printer

let get_backtrace_string { exn = _; backtrace } =
  Printexc.raw_backtrace_to_string backtrace

let to_string t =
  let ctor = get_ctor_string t in
  let bt = get_backtrace_string t in
  if bt = "" then
    ctor
  else
    ctor ^ "\n" ^ bt

let get_current_callstack_string n =
  Printexc.get_callstack n |> Printexc.raw_backtrace_to_string

let record_backtrace = Printexc.record_backtrace

(** We want to include all stack lines that come from our code,
and exclude ones that come from the standard library.
That's easy for stack lines that include a module: if the module starts "Stdlib" or "Base"
then exclude it; if it starts with anything else then include it.
But for stack lines that don't include a module then we'll use a heuristic that
only works on some build systems. Some build systems like buck include absolute pathnames
so that "/.../hack/src/hh_client.ml" implies it's our code and "src/list.ml" implies it's not.
Other build systems like dune are relative and there's no way we can distinguish "src/hh_client.ml" from "src/list.ml".
These regular expressions extract all that information.
Our heuristic in [clean_stack] will simply assume buck-style absolute pathnames. *)
let (stack_re, file_re) =
  ( Str.regexp
      {|^\(Called from\|Raised by primitive operation at\|Raised at\|Re-raised at\)\( \([^ ]*\) in\)? file "\([^"]*\)"\( (inlined)\)?, line \([0-9]+\), character.*$|},
    Str.regexp {|^.*/hack/\(.*\)$|} )

let clean_stack (stack : string) : string =
  let format_one_line (s : string) : string option =
    let open Hh_prelude in
    if Str.string_match stack_re s 0 then
      let module_ =
        try Str.matched_group 3 s with
        | _ -> ""
      in
      let file = Str.matched_group 4 s in
      let line = Str.matched_group 6 s in
      let (file, file_is_in_hack_src_tree) =
        if Str.string_match file_re file 0 then
          (Str.matched_group 1 file, true)
        else
          (file, false)
      in
      if String.equal module_ "" then
        if file_is_in_hack_src_tree then
          Some (Printf.sprintf "%s @ %s" file line)
        else
          None
      else if
        String.is_prefix module_ ~prefix:"Base"
        || String.is_prefix module_ ~prefix:"Stdlib"
        || String.is_prefix module_ ~prefix:"Lwt"
      then
        None
      else
        Some (Printf.sprintf "%s @ %s" file line)
    else
      Some s
  in
  String_utils.split_on_newlines stack
  |> List.filter_map format_one_line
  |> String.concat "\n"

let pp ppf t =
  Format.fprintf
    ppf
    "@[<1>%s: %s@]@."
    (get_ctor_string t)
    (get_backtrace_string t |> clean_stack)

let show t = get_ctor_string t
