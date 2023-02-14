use std::sync::Arc;

use ir::Class;
use ir::StringInterner;
use log::trace;

pub(crate) fn lower_class<'a>(class: Class<'a>, _strings: Arc<StringInterner>) -> Class<'a> {
    if !class.ctx_constants.is_empty() {
        textual_todo! {
            trace!("TODO: class.ctx_constants");
        }
    }

    if !class.upper_bounds.is_empty() {
        textual_todo! {
            trace!("TODO: class.upper_bounds");
        }
    }

    class
}
