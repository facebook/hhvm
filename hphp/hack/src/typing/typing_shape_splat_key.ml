(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs_core

type t = locl_ty

let transparent_marker = Typing_make_type.int Typing_reason.none

let module_opaque_marker = Typing_make_type.bool Typing_reason.none

class mapper =
  object (this)
    inherit [unit] Type_mapper_generic.deep_type_mapper

    method! on_tnewtype () reason name args _ =
      let ((), args) = this#on_locl_ty_list () args in
      let opacity =
        if Typing_reason.Predicates.is_opaque_type_from_module reason then
          module_opaque_marker
        else
          transparent_marker
      in
      ((), mk (reason, Tnewtype (name, args, opacity)))
  end

let mapper = new mapper

let of_ty ty = snd (mapper#on_type () ty)

let compare = Typing_defs.compare_locl_ty ?normalize_lists:None

module Map = Map.Make (struct
  type nonrec t = t

  let compare = compare
end)
