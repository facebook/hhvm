(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Data we get from running `hh --json --config needs_concrete=true`,
 * munged to be conducive to codemodding-away warnings.
 *)
type t = {
  warning_code: Error_codes.Warning.t;
  pos: Pos.t;
      (* The position corresponding to the code that
         needs to change.
          For example:

          class C1 {
            // We remember the position on the next line
            public static function m(): void {}
          }
          class C2 extends C1 {
            // The primary error location is here,
            // since a __NeedsConcrete method can't override
            // a non-__NeedsConcrete method. However, the fix is
            // to update the *overridden* method (`C1::m`)
            <<__NeedsConcrete>>
            public static function m(): void {}
          }
      *)
}

val parse_warnings_json_exn :
  Yojson.Safe.t -> error_message:string -> t list Relative_path.Map.t
