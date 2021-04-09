/*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/** Module consisting of the special names known to the typechecker */

pub mod classes {
    pub const PARENT: &str = "parent";

    pub const STATIC: &str = "static";

    pub const SELF: &str = "self";

    pub const UNKNOWN: &str = "\\*Unknown*";

    /* Used for dynamic classnames, e.g. new $foo(); */

    pub const AWAITABLE: &str = "\\HH\\Awaitable";

    pub const GENERATOR: &str = "\\Generator";

    pub const ASYNC_GENERATOR: &str = "\\HH\\AsyncGenerator";

    pub const HH_FORMAT_STRING: &str = "\\HH\\FormatString";

    pub fn is_format_string(x: &str) -> bool {
        match x {
            HH_FORMAT_STRING => true,
            _ => false,
        }
    }

    pub const HH_BUILTIN_ENUM: &str = "\\HH\\BuiltinEnum";

    pub const HH_BUILTIN_ENUM_CLASS: &str = "\\HH\\BuiltinEnumClass";

    pub const THROWABLE: &str = "\\Throwable";

    pub const STD_CLASS: &str = "\\stdClass";

    pub const DATE_TIME: &str = "\\DateTime";

    pub const DATE_TIME_IMMUTABLE: &str = "\\DateTimeImmutable";

    pub const ASYNC_ITERATOR: &str = "\\HH\\AsyncIterator";

    pub const ASYNC_KEYED_ITERATOR: &str = "\\HH\\AsyncKeyedIterator";

    pub const STRINGISH: &str = "\\Stringish";

    pub const XHP_CHILD: &str = "\\XHPChild";

    pub const IMEMOIZE_PARAM: &str = "\\HH\\IMemoizeParam";

    pub const CLASS_NAME: &str = "\\HH\\classname";

    pub const TYPE_NAME: &str = "\\HH\\typename";

    pub const IDISPOSABLE: &str = "\\IDisposable";

    pub const IASYNC_DISPOSABLE: &str = "\\IAsyncDisposable";

    pub const MEMBER_OF: &str = "\\HH\\MemberOf";
}

pub mod collections {
    /* concrete classes */
    pub const VECTOR: &str = "\\HH\\Vector";

    pub const IMM_VECTOR: &str = "\\HH\\ImmVector";

    pub const SET: &str = "\\HH\\Set";

    pub const IMM_SET: &str = "\\HH\\ImmSet";

    pub const MAP: &str = "\\HH\\Map";

    pub const IMM_MAP: &str = "\\HH\\ImmMap";

    pub const PAIR: &str = "\\HH\\Pair";

    /* interfaces */
    pub const CONTAINER: &str = "\\HH\\Container";

    pub const KEYED_CONTAINER: &str = "\\HH\\KeyedContainer";

    pub const TRAVERSABLE: &str = "\\HH\\Traversable";

    pub const KEYED_TRAVERSABLE: &str = "\\HH\\KeyedTraversable";

    pub const COLLECTION: &str = "\\Collection";

    pub const CONST_VECTOR: &str = "\\ConstVector";

    pub const CONST_MAP: &str = "\\ConstMap";

    pub const CONST_COLLECTION: &str = "\\ConstCollection";

    pub const DICT: &str = "\\HH\\dict";

    pub const VEC: &str = "\\HH\\vec";

    pub const KEYSET: &str = "\\HH\\keyset";
}

pub mod members {
    use hash::{HashMap, HashSet};
    use lazy_static::lazy_static;

    pub const M_CLASS: &str = "class";

    pub const __CONSTRUCT: &str = "__construct";

    pub const __DESTRUCT: &str = "__destruct";

    pub const __CALL: &str = "__call";

    pub const __CALL_STATIC: &str = "__callStatic";

    pub const __CLONE: &str = "__clone";

    pub const __DEBUG_INFO: &str = "__debugInfo";

    pub const __DISPOSE: &str = "__dispose";

    pub const __DISPOSE_ASYNC: &str = "__disposeAsync";

    pub const __GET: &str = "__get";

    pub const __INVOKE: &str = "__invoke";

    pub const __ISSET: &str = "__isset";

    pub const __SET: &str = "__set";

    pub const __SET_STATE: &str = "__set_state";

    pub const __SLEEP: &str = "__sleep";

    pub const __TO_STRING: &str = "__toString";

    pub const __UNSET: &str = "__unset";

    pub const __WAKEUP: &str = "__wakeup";

