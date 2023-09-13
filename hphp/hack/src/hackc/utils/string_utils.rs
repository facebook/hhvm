// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;
use std::cell::Cell;

use bstr::BStr;
use escaper::*;
use lazy_static::lazy_static;
use naming_special_names_rust::classes as ns_classes;
use naming_special_names_rust::members;
use regex::Regex;

lazy_static! {
    static ref HH_NS_RE: Regex = Regex::new(r"^\\?HH\\").unwrap();
    static ref NS_RE: Regex = Regex::new(r".*\\").unwrap();
    static ref TYPE_RE: Regex = Regex::new(r"<.*>").unwrap();
}

#[derive(Clone)]
pub struct GetName {
    string: Vec<u8>,

    unescape: fn(String) -> String,
}

impl GetName {
    pub fn new(string: Vec<u8>, unescape: fn(String) -> String) -> GetName {
        GetName { string, unescape }
    }

    pub fn get(&self) -> &Vec<u8> {
        &self.string
    }
    #[allow(clippy::inherent_to_string)]
    pub fn to_string(&self) -> String {
        String::from_utf8_lossy(&self.string).to_string()
    }
    pub fn to_unescaped_string(&self) -> String {
        let unescape = self.unescape;
        unescape(self.to_string())
    }
}

impl std::fmt::Debug for GetName {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "GetName {{ string: {}, unescape:? }}", self.to_string())
    }
}

thread_local!(static MANGLE_XHP_MODE: Cell<bool> = Cell::new(true));

pub fn without_xhp_mangling<T>(f: impl FnOnce() -> T) -> T {
    MANGLE_XHP_MODE.with(|cur| {
        let old = cur.replace(false);
        let ret = f();
        cur.set(old); // use old instead of true to support nested calls in the same thread
        ret
    })
}

pub fn is_xhp(name: &str) -> bool {
    name.chars().next().map_or(false, |c| c == ':')
}

pub fn clean(s: &str) -> &str {
    if is_xhp(s) { strip_colon(s) } else { s }
}

fn strip_colon(s: &str) -> &str {
    s.trim_start_matches(':')
}

fn ignore_id(name: &str) -> bool {
    name.starts_with("Closure$")
}

pub fn mangle_xhp_id(mut name: String) -> String {
    if !ignore_id(&name) && MANGLE_XHP_MODE.with(|x| x.get()) {
        if is_xhp(&name) {
            name.replace_range(..1, "xhp_")
        }
        name.replace(':', "__").replace('-', "_")
    } else {
        name
    }
}

fn unmangle_xhp_id(name: &str) -> String {
    if name.starts_with("xhp_") {
        format!(
            ":{}",
            lstrip(name, "xhp_").replace("__", ":").replace('_', "-")
        )
    } else {
        name.replace("__", ":").replace('_', "-")
    }
}

pub fn mangle(mut name: String) -> String {
    if !ignore_id(&name) {
        if let Some(pos) = name.rfind('\\') {
            name.replace_range(pos + 1.., &mangle_xhp_id(name[pos + 1..].to_string()))
        } else {
            name = mangle_xhp_id(name);
        }
    }
    name
}

pub fn unmangle(name: String) -> String {
    if ignore_id(&name) {
        return name;
    }
    let ids = name.split('\\').collect::<Vec<_>>();
    match ids.split_last() {
        None => String::new(),
        Some((last, rest)) => {
            if rest.is_empty() {
                unmangle_xhp_id(last)
            } else {
                format!("{}\\{}", rest.join("\\"), unmangle_xhp_id(last))
            }
        }
    }
}

pub fn quote_string(s: &str) -> String {
    format!("\"{}\"", escape(s))
}

pub fn prefix_namespace(n: &str, s: &str) -> String {
    format!("{}\\{}", n, s)
}

pub fn strip_global_ns(s: &str) -> &str {
    if let Some(rest) = s.strip_prefix('\\') {
        rest
    } else {
        s
    }
}

pub fn strip_global_ns_bslice(s: &[u8]) -> &[u8] {
    s.strip_prefix(b"\\").unwrap_or(s)
}

pub fn strip_global_ns_bstr(s: &BStr) -> &BStr {
    s.strip_prefix(b"\\").unwrap_or(s).into()
}

// Strip zero or more chars followed by a backslash
pub fn strip_ns(s: &str) -> &str {
    s.rfind('\\').map_or(s, |i| &s[i + 1..])
}

