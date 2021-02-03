// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug)]
enum Error {
    RusqliteError(rusqlite::Error),
    IoError(std::io::Error),
    DepgraphError(String),
    Other(String),
}
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Error::RusqliteError(ref e) => ::std::fmt::Display::fmt(e, f),
            Error::IoError(ref e) => ::std::fmt::Display::fmt(e, f),
            Error::DepgraphError(ref e) => f.write_str(e),
            Error::Other(ref e) => f.write_str(e),
        }
    }
}
impl std::error::Error for Error {}
impl std::convert::From<rusqlite::Error> for Error {
    fn from(error: rusqlite::Error) -> Self {
        Error::RusqliteError(error)
    }
}
impl std::convert::From<std::io::Error> for Error {
    fn from(error: std::io::Error) -> Self {
        Error::IoError(error)
    }
}
type Result<T> = std::result::Result<T, Error>;

/// Retrieve the `key_vertex` column of a row.
fn key_vertex(row: &rusqlite::Row) -> Result<u32> {
    Ok(row.get(0)?)
}

/// Retrieve the `value_vertex` column of a row.
fn value_vertex(row: &rusqlite::Row) -> Result<std::collections::BTreeSet<u32>> {
    let data: Vec<u8> = row.get(1)?;
    Ok(data
        .chunks_exact(std::mem::size_of::<u32>())
        .map(|slice| match *slice {
            [a, b, c, d] => u32::from_le_bytes([a, b, c, d]),
            _ => panic!("chunks_exact emitted wrong chunk size"),
        })
        .collect())
}

/// Count the number of nodes in a depgraph.
fn count_key_vertex(db: &rusqlite::Connection) -> Result<usize> {
    let mut stmt = db.prepare("select count (*) from deptable")?;
    let mut rows = stmt.query(rusqlite::NO_PARAMS)?;
    match rows.next()? {
        Some(row) => {
            let cnt: i64 = row.get(0)?;
            Ok(cnt as usize)
        }
        None => Err(Error::Other(
            "Error: Failure getting row count from deptable".to_string(),
        )),
    }
}

/// Print an ASCII representation of a 32-bit depgraph to stdout.
fn dump_depgraph32(file: &str) -> Result<()> {
    let db = rusqlite::Connection::open(file)?;
    let mut stmt = db.prepare("select * from deptable limit 10")?;
    let mut rows = stmt.query(rusqlite::NO_PARAMS)?;
    let digits = (u32::MAX as f32).log10() as usize + 1;
    while let Some(row) = rows.next()? {
        let key = key_vertex(row)?;
        let dests = value_vertex(row)?;
        for dst in dests {
            println!("  {key:>width$}  {}", dst, key = key, width = digits);
        }
    }
    Ok(())
}

/// Add edges to `es` given source vertex `k` and dest vertices `vs`.
fn add_edges<T: Ord + Clone>(es: &mut Vec<(T, T)>, k: T, vs: &std::collections::BTreeSet<T>) {
    es.extend(vs.iter().map(|v| (k.clone(), v.clone())));
}

