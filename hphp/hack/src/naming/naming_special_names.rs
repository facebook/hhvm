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

    pub const FORMAT_STRING: &str = "\\FormatString";

    pub const HH_FORMAT_STRING: &str = "\\HH\\FormatString";

    pub fn is_format_string(x: &str) -> bool {
        match x {
            FORMAT_STRING | HH_FORMAT_STRING => true,
            _ => false,
        }
    }

    pub const HH_BUILTIN_ENUM: &str = "\\HH\\BuiltinEnum";

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

    use lazy_static::lazy_static;
    use std::collections::HashSet;

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
                .fold(HashSet::<String>::new(), |mut set, special_name| {
                    set.insert(special_name.to_ascii_lowercase());
                    set
                })
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

    pub const HIPHOP_SPECIFIC: &str = "__HipHopSpecific";

    pub const ACCEPT_DISPOSABLE: &str = "__AcceptDisposable";

    pub const RETURN_DISPOSABLE: &str = "__ReturnDisposable";

    pub const PURE: &str = "__Pure";

    pub const REACTIVE: &str = "__Rx";

    pub const LOCAL_REACTIVE: &str = "__RxLocal";

    pub const SHALLOW_REACTIVE: &str = "__RxShallow";

    pub const MUTABLE: &str = "__Mutable";

    pub const MUTABLE_RETURN: &str = "__MutableReturn";

    pub const ONLY_RX_IF_IMPL: &str = "__OnlyRxIfImpl";

    pub const PROBABILISTIC_MODEL: &str = "__PPL";

    pub const LSB: &str = "__LSB";

    pub const AT_MOST_RX_AS_FUNC: &str = "__AtMostRxAsFunc";

    pub const AT_MOST_RX_AS_ARGS: &str = "__AtMostRxAsArgs";

    pub const SEALED: &str = "__Sealed";

    pub const RETURNS_VOID_TO_RX: &str = "__ReturnsVoidToRx";

    pub const MAYBE_MUTABLE: &str = "__MaybeMutable";

    pub const LATE_INIT: &str = "__LateInit";

    pub const OWNED_MUTABLE: &str = "__OwnedMutable";

    pub const NON_RX: &str = "__NonRx";

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

    pub const HAS_TOP_LEVEL_CODE: &str = "__HasTopLevelCode";

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
            HIPHOP_SPECIFIC,
            ACCEPT_DISPOSABLE,
            RETURN_DISPOSABLE,
            PURE,
            REACTIVE,
            LOCAL_REACTIVE,
            MUTABLE,
            MUTABLE_RETURN,
            SHALLOW_REACTIVE,
            ONLY_RX_IF_IMPL,
            PROBABILISTIC_MODEL,
            LSB,
            SEALED,
            RETURNS_VOID_TO_RX,
            MAYBE_MUTABLE,
            LATE_INIT,
            AT_MOST_RX_AS_FUNC,
            AT_MOST_RX_AS_ARGS,
            OWNED_MUTABLE,
            NON_RX,
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
    use lazy_static::lazy_static;
    use std::collections::HashMap;

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
        (TYPE_CONST, "a type pub constant"),
    ];

    lazy_static! {
        pub static ref PLAIN_ENGLISH_MAP: HashMap<&'static str, &'static str> =
            PLAIN_ENGLISH.iter().cloned().collect();
    }
}

/* Tested before \\-prepending name-canonicalization */
pub mod special_functions {
    use lazy_static::lazy_static;

    pub const TUPLE: &str = "tuple"; /* pseudo-function */

    pub const ECHO: &str = "echo"; /* pseudo-function */

    pub const ASSERT_: &str = "assert";

    pub const AUTOLOAD: &str = "__autoload";

    pub const HHAS_ADATA: &str = "__hhas_adata";

    pub fn is_special_function(x: &str) -> bool {
        lazy_static! {
            static ref ALL_SPECIAL_FUNCTIONS: Vec<&'static str> =
                vec![TUPLE, ECHO, ASSERT_, AUTOLOAD, HHAS_ADATA,]
                    .into_iter()
                    .collect();
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

    pub const ASSERT: &str = "\\assert";

    pub const ECHO: &str = "\\echo";

    pub const ECHO_NO_NS: &str = "echo";

    pub const EMPTY: &str = "\\empty";

    pub const EXIT: &str = "\\exit";

    pub const DIE: &str = "\\die";

    lazy_static! {
        pub static ref ALL_PSEUDO_FUNCTIONS: Vec<&'static str> = vec![
            ISSET,
            UNSET,
            HH_SHOW,
            HH_SHOW_ENV,
            HH_LOG_LEVEL,
            HH_FORCE_SOLVE,
            HH_LOOP_FOREVER,
            ASSERT,
            ECHO,
            EMPTY,
            EXIT,
            DIE,
        ];
        static ref PSEUDO_SET: HashSet<&'static str> =
            ALL_PSEUDO_FUNCTIONS.iter().cloned().collect();
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

    pub const MARK_LEGACY_HACK_ARRAY: &str = "\\HH\\mark_legacy_hack_array";

    pub const ARRAY_MARK_LEGACY: &str = "\\HH\\array_mark_legacy";
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

    pub const ARRAY: &str = "array";

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
                ARRAY,
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
        static ref RESERVED_GLOBAL_NAMES: HashSet<&'static str> = vec![
            ARRAY,
            CALLABLE,
            crate::classes::SELF,
            crate::classes::PARENT
        ]
        .into_iter()
        .collect();
    }