// Remove \HH\ or HH\ preceding a string
pub fn strip_hh_ns(s: &str) -> Cow<'_, str> {
    HH_NS_RE.replace(s, "")
}

pub fn has_ns(s: &str) -> bool {
    NS_RE.is_match(s)
}

pub fn strip_type_list(s: &str) -> Cow<'_, str> {
    TYPE_RE.replace_all(s, "")
}

pub fn cmp(s1: &str, s2: &str, case_sensitive: bool, ignore_ns: bool) -> bool {
    fn canon(s: &str, ignore_ns: bool) -> &str {
        if ignore_ns { strip_ns(s) } else { s }
    }

    let s1 = canon(s1, ignore_ns);
    let s2 = canon(s2, ignore_ns);

    if case_sensitive {
        s1 == s2
    } else {
        s1.eq_ignore_ascii_case(s2)
    }
}

pub fn is_self(s: impl AsRef<str>) -> bool {
    s.as_ref().eq_ignore_ascii_case(ns_classes::SELF)
}

pub fn is_parent(s: impl AsRef<str>) -> bool {
    s.as_ref().eq_ignore_ascii_case(ns_classes::PARENT)
}

pub fn is_static(s: impl AsRef<str>) -> bool {
    s.as_ref().eq_ignore_ascii_case(ns_classes::STATIC)
}

pub fn is_class(s: impl AsRef<str>) -> bool {
    s.as_ref().eq_ignore_ascii_case(members::M_CLASS)
}

pub fn mangle_meth_caller(mangled_cls_name: &str, f_name: &str) -> String {
    format!("\\MethCaller${}${}", mangled_cls_name, f_name)
}

pub fn lstrip<'a>(s: &'a str, p: &str) -> &'a str {
    s.strip_prefix(p).unwrap_or(s)
}

pub fn lstrip_bslice<'a>(s: &'a [u8], p: &[u8]) -> &'a [u8] {
    s.strip_prefix(p).unwrap_or(s)
}

pub mod types {
    pub fn fix_casing(s: &str) -> &str {
        match s.to_lowercase().as_str() {
            "vector" => "Vector",
            "immvector" => "ImmVector",
            "set" => "Set",
            "immset" => "ImmSet",
            "map" => "Map",
            "immmap" => "ImmMap",
            "pair" => "Pair",
            _ => s,
        }
    }
}

/* Integers are represented as strings */
pub mod integer {
    pub fn to_decimal(s: &str) -> Result<String, ocaml_helper::ParseIntError> {
        /* Don't accidentally convert 0 to 0o */
        let r = if s.len() > 1
            && s.as_bytes()[0] == b'0'
            && s.as_bytes()[1] >= b'0'
            && s.as_bytes()[1] <= b'9'
        {
            ocaml_helper::int_of_string_wrap(format!("0o{}", &s[1..]).as_bytes())
        } else {
            ocaml_helper::int_of_string_wrap(s.as_bytes())
        };
        r.map(|n| n.to_string())
    }
}

pub mod float {
    fn sprintf(f: f64) -> Option<String> {
        const BUF_SIZE: usize = 256;

        let format = "%.17g\0";
        let mut buffer = [0u8; BUF_SIZE];
        let n = unsafe {
            libc::snprintf(
                buffer.as_mut_ptr() as *mut libc::c_char,
                BUF_SIZE,
                format.as_ptr() as *const libc::c_char,
                f,
            ) as usize
        };
        if n >= BUF_SIZE {
            None
        } else {
            String::from_utf8(buffer[..n].to_vec()).ok()
        }
    }

    pub fn to_string(f: impl Into<f64>) -> String {
        let f = f.into();
        // or_else should not happen, but just in case it does fall back
        // to Rust native formatting
        let res = sprintf(f).unwrap_or_else(|| f.to_string());
        match res.as_ref() {
            "-nan" => naming_special_names_rust::math::NAN.to_string(),
            "nan" => naming_special_names_rust::math::NAN.to_string(),
            "-inf" => naming_special_names_rust::math::NEG_INF.to_string(),
            "inf" => naming_special_names_rust::math::INF.to_string(),
            _ => res,
        }
    }
}

pub mod locals {
    pub fn strip_dollar(s: &str) -> &str {
        if !s.is_empty() && s.as_bytes()[0] == b'$' {
            &s[1..]
        } else {
            s
        }
    }
}

