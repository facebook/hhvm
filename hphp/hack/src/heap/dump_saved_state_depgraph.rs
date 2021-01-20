// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.;

use structopt::StructOpt;

#[derive(Debug)]
enum Error {
    RusqliteError(rusqlite::Error),
    Other(String),
}
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Error::RusqliteError(ref e) => ::std::fmt::Display::fmt(e, f),
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
type Result<T> = std::result::Result<T, Error>;

// Retrieve the `key_vertex` column of a row.
fn key_vertex<'r>(row: &'r rusqlite::Row) -> Result<u32> {
    Ok(row.get(0)?)
}

// Retrieve the `value_vertex` column of a row.
fn value_vertex<'r>(row: &'r rusqlite::Row) -> Result<std::collections::BTreeSet<u32>> {
    let data: Vec<u8> = row.get(1)?;
    Ok(data
        .chunks_exact(std::mem::size_of::<u32>())
        .map(|slice| match *slice {
            [a, b, c, d] => u32::from_le_bytes([a, b, c, d]),
            _ => panic!("chunks_exact emitted wrong chunk size"),
        })
        .collect())
}

// Count the number of nodes in a depgraph.
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

// Print an ASCII representation of a depgraph to stdout.
fn dump_depgraph(file: &str) -> Result<()> {
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

// Add edges to `es` given source vertex `k` and dest vertices `vs`.
fn add_edges(es: &mut Vec<(u32, u32)>, k: u32, vs: &std::collections::BTreeSet<u32>) {
    es.extend(vs.iter().map(|&v| (k, v)));
}

// Calculate the edges in `rfile` not in `lfile` (missing edges) and
// the edges in `lfile` not in `rfile` (extraneous edges).
fn comp_depgraph(lfile: &str, rfile: &str) -> Result<()> {
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
    let bar = indicatif::ProgressBar::new(std::cmp::max(lnum_keys as u64, rnum_keys as u64));
    bar.println("Comparing graphs. Patience...");
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
                if rnum_keys > lnum_keys {
                    bar.inc(1); // We advanced `r` and there are more keys in `r` than `l`.
                }
            }
            (Some(lrow), None) => {
                let k = key_vertex(lrow)?;
                let vs = value_vertex(lrow)?;
                ledge_count += vs.len();
                add_edges(&mut extra, k, &vs);
                lro = lrows.next()?;
                lproc += 1;
                if lnum_keys > rnum_keys {
                    bar.inc(1); // We advanced `l` and there are more keys in `l` than `r`.
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
                    if lnum_keys >= rnum_keys {
                        bar.inc(1); // We advanced `l` and there are more keys in `l` than `r`.
                    }
                    continue;
                }
                if lk > rk {
                    // These edges are in `r` but not in `l`.
                    redge_count += rvs.len();
                    add_edges(&mut missing, rk, &rvs);
                    rro = rrows.next()?;
                    rproc += 1;
                    if rnum_keys > lnum_keys {
                        bar.inc(1); // We advanced `r` and there are more keys in `r` than `l`.
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
                bar.inc(1); // No matter whether `l` or `r` has more keys, progress was made.
            }
            (None, None) => panic!("The impossible happened!"),
        }
    }
    bar.finish_and_clear();
    let digits = (u32::MAX as f32).log10() as usize + 1;
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
}

#[derive(Debug, structopt::StructOpt)]
#[structopt(name = "options", about = "Allow options")]
struct Opt {
    #[structopt(short = "d", long = "dump", help = "Render depgraph as text")]
    dump: Option<String>,
    #[structopt(
        short = "c",
        long = "compare",
        help = "Compare two 32-bit SQLite depgraphs",
        min_values = 2,
        max_values = 2
    )]
    compare: Option<Vec<String>>,
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    // Coerce an `Result` into a value of `main`'s return type.
    fn lift_result<T>(src: Result<T>) -> std::result::Result<T, Box<dyn std::error::Error>> {
        match src {
            Ok(x) => Ok(x),
            Err(e) => Err(Box::new(e)),
        }
    }
    let opt = Opt::from_args();
    lift_result(match (opt.dump, opt.compare) {
        (Some(file), _) => dump_depgraph(&file),
        (None, Some(files)) => comp_depgraph(&files[0], &files[1]),
        (None, None) => Ok(()),
    })
}
