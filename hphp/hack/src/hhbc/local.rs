// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use naming_special_names_rust::special_idents;

use std::collections::HashMap;

/// Type of locals as they appear in instructions.
/// Named variables are those appearing in the .declvars declaration. These
/// can also be referenced by number (0 to n-1), but we use Unnamed only for
/// variables n and above not appearing in .declvars
#[derive(Debug, Clone)]
pub enum Type {
    Unnamed(Id),
    /// Named local, necessarily starting with `$`
    Named(String),
}

pub type Id = usize;

#[derive(Default, Debug)]
pub struct Gen {
    counter: Counter,
    dedicated: Dedicated,
}

impl Gen {
    pub fn get_unnamed(&mut self) -> Type {
        Type::Unnamed(self.counter.get_unnamed_id(&self.dedicated))
    }

    pub fn get_unnamed_for_tempname(&self, s: &str) -> &Type {
        special_idents::assert_tmp_var(s);
        self.dedicated
            .temp_map
            .get(s)
            .expect("Unnamed local never init'ed")
    }

    pub fn init_unnamed_for_tempname(&mut self, s: &str) -> &Type {
        special_idents::assert_tmp_var(s);
        let new_local = self.get_unnamed();
        if self
            .dedicated
            .temp_map
            .insert(s.to_owned(), new_local)
            .is_some()
        {
            panic!("Attempted to double init");
        }
        self.dedicated.temp_map.get(s).unwrap()
    }

    pub fn get_label(&mut self) -> &Type {
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.get_unnamed_id(&self.dedicated);
        let ret = self.dedicated.retval.get_or_insert_with(|| {
            counter = new_counter;
            Type::Unnamed(new_id)
        });
        self.counter = counter;
        ret
    }

    pub fn get_retval(&mut self) -> &Type {
        // this and above body cannot be factored out because of nasty
        // aliasing of &self.dedicated and &mut self.dedicated.field
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.get_unnamed_id(&self.dedicated);
        let ret = self.dedicated.retval.get_or_insert_with(|| {
            counter = new_counter;
            Type::Unnamed(new_id)
        });
        self.counter = counter;
        ret
    }

    pub fn reserve_retval_and_label_id_locals(&mut self) {
        // values are ignored because we care only about reserving them in the dedicatedal table
        self.get_label();
        self.get_retval();
    }

    pub fn scope<R, F: FnOnce() -> R>(&mut self, f: F) -> R {
        // TODO(hrust) copying HashMap may be a performance bottleneck
        // but we cannot do better without immutable collections
        let (counter, temp_map) = (self.counter, self.dedicated.temp_map.clone());
        let result = f();
        self.counter = counter;
        self.dedicated.temp_map = temp_map;
        result
    }

    pub fn store_current_state(&mut self) {
        STORED.with(|x| {
            let stored_mut: &mut Stored = &mut *x.borrow_mut();
            stored_mut.store(self.counter, self.dedicated.temp_map.clone());
        });
    }

    pub fn state_has_changed(&self) -> bool {
        STORED.with(|stored| self.counter.eq(&*stored.borrow().get_counter()))
    }

    /// Revert to the old state stored in STORED, and return the ids of newly registered
    /// unnamed locals to be unset
    pub fn revert_state(&mut self) -> Vec<Id> {
        STORED.with(|stored| {
            let stored: &Stored = &*stored.borrow();
            let old_counter = stored.get_counter().clone();
            let old_temp_map = stored.get_temp_map().clone();
            let (Counter(new_id), Counter(old_id)) = (self.counter, old_counter);
            let local_ids_to_unset = vec![old_id; new_id];
            self.counter = old_counter;
            self.dedicated.temp_map = old_temp_map;
            local_ids_to_unset
        })
    }

    pub fn reset(&mut self, base: Id) {
        self.counter = Counter(base);
        self.dedicated = Dedicated::default();
    }
}

use ::std::cell::RefCell;
thread_local! {
    static STORED: RefCell<Stored> = RefCell::new( Stored::default() );
}

#[derive(Default)]
struct Stored {
    counter: Counter,
    temp_map: HashMap<String, Type>,
}

impl Stored {
    fn store(&mut self, counter: Counter, temp_map: HashMap<String, Type>) {
        self.counter = counter;
        self.temp_map = temp_map;
    }

    fn get_counter(&self) -> &Counter {
        &self.counter
    }

    fn get_temp_map(&self) -> &HashMap<String, Type> {
        &self.temp_map
    }
}

// implementation details

#[derive(Default, Debug)]
struct Dedicated {
    label: Option<Type>,
    retval: Option<Type>,
    temp_map: HashMap<String, Type>,
}

#[derive(Default, Clone, Copy, Debug)]
struct Counter(Id);

impl Counter {
    fn get_unnamed_id(&mut self, dedicated: &Dedicated) -> Id {
        let curr = self.0;
        self.0 = curr + 1;

        // make sure that newly allocated local don't stomp on dedicated locals
        match dedicated.label {
            Some(Type::Unnamed(v)) if curr == v => self.get_unnamed_id(dedicated),
            _ => match dedicated.retval {
                Some(Type::Unnamed(v)) if curr == v => self.get_unnamed_id(dedicated),
                _ => curr,
            },
        }
    }

    fn eq(&self, other: &Self) -> bool {
        let (Counter(id1), Counter(id2)) = (self, other);
        *id1 == *id2
    }
}
