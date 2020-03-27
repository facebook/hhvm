// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust as naming_special_names;
use runtime::TypedValue;

use naming_special_names::user_attributes as ua;

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
pub struct HhasAttribute {
    pub name: String,
    pub arguments: Vec<TypedValue>,
}

impl HhasAttribute {
    pub fn is<F: Fn(&str) -> bool>(&self, f: F) -> bool {
        f(&self.name)
    }
}

fn is(s: &str, attr: &HhasAttribute) -> bool {
    attr.is(|x| x == s)
}

fn has<F>(attrs: &[HhasAttribute], f: F) -> bool
where
    F: Fn(&HhasAttribute) -> bool,
{
    attrs.iter().any(f)
}

pub fn is_no_injection(attrs: impl AsRef<[HhasAttribute]>) -> bool {
    is_native_arg(native_arg::NO_INJECTION, attrs)
}

pub fn is_native_opcode_impl(attrs: impl AsRef<[HhasAttribute]>) -> bool {
    is_native_arg(native_arg::OP_CODE_IMPL, attrs)
}

fn is_native_arg(s: &str, attrs: impl AsRef<[HhasAttribute]>) -> bool {
    attrs.as_ref().iter().any(|attr| {
        attr.is(ua::is_native)
            && attr.arguments.iter().any(|tv| match tv {
                TypedValue::String(s0) => s0 == s,
                _ => false,
            })
    })
}

fn is_foldable(attr: &HhasAttribute) -> bool {
    is("__IsFoldable", attr)
}

fn is_dynamically_constructible(attr: &HhasAttribute) -> bool {
    is("__DynamicallyConstructible", attr)
}

fn is_sealed(attr: &HhasAttribute) -> bool {
    is("__Sealed", attr)
}

fn is_const(attr: &HhasAttribute) -> bool {
    is("__Const", attr)
}

fn is_meth_caller(attr: &HhasAttribute) -> bool {
    is("__MethCaller", attr)
}

fn is_provenance_skip_frame(attr: &HhasAttribute) -> bool {
    is("__ProvenanceSkipFrame", attr)
}

fn is_dynamically_callable(attr: &HhasAttribute) -> bool {
    is("__DynamicallyCallable", attr)
}

fn is_native(attr: &HhasAttribute) -> bool {
    is("__Native", attr)
}

fn is_memoize(attr: &HhasAttribute) -> bool {
    &attr.name == ua::MEMOIZE || &attr.name == ua::MEMOIZE_LSB
}

fn is_memoize_lsb(attr: &HhasAttribute) -> bool {
    &attr.name == ua::MEMOIZE_LSB
}

pub fn has_native(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_native)
}

pub fn has_dynamically_constructible(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_dynamically_constructible)
}

pub fn has_foldable(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_foldable)
}

pub fn has_sealed(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_sealed)
}

pub fn has_const(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_const)
}

pub fn has_meth_caller(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_meth_caller)
}

pub fn has_provenance_skip_frame(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_provenance_skip_frame)
}

pub fn has_dynamically_callable(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_dynamically_callable)
}

pub fn has_is_memoize(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_memoize)
}

pub fn has_is_memoize_lsb(attrs: &[HhasAttribute]) -> bool {
    has(attrs, is_memoize_lsb)
}

pub fn deprecation_info<'a>(
    mut iter: impl Iterator<Item = &'a HhasAttribute>,
) -> Option<&'a [TypedValue]> {
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
