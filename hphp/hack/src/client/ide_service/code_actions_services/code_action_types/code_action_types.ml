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

type edit_data = {
  title: string;
  edits: edits Lazy.t;
  selection: Pos.t option;
  trigger_inline_suggest: bool;
}

type refactor = Refactor of edit_data [@@ocaml.unboxed]

type quickfix = Quickfix of edit_data [@@ocaml.unboxed]

type t =
  | Refactor_action of edit_data
  | Quickfix_action of edit_data

type find_refactor =
  entry:Provider_context.entry -> Pos.t -> Provider_context.t -> refactor list

type find_quickfix =
  entry:Provider_context.entry ->
  Pos.t ->
  Provider_context.t ->
  error_filter:Tast_provider.ErrorFilter.t ->
  quickfix list

module Type_string = struct
  type t = string Lazy.t

  (** 'like types' are encoded as the union of a static type with [Tdynamic]
       so we can remove them by filtering out [Tdynamic] from the types contained
       in such a [Tunion]. We _shouldn't_ encounter the degenerate union cases
       [Tunion [Tdynamic]] or [Tunion [Tdynamic, ..., Tdynamic]] since they
       should have been simplified but we check for this just in case *)
  let deep_strip_like_types ty =
    let open Typing_defs_core in
    let is_not_dynamic ty =
      match get_node ty with
      | Tdynamic -> false
      | _ -> true
    in
    let id = Pass.identity () in
    let top_down =
      Pass.
        {
          id with
          on_ctor_ty__Tunion =
            Some
              (fun tys ~ctx ->
                let tys =
                  match List.filter ~f:is_not_dynamic tys with
                  | [] -> tys
                  | tys -> tys
                in
                (ctx, `Continue tys));
        }
    in
    let bottom_up = id in
    transform_ty_locl_ty ty ~ctx:() ~top_down ~bottom_up

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
      (let typing_env =
         tast_env
         |> Tast_env.tast_env_as_typing_env
         |> Typing_env.map_tcopt ~f:(fun tcopt ->
                {
                  tcopt with
                  GlobalOptions.tco_type_printer_fuel =
                    lots_of_typing_print_fuel;
                })
       in
       locl_ty
       |> Tast_env.fully_expand tast_env
       |> deep_strip_like_types
       |> Typing_print.full_strip_ns ~hide_internals:true typing_env)

  let to_string = Lazy.force
end
