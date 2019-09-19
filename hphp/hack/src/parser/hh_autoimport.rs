// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use lazy_static::lazy_static;
use std::collections::HashMap;
use Alias::*;

enum Alias {
    HhOnlyType(String),
    ScalarType(String),
    HhAlias(String, String),
}

lazy_static! {
    static ref ALIASES: Vec<Alias> = vec![
        HhOnlyType("AsyncIterator".to_string()),
        HhOnlyType("AsyncKeyedIterator".to_string()),
        HhOnlyType("Traversable".to_string()),
        HhOnlyType("Container".to_string()),
        HhOnlyType("KeyedTraversable".to_string()),
        HhOnlyType("KeyedContainer".to_string()),
        HhOnlyType("Iterator".to_string()),
        HhOnlyType("KeyedIterator".to_string()),
        HhOnlyType("Iterable".to_string()),
        HhOnlyType("KeyedIterable".to_string()),
        HhOnlyType("Collection".to_string()),
        HhOnlyType("Vector".to_string()),
        HhOnlyType("Map".to_string()),
        HhOnlyType("Set".to_string()),
        HhOnlyType("Pair".to_string()),
        HhOnlyType("ImmVector".to_string()),
        HhOnlyType("ImmMap".to_string()),
        HhOnlyType("ImmSet".to_string()),
        HhOnlyType("InvariantException".to_string()),
        HhOnlyType("IMemoizeParam".to_string()),
        HhOnlyType("Shapes".to_string()),
        HhOnlyType("TypeStructureKind".to_string()),
        HhOnlyType("TypeStructure".to_string()),
        HhOnlyType("dict".to_string()),
        HhOnlyType("vec".to_string()),
        HhOnlyType("keyset".to_string()),
        HhOnlyType("varray".to_string()),
        HhOnlyType("darray".to_string()),
        HhOnlyType("Awaitable".to_string()),
        HhOnlyType("AsyncGenerator".to_string()),
        HhOnlyType("StaticWaitHandle".to_string()),
        HhOnlyType("WaitableWaitHandle".to_string()),
        HhOnlyType("ResumableWaitHandle".to_string()),
        HhOnlyType("AsyncFunctionWaitHandle".to_string()),
        HhOnlyType("AsyncGeneratorWaitHandle".to_string()),
        HhOnlyType("AwaitAllWaitHandle".to_string()),
        HhOnlyType("ConditionWaitHandle".to_string()),
        HhOnlyType("RescheduleWaitHandle".to_string()),
        HhOnlyType("SleepWaitHandle".to_string()),
        HhOnlyType("ExternalThreadEventWaitHandle".to_string()),
        ScalarType("bool".to_string()),
        ScalarType("int".to_string()),
        ScalarType("float".to_string()),
        ScalarType("string".to_string()),
        ScalarType("void".to_string()),
        HhOnlyType("num".to_string()),
        HhOnlyType("arraykey".to_string()),
        HhOnlyType("resource".to_string()),
        HhOnlyType("mixed".to_string()),
        HhOnlyType("noreturn".to_string()),
        HhOnlyType("this".to_string()),
        HhOnlyType("varray_or_darray".to_string()),
        HhOnlyType("vec_or_dict".to_string()),
        HhOnlyType("arraylike".to_string()),
        HhOnlyType("nonnull".to_string()),
        HhOnlyType("null".to_string()),
        HhOnlyType("nothing".to_string()),
        HhOnlyType("dynamic".to_string()),
        HhAlias("classname".to_string(), "string".to_string()),
        HhAlias("typename".to_string(), "string".to_string()),
        HhAlias("boolean".to_string(), "bool".to_string()),
        HhAlias("integer".to_string(), "int".to_string()),
        HhAlias("double".to_string(), "float".to_string()),
        HhAlias("real".to_string(), "float".to_string()),
        /*
        PHP7_TYPE("Throwable"; PHP7_EngineExceptions);
        PHP7_TYPE("Error"; PHP7_EngineExceptions);
        PHP7_TYPE("ArithmeticError"; PHP7_EngineExceptions);
        PHP7_TYPE("AssertionError"; PHP7_EngineExceptions);
        PHP7_TYPE("DivisionByZeroError"; PHP7_EngineExceptions);
        PHP7_TYPE("ParseError"; PHP7_EngineExceptions);
        PHP7_TYPE("TypeError"; PHP7_EngineExceptions);
        */
    ];

    static ref ALIAS_MAP: HashMap<String, String> = {
        ALIASES.iter().fold(HashMap::new(), |mut map, hh_alias| {
            match hh_alias {
                ScalarType(s) | HhOnlyType(s) => {
                    map.insert(s.to_lowercase(), "HH\\".to_string() + s);
                }
                HhAlias(s, alias) => {
                    map.insert(s.to_lowercase(), alias.to_string());
                }
            }
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
    match &s.to_lowercase()[..] {
        "callable" => Some("callable"),
        "array" => Some("array"),
        lower => match ALIAS_MAP.get(lower) {
            Some(v) => Some(normalize(v)),
            _ => None,
        },
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
        assert_eq!(normalize("double"), "HH\\float");
        assert_eq!(normalize("non-exisit"), "non-exisit");
    }

    #[test]
    fn test_opt_normalize() {
        assert_eq!(opt_normalize("float"), Some("HH\\float"));
        assert_eq!(opt_normalize("KeyedIterable"), Some("HH\\KeyedIterable"));
        assert_eq!(opt_normalize("double"), Some("HH\\float"));
        assert_eq!(opt_normalize("callable"), Some("callable"));
        assert_eq!(opt_normalize("non-exisit"), None);
    }

}
