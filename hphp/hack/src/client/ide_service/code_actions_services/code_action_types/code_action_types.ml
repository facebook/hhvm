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

type +'kind t = {
  title: string;
  edits: edits Lazy.t;
  selection: Pos.t option;
  trigger_inline_suggest: bool;
  kind: [< `Refactor | `Quickfix ] as 'kind;
}

type refactor = [ `Refactor ] t

type quickfix = [ `Quickfix ] t

type any = [ `Refactor | `Quickfix ] t

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

  let deep_strip_like_types tenv ty =
    let is_dynamic ty =
      let (_r, ty) = Typing_defs_core.deref ty in
      match ty with
      | Typing_defs_core.Tdynamic -> true
      | _ -> false
    in
    let visitor =
      object (this)
        inherit ['env] Type_mapper_generic.deep_type_mapper as super

        method! on_type env ty =
          let (reason, ty_) = Typing_defs_core.deref ty in
          let (env, reason) = this#on_reason env reason in
          match ty_ with
          | Typing_defs_core.Tunion tyl ->
            let filtered = List.filter ~f:(Fn.non is_dynamic) tyl in
            let tyl =
              if List.is_empty filtered then
                (* Avoid accidentally creating a bottom a type. Unlikely to be reached *)
                tyl
              else
                filtered
            in
            this#on_tunion env reason tyl
          | _ -> super#on_type env ty
      end
    in
    snd @@ visitor#on_type tenv ty

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
       |> deep_strip_like_types typing_env
       |> Typing_print.full_strip_ns ~hide_internals:true typing_env)

  let to_string = Lazy.force
end
