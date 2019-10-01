// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use lazy_static::lazy_static;
use std::collections::HashMap;

lazy_static! {
    static ref ALIASES: Vec<String> = vec![
        "AsyncIterator".to_string(),
        "AsyncKeyedIterator".to_string(),
        "Traversable".to_string(),
        "Container".to_string(),
        "KeyedTraversable".to_string(),
        "KeyedContainer".to_string(),
        "Iterator".to_string(),
        "KeyedIterator".to_string(),
        "Iterable".to_string(),
        "KeyedIterable".to_string(),
        "Collection".to_string(),
        "Vector".to_string(),
        "Map".to_string(),
        "Set".to_string(),
        "Pair".to_string(),
        "ImmVector".to_string(),
        "ImmMap".to_string(),
        "ImmSet".to_string(),
        "InvariantException".to_string(),
        "IMemoizeParam".to_string(),
        "Shapes".to_string(),
        "TypeStructureKind".to_string(),
        "TypeStructure".to_string(),
        "dict".to_string(),
        "vec".to_string(),
        "keyset".to_string(),
        "varray".to_string(),
        "darray".to_string(),
        "Awaitable".to_string(),
        "AsyncGenerator".to_string(),
        "StaticWaitHandle".to_string(),
        "WaitableWaitHandle".to_string(),
        "ResumableWaitHandle".to_string(),
        "AsyncFunctionWaitHandle".to_string(),
        "AsyncGeneratorWaitHandle".to_string(),
        "AwaitAllWaitHandle".to_string(),
        "ConditionWaitHandle".to_string(),
        "RescheduleWaitHandle".to_string(),
        "SleepWaitHandle".to_string(),
        "ExternalThreadEventWaitHandle".to_string(),
        "bool".to_string(),
        "int".to_string(),
        "float".to_string(),
        "string".to_string(),
        "void".to_string(),
        "num".to_string(),
        "arraykey".to_string(),
        "resource".to_string(),
        "mixed".to_string(),
        "noreturn".to_string(),
        "this".to_string(),
        "varray_or_darray".to_string(),
        "vec_or_dict".to_string(),
        "arraylike".to_string(),
        "nonnull".to_string(),
        "null".to_string(),
        "nothing".to_string(),
        "dynamic".to_string(),
    ];
    static ref ALIAS_MAP: HashMap<String, String> = {
        ALIASES.iter().fold(HashMap::new(), |mut map, s| {
            map.insert(s.to_lowercase(), "HH\\".to_string() + s);
            map
        })
    };
}

pub fn normalize(s: &str) -> &str {
    match ALIAS_MAP.get(&s.to_lowercase()[..]) {
        None => s,
        Some(alias) => normalize(alias),
    }
}

pub fn opt_normalize(s: &str) -> Option<&str> {
    match ALIAS_MAP.get(&s.to_lowercase()[..]) {
        Some(v) => Some(normalize(v)),
        None => None,
    }
}

pub fn is_hh_autoimport(s: &str) -> bool {
    ALIAS_MAP.contains_key(&s.to_lowercase()[..])
}

#[cfg(test)]
mod tests {
    use crate::hh_autoimport::is_hh_autoimport;
    use crate::hh_autoimport::normalize;
    use crate::hh_autoimport::opt_normalize;

    #[test]
    fn test_is_hh_autoimport() {
        assert_eq!(is_hh_autoimport("float"), true);
        assert_eq!(is_hh_autoimport("KeyedIterable"), true);
        assert_eq!(is_hh_autoimport("non-exisit"), false);
    }

    #[test]
    fn test_normalize() {
        assert_eq!(normalize("float"), "HH\\float");
        assert_eq!(normalize("KeyedIterable"), "HH\\KeyedIterable");
        assert_eq!(normalize("non-exisit"), "non-exisit");
    }

    #[test]
    fn test_opt_normalize() {
        assert_eq!(opt_normalize("float"), Some("HH\\float"));
        assert_eq!(opt_normalize("KeyedIterable"), Some("HH\\KeyedIterable"));
        assert_eq!(opt_normalize("non-exisit"), None);
    }

}
