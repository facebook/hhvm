// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::tast_hashes::ByNames;
use crate::tast_hashes::TastHashes;

impl ByNames {
    pub fn merge(&mut self, other: &mut Self) {
        let Self {
            fun_tast_hashes,
            class_tast_hashes,
            typedef_tast_hashes,
            gconst_tast_hashes,
            module_tast_hashes,
        } = self;
        fun_tast_hashes.append(&mut other.fun_tast_hashes);
        class_tast_hashes.append(&mut other.class_tast_hashes);
        typedef_tast_hashes.append(&mut other.typedef_tast_hashes);
        gconst_tast_hashes.append(&mut other.gconst_tast_hashes);
        module_tast_hashes.append(&mut other.module_tast_hashes);
    }
}

pub fn merge_tast_hashes(x: &mut TastHashes, y: &mut TastHashes) {
    for (path, hashes) in y {
        x.entry(path.clone()).or_default().merge(hashes);
    }
}

#[cfg(test)]
mod tests {
    use relative_path::Prefix;
    use relative_path::RelativePath;

    use super::*;
    use crate::s_map::SMap;

    #[test]
    fn test_merge() {
        let mut a = ByNames {
            fun_tast_hashes: SMap::from([("f1".to_string(), 1), ("f0".to_string(), 2)]),
            class_tast_hashes: SMap::new(),
            typedef_tast_hashes: SMap::from([("f1".to_string(), 1)]),
            gconst_tast_hashes: SMap::from([("f1".to_string(), 1)]),
            module_tast_hashes: SMap::from([("f1".to_string(), 1)]),
        };
        let mut b = ByNames {
            fun_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
            class_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
            typedef_tast_hashes: SMap::new(),
            gconst_tast_hashes: SMap::from([("f2".to_string(), 1)]),
            module_tast_hashes: SMap::from([("f1".to_string(), 2)]),
        };
        a.merge(&mut b);
        assert_eq!(
            a,
            ByNames {
                fun_tast_hashes: SMap::from([
                    ("f1".to_string(), 2),
                    ("f0".to_string(), 2),
                    ("f2".to_string(), 3)
                ]),
                class_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
                typedef_tast_hashes: SMap::from([("f1".to_string(), 1)]),
                gconst_tast_hashes: SMap::from([("f1".to_string(), 1), ("f2".to_string(), 1)]),
                module_tast_hashes: SMap::from([("f1".to_string(), 2)]),
            }
        );
    }

    #[test]
    fn test_merge_tast_hashes() {
        let by_names1 = ByNames {
            fun_tast_hashes: SMap::from([("f1".to_string(), 1), ("f0".to_string(), 2)]),
            class_tast_hashes: SMap::new(),
            typedef_tast_hashes: SMap::from([("f1".to_string(), 1)]),
            gconst_tast_hashes: SMap::from([("f1".to_string(), 1)]),
            module_tast_hashes: SMap::from([("f1".to_string(), 1)]),
        };
        let by_names2 = ByNames {
            fun_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
            class_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
            typedef_tast_hashes: SMap::new(),
            gconst_tast_hashes: SMap::from([("f2".to_string(), 1)]),
            module_tast_hashes: SMap::from([("f1".to_string(), 2)]),
        };
        let merged = ByNames {
            fun_tast_hashes: SMap::from([
                ("f1".to_string(), 2),
                ("f0".to_string(), 2),
                ("f2".to_string(), 3),
            ]),
            class_tast_hashes: SMap::from([("f1".to_string(), 2), ("f2".to_string(), 3)]),
            typedef_tast_hashes: SMap::from([("f1".to_string(), 1)]),
            gconst_tast_hashes: SMap::from([("f1".to_string(), 1), ("f2".to_string(), 1)]),
            module_tast_hashes: SMap::from([("f1".to_string(), 2)]),
        };
        let file1 = RelativePath::make(Prefix::Dummy, "file1".into());
        let file2 = RelativePath::make(Prefix::Dummy, "file2".into());
        let file3 = RelativePath::make(Prefix::Dummy, "file3".into());
        let mut x = TastHashes::from([
            (file1.clone(), by_names1.clone()),
            (file2.clone(), by_names1.clone()),
        ]);
        let mut y = TastHashes::from([
            (file1.clone(), by_names2.clone()),
            (file3.clone(), by_names2.clone()),
        ]);
        let expected = TastHashes::from([
            (file1.clone(), merged),
            (file2.clone(), by_names1.clone()),
            (file3.clone(), by_names2.clone()),
        ]);
        merge_tast_hashes(&mut x, &mut y);
        assert_eq!(x, expected);
    }
}
