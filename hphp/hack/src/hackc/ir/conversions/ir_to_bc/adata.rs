use std::sync::Arc;

use hash::HashMap;

use crate::strings::StringCache;

/// Builder for hhbc::Unit.adata.
pub(crate) struct AdataCache {
    adata: Vec<hhbc::Adata>,
    lookup: HashMap<Arc<ir::TypedValue>, usize>,
}

impl AdataCache {
    pub(crate) fn new() -> Self {
        Self {
            adata: Default::default(),
            lookup: Default::default(),
        }
    }

    pub(crate) fn intern(
        &mut self,
        tv: Arc<ir::TypedValue>,
        strings: &StringCache<'_>,
    ) -> hhbc::AdataId {
        let idx = self.lookup.entry(tv).or_insert_with_key(|tv| {
            let idx = self.adata.len();
            let id = hhbc::AdataId::new(idx);
            let value = crate::convert::convert_typed_value(tv, strings);
            self.adata.push(hhbc::Adata { id, value });
            idx
        });
        self.adata[*idx].id
    }

    pub(crate) fn finish(self) -> Vec<hhbc::Adata> {
        self.adata
    }
}
