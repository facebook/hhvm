(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

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

let color_num = function
  | Default -> "0"
  | Black   -> "30"
  | Red     -> "31"
  | Green   -> "32"
  | Yellow  -> "33"
  | Blue    -> "34"
  | Magenta -> "35"
  | Cyan    -> "36"
  | White   -> "37"

let style_num = function
  | Normal c -> color_num c
  | Bold c   -> color_num c ^ ";1"

let print_one (c,s) =
  Printf.printf "\x1b[%sm%s\x1b[0m" (style_num c) (s)

let print strs = List.iter print_one strs
