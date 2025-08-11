// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs::File;
use std::io::Read;
use std::path::Path;
use std::path::PathBuf;

use ::anyhow::Context;
use ::anyhow::Result;
use clap::Parser;
use oxidized::direct_decl_parser::Decls;
use relative_path::RelativePath;
use serde::Deserialize;
use serde::Serialize;
use walkdir::WalkDir;

#[derive(Parser, Clone, Debug)]
pub struct Opts {
    path: PathBuf,
}

fn main() -> ::anyhow::Result<()> {
    let opts = Opts::try_parse()?;
    let mut path = std::env::current_dir().expect("current path is none");
    path.push(opts.path);
    let mut results: Vec<Result<Profile, Error>> = vec![];

    for f in WalkDir::new(path).into_iter().filter(|e| {
        e.as_ref()
            .expect("expect file path")
            .path()
            .extension()
            .is_some_and(|e| e == "php")
    }) {
        let entry = f?;
        let path = entry.path();
        let content = read_file(path)?;
        let relative_path = RelativePath::make(relative_path::Prefix::Dummy, path.to_path_buf());

        let parsed_file = direct_decl_parser::parse_decls_for_bytecode(
            &Default::default(),
            relative_path,
            &content,
        );
        let decls = parsed_file.decls;

        results.push(round_trip::<Decls, Bincode>(path, decls.clone()));
        results.push(round_trip::<Decls, Cbor>(path, decls));
    }

    let (profiles, errs) = results
        .into_iter()
        .fold((vec![], vec![]), |(mut pr, mut er), r| {
            match r {
                Ok(p) => pr.push(p),
                Err(e) => er.push(e),
            };
            (pr, er)
        });

    if !errs.is_empty() {
        errs.iter().for_each(|e| println!("Faild: {:#?}", e));
        panic!("mismatch")
    }

    use std::collections::HashMap;

    let aggrate_by_provider = profiles.into_iter().fold(HashMap::new(), |mut m, p| {
        if !m.contains_key(p.provider) {
            m.insert(
                p.provider,
                Profile {
                    provider: p.provider,
                    filepath: PathBuf::new(),
                    se_in_ns: 0,
                    de_in_ns: 0,
                    data_size_in_bytes: 0,
                },
            );
        } else {
            let agg = m.get_mut(p.provider).unwrap();
            agg.se_in_ns += p.se_in_ns;
            agg.de_in_ns += p.de_in_ns;
            agg.data_size_in_bytes += p.data_size_in_bytes;
        }
        m
    });
    println!("Profiling data: {:#?}\n", aggrate_by_provider);
    Ok(())
}

fn read_file(filepath: &Path) -> Result<Vec<u8>> {
    let mut text: Vec<u8> = Vec::new();
    File::open(filepath)
        .with_context(|| format!("cannot open input file: {}", filepath.display()))?
        .read_to_end(&mut text)?;
    Ok(text)
}

fn round_trip<'a, X, P>(filepath: &Path, x: X) -> Result<Profile, Error>
where
    X: Deserialize<'a> + Serialize + Eq + std::fmt::Debug,
    P: Provider,
{
    use std::time::SystemTime;

    let provider = P::name();
    let mk_err = |msg| Error {
        provider,
        filepath: filepath.into(),
        msg,
    };

    let s = SystemTime::now();
    let data = P::se(&x).map_err(mk_err)?;
    let se_in_ns = SystemTime::now().duration_since(s).unwrap().as_nanos();

    let data_size_in_bytes = P::get_bytes(&data).len();

    let s = SystemTime::now();
    let y = P::de(data).map_err(mk_err)?;
    let de_in_ns = SystemTime::now().duration_since(s).unwrap().as_nanos();

    if x == y {
        Ok(Profile {
            provider: P::name(),
            filepath: filepath.into(),
            se_in_ns,
            de_in_ns,
            data_size_in_bytes,
        })
    } else {
        Err(mk_err("not equal".into()))
    }
}

#[derive(Debug)]
struct Profile {
    provider: &'static str,
    #[allow(dead_code)]
    filepath: PathBuf,
    se_in_ns: u128,
    de_in_ns: u128,
    data_size_in_bytes: usize,
}

#[derive(Debug)]
struct Error {
    #[allow(dead_code)]
    provider: &'static str,
    #[allow(dead_code)]
    filepath: PathBuf,
    #[allow(dead_code)]
    msg: String,
}

trait Provider {
    type Data;

    fn se<X: serde::Serialize>(x: &X) -> Result<Self::Data, String>;
    fn de<'a, X: serde::Deserialize<'a>>(data: Self::Data) -> Result<X, String>;
    fn name() -> &'static str;
    fn get_bytes(data: &Self::Data) -> &[u8];
}

struct Bincode;

impl Provider for Bincode {
    type Data = Vec<u8>;

    fn name() -> &'static str {
        "bincode"
    }

    fn se<X: serde::Serialize>(x: &X) -> Result<Self::Data, String> {
        use bincode::Options;
        let op = bincode::config::Options::with_native_endian(bincode::options());
        op.serialize(x)
            .map_err(|e| format!("{} failed to serialize, error: {}", Self::name(), e))
    }

    fn de<'a, X: serde::Deserialize<'a>>(data: Self::Data) -> Result<X, String> {
        let op = bincode::config::Options::with_native_endian(bincode::options());
        let mut de = bincode::de::Deserializer::with_reader(&data[..], op);
        X::deserialize(&mut de)
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))
    }

    fn get_bytes(data: &Self::Data) -> &[u8] {
        data.as_slice()
    }
}

struct Cbor;

impl Provider for Cbor {
    type Data = Vec<u8>;

    fn name() -> &'static str {
        "cbor"
    }

    fn se<X: serde::Serialize>(x: &X) -> Result<Self::Data, String> {
        let mut data = vec![];
        serde_cbor::to_writer(&mut data, x)
            .map_err(|e| format!("{} failed to serialize, error: {}", Self::name(), e))?;
        Ok(data)
    }

    fn de<'a, X: serde::Deserialize<'a>>(data: Self::Data) -> Result<X, String> {
        let mut de = serde_cbor::de::Deserializer::from_reader(data.as_slice());

        X::deserialize(&mut de)
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))
    }

    fn get_bytes(data: &Self::Data) -> &[u8] {
        data.as_slice()
    }
}
