// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(test)]
#[cfg(test)]
extern crate test;

fn main() {
    let command = "buck run @mode/opt //hphp/hack/src/hhbc:string_utils_bench-unittest -- --bench";
    println!("Run this bench suite with: {}", command);
}

#[cfg(test)]
mod tests {
    use super::*;
    use hhbc_string_utils_rust as string_utils;
    use test::Bencher;

    #[bench]
    fn bench_quote_string(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::quote_string(&s));
    }

    #[bench]
    fn bench_quote_string_with_escape(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::quote_string_with_escape(&s));
    }

    #[bench]
    fn bench_single_quote_string_with_escape(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::single_quote_string_with_escape(&s));
    }

    #[bench]
    fn bench_triple_quote_string(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::triple_quote_string(&s));
    }

    #[bench]
    fn bench_prefix_namespace(b: &mut Bencher) {
        let ns = String::from("a_namespace");
        let s = String::from("a_string");
        b.iter(|| string_utils::prefix_namespace(&ns, &s));
    }

    #[bench]
    fn bench_strip_global_ns_with_no_namespace(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::strip_global_ns(&s));
    }

    #[bench]
    fn bench_strip_global_ns_with_global_namespace(b: &mut Bencher) {
        let s = String::from("\\a_string");
        b.iter(|| string_utils::strip_global_ns(&s));
    }

    #[bench]
    fn bench_strip_ns_no_namespace(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::strip_ns(&s));
    }

    #[bench]
    fn bench_strip_ns(b: &mut Bencher) {
        let s = String::from("some_namespace\\a_string");
        b.iter(|| string_utils::strip_ns(&s));
    }

    #[bench]
    fn bench_has_ns_no_ns(b: &mut Bencher) {
        let s = String::from("a_string");
        b.iter(|| string_utils::has_ns(&s));
    }

    #[bench]
    fn bench_has_ns(b: &mut Bencher) {
        let s = String::from("some_ns\\a_string");
        b.iter(|| string_utils::has_ns(&s));
    }

    #[bench]
    fn bench_strip_type_list_no_types(b: &mut Bencher) {
        let s = String::from("MutableMap");
        b.iter(|| string_utils::strip_type_list(&s));
    }

    #[bench]
    fn bench_strip_type_list_with_types(b: &mut Bencher) {
        let s = String::from("MutableMap<Tk, Tv>");
        b.iter(|| string_utils::strip_type_list(&s));
    }

    #[bench]
    fn bench_cmp(b: &mut Bencher) {
        let s1 = String::from("ns1\\s1");
        let s2 = String::from("ns2\\s1");
        b.iter(|| string_utils::cmp(&s1, &s2, false, true));
    }

    #[bench]
    fn bench_is_self(b: &mut Bencher) {
        let s = String::from("self");
        b.iter(|| string_utils::is_self(&s));
    }

    #[bench]
    fn bench_is_parent(b: &mut Bencher) {
        let s = String::from("parent");
        b.iter(|| string_utils::is_parent(&s));
    }

    #[bench]
    fn bench_is_static(b: &mut Bencher) {
        let s = String::from("static");
        b.iter(|| string_utils::is_static(&s));
    }

    #[bench]
    fn bench_is_class(b: &mut Bencher) {
        let s = String::from("class");
        b.iter(|| string_utils::is_class(&s));
    }

    #[bench]
    fn bench_mangle_meth_caller(b: &mut Bencher) {
        let cls = String::from("SomeClass");
        let f = String::from("some_function");
        b.iter(|| string_utils::mangle_meth_caller(&cls, &f));
    }

    #[bench]
    fn bench_mangle_closure(b: &mut Bencher) {
        b.iter(|| string_utils::closures::mangle_closure("foo", 2));
    }

    #[bench]
    fn bench_unmangle_closure(b: &mut Bencher) {
        b.iter(|| string_utils::closures::unmangle_closure("Closure$foo#2"));
    }

    #[bench]
    fn bench_is_closure_name(b: &mut Bencher) {
        b.iter(|| string_utils::closures::is_closure_name("Closure$foo"));
    }
}