    lazy_static! {
        static ref AS_SET: HashSet<&'static str> = vec![
            __CONSTRUCT,
            __DESTRUCT,
            __CALL,
            __CALL_STATIC,
            __CLONE,
            __DEBUG_INFO,
            __DISPOSE,
            __DISPOSE_ASYNC,
            __GET,
            __INVOKE,
            __ISSET,
            __SET,
            __SET_STATE,
            __SLEEP,
            __TO_STRING,
            __UNSET,
            __WAKEUP
        ]
        .into_iter()
        .collect();
        pub static ref AS_LOWERCASE_SET: HashSet<String> = {
            AS_SET
                .iter()
                .fold(HashSet::<String>::default(), |mut set, special_name| {
                    set.insert(special_name.to_ascii_lowercase());
                    set
                })
        };
        pub static ref UNSUPPORTED_MAP: HashMap<String, &'static str> = {
            vec![__CALL, __CALL_STATIC, __GET, __ISSET, __SET, __UNSET]
                .iter()
                .fold(
                    HashMap::<String, &'static str>::default(),
                    |mut set, special_name| {
                        set.insert(special_name.to_ascii_lowercase(), special_name);
                        set
                    },
                )
        };
    }

    /* Any data- or aria- attribute is always valid, even if it is not declared
     * for a given XHP element */
    pub fn is_special_xhp_attribute(s: &str) -> bool {
        s.len() >= 6
            && match &s[..6] {
                ":data-" | ":aria-" => true,
                _ => false,
            }
    }
}

pub mod user_attributes {
    use lazy_static::lazy_static;
    use std::collections::HashSet;

    pub const OVERRIDE: &str = "__Override";

    pub const CONSISTENT_CONSTRUCT: &str = "__ConsistentConstruct";

    pub const CONST: &str = "__Const";

    pub const DEPRECATED: &str = "__Deprecated";

    pub const ENTRY_POINT: &str = "__EntryPoint";

    pub const MEMOIZE: &str = "__Memoize";

    pub const MEMOIZE_LSB: &str = "__MemoizeLSB";

    pub const PHP_STD_LIB: &str = "__PHPStdLib";

    pub const ACCEPT_DISPOSABLE: &str = "__AcceptDisposable";

    pub const RETURN_DISPOSABLE: &str = "__ReturnDisposable";

    pub const LSB: &str = "__LSB";

    pub const SEALED: &str = "__Sealed";

    pub const LATE_INIT: &str = "__LateInit";

    pub const NEWABLE: &str = "__Newable";

    pub const ENFORCEABLE: &str = "__Enforceable";

    pub const EXPLICIT: &str = "__Explicit";

    pub const SOFT: &str = "__Soft";

    pub const WARN: &str = "__Warn";

    pub const MOCK_CLASS: &str = "__MockClass";

    pub const PROVENANCE_SKIP_FRAME: &str = "__ProvenanceSkipFrame";

    pub const DYNAMICALLY_CALLABLE: &str = "__DynamicallyCallable";

    pub const DYNAMICALLY_CONSTRUCTIBLE: &str = "__DynamicallyConstructible";

    pub const REIFIABLE: &str = "__Reifiable";

    pub const NEVER_INLINE: &str = "__NEVER_INLINE";

    pub const ENABLE_UNSTABLE_FEATURES: &str = "__EnableUnstableFeatures";

    pub const ENUM_CLASS: &str = "__EnumClass";

    pub const ATOM: &str = "__Atom";

    pub const POLICIED: &str = "__Policied";

    pub const INFERFLOWS: &str = "__InferFlows";

    pub const EXTERNAL: &str = "__External";

    pub const SOUND_DYNAMIC_CALLABLE: &str = "__SoundDynamicCallable";

