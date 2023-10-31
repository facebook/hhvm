(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module AutocloseTag : sig
  type result = {
    xhp_open: Full_fidelity_positioned_syntax.t;
    xhp_open_right_angle: Full_fidelity_positioned_syntax.t;
    xhp_close: Full_fidelity_positioned_syntax.t;
    insert_text: string;
  }
end

val go_xhp_close_tag :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  string option