    pub fn is_reserved_global_name(x: &str) -> bool {
        RESERVED_GLOBAL_NAMES.contains(x)
    }

    lazy_static! {
        static ref RESERVED_HH_NAMES: HashSet<&'static str> = vec![
            VOID, NORETURN, INT, BOOL, FLOAT, NUM, STRING, RESOURCE, MIXED, ARRAY, ARRAYKEY,
            DYNAMIC, WILDCARD, NULL, NONNULL, NOTHING, THIS
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

    lazy_static! {
        static ref ALL_PSEUDO_CONSTS: Vec<&'static str> = vec![
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
        static ref PSEUDO_SET: HashSet<&'static str> = ALL_PSEUDO_CONSTS.iter().cloned().collect();
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
    pub const FREEZE: &str = "\\HH\\Rx\\freeze";

    pub const MUTABLE_: &str = "\\HH\\Rx\\mutable";

    pub const TRAVERSABLE: &str = "\\HH\\Rx\\Traversable";

    pub const IS_ENABLED: &str = "\\HH\\Rx\\IS_ENABLED";

    pub const KEYED_TRAVERSABLE: &str = "\\HH\\Rx\\KeyedTraversable";

    pub const ASYNC_ITERATOR: &str = "\\HH\\Rx\\AsyncIterator";

    pub const MOVE: &str = "\\HH\\Rx\\move";

    pub const PURE: &str = "Pure";

    pub const RX: &str = "Rx";

    pub const RX_LOCAL: &str = "RxLocal";

    pub const RX_SHALLOW: &str = "RxShallow";

    pub const MUTABLE: &str = "Mutable";

    pub const MAYBE_MUTABLE: &str = "MaybeMutable";

    pub const OWNED_MUTABLE: &str = "OwnedMutable";

    pub fn is_reactive_typehint(x: &str) -> bool {
        use lazy_static::lazy_static;
        use std::collections::HashSet;

        lazy_static! {
            static ref REACTIVE_TYPEHINTS: HashSet<&'static str> = vec![
                PURE,
                RX,
                RX_LOCAL,
                RX_SHALLOW,
                MUTABLE,
                MAYBE_MUTABLE,
                OWNED_MUTABLE,
            ]
            .into_iter()
            .collect();
        }

        REACTIVE_TYPEHINTS.contains(x)
    }
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

    lazy_static! {
        static ref SUPERGLOBALS: Vec<&'static str> = vec![
            "$_SERVER",
            "$_GET",
            "$_POST",
            "$_FILES",
            "$_COOKIE",
            "$_REQUEST",
            "$_ENV"
        ];
        static ref SUPERGLOBALS_SET: HashSet<&'static str> = SUPERGLOBALS.iter().cloned().collect();
    }
    pub fn is_superglobal(x: &str) -> bool {
        SUPERGLOBALS_SET.contains(x)
    }
    pub fn is_any_global(x: &str) -> bool {
        is_superglobal(x) || x == GLOBALS
    }
}

pub mod ppl_functions {
    use lazy_static::lazy_static;
    use std::collections::HashSet;
    lazy_static! {
        static ref ALL_RESERVED: Vec<&'static str> = vec!(
            "sample",
            "\\sample",
            "factor",
            "\\factor",
            "observe",
            "\\observe",
            "condition",
            "\\condition",
            "sample_model",
            "\\sample_model"
        );
        static ref ALL_RESERVED_SET: HashSet<&'static str> = ALL_RESERVED.iter().cloned().collect();
    }
    pub fn is_reserved(x: &str) -> bool {
        ALL_RESERVED_SET.contains(x)
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
}

pub mod math {
    pub const NAN: &str = "NAN";
    pub const INF: &str = "INF";
}

pub mod pocket_universes {
    pub const MEMBERS: &str = "Members";
}

#[cfg(test)]
mod test {
    use crate::members::is_special_xhp_attribute;
    use crate::members::AS_LOWERCASE_SET;
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
