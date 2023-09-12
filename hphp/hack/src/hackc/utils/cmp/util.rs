use std::fmt;

use hash::HashMap;
use hash::HashSet;

use crate::CmpContext;
use crate::Result;

#[derive(Debug, Hash, PartialEq, Eq)]
pub struct CmpError {
    pub(crate) what: String,
    pub(crate) loc: Option<String>,
}

impl CmpError {
    pub fn error(what: String) -> Self {
        CmpError { what, loc: None }
    }
}

impl fmt::Display for CmpError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let loc = self.loc.as_ref().map_or("", String::as_str);
        write!(f, "{}: {}", loc, self.what)
    }
}

pub(crate) fn cmp_eq<Ta, Tb>(a: Ta, b: Tb) -> Result
where
    Ta: PartialEq<Tb> + fmt::Debug,
    Tb: fmt::Debug,
{
    if a != b {
        bail!("Mismatch {:?} vs {:?}", a, b);
    }
    Ok(())
}

pub(crate) trait MapName {
    fn get_name(&self) -> String;
}

impl<T: MapName> MapName for &T {
    fn get_name(&self) -> String {
        T::get_name(self)
    }
}

pub(crate) fn cmp_map_t<'a, 'b, Ta: 'a, Tb: 'b, F>(
    a: impl IntoIterator<Item = Ta>,
    b: impl IntoIterator<Item = Tb>,
    f_eq: F,
) -> Result
where
    Ta: MapName + 'a + Copy,
    Tb: MapName + 'b + Copy,
    F: Fn(Ta, Tb) -> Result,
{
    let a_hash: HashMap<String, Ta> = a.into_iter().map(|t| (t.get_name(), t)).collect();
    let b_hash: HashMap<String, Tb> = b.into_iter().map(|t| (t.get_name(), t)).collect();
    let a_keys: HashSet<&String> = a_hash.keys().collect();
    let b_keys: HashSet<&String> = b_hash.keys().collect();
    for k in &a_keys & &b_keys {
        f_eq(a_hash[k], b_hash[k]).with_indexed(|| k.to_string())?;
    }

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("lhs has key {k} but rhs does not");
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("rhs has key {k} but lhs does not");
    }

    Ok(())
}

pub(crate) fn cmp_set_t<'a, T>(a: &'a [T], b: &'a [T]) -> Result
where
    T: std::hash::Hash + Eq + std::fmt::Debug,
{
    let a_keys: HashSet<&T> = a.iter().collect();
    let b_keys: HashSet<&T> = b.iter().collect();

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("lhs has value {k:?} but rhs does not");
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("rhs has value {k:?} but lhs does not");
    }

    Ok(())
}

pub(crate) fn cmp_option<T, F>(a: Option<T>, b: Option<T>, f_eq: F) -> Result
where
    T: std::fmt::Debug + Copy,
    F: FnOnce(T, T) -> Result,
{
    match (a, b) {
        (None, None) => Ok(()),
        (Some(a), None) => bail!("Some({a:?})\nNone"),
        (None, Some(b)) => bail!("None\nSome({b:?})"),
        (Some(lhs), Some(rhs)) => f_eq(lhs, rhs),
    }
}

pub(crate) fn cmp_slice<'a, 'b, Ta, Tb, F>(
    a: impl IntoIterator<Item = Ta>,
    b: impl IntoIterator<Item = Tb>,
    f_eq: F,
) -> Result
where
    Ta: 'a + Copy,
    Tb: 'b + Copy,
    F: Fn(Ta, Tb) -> Result,
{
    let mut a = a.into_iter();
    let mut b = b.into_iter();

    let mut idx = 0;
    loop {
        match (a.next(), b.next()) {
            (None, None) => break,
            (Some(_), None) => {
                let rest = 1 + a.count();
                bail!("Length mismatch: lhs is longer ({} vs {})", idx + rest, idx);
            }
            (None, Some(_)) => {
                let rest = 1 + b.count();
                bail!("Length mismatch: rhs is longer ({} vs {})", idx, idx + rest);
            }
            (Some(av), Some(bv)) => {
                f_eq(av, bv).with_indexed(|| idx.to_string())?;
            }
        }
        idx += 1;
    }

    Ok(())
}
