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
  | CLsp of ClientLsp.env
  | CDebug of ClientDebug.env
  | CDownloadSavedState of ClientDownloadSavedState.env

type command_keyword =
  | CKCheck
  | CKStart
  | CKStop
  | CKRestart
  | CKNone
  | CKLsp
  | CKDebug
  | CKDownloadSavedState
