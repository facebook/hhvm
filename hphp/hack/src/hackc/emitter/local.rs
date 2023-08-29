// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc::Local;

#[derive(Debug)]
pub struct LocalGen {
    pub counter: Counter,
    pub dedicated: Dedicated,
}

impl LocalGen {
    pub fn new() -> Self {
        Self {
            counter: Counter::new(),
            dedicated: Dedicated::default(),
        }
    }

    pub fn get_unnamed(&mut self) -> Local {
        self.counter.next_unnamed(&self.dedicated)
    }

    pub fn get_unnamed_for_tempname(&self, s: &str) -> &Local {
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        self.dedicated
            .temp_map
            .get(s)
            .unwrap_or_else(|| panic!("Unnamed local {} never init'ed", s))
    }

    pub fn init_unnamed_for_tempname(&mut self, s: &str) -> &Local {
        use indexmap::map::Entry::*;
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        let new_local = self.get_unnamed();
        match self.dedicated.temp_map.map.entry(s.to_owned()) {
            Occupied(_) => panic!("Attempted to double init: {}", s),
            Vacant(e) => e.insert(new_local),
        }
    }

    pub fn get_label(&mut self) -> &Local {
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_local = new_counter.next_unnamed(&self.dedicated);
        let ret = self.dedicated.label.get_or_insert_with(|| {
            counter = new_counter;
            new_local
        });
        self.counter = counter;
        ret
    }

    pub fn get_retval(&mut self) -> &Local {
        // This and above body cannot be factored out because of nasty
        // aliasing of `&self.dedicated` and `&mut
        // self.dedicated.field`.
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_local = new_counter.next_unnamed(&self.dedicated);
        let ret = self.dedicated.retval.get_or_insert_with(|| {
            counter = new_counter;
            new_local
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

    pub fn reset(&mut self, next: Local) {
        *self = Self {
            counter: Counter { next },
            dedicated: Dedicated::default(),
        }
    }
}

#[derive(Debug, Default)]
pub struct TempMap {
    stack: Vec<usize>,
    map: indexmap::IndexMap<String, Local>,
}

impl TempMap {
    pub fn get(&self, temp: impl AsRef<str>) -> Option<&Local> {
        self.map.get(temp.as_ref())
    }

    pub fn insert(&mut self, temp: impl Into<String>, local: Local) -> Option<Local> {
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
pub struct Dedicated {
    label: Option<Local>,
    retval: Option<Local>,
    pub temp_map: TempMap,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Counter {
    pub next: Local,
}

impl Counter {
    fn new() -> Self {
        Self { next: Local::ZERO }
    }

    fn next_unnamed(&mut self, dedicated: &Dedicated) -> Local {
        loop {
            let curr = self.next;
            self.next.idx += 1;

            // make sure that newly allocated local don't stomp on dedicated locals
            match dedicated.label {
                Some(id) if curr == id => continue,
                _ => match dedicated.retval {
                    Some(id) if curr == id => continue,
                    _ => return curr,
                },
            }
        }
    }
}
