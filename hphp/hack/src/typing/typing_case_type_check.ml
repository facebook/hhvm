(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Tag = Typing_data_type.Tag
module TagWithReason = Typing_data_type.TagWithReason
module DataType = Typing_data_type.DataType

type runtime_data_type = decl_ty * DataType.t

let data_type_from_hint = DataType.fromHint

let check_overlapping
    env
    ~pos
    ~name
    (ty1, (data_type1 : DataType.t))
    (ty2, (data_type2 : DataType.t)) =
  let open DataType in
  match Set.disjoint env data_type1 data_type2 with
  | Set.Sat -> None
  | Set.Unsat { left; relation; right } ->
    let why
        ((ty1, { TagWithReason.tag = tag1; _ }) as left)
        relation
        ((ty2, { TagWithReason.tag = tag2; _ }) as right) =
      let primary_why ~f =
        TagWithReason.to_message
          env
          left
          ~f:(Printf.sprintf "This is the type `%s`, which includes ")
        @ TagWithReason.to_message env right ~f
      in
      let secondary_why ~f =
        let describe tag = Markdown_lite.md_bold @@ Tag.describe env tag in
        let ty_str ty =
          Markdown_lite.md_codify
          @@ Typing_print.full_strip_ns_decl ~verbose_fun:false env ty
        in
        [
          ( Pos_or_decl.of_raw_pos pos,
            f (describe tag1) (describe tag2)
            ^ Printf.sprintf
                ", %s and %s cannot be in the same case type"
                (ty_str ty1)
                (ty_str ty2) );
        ]
      in
      if SetRelation.is_equivalent relation then
        primary_why
          ~f:(Printf.sprintf "It overlaps with `%s`, which also includes ")
      else if SetRelation.is_superset relation then
        primary_why ~f:(Printf.sprintf "It overlaps with `%s`, which includes ")
        @ secondary_why ~f:(Printf.sprintf "Because %s contains %s")
      else if SetRelation.is_subset relation then
        primary_why ~f:(Printf.sprintf "It overlaps with `%s`, which includes ")
        @ secondary_why ~f:(Printf.sprintf "Because %s are also %s")
      else
        primary_why
          ~f:(Printf.sprintf "It may overlap with `%s`, which includes ")
        @ secondary_why
            ~f:
              (Printf.sprintf
                 "Because it is possible for values to be both %s and %s")
    in

    let err =
      Typing_error.Primary.CaseType.Overlapping_variant_types
        { pos; name; why = lazy (why (ty1, left) relation (ty2, right)) }
    in
    Some err
