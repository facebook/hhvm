(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type raw_color =
  | Default
  | Black
  | Red
  | Green
  | Yellow
  | Blue
  | Magenta
  | Cyan
  | White

type style =
  | Normal of raw_color
  | Bold of raw_color
  | Dim of raw_color
  | Italics of raw_color
  | Underline of raw_color
  | BoldDim of raw_color
  | BoldItalics of raw_color
  | BoldUnderline of raw_color
  | DimUnderline of raw_color
  | NormalWithBG of raw_color * raw_color
  | BoldWithBG of raw_color * raw_color

type color_mode =
  | Color_Always
  | Color_Never
  | Color_Auto

let text_num = function
  | Default -> "39"
  | Black -> "30"
  | Red -> "31"
  | Green -> "32"
  | Yellow -> "33"
  | Blue -> "34"
  | Magenta -> "35"
  | Cyan -> "36"
  | White -> "37"

let background_num = function
  | Default -> "49"
  | Black -> "40"
  | Red -> "41"
  | Green -> "42"
  | Yellow -> "43"
  | Blue -> "44"
  | Magenta -> "45"
  | Cyan -> "46"
  | White -> "47"

let color_num = function
  | Default -> "0"
  | x -> text_num x

let style_num = function
  | Normal c -> color_num c
  | Bold c -> color_num c ^ ";1"
  | Dim c -> color_num c ^ ";2"
  | Italics c -> color_num c ^ ";3"
  | Underline c -> color_num c ^ ";4"
  | BoldDim c -> color_num c ^ ";1;2"
  | BoldItalics c -> color_num c ^ ";1;3"
  | BoldUnderline c -> color_num c ^ ";1;4"
  | DimUnderline c -> color_num c ^ ";2;4"
  | NormalWithBG (text, bg) -> text_num text ^ ";" ^ background_num bg
  | BoldWithBG (text, bg) -> text_num text ^ ";" ^ background_num bg ^ ";1"

let style_num_from_list color styles =
  List.fold_left styles ~init:(color_num color) ~f:(fun accum style ->
      match style with
      | `Bold -> accum ^ ";1"
      | `Dim -> accum ^ ";2"
      | `Italics -> accum ^ ";3"
      | `Underline -> accum ^ ";4")

let supports_color =
  let memo = ref None in
  fun () ->
    match !memo with
    | Some x -> x
    | None ->
      let value =
        match Sys_utils.getenv_term () with
        | None -> false
        | Some term -> Unix.isatty Unix.stdout && not (String.equal term "dumb")
      in
      memo := Some value;
      value

let should_color color_mode =
  let force_color =
    Sys_utils.get_env "FORCE_ERROR_COLOR"
    |> Option.value_map ~default:false ~f:(fun s -> String.equal s "true")
  in
  match color_mode with
  | Color_Always -> true
  | Color_Never -> false
  | Color_Auto -> supports_color () || force_color

(* See https://github.com/yarnpkg/yarn/issues/405. *)
let supports_emoji () =
  (not (String.equal Sys.os_type "Win32")) && supports_color ()

let apply_color ?(color_mode = Color_Auto) c s : string =
  if should_color color_mode then
    Printf.sprintf "\x1b[%sm%s\x1b[0m" (style_num c) s
  else
    Printf.sprintf "%s" s

let apply_color_from_style ?(color_mode = Color_Auto) style s : string =
  if should_color color_mode then
    Printf.sprintf "\x1b[%sm%s\x1b[0m" style s
  else
    Printf.sprintf "%s" s

let print_one ?(color_mode = Color_Auto) ?(out_channel = Stdio.stdout) c s =
  Stdlib.Printf.fprintf out_channel "%s" (apply_color ~color_mode c s)

let cprint ?(color_mode = Color_Auto) ?(out_channel = Stdio.stdout) strs =
  List.iter strs ~f:(fun (c, s) -> print_one ~color_mode ~out_channel c s)

let cprintf ?(color_mode = Color_Auto) ?(out_channel = Stdio.stdout) c =
  Printf.ksprintf (print_one ~color_mode ~out_channel c)

(* ANSI escape sequence to clear whole line *)
let clear_line_seq = "\r\x1b[0K"

let print_clear_line chan =
  if Unix.isatty (Unix.descr_of_out_channel chan) then
    Printf.fprintf chan "%s%!" clear_line_seq
  else
    ()

(* Read a single char and return immediately, without waiting for a newline.
 * `man termios` to see how termio works. *)
let read_char () =
  let tty = Unix.(openfile "/dev/tty" [O_RDWR] 0o777) in
  let termio = Unix.tcgetattr tty in
  let new_termio =
    { termio with Unix.c_icanon = false; c_vmin = 1; c_vtime = 0 }
  in
  Unix.tcsetattr tty Unix.TCSANOW new_termio;
  let buf = Bytes.create 1 in
  let bytes_read = UnixLabels.read tty ~buf ~pos:0 ~len:1 in
  Unix.tcsetattr tty Unix.TCSANOW termio;
  assert (bytes_read = 1);
  Bytes.get buf 0

(* Prompt the user to pick one character out of a given list. If other
 * characters are entered, the prompt repeats indefinitely. *)
let read_choice message choices =
  let rec loop () =
    Stdio.printf
      "%s (%s)%!"
      message
      (String.concat ~sep:"|" (List.map choices ~f:(String.make 1)));
    let choice = read_char () in
    Stdio.print_endline "";
    if List.mem ~equal:Char.equal choices choice then
      choice
    else
      loop ()
  in
  loop ()

let eprintf fmt =
  if Unix.(isatty stderr) then
    Printf.eprintf fmt
  else
    Printf.ifprintf stderr fmt

(* Gets the number of columns in the current terminal window through
 * [`tput cols`][1]. If the command fails in any way then `None` will
 * be returned.
 *
 * This value may change over the course of program execution if a user resizes
 * their terminal.
 *
 * [1]: http://invisible-island.net/ncurses/man/tput.1.html
 *)
let get_term_cols () =
  if (not Sys.unix) || not (supports_color ()) then
    None
  else
    Option.map ~f:int_of_string (Sys_utils.exec_read "tput cols")
