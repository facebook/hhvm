// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use naming_special_names_rust::special_idents;

use indexmap::IndexMap;

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
    pub counter: Counter,
    pub dedicated: Dedicated,
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
        if self.dedicated.temp_map.insert(s, new_local).is_some() {
            panic!("Attempted to double init");
        }
        self.dedicated.temp_map.get(s).unwrap()
    }

    pub fn get_label(&mut self) -> &Type {
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.get_unnamed_id(&self.dedicated);
        let ret = self.dedicated.label.get_or_insert_with(|| {
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

    pub fn reset(&mut self, base: Id) {
        self.counter = Counter(base);
        self.dedicated = Dedicated::default();
    }
}

#[derive(Debug, Default)]
pub struct TempMap {
    stack: Vec<usize>,
    map: IndexMap<String, Type>,
}

impl TempMap {
    pub fn get(&self, temp: impl AsRef<str>) -> Option<&Type> {
        self.map.get(temp.as_ref())
    }

    pub fn insert(&mut self, temp: impl Into<String>, local: Type) -> Option<Type> {
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
    label: Option<Type>,
    retval: Option<Type>,
    pub temp_map: TempMap,
}

#[derive(Default, Clone, Copy, Debug, PartialEq, Eq)]
pub struct Counter(pub Id);

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
}