/// Compare two 32-bit dependency graphs.
///
/// Calculate the edges in `rfile` not in `lfile` (missing edges) and
/// the edges in `lfile` not in `rfile` (extraneous edges).
fn comp_depgraph32(no_progress_bar: bool, lfile: &str, rfile: &str) -> Result<()> {
    let (ldb, rdb) = (
        rusqlite::Connection::open(lfile)?,
        rusqlite::Connection::open(rfile)?,
    );
    let (mut lstmt, mut rstmt) = (
        ldb.prepare("select * from deptable")?,
        rdb.prepare("select * from deptable")?,
    );
    let (lnum_keys, rnum_keys) = (count_key_vertex(&ldb)?, count_key_vertex(&rdb)?);
    let (mut lproc, mut rproc) = (0, 0);
    let (mut missing, mut extra) = (vec![], vec![]);
    let (mut lrows, mut rrows) = (
        lstmt.query(rusqlite::NO_PARAMS)?,
        rstmt.query(rusqlite::NO_PARAMS)?,
    );
    let (mut lro, mut rro) = (lrows.next()?, rrows.next()?);
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
    while lro.is_some() || rro.is_some() {
        match (lro, rro) {
            (None, Some(rrow)) => {
                // These edges are in `r` and not in `l`.
                let k = key_vertex(rrow)?;
                let vs = value_vertex(rrow)?;
                redge_count += vs.len();
                add_edges(&mut missing, k, &vs);
                rro = rrows.next()?;
                rproc += 1;
                if bar.is_some() && rnum_keys > lnum_keys {
                    bar.as_ref().unwrap().inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                }
            }
            (Some(lrow), None) => {
                let k = key_vertex(lrow)?;
                let vs = value_vertex(lrow)?;
                ledge_count += vs.len();
                add_edges(&mut extra, k, &vs);
                lro = lrows.next()?;
                lproc += 1;
                if bar.is_some() && lnum_keys > rnum_keys {
                    bar.as_ref().unwrap().inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                }
            }
            (Some(lrow), Some(rrow)) => {
                let (lk, rk) = (key_vertex(lrow)?, key_vertex(rrow)?);
                let (lvs, rvs) = (value_vertex(lrow)?, value_vertex(rrow)?);
                if lk < rk {
                    // These edges are in `l` but not in `r`.
                    ledge_count += lvs.len();
                    add_edges(&mut extra, lk, &lvs);
                    lro = lrows.next()?;
                    lproc += 1;
                    if bar.is_some() && lnum_keys >= rnum_keys {
                        bar.as_ref().unwrap().inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                    }
                    continue;
                }
                if lk > rk {
                    // These edges are in `r` but not in `l`.
                    redge_count += rvs.len();
                    add_edges(&mut missing, rk, &rvs);
                    rro = rrows.next()?;
                    rproc += 1;
                    if bar.is_some() && rnum_keys > lnum_keys {
                        bar.as_ref().unwrap().inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                    }
                    continue;
                }
                ledge_count += lvs.len();
                redge_count += rvs.len();
                // Vertices in `rvs` not in `lvs` indicate missing edges.
                let mut dests: std::collections::BTreeSet<u32> = std::collections::BTreeSet::new();
                dests.extend(rvs.iter().filter(|&v| !lvs.contains(v)));
                add_edges(&mut missing, lk, &dests);
                // Vertices in `lvs` not in `rvs` indicate extra edges.
                dests.clear();
                dests.extend(lvs.iter().filter(|&v| !rvs.contains(v)));
                add_edges(&mut extra, lk, &dests);
                lro = lrows.next()?;
                rro = rrows.next()?;
                lproc += 1;
                rproc += 1;
                if bar.is_some() {
                    bar.as_ref().unwrap().inc(1)
                }; // No matter whether `l` or `r` has more keys, progress was made.
            }
            (None, None) => panic!("The impossible happened!"),
        }
    }
    if let Some(bar) = bar {
        bar.finish_and_clear()
    };
    let digits = (u32::MAX as f32).log10() as usize + 1;
    let num_edges_missing = missing.len(); // If non-zero, `l` is broken.
    println!("\nResults\n=======");
    println!("Processed {}/{} of nodes in 'l'", lproc, lnum_keys);
    println!("Processed {}/{} of nodes in 'r'", rproc, rnum_keys);
    println!("Edges in 'l': {}", ledge_count);
    println!("Edges in 'r': {}", redge_count);
    println!("Edges in 'r' missing in 'l' (there are {}):", missing.len());
    for (key, dst) in missing {
        println!("  {key:>width$}  {}", dst, key = key, width = digits);
    }
    println!("Edges in 'l' missing in 'r' (there are {}):", extra.len());
    for (key, dst) in extra {
        println!("  {key:>width$}  {}", dst, key = key, width = digits);
    }

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

/// Retrieve the adjacency list for `k` in `g`.
///
/// This is the analog of `value_vertex` for 64-bit depgraphs.
fn hashes(g: &depgraph::reader::DepGraph, k: u64) -> std::collections::BTreeSet<u64> {
    match g.hash_list_for(depgraph::dep::Dep::new(k)) {
        None => std::collections::BTreeSet::<u64>::new(),
        Some(hashes) => g.hash_list_hashes(hashes).map(|x| x.into()).collect(),
    }
}

/// Print an ASCII representation of a 64-bit depgraph to stdout.
fn dump_depgraph64(file: &str) -> Result<()> {
    let digits = (u64::MAX as f64).log10() as usize + 1;
    let o = depgraph::reader::DepGraphOpener::from_path(file)?;
    match (|| {
        let dg = o.open()?;
        let () = dg.validate_hash_lists()?;
        for &key in dg.all_hashes().iter() {
            let dests = hashes(&dg, key);
            for dst in dests {
                println!("  {key:>width$}  {}", dst, key = key, width = digits);
            }
        }
        Ok(())
    })() {
        Ok(()) => Ok(()),
        Err(msg) => Err(Error::DepgraphError(msg)),
    }
}

/// Compare two 64-bit dependency graphs.
///
/// Calculate the edges in `rfile` not in `lfile` (missing edges) and
/// the edges in `lfile` not in `rfile` (extraneous edges).
fn comp_depgraph64(no_progress_bar: bool, lfile: &str, rfile: &str) -> Result<()> {
    let lo = depgraph::reader::DepGraphOpener::from_path(lfile)?;
    let ro = depgraph::reader::DepGraphOpener::from_path(rfile)?;
    let mut num_edges_missing = 0;
    match (|| {
        let (ldg, rdg) = (lo.open()?, ro.open()?);
        let ((), ()) = (ldg.validate_hash_lists()?, rdg.validate_hash_lists()?);
        let (lvs, rvs) = (ldg.all_hashes(), rdg.all_hashes());
        let (lnum_keys, rnum_keys) = (lvs.len(), rvs.len());
        let (mut lproc, mut rproc) = (0, 0);
        let (mut missing, mut extra) = (vec![], vec![]);
        let (mut lrows, mut rrows) = (lvs.iter(), rvs.iter());
        let (mut lro, mut rro) = (lrows.next(), rrows.next());
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
        while lro.is_some() || rro.is_some() {
            match (lro, rro) {
                (None, Some(&rk)) => {
                    // These edges are in `r` and not in `l`.
                    let k = rk;
                    let vs = hashes(&rdg, k);
                    redge_count += vs.len();
                    add_edges(&mut missing, k, &vs);
                    rro = rrows.next();
                    rproc += 1;
                    if bar.is_some() && rnum_keys > lnum_keys {
                        bar.as_ref().unwrap().inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                    }
                }
                (Some(&lk), None) => {
                    // These edges are in `l` and not in `r`.
                    let k = lk;
                    let vs = hashes(&ldg, k);
                    lro = lrows.next();
                    ledge_count += vs.len();
                    add_edges(&mut extra, k, &vs);
                    lproc += 1;
                    if bar.is_some() && lnum_keys > rnum_keys {
                        bar.as_ref().unwrap().inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                    }
                }
                (Some(&lk), Some(&rk)) => {
                    let (lvs, rvs) = (hashes(&ldg, lk), hashes(&rdg, rk));
                    if lk < rk {
                        // These edges are in `l` but not in `r`.
                        ledge_count += lvs.len();
                        add_edges(&mut extra, lk, &lvs);
                        lro = lrows.next();
                        lproc += 1;
                        if bar.is_some() && lnum_keys >= rnum_keys {
                            bar.as_ref().unwrap().inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                        }
                        continue;
                    }
                    if lk > rk {
                        // These edges are in `r` but not in `l`.
                        redge_count += rvs.len();
                        add_edges(&mut missing, rk, &rvs);
                        rro = rrows.next();
                        rproc += 1;
                        if bar.is_some() && rnum_keys > lnum_keys {
                            bar.as_ref().unwrap().inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                        }
                        continue;
                    }
                    ledge_count += lvs.len();
                    redge_count += rvs.len();
                    // Vertices in `rvs` not in `lvs` indicate missing edges.
                    let mut dests: std::collections::BTreeSet<u64> =
                        std::collections::BTreeSet::new();
                    dests.extend(rvs.iter().filter(|&v| !lvs.contains(v)));
                    add_edges(&mut missing, lk, &dests);
                    // Vertices in `lvs` not in `rvs` indicate extra edges.
                    dests.clear();
                    dests.extend(lvs.iter().filter(|&v| !rvs.contains(v)));
                    add_edges(&mut extra, lk, &dests);
                    lro = lrows.next();
                    rro = rrows.next();
                    lproc += 1;
                    rproc += 1;
                    if bar.is_some() {
                        bar.as_ref().unwrap().inc(1)
                    }; // No matter whether `l` or `r` has more keys, progress was made.
                }
                (None, None) => panic!("The impossible happened!"),
            }
        }
        if let Some(bar) = bar {
            bar.finish_and_clear()
        };
        let digits = (u64::MAX as f64).log10() as usize + 1;
        num_edges_missing = missing.len();
        println!("\nResults\n=======");
        println!("Processed {}/{} of nodes in 'l'", lproc, lnum_keys);
        println!("Processed {}/{} of nodes in 'r'", rproc, rnum_keys);
        println!("Edges in 'l': {}", ledge_count);
        println!("Edges in 'r': {}", redge_count);
        println!("Edges in 'r' missing in 'l' (there are {}):", missing.len());
        for (key, dst) in missing {
            println!("  {key:>width$}  {}", dst, key = key, width = digits);
        }
        println!("Edges in 'l' missing in 'r' (there are {}):", extra.len());
        for (key, dst) in extra {
            println!("  {key:>width$}  {}", dst, key = key, width = digits);
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

use structopt::StructOpt;
#[derive(Debug, structopt::StructOpt)]
#[structopt(name = "options", about = "Allow options")]
struct Opt {
    #[structopt(long = "with-progress-bar", help = "Enable graphical output")]
    with_progress_bar: bool,

    #[structopt(long = "dump32", help = "Render 32-bit depgraph as text")]
    dump32: Option<String>,

    #[structopt(
        long = "compare32",
        help = "Compare two 32-bit depgraphs",
        min_values = 2,
        max_values = 2
    )]
    compare32: Option<Vec<String>>,

    #[structopt(long = "dump64", help = "Render 64-bit depgraph as text")]
    dump64: Option<String>,

    #[structopt(
        long = "compare64",
        help = "Compare two 64-bit depgraphs",
        min_values = 2,
        max_values = 2
    )]
    compare64: Option<Vec<String>>,
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    let opt = Opt::from_args();
    let no_progress_bar = !opt.with_progress_bar;
    match match (opt.dump32, opt.compare32, opt.dump64, opt.compare64) {
        (Some(file), _, _, _) => dump_depgraph32(&file),
        (None, Some(files), _, _) => comp_depgraph32(no_progress_bar, &files[0], &files[1]),
        (None, None, Some(file), _) => dump_depgraph64(&file),
        (None, None, None, Some(files)) => comp_depgraph64(no_progress_bar, &files[0], &files[1]),
        _ => Ok(()),
    } {
        Ok(x) => Ok(x),
        Err(e) => Err(Box::new(e)),
    }
}
