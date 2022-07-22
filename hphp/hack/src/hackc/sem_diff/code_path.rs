use std::fmt;

pub(crate) enum CodePath<'a> {
    Name(&'a str),
    Qualified(&'a CodePath<'a>, &'a str),
    IndexStr(&'a CodePath<'a>, &'a str),
    IndexN(&'a CodePath<'a>, i64),
}

impl<'a> CodePath<'a> {
    pub(crate) fn name(name: &'a str) -> Self {
        Self::Name(name)
    }

    pub(crate) fn index_str(&'a self, idx: &'a str) -> CodePath<'a> {
        Self::IndexStr(self, idx)
    }

    pub(crate) fn index(&'a self, idx: i64) -> CodePath<'a> {
        Self::IndexN(self, idx)
    }

    pub(crate) fn qualified(&'a self, name: &'a str) -> CodePath<'a> {
        Self::Qualified(self, name)
    }
}

impl fmt::Display for CodePath<'_> {
    fn fmt(&self, fmt: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CodePath::Name(name) => write!(fmt, "{}", name),
            CodePath::IndexStr(root, idx) => write!(fmt, "{}[{}]", root, idx),
            CodePath::IndexN(root, idx) => write!(fmt, "{}[{}]", root, idx),
            CodePath::Qualified(root, name) => write!(fmt, "{}.{}", root, name),
        }
    }
}
