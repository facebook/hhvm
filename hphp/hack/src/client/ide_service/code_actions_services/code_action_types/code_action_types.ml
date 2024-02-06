(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type edit = {
  pos: Pos.t;
  text: string;
}

type edits = edit list Relative_path.Map.t

module Refactor = struct
  type t = {
    title: string;
    edits: edits Lazy.t;
  }

  type find =
    entry:Provider_context.entry -> Pos.t -> Provider_context.t -> t list
end

module Quickfix = struct
  type t = {
    title: string;
    edits: edits Lazy.t;
  }
end

module Type_string = struct
  type t = string Lazy.t

  (** Don't truncate types in printing unless they are really big,
     so we almost always generate valid code.
     The number is somewhat arbitrary: it's the smallest power of 2
     that could print without truncation for
     extract_shape_type_13.php in our test suite.
     We *do* want to truncate at some finite number so editors can
     handle that much text. *)
  let lots_of_typing_print_fuel = 2048

  let of_locl_ty tast_env locl_ty =
    lazy
      (let locl_ty = Tast_env.fully_expand tast_env locl_ty in
       let typing_env =
         tast_env
         |> Tast_env.tast_env_as_typing_env
         |> Typing_env.map_tcopt ~f:(fun tcopt ->
                {
                  tcopt with
                  GlobalOptions.tco_type_printer_fuel =
                    lots_of_typing_print_fuel;
                })
       in
       Typing_print.full_strip_ns typing_env locl_ty)

  let to_string = Lazy.force
end
