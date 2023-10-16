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
  | CSavedStateProjectMetadata of ClientEnv.client_check_env
  | CDownloadSavedState of ClientDownloadSavedState.env
  | CRage of ClientRage.env
  | CDecompressZhhdg of ClientDecompressZhhdg.env

type command_keyword =
  | CKCheck
  | CKStart
  | CKStop
  | CKRestart
  | CKNone
  | CKLsp
  | CKSavedStateProjectMetadata
  | CKDownloadSavedState
  | CKRage
  | CKDecompressZhhdg

let get_custom_telemetry_data command =
  match command with
  | CCheck { ClientEnv.custom_telemetry_data; _ }
  | CStart { ClientStart.custom_telemetry_data; _ }
  | CRestart { ClientStart.custom_telemetry_data; _ } ->
    custom_telemetry_data
  | CStop _
  | CLsp _
  | CSavedStateProjectMetadata _
  | CDownloadSavedState _
  | CRage _
  | CDecompressZhhdg _ ->
    []

let command_name = function
  | CKCheck -> "check"
  | CKStart -> "start"
  | CKStop -> "stop"
  | CKRestart -> "restart"
  | CKLsp -> "lsp"
  | CKSavedStateProjectMetadata -> "saved-state-project-metadata"
  | CKDownloadSavedState -> "download-saved-state"
  | CKRage -> "rage"
  | CKDecompressZhhdg -> "decompress-zhhdg"
  | CKNone -> ""
