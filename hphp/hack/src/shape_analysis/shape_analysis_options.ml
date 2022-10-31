(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types

let parse_mode str =
  let parse_command = function
    | "dump" -> Some DumpConstraints
    | "dump-derived" -> Some DumpDerivedConstraints
    | "simplify" -> Some SimplifyConstraints
    | "codemod" -> Some Codemod
    | "solve" -> Some SolveConstraints
    | "close" -> Some CloseConstraints
    | _ -> None
  in
  let parse_mode = function
    | "local" -> Some Local
    | "global" -> Some Global
    | _ -> None
  in
  let components = String.split str ~on:':' in
  let open Option.Monad_infix in
  match components with
  | [command; mode] ->
    parse_command command >>= fun command ->
    parse_mode mode >>= fun mode -> Some (command, mode)
  | _ -> None

let mk ~command ~mode ~verbosity = { command; mode; verbosity }
