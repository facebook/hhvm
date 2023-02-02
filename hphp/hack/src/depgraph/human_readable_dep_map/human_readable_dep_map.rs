// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::path::Path;

use depgraph_reader::Dep;
use hash::DashMap;
use typing_deps_hash::DepType;

/// A map of `Dep`s to a human readable name.
/// E.g. 9f886cce32ff8ed -> Type Child A
#[derive(Default)]
pub struct HumanReadableDepMap {
    map: DashMap<Dep, (DepType, String)>,
}

impl HumanReadableDepMap {
    pub fn new() -> Self {
        Default::default()
    }

    pub fn contains(&self, dep: Dep) -> bool {
        self.map.contains_key(&dep)
    }

    /// Parse a text file where each line has the format:
    /// DEP_HASH DEP_TYPE SYMBOL_NAME
    /// For example:
    ///
    /// 718470156085360007 Type ChildB
    /// 1154245141631872205 Type Base
    /// 8150603439003883592 Constructor Base
    /// 1154245141631872204 Extends Base
    /// 718472355108616429 Type ChildA
    pub fn load(&self, path: &Path) -> anyhow::Result<()> {
        for line in BufReader::new(File::open(path)?).lines() {
            let line = line?;
            let mut parts = line.split(' ');
            let hash: u64 = match parts.next() {
                Some(s) => s.parse()?,
                None => anyhow::bail!("expected hash"),
            };
            let kind: DepType = match parts.next() {
                Some(s) => match s {
                    "GConst" => DepType::GConst,
                    "Fun" => DepType::Fun,
                    "Type" => DepType::Type,
                    "Extends" => DepType::Extends,
                    "Const" => DepType::Const,
                    "Constructor" => DepType::Constructor,
                    "Prop" => DepType::Prop,
                    "SProp" => DepType::SProp,
                    "Method" => DepType::Method,
                    "SMethod" => DepType::SMethod,
                    "AllMembers" => DepType::AllMembers,
                    "GConstName" => DepType::GConstName,
                    "Module" => DepType::Module,
                    bad => anyhow::bail!("unexpected DepType {}", bad),
                },
                None => anyhow::bail!("expected DepType"),
            };
            let sym: &str = match parts.next() {
                Some(s) => s,
                None => anyhow::bail!("expected symbol"),
            };
            match self.map.insert(Dep::new(hash), (kind, sym.into())) {
                Some((old_kind, old_sym)) if old_kind != kind || old_sym != sym => {
                    println!(
                        "{}: collision: ({:?},{}) != ({:?},{})",
                        hash, old_kind, old_sym, kind, sym
                    );
                }
                _ => {}
            }
        }
        Ok(())
    }

    /// Get the human readable name of a `Dep`. If the human readable name isn't
    /// found, simply return the `Dep` hash.
    pub fn fmt(&self, dep: Dep) -> String {
        match self.map.get(&dep) {
            Some(e) => {
                let (kind, sym) = &*e;
                format!("{kind:?} {sym}")
            }
            None => {
                format!("{dep:016x} ({dep})")
            }
        }
    }
}
