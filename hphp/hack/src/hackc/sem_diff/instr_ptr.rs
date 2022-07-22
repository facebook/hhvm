use std::fmt;

#[derive(Debug, Copy, Clone, Hash, Eq, PartialEq)]
pub(crate) enum InstrPtr {
    None,
    Index(u32),
}

impl InstrPtr {
    pub(crate) fn from_usize(n: usize) -> InstrPtr {
        InstrPtr::Index(n as u32)
    }

    pub(crate) fn as_usize(&self) -> usize {
        match *self {
            InstrPtr::None => panic!("attept to unwrap none InstrPtr"),
            InstrPtr::Index(n) => n as usize,
        }
    }

    pub(crate) fn is_none(self) -> bool {
        matches!(self, InstrPtr::None)
    }

    pub(crate) fn into_option(self) -> Option<InstrPtr> {
        match self {
            InstrPtr::None => None,
            InstrPtr::Index(_) => Some(self),
        }
    }

    pub(crate) fn next(&self, end: usize) -> InstrPtr {
        match self {
            InstrPtr::None => InstrPtr::None,
            InstrPtr::Index(mut i) => {
                i += 1;
                if i >= end as u32 {
                    InstrPtr::None
                } else {
                    InstrPtr::Index(i)
                }
            }
        }
    }
}

impl From<InstrPtr> for usize {
    fn from(ip: InstrPtr) -> Self {
        match ip {
            InstrPtr::None => usize::MAX,
            InstrPtr::Index(n) => n as usize,
        }
    }
}

impl fmt::Display for InstrPtr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            InstrPtr::None => write!(f, "none"),
            InstrPtr::Index(idx) => write!(f, "#{}", idx),
        }
    }
}
