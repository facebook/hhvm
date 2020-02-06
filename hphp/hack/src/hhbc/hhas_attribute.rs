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

pub fn is_no_injection(attrs: &[HhasAttribute]) -> bool {
    is_native_arg(native_arg::NO_INJECTION, attrs)
}

pub fn is_native_opcode_impl(attrs: &[HhasAttribute]) -> bool {
    is_native_arg(native_arg::OP_CODE_IMPL, attrs)
}

fn is_native_arg(s: &str, attrs: &[HhasAttribute]) -> bool {
    attrs.iter().any(|attr| {
        attr.is(ua::is_native)
            && attr.arguments.iter().any(|tv| match tv {
                TypedValue::String(s0) => s0 == s,
                _ => false,
            })
    })
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
