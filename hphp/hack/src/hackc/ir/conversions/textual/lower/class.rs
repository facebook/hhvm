use std::sync::Arc;

use ir::Class;
use ir::StringInterner;
use log::trace;

use crate::lower::types::lower_ty;
use crate::lower::types::lower_ty_in_place;

pub(crate) fn lower_class<'a>(mut class: Class<'a>, strings: Arc<StringInterner>) -> Class<'a> {
    if !class.constants.is_empty() {
        textual_todo! {
            trace!("TODO: class.constants");
        }
    }

    if !class.ctx_constants.is_empty() {
        textual_todo! {
            trace!("TODO: class.ctx_constants");
        }
    }

    if let Some(ty) = class.enum_type {
        class.enum_type = Some(lower_ty(ty, &strings));
    }

    for prop in &mut class.properties {
        lower_ty_in_place(&mut prop.type_info, &strings);
    }

    if !class.type_constants.is_empty() {
        textual_todo! {
            trace!("TODO: class.type_constants");
        }
    }

    if !class.upper_bounds.is_empty() {
        textual_todo! {
            trace!("TODO: class.upper_bounds");
        }
    }

    class
}
