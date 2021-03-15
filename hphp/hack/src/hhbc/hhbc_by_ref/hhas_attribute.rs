// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_runtime::TypedValue;
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;

/// Attributes with a name from [naming_special_names::user_attributes] and
/// a series of arguments.  Emitter code can match on an attribute as follows:
/// ```
///   use naming_special_names::user_attributes as ua;
///   fn is_memoized(attr: &HhasAttribute) -> bool {
///      attr.is(ua::memoized)
///   }
///   fn has_dynamically_callable(attrs: &Vec<HhasAttribute>) {
///       attrs.iter().any(|a| a.name == ua::DYNAMICALLY_CALLABLE)
///   }
/// ```

#[derive(Clone, Debug)]
pub struct HhasAttribute<'arena> {
    pub name: String,
    pub arguments: Vec<TypedValue<'arena>>,
}

impl<'arena> HhasAttribute<'arena> {
    pub fn is<F: Fn(&str) -> bool>(&self, f: F) -> bool {
        f(&self.name)
    }
}

fn is<'arena>(s: &str, attr: &HhasAttribute<'arena>) -> bool {
    attr.is(|x| x == s)
}

#[allow(clippy::needless_lifetimes)]
fn has<'arena, F>(attrs: &[HhasAttribute<'arena>], f: F) -> bool
where
    F: Fn(&HhasAttribute<'arena>) -> bool,
{
    attrs.iter().any(f)
}

pub fn is_no_injection<'arena>(attrs: impl AsRef<[HhasAttribute<'arena>]>) -> bool {
    is_native_arg(native_arg::NO_INJECTION, attrs)
}

pub fn is_native_opcode_impl<'arena>(attrs: impl AsRef<[HhasAttribute<'arena>]>) -> bool {
    is_native_arg(native_arg::OP_CODE_IMPL, attrs)
}

fn is_native_arg<'arena>(s: &str, attrs: impl AsRef<[HhasAttribute<'arena>]>) -> bool {
    attrs.as_ref().iter().any(|attr| {
        attr.is(ua::is_native)
            && attr.arguments.iter().any(|tv| match *tv {
                TypedValue::String(s0) => s0 == s,
                _ => false,
            })
    })
}

#[allow(clippy::needless_lifetimes)]
fn is_foldable<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__IsFoldable", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_dynamically_constructible<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__DynamicallyConstructible", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_sealed<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__Sealed", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_const<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__Const", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_meth_caller<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__MethCaller", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_no_context<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__NoContext", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_provenance_skip_frame<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__ProvenanceSkipFrame", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_dynamically_callable<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__DynamicallyCallable", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_native<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__Native", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_enum_class<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    is("__EnumClass", attr)
}

#[allow(clippy::needless_lifetimes)]
fn is_memoize<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    attr.name == ua::MEMOIZE || attr.name == ua::MEMOIZE_LSB
}

#[allow(clippy::needless_lifetimes)]
fn is_memoize_lsb<'arena>(attr: &HhasAttribute<'arena>) -> bool {
    attr.name == ua::MEMOIZE_LSB
}

#[allow(clippy::needless_lifetimes)]
pub fn has_native<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_native)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_enum_class<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_enum_class)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_dynamically_constructible<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_dynamically_constructible)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_foldable<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_foldable)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_sealed<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_sealed)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_const<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_const)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_meth_caller<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_meth_caller)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_no_context<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_no_context)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_provenance_skip_frame<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_provenance_skip_frame)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_dynamically_callable<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_dynamically_callable)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_is_memoize<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_memoize)
}

#[allow(clippy::needless_lifetimes)]
pub fn has_is_memoize_lsb<'arena>(attrs: &[HhasAttribute<'arena>]) -> bool {
    has(attrs, is_memoize_lsb)
}

pub fn deprecation_info<'a, 'arena>(
    mut iter: impl Iterator<Item = &'a HhasAttribute<'arena>>,
) -> Option<&'a [TypedValue<'arena>]> {
    iter.find_map(|attr| {
        if attr.name == ua::DEPRECATED {
            Some(attr.arguments.as_slice())
        } else {
            None
        }
    })
}

pub mod native_arg {
    pub const OP_CODE_IMPL: &str = "OpCodeImpl";
    pub const NO_INJECTION: &str = "NoInjection";
}

#[cfg(test)]
mod tests {
    use super::*;

    use naming_special_names::user_attributes as ua;

    #[test]
    fn example_is_memoized_vs_eq_memoize() {
        let attr = HhasAttribute {
            name: ua::MEMOIZE_LSB.to_owned(),
            arguments: vec![],
        };
        assert_eq!(true, attr.is(ua::is_memoized));
        assert_eq!(false, attr.is(|s| s == ua::MEMOIZE));
        assert_eq!(true, attr.is(|s| s == ua::MEMOIZE_LSB));
    }

    #[test]
    fn example_has_dynamically_callable() {
        let mk_attr = |name: &str| HhasAttribute {
            name: name.to_owned(),
            arguments: vec![],
        };
        let attrs = vec![mk_attr(ua::CONST), mk_attr(ua::DYNAMICALLY_CALLABLE)];
        let has_result = attrs.iter().any(|a| a.name == ua::DYNAMICALLY_CALLABLE);
        assert_eq!(true, has_result);
    }
}
