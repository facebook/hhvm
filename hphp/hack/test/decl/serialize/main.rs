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
use oxidized_by_ref::direct_decl_parser::Decls;
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
            .map_or(false, |e| e == "php")
    }) {
        let entry = f?;
        let path = entry.path();
        let content = read_file(path)?;
        let relative_path = RelativePath::make(relative_path::Prefix::Dummy, path.to_path_buf());

        let arena = bumpalo::Bump::new();
        let parsed_file = direct_decl_parser::parse_decls_for_typechecking(
            &Default::default(),
            relative_path,
            &content,
            &arena,
        );
        let decls = parsed_file.decls;

        results.push(round_trip::<Decls<'_>, Json>(&arena, path, decls));
        results.push(round_trip::<Decls<'_>, FlexBuffer>(&arena, path, decls));
        results.push(round_trip::<Decls<'_>, Bincode>(&arena, path, decls));
        results.push(round_trip::<Decls<'_>, Cbor>(&arena, path, decls));
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
        if m.get(p.provider).is_none() {
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

fn round_trip<'a, X, P: Provider>(
    arena: &'a bumpalo::Bump,
    filepath: &Path,
    x: X,
) -> Result<Profile, Error>
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
    let y = P::de(&arena, data).map_err(mk_err)?;
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
    fn de<'a, X: serde::Deserialize<'a>>(
        arena: &'a bumpalo::Bump,
        data: Self::Data,
    ) -> Result<X, String>;
    fn name() -> &'static str;
    fn get_bytes(data: &Self::Data) -> &[u8];
}

struct Json;

impl Provider for Json {
    type Data = String;

    fn name() -> &'static str {
        "serde_json"
    }

    fn se<X: serde::Serialize>(x: &X) -> Result<Self::Data, String> {
        serde_json::to_string(x)
            .map_err(|e| format!("{} failed to serialize, error: {}", Self::name(), e))
    }

    fn de<'a, X: serde::Deserialize<'a>>(
        arena: &'a bumpalo::Bump,
        data: Self::Data,
    ) -> Result<X, String> {
        let mut de = serde_json::Deserializer::from_str(&data);
        let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
        X::deserialize(de)
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))
    }

    fn get_bytes(data: &Self::Data) -> &[u8] {
        data.as_bytes()
    }
}

struct FlexBuffer;

impl Provider for FlexBuffer {
    type Data = flexbuffers::FlexbufferSerializer;

    fn name() -> &'static str {
        "flexbuffers"
    }

    fn se<X: serde::Serialize>(x: &X) -> Result<Self::Data, String> {
        let mut s = flexbuffers::FlexbufferSerializer::new();
        x.serialize(&mut s)
            .map_err(|e| format!("{} failed to serialize, error: {}", Self::name(), e))?;
        Ok(s)
    }

    fn de<'a, X: serde::Deserialize<'a>>(
        arena: &'a bumpalo::Bump,
        data: Self::Data,
    ) -> Result<X, String> {
        let de = flexbuffers::Reader::get_root(data.view()).map_err(|e| {
            format!(
                "{} failed to create deserializer, message {}",
                Self::name(),
                e
            )
        })?;

        let de = arena_deserializer::ArenaDeserializer::new(arena, de);
        X::deserialize(de)
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))
    }

    fn get_bytes(data: &Self::Data) -> &[u8] {
        data.view()
    }
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

    fn de<'a, X: serde::Deserialize<'a>>(
        arena: &'a bumpalo::Bump,
        data: Self::Data,
    ) -> Result<X, String> {
        let op = bincode::config::Options::with_native_endian(bincode::options());
        let mut de = bincode::de::Deserializer::from_slice(&data, op);

        let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
        X::deserialize(de)
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
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))?;
        Ok(data)
    }

    fn de<'a, X: serde::Deserialize<'a>>(
        arena: &'a bumpalo::Bump,
        data: Self::Data,
    ) -> Result<X, String> {
        let mut de = serde_cbor::de::Deserializer::from_reader(data.as_slice());

        let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
        X::deserialize(de)
            .map_err(|e| format!("{} failed to deserialize, error: {}", Self::name(), e))
    }

    fn get_bytes(data: &Self::Data) -> &[u8] {
        data.as_slice()
    }
}
