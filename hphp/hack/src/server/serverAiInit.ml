(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerEnv

let ai_check
    (genv : ServerEnv.genv)
    (_files_info : Naming_table.t)
    (env : ServerEnv.env)
    (t : float) : ServerEnv.env * float =
  match ServerArgs.ai_mode genv.options with
  | Some ai_opt when ai_opt.Ai_options.analyses <> [] ->
    Hh_logger.log "--ai only works with the zonk binary";
    exit (-1)
  | _ -> (env, t)
