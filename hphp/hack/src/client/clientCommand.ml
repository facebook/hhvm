(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type command = 
  | CCheck of ClientEnv.client_check_env
  | CStart of ClientStart.env
  | CStop of ClientStop.env
  | CRestart of ClientStart.env
  | CStatus of ClientStatus.env
  | CBuild of ClientBuild.env
  | CProlog of ClientProlog.env

type command_keyword =
  | CKCheck
  | CKStart
  | CKStop
  | CKRestart
  | CKStatus
  | CKBuild
  | CKProlog
  | CKNone
