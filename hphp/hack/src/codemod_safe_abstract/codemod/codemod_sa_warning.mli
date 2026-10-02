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
  warning_code: int;
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

type parse_result = {
  warnings: t list Relative_path.Map.t;
  input_diagnostics: int;
  matching_diagnostics: int;
  codemoddable_diagnostics: int;
  unique_targets: int;
}

(** Parses the raw [hh_distc --json] file incrementally. Memory use is bounded
    by one diagnostic plus the deduplicated codemod targets and one source file.
    Accepts Yojson syntax, including its extensions. Invalid syntax or diagnostic
    structure returns [Error] with the input path and a description. The entire
    document is validated before loading source files. I/O and source-position
    conversion exceptions are not caught. *)
val parse_warnings_json_file : string -> (parse_result, string) result
