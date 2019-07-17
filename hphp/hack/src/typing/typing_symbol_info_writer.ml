(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json
open Tast
open SymbolInfoJsonBuilder

let get_decls
  (tast: (Relative_path.t * Tast.program) list)
  : decls =
  let empty_decls = SymbolInfoJsonBuilder.default_decls in
  List.fold tast ~init:empty_decls ~f:begin fun acc (_,prog) ->
    List.fold prog ~init:acc ~f:begin fun acc2 def ->
      let {funs=acc_funs; classes=acc_clss; typedefs=acc_typedef; consts=acc_consts} = acc2 in
      match def with
      | Fun f -> { acc2 with funs = f::acc_funs }
      | Class c  -> { acc2 with classes = c::acc_clss }
      | Typedef t  -> { acc2 with typedefs = t::acc_typedef }
      | Constant g -> { acc2 with consts = g::acc_consts }
      | _ -> acc2
    end
  end

let write_json
  (tcopt: TypecheckerOptions.t)
  (tast_lst: (Relative_path.t * Tast.program) list)
  (file_dir: string)
  : unit =

  let decls = get_decls tast_lst in
  let json_chunks = SymbolInfoJsonBuilder.build_json tcopt decls in
  Sys_utils.mkdir_p file_dir;

  List.iteri json_chunks ~f:(fun idx json ->
    let filename = Printf.sprintf "%s/%s_chunk_%d.json" file_dir file_dir idx in
    let json = JSON_Array([json]) in
    let channel = Out_channel.create ~fail_if_exists:true filename in
    Out_channel.output_string channel (json_to_string ~pretty:true json);
    Out_channel.close channel
  );
