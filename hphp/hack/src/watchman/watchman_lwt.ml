(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

include Watchman.Process (struct
  type 'a future = 'a Lwt.t

  type error = Lwt_utils.Process_failure.t

  let error_to_string = Lwt_utils.Process_failure.to_string

  module Monad_infix = struct
    let ( >>| ) a f = Lwt.map f a

    let ( >|= ) a f = Lwt.map (Result.bind ~f) a
  end

  let exec cmd args =
    let open Monad_infix in
    Lwt_utils.exec_checked cmd (Array.of_list args)
    >>| Result.map ~f:(fun { Lwt_utils.Process_success.stdout; _ } -> stdout)
end)
