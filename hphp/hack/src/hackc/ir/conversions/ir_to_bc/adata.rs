use std::sync::Arc;

use hash::HashMap;

use crate::strings::StringCache;

/// Builder for hhbc::Unit.adata.
pub(crate) struct AdataCache<'a> {
    alloc: &'a bumpalo::Bump,
    adata: Vec<hhbc::Adata<'a>>,
    lookup: HashMap<Arc<ir::TypedValue>, usize>,
}

impl<'a> AdataCache<'a> {
    pub(crate) fn new(alloc: &'a bumpalo::Bump) -> Self {
        Self {
            alloc,
            adata: Default::default(),
            lookup: Default::default(),
        }
    }

    pub(crate) fn intern(
        &mut self,
        tv: Arc<ir::TypedValue>,
        strings: &StringCache<'a>,
    ) -> hhbc::AdataId<'a> {
        let idx = self.lookup.entry(tv).or_insert_with_key(|tv| {
            let idx = self.adata.len();
            let id = hhbc::AdataId::new(ffi::Str::new_str(self.alloc, &format!("A_{}", idx)));
            let value = crate::convert::convert_typed_value(tv, strings);
            self.adata.push(hhbc::Adata { id, value });
            idx
        });
        self.adata[*idx].id
    }

    pub(crate) fn finish(self) -> Vec<hhbc::Adata<'a>> {
        self.adata
    }
}
