(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = Path.t * Path.t * bool * bool * ISet.t

let save () =
  Path.make (Relative_path.(path_of_prefix Root)),
  Path.make (Relative_path.(path_of_prefix Hhi)),
  !Typing_deps.trace,
  !HackSearchService.fuzzy,
  !Errors.ignored_fixme_codes

let restore (saved_root, saved_hhi, trace, fuzzy, fixme_codes) =
  HackSearchService.fuzzy := fuzzy;
  Relative_path.(set_path_prefix Root saved_root);
  Relative_path.(set_path_prefix Hhi saved_hhi);
  Typing_deps.trace := trace;
  Errors.ignored_fixme_codes := fixme_codes

let get_hhi_path (hhi_path, _, _, _, _) = hhi_path
let get_root_path (_, root_path, _, _, _) = root_path

let fake_state = Path.make ".", Path.make ".", false, false, ISet.empty
