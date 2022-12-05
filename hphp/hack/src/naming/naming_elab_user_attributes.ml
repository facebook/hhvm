(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error

module Env = struct
  type t = unit

  let empty = ()
end

let visitor =
  object (self)
    inherit [_] Naming_visitors.mapreduce as super

    method! on_user_attributes env us =
      let seen = Caml.Hashtbl.create 0 in
      let dedup (attrs, err) (Aast.{ ua_name = (pos, attr_name); _ } as attr) =
        match Caml.Hashtbl.find_opt seen attr_name with
        | Some prev_pos ->
          ( attrs,
            self#plus err
            @@ Err.naming
            @@ Naming_error.Duplicate_user_attribute
                 { pos; prev_pos; attr_name } )
        | _ ->
          Caml.Hashtbl.add seen attr_name pos;
          let (attr, attr_err) = super#on_user_attribute env attr in
          (attr :: attrs, self#plus err attr_err)
      in
      Tuple2.map_fst ~f:List.rev
      @@ List.fold_left us ~init:([], self#zero) ~f:dedup
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
