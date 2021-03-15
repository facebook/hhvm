// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_global_state::GlobalState;

pub fn set_state<'a, 'arena: 'a>(
    alloc: &'arena bumpalo::Bump,
    e: &'a mut Emitter<'arena>,
    global_state: GlobalState,
) {
    *e.emit_global_state_mut(alloc) = global_state;
}
