use std::sync::Arc;

use ffi::Slice;
use hash::HashMap;

/// Builder for hhbc::Unit.adata.
pub(crate) struct AdataCache<'a> {
    alloc: &'a bumpalo::Bump,
    adata: Vec<hhbc::Adata<'a>>,
    lookup: HashMap<Arc<hhbc::TypedValue<'a>>, usize>,
}

impl<'a> AdataCache<'a> {
    pub(crate) fn new(alloc: &'a bumpalo::Bump) -> Self {
        Self {
            alloc,
            adata: Default::default(),
            lookup: Default::default(),
        }
    }

    pub(crate) fn intern(&mut self, tv: &Arc<hhbc::TypedValue<'a>>) -> hhbc::AdataId<'a> {
        let tv = Arc::clone(tv);
        let idx = self.lookup.entry(tv).or_insert_with_key(|value| {
            let idx = self.adata.len();
            let id = hhbc::AdataId::new(ffi::Str::new_str(self.alloc, &format!("A_{}", idx)));
            let value = value.as_ref().clone();
            self.adata.push(hhbc::Adata { id, value });
            idx
        });
        self.adata[*idx].id
    }

    pub(crate) fn finish(self) -> Slice<'a, hhbc::Adata<'a>> {
        Slice::fill_iter(self.alloc, self.adata.into_iter())
    }
}