pub mod classes {
    pub fn mangle_class(prefix: &str, scope: &str, idx: u32) -> String {
        if idx == 1 {
            format!("{}${}", prefix, scope)
        } else {
            format!("{}${}#{}", prefix, scope, idx)
        }
    }
}

pub mod closures {
    pub fn mangle_closure(scope: &str, idx: u32) -> String {
        super::classes::mangle_class("Closure", scope, idx)
    }

    /* Closure classes have names of the form
     *   Closure$ scope ix ; num
     * where
     *   scope  ::=
     *     <function-name>
     *   | <class-name> :: <method-name>
     *   |
     *   ix ::=
     *     # <digits>
     */
    pub fn unmangle_closure(mangled_name: &str) -> Option<&str> {
        if is_closure_name(mangled_name) {
            let prefix_length = "Closure$".chars().count();
            match mangled_name.find('#') {
                Some(pos) => Some(&mangled_name[prefix_length..pos]),
                None => Some(&mangled_name[prefix_length..]),
            }
        } else {
            None
        }
    }

    pub fn is_closure_name(s: &str) -> bool {
        s.starts_with("Closure$")
    }
}

pub mod reified {
    pub static PROP_NAME: &str = "86reified_prop";
    pub static INIT_METH_NAME: &str = "86reifiedinit";
    pub static INIT_METH_PARAM_NAME: &str = "$__typestructures";
    pub static GENERICS_LOCAL_NAME: &str = "$0ReifiedGenerics";
    pub static CAPTURED_PREFIX: &str = "$__captured$reifiedgeneric$";

    pub fn reified_generic_captured_name(is_fun: bool, i: usize) -> String {
        let type_ = if is_fun { "function" } else { "class" };
        // to_string() due to T52404885
        format!("$__captured$reifiedgeneric${}${}", type_, i)
    }

    pub fn mangle_reified_param(no_dollar: bool, s: &str) -> String {
        format!("{}__reified${}", if no_dollar { "" } else { "$" }, s)
    }

    pub fn captured_name(is_fun: bool, i: usize) -> String {
        format!(
            "{}{}${}",
            CAPTURED_PREFIX,
            if is_fun { "function" } else { "class" },
            i
        )
    }

    pub fn is_captured_generic(id: &str) -> Option<(bool, u32)> {
        if id.starts_with(CAPTURED_PREFIX) {
            if let [name, i] = id
                .trim_start_matches(CAPTURED_PREFIX)
                .splitn(2, '$')
                .collect::<Vec<_>>()
                .as_slice()
            {
                let is_captured = match *name {
                    "function" => true,
                    "class" => false,
                    _ => return None,
                };
                let captured_id = i.parse();
                if let Ok(captured) = captured_id {
                    return Some((is_captured, captured));
                };
            }
        };
        None
    }
}

pub mod coeffects {
    #[allow(clippy::redundant_static_lifetimes)]
    pub static LOCAL_NAME: &'static str = "$0Coeffects";
    #[allow(clippy::redundant_static_lifetimes)]
    pub static CALLER: &'static str = "86caller";
}

#[cfg(test)]
mod string_utils_tests {
    use pretty_assertions::assert_eq;

    #[test]
    fn quote_string_test() {
        let some_string = "test";
        assert_eq!(super::quote_string(some_string), "\"test\"");
    }

    #[test]
    fn prefix_namespace_test() {
        let namespace = "ns";
        let some_string = "test";
        assert_eq!(super::prefix_namespace(namespace, some_string), "ns\\test");
    }

    #[test]
    fn strip_global_ns_test() {
        let some_string = "\\test";
        let another_string = "\\\\";
        assert_eq!(super::strip_global_ns(some_string), "test");
        assert_eq!(super::strip_global_ns(another_string), "\\");

        let some_string = b"\\test";
        let another_string = b"\\\\";
        assert_eq!(super::strip_global_ns_bslice(some_string), b"test");
        assert_eq!(super::strip_global_ns_bslice(another_string), b"\\");
    }

    #[test]
    fn strip_ns_test() {
        let with_ns = "ns1\\test";
        let without_ns = "test";
        assert_eq!(super::strip_ns(with_ns), "test");
        assert_eq!(super::strip_ns(without_ns), without_ns);
    }

    #[test]
    fn strip_hh_ns() {
        let with_ns = "HH\\test";
        let without_ns = "test";
        assert_eq!(super::strip_ns(with_ns), "test");
        assert_eq!(super::strip_ns(without_ns), without_ns);
    }

