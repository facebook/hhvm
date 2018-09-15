(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Ast_defs_visitors_ancestors

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type pos = Pos.t [@visitors.opaque]
and id = pos * string
and pstring = pos * string

and shape_field_name =
  | SFlit_int of pstring
  | SFlit_str of pstring
  | SFclass_const of id * pstring

and variance =
  | Covariant
  | Contravariant
  | Invariant

and constraint_kind =
  | Constraint_as
  | Constraint_eq
  | Constraint_super

and reified = bool

and class_kind =
  | Cabstract
  | Cnormal
  | Cinterface
  | Ctrait
  | Cenum

and param_kind =
  | Pinout

and og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe

and fun_kind =
  | FSync
  | FAsync
  | FGenerator
  | FAsyncGenerator
  | FCoroutine

and bop =
| Plus
| Minus | Star | Slash | Eqeq | EQeqeq | Starstar
| Diff | Diff2 | AMpamp | BArbar | LogXor | Lt
| Lte | Gt | Gte | Dot | Amp | Bar | Ltlt
| Gtgt | Percent | Xor | Cmp
| QuestionQuestion
| Eq of bop option

and uop =
| Utild
| Unot | Uplus | Uminus | Uincr
| Udecr | Upincr | Updecr
| Uref | Usilence
[@@deriving show { with_path = false },
            visitors {
              name="iter_defs";
              variety = "iter";
              nude=true;
              visit_prefix="on_";
              ancestors=["iter_defs_base"];
            },
            visitors {
              name="endo_defs";
              variety = "endo";
              nude=true;
              visit_prefix="on_";
              ancestors=["endo_defs_base"];
            },
            visitors {
              name="reduce_defs";
              variety = "reduce";
              nude=true;
              visit_prefix="on_";
              ancestors=["reduce_defs_base"];
            },
            visitors {
              name="map_defs";
              variety = "map";
              nude=true;
              visit_prefix="on_";
              ancestors=["map_defs_base"];
            }]

(* This type is not used in the AST so no visitor is generated for it *)
type fun_decl_kind =
  | FDeclAsync
  | FDeclSync
  | FDeclCoroutine
  [@@deriving show { with_path = false }]

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let string_of_class_kind = function
  | Cabstract -> "an abstract class"
  | Cnormal -> "a class"
  | Cinterface -> "an interface"
  | Ctrait -> "a trait"
  | Cenum -> "an enum"

let string_of_param_kind = function
  | Pinout -> "inout"

module ShapeField = struct
  type t = shape_field_name
  (* We include span information in shape_field_name to improve error
   * messages, but we don't want it being used in the comparison, so
   * we have to write our own compare. *)
  let compare x y =
    match x, y with
      | SFlit_int (_, s1), SFlit_int (_, s2) -> Pervasives.compare s1 s2
      | SFlit_str (_, s1), SFlit_str (_, s2) -> Pervasives.compare s1 s2
      | SFclass_const ((_, s1), (_, s1')),
        SFclass_const ((_, s2), (_, s2')) ->
        Pervasives.compare (s1, s1') (s2, s2')
      | SFlit_int _, _ -> -1
      | SFlit_str _, SFlit_int _ -> 1
      | SFlit_str _, _ -> -1
      | SFclass_const _, _ -> 1
end

module ShapeMap = struct
  include MyMap.Make (ShapeField)

  let map_and_rekey m f1 f2 =
    fold (fun k v acc -> add (f1 k) (f2 v) acc) m empty

  let pp _ fmt _ = Format.pp_print_string fmt "[ShapeMap]"
end
