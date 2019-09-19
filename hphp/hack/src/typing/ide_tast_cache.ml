(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module IdeTastCache =
  SharedMem.FreqCache
    (StringKey)
    (struct
      type value = Tast.program * Errors.t

      let capacity = 10
    end)

let is_enabled_ref = ref false

let enable () =
  Hh_logger.log "Enabling Ide_tast_cache";
  is_enabled_ref := true

let get path content make_tast =
  if not !is_enabled_ref then
    make_tast ()
  else
    let digest = Ide_parser_cache.get_digest path content in
    match IdeTastCache.get digest with
    | Some (tast, errors) ->
      Errors.merge_into_current errors;
      tast
    | None ->
      let (errors, tast) = Errors.do_ make_tast in
      Errors.merge_into_current errors;
      IdeTastCache.add digest (tast, errors);
      tast

let invalidate = IdeTastCache.clear
