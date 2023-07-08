// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;
use std::hash::Hash;

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::ser::SerializeSeq;
use serde::Deserialize;
use serde::Serialize;
use serde::Serializer;

#[derive(Clone, PartialEq, Eq, Hash, Ord, PartialOrd)]
#[derive(Serialize, Deserialize, FromOcamlRep, ToOcamlRep)]
#[derive(EqModuloPos, NoPosHash)]
pub struct SortedAssocList<K: Serialize + Ord + Hash, V: Serialize> {
    #[serde(serialize_with = "serialize_seq")] // TODO: deserialize_with
    entries: BTreeMap<K, V>,
}

fn serialize_seq<S, K, V>(map: &BTreeMap<K, V>, ser: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
    K: Serialize + Ord,
    V: Serialize,
{
    let mut seq = ser.serialize_seq(Some(map.len()))?;
    for e in map.iter() {
        seq.serialize_element(&e)?;
    }
    seq.end()
}

impl<K, V> IntoIterator for SortedAssocList<K, V>
where
    K: Serialize + Ord + Hash,
    V: Serialize,
{
    type Item = (K, V);
    type IntoIter = std::collections::btree_map::IntoIter<K, V>;

    fn into_iter(self) -> Self::IntoIter {
        self.entries.into_iter()
    }
}

impl<K, V> std::fmt::Debug for SortedAssocList<K, V>
where
    K: std::fmt::Debug + Serialize + Ord + Hash,
    V: std::fmt::Debug + Serialize,
{
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.entries.iter()).finish()
    }
}

impl<K, V> SortedAssocList<K, V>
where
    K: Serialize + Ord + Hash,
    V: Serialize,
{
    pub fn new() -> Self {
        Self {
            entries: Default::default(),
        }
    }

    /// Insert both key and value, even if an Ordering::Equal K is already present.
    /// This is significant if K has extra state not considered by its Ord implementation.
    pub fn insert(&mut self, k: K, v: V) {
        self.entries.remove(&k);
        self.entries.insert(k, v);
    }
}
