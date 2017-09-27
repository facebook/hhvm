(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
    saved_root : Path.t;
    saved_hhi : Path.t;
    saved_tmp : Path.t;
    trace : bool;
    fuzzy : bool;
    profile_log : bool;
    fixme_codes : ISet.t;
  }

let save () = {
    saved_root = Path.make (Relative_path.(path_of_prefix Root));
    saved_hhi = Path.make (Relative_path.(path_of_prefix Hhi));
    saved_tmp = Path.make (Relative_path.(path_of_prefix Tmp));
    trace = !Typing_deps.trace;
    fuzzy = !HackSearchService.fuzzy;
    profile_log = !Utils.profile;
    fixme_codes = !Errors.ignored_fixme_codes;
  }

let restore state =
  Relative_path.(set_path_prefix Root state.saved_root);
  Relative_path.(set_path_prefix Hhi state.saved_hhi);
  Relative_path.(set_path_prefix Tmp state.saved_tmp);
  Typing_deps.trace := state.trace;
  HackSearchService.fuzzy := state.fuzzy;
  Utils.profile := state.profile_log;
  Errors.ignored_fixme_codes := state.fixme_codes


let get_hhi_path state = state.saved_hhi
let get_root_path state = state.saved_root

let fake_state = {
    saved_root = Path.make ".";
    saved_hhi = Path.make ".";
    saved_tmp = Path.make ".";
    trace = false;
    fuzzy = false;
    profile_log = false;
    fixme_codes = ISet.empty;
  }