    lazy_static! {
        static ref AS_SET: HashSet<&'static str> = vec![
            OVERRIDE,
            CONSISTENT_CONSTRUCT,
            CONST,
            DEPRECATED,
            ENTRY_POINT,
            MEMOIZE,
            MEMOIZE_LSB,
            PHP_STD_LIB,
            ACCEPT_DISPOSABLE,
            RETURN_DISPOSABLE,
            LSB,
            SEALED,
            LATE_INIT,
            NEWABLE,
            ENFORCEABLE,
            EXPLICIT,
            SOFT,
            WARN,
            MOCK_CLASS,
            PROVENANCE_SKIP_FRAME,
            DYNAMICALLY_CALLABLE,
            DYNAMICALLY_CONSTRUCTIBLE,
            REIFIABLE,
            NEVER_INLINE,
            ENABLE_UNSTABLE_FEATURES,
            ENUM_CLASS,
            ATOM,
            POLICIED,
            INFERFLOWS,
            EXTERNAL,
            SOUND_DYNAMIC_CALLABLE,
        ]
        .into_iter()
        .collect();
    }

    pub fn is_memoized(name: &str) -> bool {
        name == MEMOIZE || name == MEMOIZE_LSB
    }

    // TODO(hrust) these should probably be added to the above map/fields, too

    pub fn is_native(name: &str) -> bool {
        name == "__Native"
    }

    pub fn is_foldable(name: &str) -> bool {
        name == "__IsFoldable"
    }

    pub fn is_meth_caller(name: &str) -> bool {
        name == "__MethCaller"
    }

    pub fn is_reserved(name: &str) -> bool {
        name.starts_with("__")
    }

    pub fn is_soft(name: &str) -> bool {
        name == SOFT
    }
}

pub mod attribute_kinds {
    use hash::HashMap;
    use lazy_static::lazy_static;

    pub const CLS: &str = "\\HH\\ClassAttribute";

    pub const ENUM: &str = "\\HH\\EnumAttribute";

    pub const TYPE_ALIAS: &str = "\\HH\\TypeAliasAttribute";

    pub const FN: &str = "\\HH\\FunctionAttribute";

    pub const MTHD: &str = "\\HH\\MethodAttribute";

    pub const INST_PROPERTY: &str = "\\HH\\InstancePropertyAttribute";

    pub const STATIC_PROPERTY: &str = "\\HH\\StaticPropertyAttribute";

    pub const PARAMETER: &str = "\\HH\\ParameterAttribute";

    pub const TYPE_PARAM: &str = "\\HH\\TypeParameterAttribute";

    pub const FILE: &str = "\\HH\\FileAttribute";

    pub const TYPE_CONST: &str = "\\HH\\TypeConstantAttribute";

    pub const LAMBDA: &str = "\\HH\\LambdaAttribute";

    pub const ENUM_CLS: &str = "\\HH\\EnumClassAttribute";

    pub static PLAIN_ENGLISH: &[(&str, &str)] = &[
        (CLS, "a class"),
        (ENUM, "an enum"),
        (TYPE_ALIAS, "a typealias"),
        (FN, "a function"),
        (MTHD, "a method"),
        (INST_PROPERTY, "an instance property"),
        (STATIC_PROPERTY, "a static property"),
        (PARAMETER, "a parameter"),
        (TYPE_PARAM, "a type parameter"),
        (FILE, "a file"),
        (TYPE_CONST, "a type constant"),
        (LAMBDA, "a lambda expression"),
        (ENUM_CLS, "an enum class"),
    ];

    lazy_static! {
        pub static ref PLAIN_ENGLISH_MAP: HashMap<&'static str, &'static str> =
            PLAIN_ENGLISH.iter().copied().collect();
    }
}

/* Tested before \\-prepending name-canonicalization */
pub mod special_functions {
    use lazy_static::lazy_static;

    pub const TUPLE: &str = "tuple"; /* pseudo-function */

    pub const ECHO: &str = "echo"; /* pseudo-function */

    pub const HHAS_ADATA: &str = "__hhas_adata";

    pub fn is_special_function(x: &str) -> bool {
        lazy_static! {
            static ref ALL_SPECIAL_FUNCTIONS: Vec<&'static str> =
                vec![TUPLE, ECHO, HHAS_ADATA,].into_iter().collect();
        }
        ALL_SPECIAL_FUNCTIONS.contains(&x)
    }
}

pub mod autoimported_functions {
    pub const INVARIANT: &str = "\\HH\\invariant";

    pub const INVARIANT_VIOLATION: &str = "\\HH\\invariant_violation";

    pub const FUN_: &str = "\\HH\\fun";

    pub const INST_METH: &str = "\\HH\\inst_meth";

    pub const CLASS_METH: &str = "\\HH\\class_meth";

    pub const METH_CALLER: &str = "\\HH\\meth_caller";
}

pub mod special_idents {
    pub const THIS: &str = "$this";

    pub const PLACEHOLDER: &str = "$_";

    pub const DOLLAR_DOLLAR: &str = "$$";

    /* Intentionally using an invalid variable name to ensure it's translated */
    pub const TMP_VAR_PREFIX: &str = "__tmp$";

