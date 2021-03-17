// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(test)]
#[cfg(test)]
extern crate test;

fn main() {
    let command = "buck run @mode/opt //hphp/hack/src/hhbc:unique_list_bench-unittest -- --bench";
    println!("Run this bench suite with: {}", command);
}

#[cfg(test)]
mod tests {
    use super::*;
    use hhbc_by_ref_unique_list::UniqueList;
    use test::Bencher;

    fn make_sample_unique_list() -> UniqueList<String> {
        let mut ul = UniqueList::new();
        ul.add("a not very long string".to_owned());
        ul.add("another short string".to_owned());
        ul.add("yet another string".to_owned());
        ul.add("abcde".to_owned());
        ul.add("fghijkl".to_owned());
        ul.add("dkl;vnm,b.zmcnv".to_owned());
        ul.add("".to_owned());
        ul.add("ieo2930nvldkajsf".to_owned());
        ul
    }

    fn make_big_usize_list() -> UniqueList<usize> {
        let mut ul = UniqueList::new();
        for i in 0..1000000 {
            ul.add(i);
        }
        ul
    }

    fn make_even_usize_list() -> UniqueList<usize> {
        let mut ul = UniqueList::new();
        for i in 0..1000000 {
            if i % 2 == 0 {
                ul.add(i);
            }
        }
        ul
    }

    #[bench]
    fn bench_add_string(b: &mut Bencher) {
        let mut ul = UniqueList::new();
        b.iter(|| ul.add("a boring string".to_string()));
    }

    #[bench]
    fn bench_new_and_add_eight_strings(b: &mut Bencher) {
        b.iter(make_sample_unique_list);
    }

    #[bench]
    fn bench_add_eight_strings(b: &mut Bencher) {
        let mut ul = UniqueList::new();
        b.iter(|| {
            ul.add("a not very long string".to_owned());
            ul.add("another short string".to_owned());
            ul.add("yet another string".to_owned());
            ul.add("abcde".to_owned());
            ul.add("fghijkl".to_owned());
            ul.add("dkl;vnm,b.zmcnv".to_owned());
            ul.add("".to_owned());
            ul.add("ieo2930nvldkajsf".to_owned());
        });
    }

    #[bench]
    fn bench_remove_string(b: &mut Bencher) {
        let mut ul = make_sample_unique_list();
        b.iter(|| ul.remove("abcde"));
    }

    #[bench]
    fn bench_empty_ul(b: &mut Bencher) {
        let mut ul = make_sample_unique_list();
        b.iter(|| ul.empty());
    }

    #[bench]
    fn bench_small_diff(b: &mut Bencher) {
        let mut ul = UniqueList::new();
        ul.add("a".to_owned());
        ul.add("b".to_owned());
        ul.add("c".to_owned());
        ul.add("d".to_owned());
        ul.add("e".to_owned());
        ul.add("f".to_owned());
        ul.add("g".to_owned());
        ul.add("h".to_owned());
        ul.add("i".to_owned());
        ul.add("j".to_owned());
        ul.add("k".to_owned());
        let mut ul2 = UniqueList::new();
        ul2.add("b".to_owned());
        ul2.add("d".to_owned());
        ul2.add("f".to_owned());
        ul2.add("h".to_owned());
        ul2.add("j".to_owned());
        ul2.add("k".to_owned());
        b.iter(|| ul.diff(&ul2));
    }

    #[bench]
    fn bench_make_big_usize_ul(b: &mut Bencher) {
        b.iter(make_big_usize_list);
    }

    #[bench]
    fn bench_remove_from_big_ul(b: &mut Bencher) {
        let mut ul = make_big_usize_list();
        b.iter(|| ul.remove(&228491));
    }

    #[bench]
    fn bench_big_ul_cardinal(b: &mut Bencher) {
        let ul = make_big_usize_list();
        b.iter(|| ul.cardinal());
    }

    #[bench]
    fn bench_big_ul_diff(b: &mut Bencher) {
        let ul = make_big_usize_list();
        let even_ul = make_even_usize_list();
        b.iter(|| ul.diff(&even_ul));
    }
}
