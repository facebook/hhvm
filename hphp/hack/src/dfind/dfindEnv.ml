(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* The environment shared by everyone *)
(*****************************************************************************)

open Utils

module Time = struct
  type t = int

  let counter = ref 0
  let get () = incr counter; !counter
  let compare = (-)

  (* The beginning of times *)
  let bot = 0

  let to_string x = string_of_int x
end

module TimeFiles = MonoidAvl.Make(struct
  (* Timestamp + filename *)
  type elt = Time.t * string
  let compare (_, x) (_, y) = String.compare x y

  type monoelt = Time.t
  let neutral = Time.bot
  let make = fst
  let compose = max
end)

type t = {
    (* The fsnotify environment, we use this for interacting with fsnotify  *)
            fsnotify  : Fsnotify.env                          ;

    (* The set of files with their timestamp *)
    mutable files     : TimeFiles.t                           ;

    (* The set of new files (files created during an event) *)
    mutable new_files : SSet.t                                ;

    (* The directories (and the files they contain) *)
    mutable dirs      : SSet.t SMap.t                         ;

    (* The output channel where we want to log errors (/tmp/dfind_{user}.log) *)
            log       : out_channel                           ;
  }

(*****************************************************************************)
(* Building the original environment, this call is called only once
 * by the server (cf dfindServer.ml)
 *)
(*****************************************************************************)

let make root =
  let user = Sys_utils.logname in
  let log = open_out ("/tmp/dfind_"^user^".log") in
  let fsnotify = Fsnotify.init root log in
  {
    fsnotify  = fsnotify          ;
    files     = TimeFiles.empty   ;
    new_files = SSet.empty        ;
    dirs      = SMap.empty        ;
    log       = log               ;
  }