    #[test]
    fn strip_hh_ns_2() {
        let with_ns = "\\HH\\test";
        let without_ns = "test";
        assert_eq!(super::strip_ns(with_ns), "test");
        assert_eq!(super::strip_ns(without_ns), without_ns);
    }

    #[test]
    fn has_ns_test() {
        let with_ns = "\\test";
        let without_ns = "test";
        assert!(super::has_ns(with_ns));
        assert!(!super::has_ns(without_ns));
    }

    #[test]
    fn strip_type_list_test() {
        let s = "MutableMap<Tk, Tv>";
        assert_eq!(super::strip_type_list(s).into_owned(), "MutableMap");
    }

    #[test]
    fn cmp_test() {
        let s1 = "ns1\\s1";
        let s1_uppercase = "NS1\\S1";

        let ns2_s1 = "ns2\\s1";
        let ns2_s1_uppercase = "NS2\\S1";

        let ns2_s2 = "ns2\\s2";

        assert!(super::cmp(s1, s1_uppercase, false, false));
        assert!(!super::cmp(s1, s1_uppercase, true, false));
        assert!(super::cmp(s1, s1_uppercase, false, true));
        assert!(!super::cmp(s1, s1_uppercase, true, true));

        assert!(!super::cmp(s1, ns2_s1, false, false));
        assert!(!super::cmp(s1, ns2_s1, true, false));
        assert!(super::cmp(s1, ns2_s1, false, true));
        assert!(super::cmp(s1, ns2_s1, true, true));

        assert!(!super::cmp(s1, ns2_s1_uppercase, false, false));
        assert!(!super::cmp(s1, ns2_s1_uppercase, true, false));
        assert!(super::cmp(s1, ns2_s1_uppercase, false, true));
        assert!(!super::cmp(s1, ns2_s1_uppercase, true, true));

        assert!(!super::cmp(s1, ns2_s2, false, false));
        assert!(!super::cmp(s1, ns2_s2, true, false));
        assert!(!super::cmp(s1, ns2_s2, false, true));
        assert!(!super::cmp(s1, ns2_s2, true, true));
    }

    #[test]
    fn is_self_test() {
        let s1 = "self";
        let s2 = "not_self";

        assert!(super::is_self(s1));
        assert!(!super::is_self(s2));
    }

    #[test]
    fn is_parent_test() {
        let s1 = "parent";
        let s2 = "not_parent";

        assert!(super::is_parent(s1));
        assert!(!super::is_parent(s2));
    }

    #[test]
    fn is_static_test() {
        let s1 = "static";
        let s2 = "not_static";

        assert!(super::is_static(s1));
        assert!(!super::is_static(s2));
    }

    #[test]
    fn is_class_test() {
        let s1 = "class";
        let s2 = "not_a_class";

        assert!(super::is_class(s1));
        assert!(!super::is_class(s2));
    }

    #[test]
    fn mangle_meth_caller_test() {
        let cls = "SomeClass";
        let f = "some_function";

        assert_eq!(
            super::mangle_meth_caller(cls, f),
            "\\MethCaller$SomeClass$some_function"
        );
    }

    #[test]
    fn mangle_test_1() {
        assert_eq!(
            super::mangle(":foo:bar-and-baz".into()),
            "xhp_foo__bar_and_baz"
        );
    }

    #[test]
    fn mangle_test_2() {
        assert_eq!(super::mangle("\\:base".into()), "\\xhp_base");
    }

    #[test]
    fn mangle_test_3() {
        assert_eq!(
            super::mangle("\\NS1\\NS2\\:base".into()),
            "\\NS1\\NS2\\xhp_base"
        );
    }

    mod types {
        mod fix_casing {
            macro_rules! test_case {
                ($name: ident, $input: expr, $expected: expr) => {
                    #[test]
                    fn $name() {
                        assert_eq!(crate::types::fix_casing($input), $expected);
                    }
                };
            }

            test_case!(lowercase_vector, "vector", "Vector");
            test_case!(mixedcase_vector, "vEcTor", "Vector");
            test_case!(uppercase_vector, "VECTOR", "Vector");

            test_case!(lowercase_immvector, "immvector", "ImmVector");
            test_case!(mixedcase_immvector, "immvEcTor", "ImmVector");
            test_case!(uppercase_immvector, "IMMVECTOR", "ImmVector");

