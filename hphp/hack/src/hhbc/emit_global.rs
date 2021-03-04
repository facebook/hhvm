use env::emitter::Emitter;
use global_state::GlobalState;

pub fn set_state(e: &mut Emitter, global_state: GlobalState) {
    *e.emit_global_state_mut() = global_state;
}
