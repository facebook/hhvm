// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Local variable numbers are ultimately encoded as IVA, limited to u32.
#[derive(Copy, Clone, Debug, Default, Eq, PartialEq)]
#[repr(C)]
pub struct LocalId {
    /// 0-based index into HHBC stack frame locals.
    pub idx: u32,
}

impl std::fmt::Display for LocalId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        self.idx.fmt(f)
    }
}

impl LocalId {
    pub fn from_usize(x: usize) -> Self {
        Self { idx: x as u32 }
    }
}

/// Type of locals as they appear in instructions. Named variables are
/// those appearing in parameters and the HHAS .declvars directive.
/// These can also be referenced by number [0 to N-1]. Use Unnamed
/// only for variables N and above not appearing in parameters or .declvars.
#[derive(Clone, Copy, Debug)]
#[repr(C)]
pub enum Local {
    Unnamed(LocalId),
    Named(LocalId),
}

impl Local {
    pub const INVALID: Self = Self::Unnamed(LocalId { idx: u32::MAX });

    pub fn is_valid(&self) -> bool {
        !matches!(self, Self::Unnamed(LocalId { idx: u32::MAX }))
    }

    pub fn expect_unnamed(&self) -> LocalId {
        match self {
            Self::Unnamed(id) => *id,
            Self::Named(_) => panic!("Expected unnamed local"),
        }
    }

    pub fn expect_named(&self) -> LocalId {
        match self {
            Self::Named(id) => *id,
            Self::Unnamed(_) => panic!("Expected named local"),
        }
    }

    pub fn named(i: usize) -> Self {
        Self::Named(LocalId { idx: i as u32 })
    }

    pub fn unnamed(i: usize) -> Self {
        Self::Unnamed(LocalId { idx: i as u32 })
    }
}

#[derive(Default, Debug)]
pub struct Gen {
    pub counter: Counter,
    pub dedicated: Dedicated,
}

impl Gen {
    pub fn get_unnamed(&mut self) -> Local {
        Local::Unnamed(self.counter.next_unnamed(&self.dedicated))
    }

    pub fn get_unnamed_for_tempname(&self, s: &str) -> &Local {
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        self.dedicated
            .temp_map
            .get(s)
            .expect("Unnamed local never init'ed")
    }

    pub fn init_unnamed_for_tempname(&mut self, s: &str) -> &Local {
        naming_special_names_rust::special_idents::assert_tmp_var(s);
        let new_local = self.get_unnamed();
        if self.dedicated.temp_map.insert(s, new_local).is_some() {
            panic!("Attempted to double init");
        }
        self.dedicated.temp_map.get(s).unwrap()
    }

    pub fn get_label(&mut self) -> &Local {
        let mut counter = self.counter;
        let mut new_counter = self.counter;
        let new_id = new_counter.next_unnamed(&self.dedicated);
        let ret = self.dedicated.label.get_or_insert_with(|| {
            counter = new_counter;
            Local::Unnamed(new_id)
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
        let new_id = new_counter.next_unnamed(&self.dedicated);
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

    pub fn reset(&mut self, next: LocalId) {
        *self = Self {
            counter: Counter { next },
            ..Default::default()
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

#[derive(Default, Clone, Copy, Debug, PartialEq, Eq)]
pub struct Counter {
    pub next: LocalId,
}

impl Counter {
    fn next_unnamed(&mut self, dedicated: &Dedicated) -> LocalId {
        loop {
            let curr = self.next;
            self.next.idx += 1;

            // make sure that newly allocated local don't stomp on dedicated locals
            match dedicated.label {
                Some(Local::Unnamed(id)) if curr == id => continue,
                _ => match dedicated.retval {
                    Some(Local::Unnamed(id)) if curr == id => continue,
                    _ => return curr,
                },
            }
        }
    }
}