    pub fn is_tmp_var(name: &str) -> bool {
        name.len() > 6 && &name.as_bytes()[..6] == TMP_VAR_PREFIX.as_bytes()
    }

    pub fn assert_tmp_var(name: &str) {
        assert!(is_tmp_var(name))
    }
}

pub mod pseudo_functions {
    use lazy_static::lazy_static;
    use std::collections::HashSet;

    pub const ISSET: &str = "\\isset";

    pub const UNSET: &str = "\\unset";

    pub const HH_SHOW: &str = "\\hh_show";

    pub const HH_SHOW_ENV: &str = "\\hh_show_env";

    pub const HH_LOG_LEVEL: &str = "\\hh_log_level";

    pub const HH_FORCE_SOLVE: &str = "\\hh_force_solve";

    pub const HH_LOOP_FOREVER: &str = "\\hh_loop_forever";

    pub const ECHO: &str = "\\echo";

    pub const ECHO_NO_NS: &str = "echo";

    pub const EMPTY: &str = "\\empty";

    pub const EXIT: &str = "\\exit";

    pub const DIE: &str = "\\die";

    pub static ALL_PSEUDO_FUNCTIONS: &[&str] = &[
        ISSET,
        UNSET,
        HH_SHOW,
        HH_SHOW_ENV,
        HH_LOG_LEVEL,
        HH_FORCE_SOLVE,
        HH_LOOP_FOREVER,
        ECHO,
        EMPTY,
        EXIT,
        DIE,
    ];

    lazy_static! {
        static ref PSEUDO_SET: HashSet<&'static str> =
            ALL_PSEUDO_FUNCTIONS.iter().copied().collect();
    }

    pub fn is_pseudo_function(x: &str) -> bool {
        PSEUDO_SET.contains(x)
    }
}

pub mod std_lib_functions {
    pub const IS_ARRAY: &str = "\\is_array";

    pub const IS_NULL: &str = "\\is_null";

    pub const GET_CLASS: &str = "\\get_class";

    pub const ARRAY_FILTER: &str = "\\array_filter";

    pub const ARRAY_MAP: &str = "\\array_map";

    pub const CALL_USER_FUNC: &str = "\\call_user_func";

    pub const TYPE_STRUCTURE: &str = "\\HH\\type_structure";

    pub const ARRAY_MARK_LEGACY: &str = "\\HH\\array_mark_legacy";

    pub const ARRAY_UNMARK_LEGACY: &str = "\\HH\\array_unmark_legacy";
}

pub mod typehints {
    use lazy_static::lazy_static;
    use std::collections::HashSet;

    pub const NULL: &str = "null";

    pub const VOID: &str = "void";

    pub const RESOURCE: &str = "resource";

    pub const NUM: &str = "num";

    pub const ARRAYKEY: &str = "arraykey";

    pub const NORETURN: &str = "noreturn";

    pub const MIXED: &str = "mixed";

    pub const NONNULL: &str = "nonnull";

    pub const THIS: &str = "this";

    pub const DYNAMIC: &str = "dynamic";

    pub const NOTHING: &str = "nothing";

    pub const INT: &str = "int";

    pub const BOOL: &str = "bool";

    pub const FLOAT: &str = "float";

    pub const STRING: &str = "string";

    pub const DARRAY: &str = "darray";

    pub const VARRAY: &str = "varray";

    pub const VARRAY_OR_DARRAY: &str = "varray_or_darray";

    pub const CALLABLE: &str = "callable";

    pub const OBJECT_CAST: &str = "object";

    pub const WILDCARD: &str = "_";

    pub fn is_reserved_type_hint(x: &str) -> bool {
        lazy_static! {
            static ref RESERVED_TYPEHINTS: HashSet<&'static str> = vec![
                NULL,
                VOID,
                RESOURCE,
                NUM,
                ARRAYKEY,
                NORETURN,
                MIXED,
                NONNULL,
                THIS,
                DYNAMIC,
                NOTHING,
                INT,
                BOOL,
                FLOAT,
                STRING,
                DARRAY,
                VARRAY,
                VARRAY_OR_DARRAY,
                CALLABLE,
                WILDCARD,
            ]
            .into_iter()
            .collect();
        }

