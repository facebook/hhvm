use std::fmt;

use hash::HashMap;

use crate::node::Input;
use crate::node::NodeInstr;

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
pub(crate) enum Value {
    Constant(u32),
    Defined(u32),
    Undefined,
}

impl Value {
    pub(crate) fn is_undefined(&self) -> bool {
        matches!(self, Value::Undefined)
    }

    #[allow(dead_code)]
    pub(crate) fn is_constant(&self) -> bool {
        matches!(self, Value::Constant(_))
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Constant(n) => write!(f, "#{}", n),
            Value::Defined(n) => write!(f, "@{}", n),
            Value::Undefined => write!(f, "@undefined"),
        }
    }
}

#[derive(Clone)]
pub(crate) struct ValueBuilder<'arena> {
    next_value_idx: u32,
    // Store a hash to the Instruct instead of ref so that we don't run into
    // lifetime annoyances.
    values: HashMap<(u64, usize, Box<[Input<'arena>]>), Value>,
}

impl<'arena> ValueBuilder<'arena> {
    pub(crate) fn new() -> Self {
        Self {
            next_value_idx: 0,
            values: Default::default(),
        }
    }

    pub(crate) fn alloc(&mut self) -> Value {
        Self::alloc_internal(&mut self.next_value_idx, false)
    }

    pub(crate) fn compute_constant(&mut self, instr: &NodeInstr) -> Value {
        // Instruct doesn't support `==`. :(
        let hash = compute_hash(instr);
        *self
            .values
            .entry((hash, 0, [].into()))
            .or_insert_with(|| Self::alloc_internal(&mut self.next_value_idx, true))
    }

    pub(crate) fn compute_value(
        &mut self,
        instr: &NodeInstr,
        output_idx: usize,
        mut inputs: Box<[Input<'arena>]>,
    ) -> Value {
        // Instruct doesn't support `==`. :(
        let hash = compute_hash(instr);

        // Strip reffyness out of the inputs.
        inputs.iter_mut().for_each(|i| match *i {
            Input::Shared(n) | Input::Owned(n) => *i = Input::Unowned(n),
            _ => {}
        });

        *self
            .values
            .entry((hash, output_idx, inputs))
            .or_insert_with(|| Self::alloc_internal(&mut self.next_value_idx, false))
    }

    fn alloc_internal(next_value_idx: &mut u32, constant: bool) -> Value {
        let idx = *next_value_idx;
        *next_value_idx += 1;
        if constant {
            Value::Constant(idx)
        } else {
            Value::Defined(idx)
        }
    }
}

fn compute_hash<T: std::hash::Hash>(value: T) -> u64 {
    use std::hash::Hasher;
    let mut hasher = std::collections::hash_map::DefaultHasher::new();
    value.hash(&mut hasher);
    hasher.finish()
}
