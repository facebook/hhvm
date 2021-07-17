// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;

pub type Id = usize;

/// Type of locals as they appear in instructions. Named variables are
/// those appearing in the .declvars declaration. These can also be
/// referenced by number (0 to n-1), but we use Unnamed only for
/// variables n and above not appearing in .declvars
#[derive(Copy, Debug)]
#[repr(C)]
pub enum Local<'arena> {
    Unnamed(Id),
    /// Named local, necessarily starting with `$`
    Named(Str<'arena>),
}
impl<'arena> Clone for Local<'arena> {
    fn clone(&self) -> Local<'arena> {
        match self {
            Local::Unnamed(u) => Local::Unnamed(u.clone()),
            Local::Named(r) => Local::Named(r.clone()),
        }
    }
}

#[derive(Default, Debug)]
pub struct Gen<'arena> {
    pub counter: Counter,
    pub dedicated: Dedicated<'arena>,
}

impl<'arena> Gen<'arena> {
    pub fn get_unnamed(&mut self) -> Local<'arena> {
        Local::Unnamed(self.counter.get_unnamed_id(&self.dedicated))
    }

    pub fn get_unnamed_for_tempname(&self, s: &str) -> &Local<'arena> {
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        self.dedicated
            .temp_map
            .get(s)
            .expect("Unnamed local never init'ed")
    }

    pub fn init_unnamed_for_tempname(&mut self, s: &str) -> &Local<'arena> {
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        let new_local = self.get_unnamed();
        if self.dedicated.temp_map.insert(s, new_local).is_some() {
            panic!("Attempted to double init");
        }
        self.dedicated.temp_map.get(s).unwrap()
    }

    pub fn get_label(&mut self) -> &Local<'arena> {
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.get_unnamed_id(&self.dedicated);
        let ret = self.dedicated.label.get_or_insert_with(|| {
            counter = new_counter;
            Local::Unnamed(new_id)
        });
        self.counter = counter;
        ret
    }

    pub fn get_retval(&mut self) -> &Local<'arena> {
        // This and above body cannot be factored out because of nasty
        // aliasing of `&self.dedicated` and `&mut
        // self.dedicated.field`.
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.get_unnamed_id(&self.dedicated);
        let ret = self.dedicated.retval.get_or_insert_with(|| {
            counter = new_counter;
            Local::Unnamed(new_id)
        });
        self.counter = counter;
        ret
    }

    pub fn reserve_retval_and_label_id_locals(&mut self) {
        // Values are ignored because we care only about reserving
        // them in the dedicated table.
        self.get_label();
        self.get_retval();
    }

    pub fn reset(&mut self, base: Id) {
        self.counter = Counter(base);
        self.dedicated = Dedicated::default();
    }
}

#[derive(Debug, Default)]
pub struct TempMap<'arena> {
    stack: std::vec::Vec<usize>,
    map: indexmap::IndexMap<String, Local<'arena>>,
}

impl<'arena> TempMap<'arena> {
    pub fn get(&self, temp: impl AsRef<str>) -> Option<&Local<'arena>> {
        self.map.get(temp.as_ref())
    }

    pub fn insert(
        &mut self,
        temp: impl Into<String>,
        local: Local<'arena>,
    ) -> Option<Local<'arena>> {
        self.map.insert(temp.into(), local)
    }

    pub fn push(&mut self) {
        self.stack.push(self.map.len())
    }

    pub fn pop(&mut self) {
        if let Some(j) = self.stack.pop() {
            while self.map.len() > j {
                self.map.pop();
            }
        }
    }
}
// implementation details

#[derive(Default, Debug)]
pub struct Dedicated<'arena> {
    label: Option<Local<'arena>>,
    retval: Option<Local<'arena>>,
    pub temp_map: TempMap<'arena>,
}

#[derive(Default, Clone, Copy, Debug, PartialEq, Eq)]
pub struct Counter(pub Id);

impl Counter {
    fn get_unnamed_id<'arena>(&mut self, dedicated: &Dedicated<'arena>) -> Id {
        let curr = self.0;
        self.0 = curr + 1;

        // make sure that newly allocated local don't stomp on dedicated locals
        match (*dedicated).label {
            Some(Local::Unnamed(v)) if curr == v => self.get_unnamed_id(dedicated),
            _ => match dedicated.retval {
                Some(Local::Unnamed(v)) if curr == v => self.get_unnamed_id(dedicated),
                _ => curr,
            },
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhbc_local<'a, 'arena>(_: Local<'arena>) {}
