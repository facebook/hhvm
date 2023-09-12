use crate::CmpError;

pub(crate) trait CmpContext {
    fn with_indexed<F: FnOnce() -> String>(self, f: F) -> Self;
    fn indexed(self, idx: &str) -> Self;
    fn qualified(self, name: &str) -> Self;
    fn with_raw<F: FnOnce() -> String>(self, f: F) -> Self;
}

impl<T> CmpContext for Result<T, CmpError> {
    fn with_raw<F>(self, f: F) -> Self
    where
        F: FnOnce() -> String,
    {
        match self {
            Ok(_) => self,
            Err(CmpError { what, loc: None }) => Err(CmpError {
                what,
                loc: Some(f()),
            }),
            Err(CmpError {
                what,
                loc: Some(loc),
            }) => Err(CmpError {
                what,
                loc: Some(format!("{}{loc}", f())),
            }),
        }
    }

    fn with_indexed<F>(self, f: F) -> Self
    where
        F: FnOnce() -> String,
    {
        self.with_raw(|| format!("[\"{}\"]", f()))
    }

    fn indexed(self, idx: &str) -> Self {
        self.with_indexed(|| idx.to_string())
    }

    fn qualified(self, name: &str) -> Self {
        self.with_raw(|| format!(".{name}"))
    }
}