        RESERVED_TYPEHINTS.contains(x)
    }

    lazy_static! {
        static ref RESERVED_GLOBAL_NAMES: HashSet<&'static str> =
            vec![CALLABLE, crate::classes::SELF, crate::classes::PARENT]
                .into_iter()
                .collect();
    }

    pub fn is_reserved_global_name(x: &str) -> bool {
        RESERVED_GLOBAL_NAMES.contains(x)
    }

    lazy_static! {
        static ref RESERVED_HH_NAMES: HashSet<&'static str> = vec![
            VOID, NORETURN, INT, BOOL, FLOAT, NUM, STRING, RESOURCE, MIXED, ARRAYKEY, DYNAMIC,
            WILDCARD, NULL, NONNULL, NOTHING, THIS
        ]
        .into_iter()
        .collect();
    }

    pub fn is_reserved_hh_name(x: &str) -> bool {
        RESERVED_HH_NAMES.contains(x)
    }

    // This function checks if this is a namespace of the "(not HH)\\(...)*\\(reserved_name)"
    pub fn is_namespace_with_reserved_hh_name(x: &str) -> bool {
        // This splits the string into its namespaces
        fn unqualify(x: &str) -> (Vec<&str>, &str) {
            let mut as_list = x.split('\\').collect::<Vec<&str>>();
            // Retain if not empty
            as_list.retain(|&split| match split {
                "" => false,
                _ => true,
            });
            let last_split = match as_list.pop() {
                None => "",
                Some(x) => x,
            };

            (as_list, last_split)
        }

        // This returns a bool whether or not the list is just the string "HH"
        fn is_hh(qualifier: &[&str]) -> bool {
            match qualifier.len() {
                1 => qualifier[0] == "HH",
                _ => false,
            }
        }
        let (qualifier, name) = unqualify(x);
        !is_hh(&qualifier) && !qualifier.is_empty() && is_reserved_hh_name(name)
    }
}

pub mod literal {
    pub const TRUE: &str = "true";
    pub const FALSE: &str = "false";
    pub const NULL: &str = "null";
}

pub mod pseudo_consts {
    use lazy_static::lazy_static;
    use std::collections::HashSet;

    pub const G__LINE__: &str = "\\__LINE__";

    pub const G__CLASS__: &str = "\\__CLASS__";

    pub const G__TRAIT__: &str = "\\__TRAIT__";

    pub const G__FILE__: &str = "\\__FILE__";

    pub const G__DIR__: &str = "\\__DIR__";

    pub const G__FUNCTION__: &str = "\\__FUNCTION__";

    pub const G__METHOD__: &str = "\\__METHOD__";

    pub const G__NAMESPACE__: &str = "\\__NAMESPACE__";

    pub const G__COMPILER_FRONTEND__: &str = "\\__COMPILER_FRONTEND__";

    pub const G__FUNCTION_CREDENTIAL__: &str = "\\__FUNCTION_CREDENTIAL__";

    pub const DIE: &str = "\\die";

    pub const EXIT: &str = "\\exit";

    pub static ALL_PSEUDO_CONSTS: &[&str] = &[
        G__LINE__,
        G__CLASS__,
        G__TRAIT__,
        G__FILE__,
        G__DIR__,
        G__FUNCTION__,
        G__METHOD__,
        G__NAMESPACE__,
        G__COMPILER_FRONTEND__,
        G__FUNCTION_CREDENTIAL__,
        DIE,
        EXIT,
    ];

    lazy_static! {
        static ref PSEUDO_SET: HashSet<&'static str> = ALL_PSEUDO_CONSTS.iter().copied().collect();
    }

    pub fn is_pseudo_const(x: &str) -> bool {
        PSEUDO_SET.contains(x)
    }
}

pub mod fb {
    pub const ENUM: &str = "\\Enum";

    pub const IDX: &str = "\\HH\\idx";

    pub const TYPE_STRUCTURE: &str = "\\HH\\TypeStructure";

    pub const INCORRECT_TYPE: &str = "\\HH\\INCORRECT_TYPE";

    pub const INCORRECT_TYPE_NO_NS: &str = "HH\\INCORRECT_TYPE";
}

pub mod hh {
    pub const CONTAINS: &str = "\\HH\\Lib\\C\\contains";

    pub const CONTAINS_KEY: &str = "\\HH\\Lib\\C\\contains_key";
}

pub mod rx {
    pub const IS_ENABLED: &str = "\\HH\\Rx\\IS_ENABLED";
}
pub mod coeffects {
    pub const DEFAULTS: &str = "defaults";

    pub const RX_LOCAL: &str = "rx_local";

    pub const RX_SHALLOW: &str = "rx_shallow";

    pub const RX: &str = "rx";

    pub const WRITE_PROPS: &str = "write_props";

    pub const POLICIED_LOCAL: &str = "policied_local";

