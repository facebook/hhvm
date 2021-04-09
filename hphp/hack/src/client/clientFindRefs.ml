(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

let go (res : ServerCommandTypes.Find_refs.result) output_json =
  if output_json then
    ClientFindRefsPrint.print_json res
  else
    ClientFindRefsPrint.print_readable res

let go_ide (res : ServerCommandTypes.Find_refs.ide_result) output_json =
  if output_json then
    ClientFindRefsPrint.print_ide_json res
  else
    ClientFindRefsPrint.print_ide_readable res
