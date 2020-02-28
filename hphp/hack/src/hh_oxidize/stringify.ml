(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open Oxidized_module

let stringify m =
  let { extern_uses; glob_uses; aliases; includes; ty_reexports; decls } = m in
  let extern_uses =
    extern_uses
    |> SSet.elements
    |> List.map ~f:(sprintf "use %s;")
    |> String.concat ~sep:"\n"
  in
  let uses = "#[allow(unused_imports)]\nuse crate::*;" in
  let glob_uses =
    glob_uses
    |> SSet.elements
    |> List.map ~f:(sprintf "pub use %s::*;")
    |> String.concat ~sep:"\n"
  in
  let bound_aliases =
    aliases
    |> List.map ~f:snd
    |> List.fold ~init:SSet.empty ~f:(fun bound alias -> SSet.add bound alias)
  in
  let aliases =
    aliases
    |> List.map ~f:(fun (m, a) ->
           (* Annoyingly, we can't seem to write an alias or re-export for a
              module imported via the glob-import in `uses`. If the crate
              exports a module `map`, we and write in foo.rs:

                  use crate::*;
                  pub use map;

              Then other modules will be unable to `use foo::map;`. It is as if
              `use crate::*;` introduces an implicit private submodule named
              `map` (which is an alias to `crate::map`), and while publicly
              exporting it (via `pub use map;`) is apparently allowed, it is not
              actually visible to other modules. We must write instead:

                  use crate::*;
                  pub use crate::map;

              We cannot prefix every alias with `crate::`, however, since we
              would like to allow referencing aliases in other aliases within
              the same file. For instance, if we have this OCaml:

                  mod F = Foo
                  mod M = F.Map

              We ought to generate this Rust:

                  use crate::*;
                  pub use crate::foo as f;
                  pub use f::map as m;

              So we do not add the `crate::` prefix when referring to an alias
              bound in the same file. *)
           let root_module = String.split m ~on:':' |> List.hd_exn in
           if SSet.mem bound_aliases root_module || root_module = "crate" then
             sprintf "pub use %s as %s;" m a
           else
             sprintf "pub use crate::%s as %s;" m a)
    |> String.concat ~sep:"\n"
  in
  let includes =
    includes
    |> SSet.elements
    |> List.map ~f:(fun m -> sprintf "pub use %s::*;" m)
    |> String.concat ~sep:"\n"
  in
  let ty_reexports =
    ty_reexports
    |> List.map ~f:(sprintf "pub use %s;")
    |> List.dedup_and_sort ~compare
    |> String.concat ~sep:"\n"
  in
  let decls = decls |> List.rev_map ~f:snd |> String.concat ~sep:"\n\n" in
  sprintf
    "%s\n\n%s\n\n%s\n\n%s\n\n%s%s\n\n%s\n"
    extern_uses
    uses
    glob_uses
    aliases
    includes
    ty_reexports
    decls
