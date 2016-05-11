(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type result =
  ((string SymbolOccurrence.t) * (string SymbolDefinition.t option)) option

let go content line char tcopt =
  let symbol_occurence = ref None in
  IdentifySymbolService.attach_hooks symbol_occurence line char;
  let path = Relative_path.default in
  let (funs, classes, typedefs), ast =
    ServerIdeUtils.declare_and_check_get_ast path content in

  let result = Option.map !symbol_occurence begin fun x ->
    let symbol_definition = ServerSymbolDefinition.go tcopt ast x in
      x, symbol_definition
  end in
  ServerIdeUtils.revive funs classes typedefs path;
  IdentifySymbolService.detach_hooks ();
  result

let go_absolute content line char tcopt =
  Option.map (go content line char tcopt) begin fun (x, y) ->
    SymbolOccurrence.to_absolute x, Option.map y SymbolDefinition.to_absolute
  end
