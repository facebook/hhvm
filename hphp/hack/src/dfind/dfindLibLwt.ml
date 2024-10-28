(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module MarshalToolsLwt :
  DfindLib.MARSHAL_TOOLS
    with type 'a result = 'a Lwt.t
     and type fd = Lwt_unix.file_descr = struct
  type 'a result = 'a Lwt.t

  type fd = Lwt_unix.file_descr

  let return = Lwt.return

  let ( >>= ) = Lwt.( >>= )

  let descr_of_in_channel ic =
    Lwt_unix.of_unix_file_descr
      ~blocking:false
      ~set_flags:true
      (Daemon.descr_of_in_channel ic)

  let descr_of_out_channel oc =
    Lwt_unix.of_unix_file_descr
      ~blocking:false
      ~set_flags:true
      (Daemon.descr_of_out_channel oc)

  let to_fd_with_preamble ?flags fd v =
    Marshal_tools_lwt.to_fd_with_preamble ?flags fd v

  let from_fd_with_preamble fd = Marshal_tools_lwt.from_fd_with_preamble fd
end

include DfindLib.DFindLibFunctor (MarshalToolsLwt)

let get_changes handle = get_changes handle
