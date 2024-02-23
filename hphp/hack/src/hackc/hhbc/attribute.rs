// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Str;
use ffi::Vector;
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;
use serde::Serialize;

use crate::typed_value::TypedValue;

/// Attributes with a name from [naming_special_names::user_attributes] and
/// a series of arguments.  Emitter code can match on an attribute as follows:
/// ```
/// use naming_special_names::user_attributes as ua;
/// fn is_memoized(attr: &Attribute) -> bool {
///     attr.is(ua::memoized)
/// }
/// fn has_dynamically_callable(attrs: &Vec<Attribute>) {
///     attrs.iter().any(|a| a.name == ua::DYNAMICALLY_CALLABLE)
/// }
/// ```

#[derive(Clone, Debug, Eq, PartialEq, Hash, Serialize)]
#[repr(C)]
pub struct Attribute<'arena> {
    pub name: Str<'arena>,
    pub arguments: Vector<TypedValue>,
}

impl<'arena> Attribute<'arena> {
    pub fn is<F: Fn(&str) -> bool>(&self, f: F) -> bool {
        f(self.name.unsafe_as_str())
    }
}

fn is<'arena>(s: &str, attr: &Attribute<'arena>) -> bool {
    attr.is(|x| x == s)
}

fn has<'arena, F>(attrs: &[Attribute<'arena>], f: F) -> bool
where
    F: Fn(&Attribute<'arena>) -> bool,
{
    attrs.iter().any(f)
}

pub fn is_no_injection<'arena>(attrs: impl AsRef<[Attribute<'arena>]>) -> bool {
    is_native_arg(native_arg::NO_INJECTION, attrs)
}

pub fn is_native_opcode_impl<'arena>(attrs: impl AsRef<[Attribute<'arena>]>) -> bool {
    is_native_arg(native_arg::OP_CODE_IMPL, attrs)
}

fn is_native_arg<'arena>(s: &str, attrs: impl AsRef<[Attribute<'arena>]>) -> bool {
    attrs.as_ref().iter().any(|attr| {
        attr.is(ua::is_native)
            && attr.arguments.as_ref().iter().any(|tv| match *tv {
                TypedValue::String(s0) => s0.as_bytes() == s.as_bytes(),
                _ => false,
            })
    })
}

fn is_memoize_with<'arena>(attrs: impl AsRef<[Attribute<'arena>]>, arg: &str) -> bool {
    attrs.as_ref().iter().any(|attr| {
        ua::is_memoized(attr.name.unsafe_as_str())
            && attr.arguments.as_ref().iter().any(|tv| match *tv {
                TypedValue::String(s0) => s0.as_bytes() == arg.as_bytes(),
                _ => false,
            })
    })
}

pub fn is_keyed_by_ic_memoize<'arena>(attrs: impl AsRef<[Attribute<'arena>]>) -> bool {
    is_memoize_with(attrs, "KeyedByIC")
}

pub fn is_make_ic_inaccessible_memoize<'arena>(attrs: impl AsRef<[Attribute<'arena>]>) -> bool {
    is_memoize_with(attrs, "MakeICInaccessible")
}

pub fn is_soft_make_ic_inaccessible_memoize<'arena>(
    attrs: impl AsRef<[Attribute<'arena>]>,
) -> bool {
    is_memoize_with(attrs, "SoftMakeICInaccessible")
}

fn is_foldable<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::IS_FOLDABLE, attr)
}

fn is_dynamically_constructible<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::DYNAMICALLY_CONSTRUCTIBLE, attr)
}

fn is_dynamically_referenced<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::DYNAMICALLY_REFERENCED, attr)
}

fn is_sealed<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::SEALED, attr)
}

fn is_const<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::CONST, attr)
}

fn is_meth_caller<'arena>(attr: &Attribute<'arena>) -> bool {
    is("__MethCaller", attr)
}

fn is_provenance_skip_frame<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::PROVENANCE_SKIP_FRAME, attr)
}

fn is_dynamically_callable<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::DYNAMICALLY_CALLABLE, attr)
}

fn is_enum_class<'arena>(attr: &Attribute<'arena>) -> bool {
    is(ua::ENUM_CLASS, attr)
}

pub fn has_enum_class<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_enum_class)
}

pub fn has_dynamically_constructible<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_dynamically_constructible)
}

pub fn has_dynamically_referenced<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_dynamically_referenced)
}

pub fn has_foldable<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_foldable)
}

pub fn has_sealed<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_sealed)
}

pub fn has_const<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_const)
}

pub fn has_meth_caller<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_meth_caller)
}

pub fn has_provenance_skip_frame<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_provenance_skip_frame)
}

pub fn has_dynamically_callable<'arena>(attrs: &[Attribute<'arena>]) -> bool {
    has(attrs, is_dynamically_callable)
}

pub fn deprecation_info<'arena>(
    mut iter: impl Iterator<Item = &'arena Attribute<'arena>>,
) -> Option<&'arena [TypedValue]> {
    iter.find_map(|attr| {
        if attr.name.unsafe_as_str() == ua::DEPRECATED {
            Some(attr.arguments.as_ref())
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
    use naming_special_names::user_attributes as ua;

    use super::*;

    #[test]
    fn example_is_memoized_vs_eq_memoize() {
        let attr = Attribute {
            name: ua::MEMOIZE_LSB.into(),
            arguments: vec![].into(),
        };
        assert!(attr.is(ua::is_memoized));
        assert!(!attr.is(|s| s == ua::MEMOIZE));
        assert!(attr.is(|s| s == ua::MEMOIZE_LSB));
    }

    #[test]
    fn example_has_dynamically_callable() {
        let mk_attr = |name: &'static str| Attribute {
            name: name.into(),
            arguments: vec![].into(),
        };
        let attrs = vec![mk_attr(ua::CONST), mk_attr(ua::DYNAMICALLY_CALLABLE)];
        let has_result = attrs
            .iter()
            .any(|a| a.name.unsafe_as_str() == ua::DYNAMICALLY_CALLABLE);
        assert!(has_result);
    }
}
