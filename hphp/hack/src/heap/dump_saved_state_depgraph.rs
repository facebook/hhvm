// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use depgraph_reader::Dep;
use depgraph_reader::DepGraph;

#[derive(Debug)]
enum Error {
    IoError(std::io::Error),
    DepgraphError(String),
    Other(String),
}
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Error::IoError(ref e) => ::std::fmt::Display::fmt(e, f),
            Error::DepgraphError(ref e) => f.write_str(e),
            Error::Other(ref e) => f.write_str(e),
        }
    }
}
impl std::error::Error for Error {}
impl std::convert::From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Error::IoError(error)
    }
}
type Result<T> = std::result::Result<T, Error>;

const MAX_DIGITS_IN_HASH: usize = 20; // (u64::MAX as f64).log10() as usize + 1;

fn print_edges_header() {
    println!(
        "  {:>width$}  dependent",
        "dependency",
        width = MAX_DIGITS_IN_HASH
    );
}

/// Auxiliary function to print a 64-bit edge
fn print_edge_u64(dependency: Dep, dependent: Dep, hex_dump: bool) {
    if hex_dump {
        println!(
            "  {dependency:#016x} {dependent:#016x}",
            dependent = dependent,
            dependency = dependency
        );
    } else {
        println!(
            "  {dependency:>width$}  {dependent}",
            dependent = dependent,
            dependency = dependency,
            width = MAX_DIGITS_IN_HASH
        );
    }
}

/// Add edges to `es` given source vertex `k` and dest vertices `vs`.
fn add_edges<T: Ord + Clone>(es: &mut Vec<(T, T)>, k: T, vs: &std::collections::BTreeSet<T>) {
    es.extend(vs.iter().map(|v| (k.clone(), v.clone())));
}

/// Retrieve the adjacency list for `k` in `g`.
///
/// This is the analog of `value_vertex` for 64-bit depgraphs.
fn hashes(g: &DepGraph, k: Dep) -> std::collections::BTreeSet<Dep> {
    match g.hash_list_for(k) {
        None => std::collections::BTreeSet::new(),
        Some(hashes) => g.hash_list_hashes(hashes).collect(),
    }
}

/// Print an ASCII representation of a 64-bit depgraph to stdout.
fn dump_depgraph64(file: &str, dependency_hash: Option<Dep>, hex_dump: bool) -> Result<()> {
    let dg = DepGraph::from_path(file)?;
    let () = dg.validate_hash_lists().map_err(Error::DepgraphError)?;

    let print_edges_for_key = |key: Dep| {
        let dests = hashes(&dg, key);
        for dst in dests {
            print_edge_u64(key, dst, hex_dump);
        }
    };

    match dependency_hash {
        None => {
            for key in dg.all_hashes() {
                print_edges_for_key(key)
            }
        }
        Some(dependency_hash) => print_edges_for_key(dependency_hash),
    };

    Ok(())
}