            test_case!(lowercase_set, "set", "Set");
            test_case!(mixedcase_set, "SeT", "Set");
            test_case!(uppercase_set, "SET", "Set");

            test_case!(lowercase_immset, "immset", "ImmSet");
            test_case!(mixedcase_immset, "ImMSeT", "ImmSet");
            test_case!(uppercase_immset, "IMMSET", "ImmSet");

            test_case!(lowercase_map, "map", "Map");
            test_case!(mixedcase_map, "MaP", "Map");
            test_case!(uppercase_map, "MAP", "Map");

            test_case!(lowercase_immmap, "immmap", "ImmMap");
            test_case!(mixedcase_immmap, "immMaP", "ImmMap");
            test_case!(uppercase_immmap, "IMMMAP", "ImmMap");

            test_case!(lowercase_pair, "pair", "Pair");
            test_case!(mixedcase_pair, "pAiR", "Pair");
            test_case!(uppercase_pair, "PAIR", "Pair");

            test_case!(
                non_hack_collection_returns_original_string,
                "SomeStRinG",
                "SomeStRinG"
            );
            test_case!(
                hack_collection_with_leading_whitespace_returns_original_string,
                " pair",
                " pair"
            );
            test_case!(
                hack_collection_with_trailing_whitespace_returns_original_string,
                "pair ",
                "pair "
            );
        }
    }

    mod float {
        use crate::float;
        #[test]
        fn test_no_float_part() {
            assert_eq!(float::to_string(1.0), "1")
        }

        #[test]
        fn test_precision() {
            assert_eq!(float::to_string(1.1), "1.1000000000000001")
        }

        #[test]
        fn test_no_trailing_zeroes() {
            assert_eq!(float::to_string(1.2), "1.2")
        }

        #[test]
        fn test_scientific() {
            assert_eq!(float::to_string(1e+100), "1e+100")
        }

        #[test]
        fn test_scientific_precision() {
            assert_eq!(float::to_string(-2.1474836480001e9), "-2147483648.0001001")
        }

        #[test]
        fn test_negative_nan() {
            assert_eq!(float::to_string(-std::f32::NAN), "NAN")
        }
    }

    mod integer {
        mod to_decimal {
            use crate::integer;

            #[test]
            fn decimal_zero() {
                assert_eq!(integer::to_decimal("0"), Ok("0".to_string()));
            }

            #[test]
            fn octal_zero() {
                assert_eq!(integer::to_decimal("00"), Ok("0".to_string()));
            }

            #[test]
            fn binary_zero_lowercase() {
                assert_eq!(integer::to_decimal("0b0"), Ok("0".to_string()));
            }

            #[test]
            fn binary_zero_uppercase() {
                assert_eq!(integer::to_decimal("0B0"), Ok("0".to_string()));
            }

            #[test]
            fn hex_zero_lowercase() {
                assert_eq!(integer::to_decimal("0x0"), Ok("0".to_string()));
            }

            #[test]
            fn hex_zero_uppercase() {
                assert_eq!(integer::to_decimal("0X0"), Ok("0".to_string()));
            }

            #[test]
            fn decimal_random_value() {
                assert_eq!(integer::to_decimal("1245"), Ok("1245".to_string()));
            }

            #[test]
            fn octal_random_value() {
                assert_eq!(integer::to_decimal("02335"), Ok("1245".to_string()));
            }

            #[test]
            fn binary_random_value_lowercase() {
                assert_eq!(integer::to_decimal("0b10011011101"), Ok("1245".to_string()));
            }

            #[test]
            fn binary_random_value_uppercase() {
                assert_eq!(integer::to_decimal("0B10011011101"), Ok("1245".to_string()));
            }

            #[test]
            fn hex_random_value_lowercase() {
                assert_eq!(integer::to_decimal("0x4DD"), Ok("1245".to_string()));
            }

            #[test]
            fn hex_random_value_uppercase() {
                assert_eq!(integer::to_decimal("0X4DD"), Ok("1245".to_string()));
            }