    pub const POLICIED_SHALLOW: &str = "policied_shallow";

    pub const POLICIED: &str = "policied";

    pub const POLICIED_OF_LOCAL: &str = "policied_of_local";

    pub const POLICIED_OF_SHALLOW: &str = "policied_of_shallow";

    pub const POLICIED_OF: &str = "policied_of";

    pub const PURE: &str = "pure";
}

pub mod shapes {
    pub const SHAPES: &str = "\\HH\\Shapes";

    pub const IDX: &str = "idx";

    pub const AT: &str = "at";

    pub const KEY_EXISTS: &str = "keyExists";

    pub const REMOVE_KEY: &str = "removeKey";

    pub const TO_ARRAY: &str = "toArray";

    pub const TO_DICT: &str = "toDict";
}

pub mod superglobals {
    use lazy_static::lazy_static;
    use std::collections::HashSet;
    pub const GLOBALS: &str = "$GLOBALS";

    pub static SUPERGLOBALS: &[&str] = &[
        "$_SERVER",
        "$_GET",
        "$_POST",
        "$_FILES",
        "$_COOKIE",
        "$_REQUEST",
        "$_ENV",
    ];

    lazy_static! {
        static ref SUPERGLOBALS_SET: HashSet<&'static str> = SUPERGLOBALS.iter().copied().collect();
    }
    pub fn is_superglobal(x: &str) -> bool {
        SUPERGLOBALS_SET.contains(x)
    }
    pub fn is_any_global(x: &str) -> bool {
        is_superglobal(x) || x == GLOBALS
    }
}

pub mod xhp {
    pub const PCDATA: &str = "pcdata";
    pub const ANY: &str = "any";
    pub const EMPTY: &str = "empty";
    pub fn is_reserved(x: &str) -> bool {
        x == PCDATA || x == ANY || x == EMPTY
    }
    pub fn is_xhp_category(x: &str) -> bool {
        x.starts_with('%')
    }
}

pub mod regex {
    pub const T_PATTERN: &str = "\\HH\\Lib\\Regex\\Pattern";
}

pub mod emitter_special_functions {
    pub const EVAL: &str = "\\eval";
    pub const SET_FRAME_METADATA: &str = "\\HH\\set_frame_metadata";
    pub const SYSTEMLIB_REIFIED_GENERICS: &str = "\\__systemlib_reified_generics";
}

pub mod math {
    pub const NAN: &str = "NAN";
    pub const INF: &str = "INF";
    pub const NEG_INF: &str = "-INF";
}

#[cfg(test)]
mod test {
    use crate::members::is_special_xhp_attribute;
    use crate::members::AS_LOWERCASE_SET;
    use crate::members::UNSUPPORTED_MAP;
    use crate::special_idents::is_tmp_var;
    use crate::typehints::is_namespace_with_reserved_hh_name;

    #[test]
    fn test_special_idents_is_tmp_var() {
        assert!(!is_tmp_var("_tmp$Blah"));
        assert!(!is_tmp_var("__tmp$"));
        assert!(!is_tmp_var("О БОЖЕ"));

        assert!(is_tmp_var("__tmp$Blah"));
    }

    #[test]
    fn test_members_as_lowercase_set() {
        assert!(AS_LOWERCASE_SET.contains("__tostring"));
        assert!(!AS_LOWERCASE_SET.contains("__toString"));
    }

    #[test]
    fn test_members_unsupported_map() {
        assert_eq!(UNSUPPORTED_MAP.get("__callstatic"), Some(&"__callStatic"));
        assert!(!UNSUPPORTED_MAP.contains_key("__callStatic"));
    }

    #[test]
    fn test_members_is_special_xhp_attribute() {
        assert!(is_special_xhp_attribute(":data-blahblah"));
        assert!(is_special_xhp_attribute(":aria-blahblah"));

        assert!(!is_special_xhp_attribute(":arla-blahblah"));
        assert!(!is_special_xhp_attribute(":aria"));
    }

    #[test]
    fn test_typehint_is_namespace_with_reserved_hh_name() {
        assert!(!is_namespace_with_reserved_hh_name("HH\\void"));
        assert!(!is_namespace_with_reserved_hh_name("void"));
        assert!(!is_namespace_with_reserved_hh_name("ReturnType\\Lloyd"));
        assert!(!is_namespace_with_reserved_hh_name("Lloyd"));

        assert!(is_namespace_with_reserved_hh_name("Anything\\Else\\void"));
    }
}
