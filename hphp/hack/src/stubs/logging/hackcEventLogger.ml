(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let init _ = ()

let init_sync _ = ()

let success ~filename:_ ~parsing_t:_ ~codegen_t:_ ~printing_t:_ ~mode:_ = ()

let fail ~filename:_ ~mode:_ ~exc:_ = ()

let log_peak_mem ~filename:_ _vm_rss _vm_hwm _action = ()