            #[test]
            fn decimal_max_value() {
                assert_eq!(
                    integer::to_decimal("9223372036854775807"),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn octal_max_value() {
                assert_eq!(
                    integer::to_decimal("0777777777777777777777"),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn binary_max_value_lowercase() {
                assert_eq!(
                    integer::to_decimal(
                        "0b111111111111111111111111111111111111111111111111111111111111111"
                    ),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn binary_max_value_uppercase() {
                assert_eq!(
                    integer::to_decimal(
                        "0B111111111111111111111111111111111111111111111111111111111111111"
                    ),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn hex_max_value_lowercase() {
                assert_eq!(
                    integer::to_decimal("0x7FFFFFFFFFFFFFFF"),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn hex_max_value_uppercase() {
                assert_eq!(
                    integer::to_decimal("0X7FFFFFFFFFFFFFFF"),
                    Ok("9223372036854775807".to_string())
                );
            }

            #[test]
            fn unparsable_string() {
                assert!(integer::to_decimal("bad_string").is_err());
            }
        }
    }

    mod locals {
        use crate::locals;

        #[test]
        fn strip_single_leading_dollar() {
            assert_eq!(locals::strip_dollar("$foo"), "foo");
        }

        #[test]
        fn return_string_if_no_leading_dollar() {
            assert_eq!(locals::strip_dollar("foo"), "foo");
        }

        #[test]
        fn empty_string() {
            assert_eq!(locals::strip_dollar(""), "");
        }

        #[test]
        fn string_of_single_dollar() {
            assert_eq!(locals::strip_dollar("$"), "");
        }
    }

    mod classes {
        mod mangle_class {
            use crate::classes::mangle_class;

            #[test]
            fn idx_of_one() {
                assert_eq!(mangle_class("foo", "bar", 1), "foo$bar")
            }

            #[test]
            fn idx_of_two() {
                assert_eq!(mangle_class("foo", "bar", 2), "foo$bar#2")
            }
        }
    }

    mod closures {
        mod mangle_closure {
            use crate::closures::mangle_closure;

            #[test]
            fn idx_of_one() {
                assert_eq!(mangle_closure("foo", 1), "Closure$foo")
            }

            #[test]
            fn idx_of_two() {
                assert_eq!(mangle_closure("foo", 2), "Closure$foo#2")
            }
        }

        mod unmangle_closure {
            use crate::closures::unmangle_closure;

            #[test]
            fn idx_of_one() {
                assert_eq!(unmangle_closure("Closure$foo"), Some("foo"))
            }

            #[test]
            fn idx_of_two() {
                assert_eq!(unmangle_closure("Closure$foo#2"), Some("foo"))
            }

            #[test]
            fn non_closure() {
                assert_eq!(unmangle_closure("SomePrefix$foo"), None);
                assert_eq!(unmangle_closure("SomePrefix$foo#2"), None)
            }
        }

        mod is_closure_name {
            use crate::closures::is_closure_name;

            #[test]
            fn closure_1() {
                assert!(is_closure_name("Closure$foo"))
            }

            #[test]
            fn closure_2() {
                assert!(is_closure_name("Closure$foo#2"))
            }

            #[test]
            fn non_closure() {
                assert!(!is_closure_name("SomePrefix$foo"));
                assert!(!is_closure_name("SomePrefix$foo#2"))
            }
        }
    }

    mod reified {
        use crate::reified;

        #[test]
        fn test_mangle_reified_param() {
            assert_eq!(reified::mangle_reified_param(false, "x"), "$__reified$x");
            assert_eq!(reified::mangle_reified_param(true, "x"), "__reified$x")
        }

        #[test]
        fn test_is_captured_generic() {
            assert_eq!(
                reified::is_captured_generic("$__captured$reifiedgeneric$function$1"),
                Some((true, 1))
            );
            assert_eq!(
                reified::is_captured_generic("$__captured$reifiedgeneric$class$1"),
                Some((false, 1))
            );
            assert_eq!(reified::is_captured_generic("function$1"), None);
            assert_eq!(
                reified::is_captured_generic("$__captured$reifiedgeneric$function1"),
                None
            );
        }

        #[test]
        fn test_captured_name() {
            assert_eq!(
                reified::captured_name(true, 1),
                "$__captured$reifiedgeneric$function$1"
            );
            assert_eq!(
                reified::captured_name(false, 1),
                "$__captured$reifiedgeneric$class$1"
            );
        }
    }

    #[test]
    fn test_lstrip() {
        use super::lstrip;
        assert_eq!(lstrip("a", "a"), "");
        assert_eq!(lstrip("a", "ab"), "a");
        assert_eq!(lstrip("", "ab"), "");
        assert_eq!(lstrip("", ""), "");
        assert_eq!(lstrip("a", ""), "a");
        assert_eq!(lstrip("aa", "a"), "a");
        assert_eq!(lstrip("aa", "a"), "a");
    }
}