/// Compare two 64-bit dependency graphs.
///
/// Calculate the edges in `control_file` not in `test_file` (missing edges) and
/// the edges in `test_file` not in `control_file` (extraneous edges).
fn comp_depgraph64(
    no_progress_bar: bool,
    test_file: &str,
    control_file: &str,
    hex_dump: bool,
) -> Result<()> {
    let mut num_edges_missing = 0;
    let l_depgraph = DepGraph::from_path(test_file)?;
    let r_depgraph = DepGraph::from_path(control_file)?;

    match (|| {
        let ((), ()) = (
            l_depgraph.validate_hash_lists()?,
            r_depgraph.validate_hash_lists()?,
        );
        let (mut l_dependencies_iter, mut r_dependencies_iter) =
            (l_depgraph.all_hashes(), r_depgraph.all_hashes());
        let (lnum_keys, rnum_keys) = (l_dependencies_iter.len(), r_dependencies_iter.len());
        let (mut lproc, mut rproc) = (0, 0);
        let (mut in_r_not_l, mut in_l_not_r) = (vec![], vec![]);
        let (mut l_dependency_opt, mut r_dependency_opt) =
            (l_dependencies_iter.next(), r_dependencies_iter.next());
        let (mut ledge_count, mut redge_count) = (0, 0);
        let bar = if no_progress_bar {
            None
        } else {
            Some(indicatif::ProgressBar::new(
                std::cmp::max(lnum_keys, rnum_keys) as u64,
            ))
        };
        if let Some(bar) = bar.as_ref() {
            bar.println("Comparing graphs. Patience...")
        };
        while l_dependency_opt.is_some() || r_dependency_opt.is_some() {
            match (l_dependency_opt, r_dependency_opt) {
                (None, Some(r_dependency)) => {
                    // These edges are in `r` and not in `l`.
                    let dependency = r_dependency;
                    let dependents = hashes(&r_depgraph, dependency);
                    redge_count += dependents.len();
                    add_edges(&mut in_r_not_l, dependency, &dependents);
                    r_dependency_opt = r_dependencies_iter.next();
                    rproc += 1;
                    if let Some(bar) = bar.as_ref() {
                        if rnum_keys > lnum_keys {
                            bar.inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                        }
                    }
                }
                (Some(l_dependency), None) => {
                    // These edges are in `l` and not in `r`.
                    let dependency = l_dependency;
                    let dependents = hashes(&l_depgraph, dependency);
                    l_dependency_opt = l_dependencies_iter.next();
                    ledge_count += dependents.len();
                    add_edges(&mut in_l_not_r, dependency, &dependents);
                    lproc += 1;
                    if let Some(bar) = bar.as_ref() {
                        if lnum_keys > rnum_keys {
                            bar.inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                        }
                    }
                }
                (Some(l_dependency), Some(r_dependency)) => {
                    let (l_dependencies, r_dependencies) = (
                        hashes(&l_depgraph, l_dependency),
                        hashes(&r_depgraph, r_dependency),
                    );
                    if l_dependency < r_dependency {
                        // These edges are in `l` but not in `r`.
                        ledge_count += l_dependencies.len();
                        add_edges(&mut in_l_not_r, l_dependency, &l_dependencies);
                        l_dependency_opt = l_dependencies_iter.next();
                        lproc += 1;
                        if let Some(bar) = bar.as_ref() {
                            if lnum_keys >= rnum_keys {
                                bar.inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                            }
                        }
                        continue;
                    }
                    if l_dependency > r_dependency {
                        // These edges are in `r` but not in `l`.
                        redge_count += r_dependencies.len();
                        add_edges(&mut in_r_not_l, r_dependency, &r_dependencies);
                        r_dependency_opt = r_dependencies_iter.next();
                        rproc += 1;
                        if let Some(bar) = bar.as_ref() {
                            if rnum_keys > lnum_keys {
                                bar.inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                            }
                        }
                        continue;
                    }
                    ledge_count += l_dependencies.len();
                    redge_count += r_dependencies.len();
                    let mut dests: std::collections::BTreeSet<Dep> =
                        std::collections::BTreeSet::new();
                    dests.extend(
                        r_dependencies
                            .iter()
                            .filter(|&v| !l_dependencies.contains(v)),
                    );
                    add_edges(&mut in_r_not_l, l_dependency, &dests);
                    dests.clear();
                    dests.extend(
                        l_dependencies
                            .iter()
                            .filter(|&v| !r_dependencies.contains(v)),
                    );
                    add_edges(&mut in_l_not_r, l_dependency, &dests);
                    l_dependency_opt = l_dependencies_iter.next();
                    r_dependency_opt = r_dependencies_iter.next();
                    lproc += 1;
                    rproc += 1;
                    if let Some(bar) = bar.as_ref() {
                        bar.inc(1)
                    }; // No matter whether `l` or `r` has more keys, progress was made.
                }
                (None, None) => panic!("The impossible happened!"),
            }
        }
        if let Some(bar) = bar {
            bar.finish_and_clear()
        };
        num_edges_missing = in_r_not_l.len();
        println!("\nResults\n=======");
        println!("Processed {}/{} of nodes in 'test'", lproc, lnum_keys);
        println!("Processed {}/{} of nodes in 'control'", rproc, rnum_keys);
        println!("Edges in 'test': {}", ledge_count);
        println!("Edges in 'control': {}", redge_count);
        println!(
            "Edges in 'control' missing in 'test' (there are {}):",
            in_r_not_l.len()
        );
        print_edges_header();
        for (key, dst) in in_r_not_l {
            print_edge_u64(key, dst, hex_dump);
        }
        println!(
            "Edges in 'test' missing in 'control' (there are {}):",
            in_l_not_r.len()
        );
        print_edges_header();
        for (key, dst) in in_l_not_r {
            print_edge_u64(key, dst, hex_dump);
        }
        Ok(())
    })() {
        Ok(()) => {
            if num_edges_missing == 0 {
                Ok(())
            } else {
                // Rust 2018 semantics are such that this will result in a
                // non-zero error code
                // (https://doc.rust-lang.org/edition-guide/rust-2018/error-handling-and-panics/question-mark-in-main-and-tests.html).
                Err(Error::Other(format!(
                    "{} missing edges detected",
                    num_edges_missing
                )))
            }
        }
        Err(msg) => Err(Error::DepgraphError(msg)),
    }
}

use clap::Parser;

fn parse_hex_or_decimal(src: &str) -> std::result::Result<u64, std::num::ParseIntError> {
    let src_trim = src.trim_start_matches("0x");
    if src_trim.len() != src.len() {
        u64::from_str_radix(src_trim, 16)
    } else {
        src_trim.parse::<u64>()
    }
}

#[derive(Debug, Parser)]
#[clap(
    name = "dump_saved_state_depgraph",
    about = "
Common usage is to provide two file arguments to compare, 'test' and 'control'.

Example invocation:

  dump_saved_state_depgraph \\
      --test path/to/test.bin --control path/to/control.bin

Exit code will be 0 if 'test' >= 'control' and 1 if 'test' < 'control'."
)]
struct Opt {
    #[clap(long = "with-progress-bar", help = "Enable progress bar display")]
    with_progress_bar: bool,

    #[clap(long = "dump", help = "graph to render as text")]
    dump: Option<String>,

    #[clap(
        long = "dependency-hash",
        help = "(with --dump; only for 64-bit) only dump edges for the given dependency hash",
        value_parser = parse_hex_or_decimal
    )]
    dependency_hash: Option<u64>,

    #[clap(long = "print-hex", help = "print hexadecimal hashes")]
    print_hex: bool,

    #[clap(long = "test", help = "'test' graph")]
    test: Option<String>,

    #[clap(long = "control", help = "'control' graph")]
    control: Option<String>,
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    let opt = Opt::try_parse()?;
    match match opt {
        Opt {
            dump: Some(file),
            dependency_hash,
            print_hex,
            ..
        } => dump_depgraph64(&file, dependency_hash.map(Dep::new), print_hex),
        Opt {
            with_progress_bar,
            test: Some(test),
            control: Some(control),
            print_hex,
            ..
        } => comp_depgraph64(!with_progress_bar, &test, &control, print_hex),
        _ => Ok(()),
    } {
        Ok(()) => Ok(()),
        Err(e) => Err(Box::new(e)),
    }
}
