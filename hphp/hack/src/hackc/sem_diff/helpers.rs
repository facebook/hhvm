use std::fmt;

use anyhow::bail;
use anyhow::Result;
use ffi::Slice;
use ffi::Str;
use hash::HashMap;
use hash::HashSet;

use crate::code_path::CodePath;

pub(crate) trait MapName {
    fn get_name(&self) -> &str;
}

impl MapName for hhbc::hhas_adata::HhasAdata<'_> {
    fn get_name(&self) -> &str {
        self.id.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_class::HhasClass<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_constant::HhasConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_coeffects::HhasCtxConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_function::HhasFunction<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_method::HhasMethod<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_module::HhasModule<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_property::HhasProperty<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_typedef::HhasTypedef<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for hhbc::hhas_type_const::HhasTypeConstant<'_> {
    fn get_name(&self) -> &str {
        self.name.unsafe_as_str()
    }
}

impl MapName for ffi::Pair<hhbc::ClassName<'_>, hhbc::hhas_class::TraitReqKind> {
    fn get_name(&self) -> &str {
        self.0.unsafe_as_str()
    }
}

impl MapName for ffi::Pair<Str<'_>, Slice<'_, hhbc::hhas_type::HhasTypeInfo<'_>>> {
    fn get_name(&self) -> &str {
        self.0.unsafe_as_str()
    }
}

pub(crate) fn sem_diff_eq<Ta, Tb>(path: &CodePath<'_>, a: &Ta, b: &Tb) -> Result<()>
where
    Ta: PartialEq<Tb> + fmt::Debug,
    Tb: fmt::Debug,
{
    if a != b {
        bail!("Mismatch in {}:\n{:?}\n{:?}", path, a, b);
    }
    Ok(())
}

pub(crate) fn sem_diff_map_t<'a, 'b, Ta: 'a, Tb: 'b, F>(
    path: &CodePath<'_>,
    a: &'a [Ta],
    b: &'b [Tb],
    f_eq: F,
) -> Result<()>
where
    Ta: MapName,
    Tb: MapName,
    F: Fn(&CodePath<'_>, &'a Ta, &'b Tb) -> Result<()>,
{
    let a_hash: HashMap<&str, &Ta> = a.iter().map(|t| (t.get_name(), t)).collect();
    let b_hash: HashMap<&str, &Tb> = b.iter().map(|t| (t.get_name(), t)).collect();
    let a_keys: HashSet<&str> = a_hash.keys().copied().collect();
    let b_keys: HashSet<&str> = b_hash.keys().copied().collect();
    for k in &a_keys & &b_keys {
        f_eq(&path.index_str(k), a_hash[k], b_hash[k])?;
    }

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("In {} lhs has key {} but rhs does not", path, k.to_string());
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("In {} rhs has key {} but lhs does not", path, k.to_string());
    }

    Ok(())
}

#[allow(dead_code)]
pub(crate) fn sem_diff_set_t<'a, T>(path: &CodePath<'_>, a: &'a [T], b: &'a [T]) -> Result<()>
where
    T: std::hash::Hash + Eq + std::fmt::Debug,
{
    let a_keys: HashSet<&T> = a.iter().collect();
    let b_keys: HashSet<&T> = b.iter().collect();

    if let Some(k) = (&a_keys - &b_keys).into_iter().next() {
        bail!("In {} lhs has value {:?} but rhs does not", path, k);
    }

    if let Some(k) = (&b_keys - &a_keys).into_iter().next() {
        bail!("In {} rhs has value {:?} but lhs does not", path, k);
    }

    Ok(())
}

pub(crate) fn sem_diff_option<T, F>(
    path: &CodePath<'_>,
    a: Option<&T>,
    b: Option<&T>,
    f_eq: F,
) -> Result<()>
where
    T: fmt::Debug,
    F: FnOnce(&CodePath<'_>, &T, &T) -> Result<()>,
{
    match (a, b) {
        (None, None) => Ok(()),
        (Some(inner), None) => bail!("Mismatch in {}:\nSome({:?})\nNone", path, inner),
        (None, Some(inner)) => bail!("Mismatch in {}:\nNone\nSome({:?})", path, inner),
        (Some(lhs), Some(rhs)) => f_eq(&path.qualified("unwrap()"), lhs, rhs),
    }
}

pub(crate) fn sem_diff_slice<'a, V, F>(
    path: &CodePath<'_>,
    a: &'a [V],
    b: &'a [V],
    f_eq: F,
) -> Result<()>
where
    F: Fn(&CodePath<'_>, &'a V, &'a V) -> Result<()>,
{
    if a.len() > b.len() {
        bail!("Mismatch in {}: A side is longer.", path);
    }
    if b.len() > a.len() {
        bail!("Mismatch in {}: B side is longer.", path);
    }
    for (i, (av, bv)) in a.iter().zip(b.iter()).enumerate() {
        f_eq(&path.index(i as i64), av, bv)?;
    }
    Ok(())
}
