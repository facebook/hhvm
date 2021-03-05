use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_global_state::GlobalState;

pub fn set_state<'arena>(
    e: &mut Emitter<'arena>,
    alloc: &'arena bumpalo::Bump,
    global_state: GlobalState,
) {
    *e.emit_global_state_mut(alloc) = global_state;
}
