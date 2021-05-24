(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type command =
  | CCheck of ClientEnv.client_check_env
  | CStart of ClientStart.env
  | CStop of ClientStop.env
  | CRestart of ClientStart.env
  | CLsp of ClientLsp.args
  | CDebug of ClientDebug.env
  | CDownloadSavedState of ClientDownloadSavedState.env
  | CRage of ClientRage.env

type command_keyword =
  | CKCheck
  | CKStart
  | CKStop
  | CKRestart
  | CKNone
  | CKLsp
  | CKDebug
  | CKDownloadSavedState
  | CKRage

let get_custom_telemetry_data command =
  match command with
  | CCheck { ClientEnv.custom_telemetry_data; _ } -> custom_telemetry_data
  | CStart { ClientStart.custom_telemetry_data; _ }
  | CRestart { ClientStart.custom_telemetry_data; _ } ->
    custom_telemetry_data
  | _ -> []
