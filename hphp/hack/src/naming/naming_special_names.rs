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

    pub const HH_BUILTIN_ABSTRACT_ENUM_CLASS: &str = "\\HH\\BuiltinAbstractEnumClass";

    pub const THROWABLE: &str = "\\Throwable";

    pub const STD_CLASS: &str = "\\stdClass";

    pub const DATE_TIME: &str = "\\DateTime";

    pub const DATE_TIME_IMMUTABLE: &str = "\\DateTimeImmutable";

    pub const ASYNC_ITERATOR: &str = "\\HH\\AsyncIterator";

    pub const ASYNC_KEYED_ITERATOR: &str = "\\HH\\AsyncKeyedIterator";

    pub const STRINGISH: &str = "\\Stringish";

    pub const STRINGISH_OBJECT: &str = "\\StringishObject";

    pub const XHP_CHILD: &str = "\\XHPChild";

    pub const IMEMOIZE_PARAM: &str = "\\HH\\IMemoizeParam";

    pub const UNSAFE_SINGLETON_MEMOIZE_PARAM: &str = "\\HH\\UNSAFESingletonMemoizeParam";

    pub const CLASS_NAME: &str = "\\HH\\classname";

    pub const TYPE_NAME: &str = "\\HH\\typename";

    pub const IDISPOSABLE: &str = "\\IDisposable";

    pub const IASYNC_DISPOSABLE: &str = "\\IAsyncDisposable";

    pub const MEMBER_OF: &str = "\\HH\\MemberOf";

    pub const ENUM_CLASS_LABEL: &str = "\\HH\\EnumClass\\Label";

    /// Classes that can be spliced into ExpressionTrees
    pub const SPLICEABLE: &str = "\\Spliceable";

    pub const SUPPORT_DYN: &str = "\\HH\\supportdyn";
}

pub mod collections {
    /* concrete classes */
    pub const VECTOR: &str = "\\HH\\Vector";

    pub const MUTABLE_VECTOR: &str = "\\MutableVector";

    pub const IMM_VECTOR: &str = "\\HH\\ImmVector";

    pub const SET: &str = "\\HH\\Set";

    pub const CONST_SET: &str = "\\ConstSet";

    pub const MUTABLE_SET: &str = "\\MutableSet";

    pub const IMM_SET: &str = "\\HH\\ImmSet";

    pub const MAP: &str = "\\HH\\Map";

    pub const MUTABLE_MAP: &str = "\\MutableMap";

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

    pub const ANY_ARRAY: &str = "\\HH\\AnyArray";

    pub const DICT: &str = "\\HH\\dict";

    pub const VEC: &str = "\\HH\\vec";

    pub const KEYSET: &str = "\\HH\\keyset";
}

pub mod members {
    use hash::HashSet;
    use lazy_static::lazy_static;

    pub const M_GET_INSTANCE_KEY: &str = "getInstanceKey";

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
        pub static ref AS_SET: HashSet<&'static str> = vec![
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
        pub static ref UNSUPPORTED_SET: HashSet<&'static str> =
            vec![__CALL, __CALL_STATIC, __GET, __ISSET, __SET, __UNSET]
                .into_iter()
                .collect();
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
    pub const UNSAFE_ALLOW_MULTIPLE_INSTANTIATIONS: &str = "__UNSAFE_AllowMultipleInstantiations";

    pub const OVERRIDE: &str = "__Override";

    pub const AUTOCOMPLETE_SORT_TEXT: &str = "__AutocompleteSortText";

    pub const CONSISTENT_CONSTRUCT: &str = "__ConsistentConstruct";

    pub const CONST: &str = "__Const";

    pub const DEPRECATED: &str = "__Deprecated";

    pub const DOCS: &str = "__Docs";

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

    pub const NON_DISJOINT: &str = "__NonDisjoint";

    pub const SOFT: &str = "__Soft";

    pub const SOFT_INTERNAL: &str = "__SoftInternal";

    pub const WARN: &str = "__Warn";

    pub const MOCK_CLASS: &str = "__MockClass";

    pub const PROVENANCE_SKIP_FRAME: &str = "__ProvenanceSkipFrame";

    pub const DYNAMICALLY_CALLABLE: &str = "__DynamicallyCallable";

    pub const DYNAMICALLY_CONSTRUCTIBLE: &str = "__DynamicallyConstructible";

    pub const DYNAMICALLY_REFERENCED: &str = "__DynamicallyReferenced";

    pub const REIFIABLE: &str = "__Reifiable";

    pub const NEVER_INLINE: &str = "__NEVER_INLINE";

    pub const DISABLE_TYPECHECKER_INTERNAL: &str = "__DisableTypecheckerInternal";

    pub const HAS_TOP_LEVEL_CODE: &str = "__HasTopLevelCode";

    pub const IS_FOLDABLE: &str = "__IsFoldable";

    pub const NATIVE: &str = "__Native";

    pub const OUT_ONLY: &str = "__OutOnly";

    pub const ALWAYS_INLINE: &str = "__ALWAYS_INLINE";

    pub const ENABLE_UNSTABLE_FEATURES: &str = "__EnableUnstableFeatures";

    pub const ENUM_CLASS: &str = "__EnumClass";

    pub const POLICIED: &str = "__Policied";

    pub const INFERFLOWS: &str = "__InferFlows";

    pub const EXTERNAL: &str = "__External";

    pub const CAN_CALL: &str = "__CanCall";

    pub const SUPPORT_DYNAMIC_TYPE: &str = "__SupportDynamicType";

    pub const NO_AUTO_DYNAMIC: &str = "__NoAutoDynamic";

    pub const NO_AUTO_LIKES: &str = "__NoAutoLikes";

    pub const NO_AUTO_BOUND: &str = "__NoAutoBound";

    pub const REQUIRE_DYNAMIC: &str = "__RequireDynamic";

    pub const ENABLE_METHOD_TRAIT_DIAMOND: &str = "__EnableMethodTraitDiamond";

    pub const IGNORE_READONLY_LOCAL_ERRORS: &str = "__IgnoreReadonlyLocalErrors";

    pub const IGNORE_COEFFECT_LOCAL_ERRORS: &str = "__IgnoreCoeffectLocalErrors";

    pub const CROSS_PACKAGE: &str = "__CrossPackage";

    pub const MODULE_LEVEL_TRAIT: &str = "__ModuleLevelTrait";

    pub const STRICT_SWITCH: &str = "__StrictSwitch";

    pub fn is_memoized(name: &str) -> bool {
        name == MEMOIZE || name == MEMOIZE_LSB
    }

    // TODO(hrust) these should probably be added to the above map/fields, too
    // These attributes are only valid in systemlib, and therefore are not passed to the typechecker
    // or hhi files
    pub fn is_native(name: &str) -> bool {
        name == "__Native"
    }

    pub fn ignore_coeffect_local_errors(name: &str) -> bool {
        name == "__IgnoreCoeffectLocalErrors"
    }

    pub fn ignore_readonly_local_errors(name: &str) -> bool {
        name == "__IgnoreReadonlyLocalErrors"
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

    pub fn is_cross_package(name: &str) -> bool {
        name == CROSS_PACKAGE
    }
}

pub mod memoize_option {
    use hash::HashSet;
    use lazy_static::lazy_static;

    pub const KEYED_BY_IC: &str = "KeyedByIC";

    pub const MAKE_IC_INACCESSSIBLE: &str = "MakeICInaccessible";

    pub const SOFT_MAKE_IC_INACCESSSIBLE: &str = "SoftMakeICInaccessible";

    pub const UNCATEGORIZED: &str = "Uncategorized";

    pub static _ALL: &[&str] = &[
        KEYED_BY_IC,
        MAKE_IC_INACCESSSIBLE,
        SOFT_MAKE_IC_INACCESSSIBLE,
        UNCATEGORIZED,
    ];

    lazy_static! {
        static ref VALID_SET: HashSet<&'static str> = _ALL.iter().copied().collect();
    }

    pub fn is_valid(x: &str) -> bool {
        VALID_SET.contains(x)
    }
}

pub mod attribute_kinds {
    use hash::HashMap;
    use lazy_static::lazy_static;

    pub const CLS: &str = "\\HH\\ClassAttribute";

    pub const CLS_CST: &str = "\\HH\\ClassConstantAttribute";

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
        (CLS_CST, "a constant of a class"),
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

    pub const ECHO: &str = "echo"; /* pseudo-function */

    pub fn is_special_function(x: &str) -> bool {
        lazy_static! {
            static ref ALL_SPECIAL_FUNCTIONS: Vec<&'static str> = vec![ECHO].into_iter().collect();
        }
        ALL_SPECIAL_FUNCTIONS.contains(&x)
    }
}

pub mod autoimported_functions {
    pub const INVARIANT: &str = "\\HH\\invariant";

    pub const INVARIANT_VIOLATION: &str = "\\HH\\invariant_violation";

    pub const METH_CALLER: &str = "\\HH\\meth_caller";
}

pub mod special_idents {
    pub const THIS: &str = "$this";

    pub const PLACEHOLDER: &str = "$_";

    pub const DOLLAR_DOLLAR: &str = "$$";

    /* Intentionally using an invalid variable name to ensure it's translated */
    pub const TMP_VAR_PREFIX: &str = "__tmp$";

    pub fn is_tmp_var(name: &str) -> bool {
        name.len() > TMP_VAR_PREFIX.len() && name.starts_with(TMP_VAR_PREFIX)
    }

    pub fn assert_tmp_var(name: &str) {
        assert!(is_tmp_var(name))
    }
}

pub mod pseudo_functions {
    use hash::HashSet;
    use lazy_static::lazy_static;

    pub const ISSET: &str = "\\isset";

    pub const UNSET: &str = "\\unset";

    pub const HH_SHOW: &str = "\\hh_show";

    pub const HH_EXPECT: &str = "\\hh_expect";

    pub const HH_EXPECT_EQUIVALENT: &str = "\\hh_expect_equivalent";

    pub const HH_SHOW_ENV: &str = "\\hh_show_env";

    pub const HH_LOG_LEVEL: &str = "\\hh_log_level";

    pub const HH_FORCE_SOLVE: &str = "\\hh_force_solve";

    pub const HH_LOOP_FOREVER: &str = "\\hh_loop_forever";

    pub const ECHO: &str = "\\echo";

    pub const ECHO_NO_NS: &str = "echo";

    pub const EMPTY: &str = "\\empty";

    pub const EXIT: &str = "\\exit";

    pub const UNSAFE_CAST: &str = "\\HH\\FIXME\\UNSAFE_CAST";

    pub const UNSAFE_NONNULL_CAST: &str = "\\HH\\FIXME\\UNSAFE_NONNULL_CAST";

    pub const ENFORCED_CAST: &str = "\\HH\\FIXME\\ENFORCED_CAST";

    pub const PACKAGE_EXISTS: &str = "\\HH\\package_exists";

    pub static ALL_PSEUDO_FUNCTIONS: &[&str] = &[
        ISSET,
        UNSET,
        HH_SHOW,
        HH_EXPECT,
        HH_EXPECT_EQUIVALENT,
        HH_SHOW_ENV,
        HH_LOG_LEVEL,
        HH_FORCE_SOLVE,
        HH_LOOP_FOREVER,
        ECHO,
        EMPTY,
        EXIT,
        UNSAFE_CAST,
        UNSAFE_NONNULL_CAST,
        PACKAGE_EXISTS,
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

    pub const CALL_USER_FUNC: &str = "\\call_user_func";

    pub const TYPE_STRUCTURE: &str = "\\HH\\type_structure";

    pub const ARRAY_MARK_LEGACY: &str = "\\HH\\array_mark_legacy";

    pub const ARRAY_UNMARK_LEGACY: &str = "\\HH\\array_unmark_legacy";

    pub const IS_PHP_ARRAY: &str = "\\HH\\is_php_array";

    pub const IS_ANY_ARRAY: &str = "\\HH\\is_any_array";

    pub const IS_DICT_OR_DARRAY: &str = "\\HH\\is_dict_or_darray";

    pub const IS_VEC_OR_VARRAY: &str = "\\HH\\is_vec_or_varray";
}

pub mod typehints {
    use hash::HashSet;
    use lazy_static::lazy_static;

    pub const NULL: &str = "null";
    pub const HH_NULL: &str = "\\HH\\null";

    pub const VOID: &str = "void";
    pub const HH_VOID: &str = "\\HH\\void";

    pub const RESOURCE: &str = "resource";
    pub const HH_RESOURCE: &str = "\\HH\\resource";

    pub const NUM: &str = "num";
    pub const HH_NUM: &str = "\\HH\\num";

    pub const ARRAYKEY: &str = "arraykey";
    pub const HH_ARRAYKEY: &str = "\\HH\\arraykey";

    pub const NORETURN: &str = "noreturn";
    pub const HH_NORETURN: &str = "\\HH\\noreturn";

    pub const MIXED: &str = "mixed";

    pub const NONNULL: &str = "nonnull";
    pub const HH_NONNULL: &str = "\\HH\\nonnull";

    pub const THIS: &str = "this";
    pub const HH_THIS: &str = "\\HH\\this";

    pub const DYNAMIC: &str = "dynamic";

    pub const NOTHING: &str = "nothing";
    pub const HH_NOTHING: &str = "\\HH\\nothing";

    pub const INT: &str = "int";
    pub const HH_INT: &str = "\\HH\\int";

    pub const BOOL: &str = "bool";
    pub const HH_BOOL: &str = "\\HH\\bool";

    pub const FLOAT: &str = "float";
    pub const HH_FLOAT: &str = "\\HH\\float";

    pub const STRING: &str = "string";
    pub const HH_STRING: &str = "\\HH\\string";

    pub const DARRAY: &str = "darray";
    pub const HH_DARRAY: &str = "\\HH\\darray";

    pub const VARRAY: &str = "varray";
    pub const HH_VARRAY: &str = "\\HH\\varray";

    pub const VARRAY_OR_DARRAY: &str = "varray_or_darray";
    pub const HH_VARRAY_OR_DARRAY: &str = "\\HH\\varray_or_darray";

    pub const VEC_OR_DICT: &str = "vec_or_dict";
    pub const HH_VEC_OR_DICT: &str = "\\HH\\vec_or_dict";

    pub const CALLABLE: &str = "callable";

    pub const OBJECT_CAST: &str = "object";

    pub const SUPPORTDYN: &str = "supportdyn";

    pub const HH_FUNCTIONREF: &str = "\\HH\\FunctionRef";

    pub const HH_SUPPORTDYN: &str = "\\HH\\supportdyn";

    pub const POISON_MARKER: &str = "\\HH\\FIXME\\POISON_MARKER";

    pub const WILDCARD: &str = "_";

    lazy_static! {
        // matches definition in Tprim
        pub static ref PRIMITIVE_TYPEHINTS: HashSet<&'static str> = vec![
            NULL, VOID, INT, BOOL, FLOAT, STRING, RESOURCE, NUM, ARRAYKEY, NORETURN
        ]
        .into_iter()
        .collect();
    }

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
                VEC_OR_DICT,
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
    use hash::HashSet;
    use lazy_static::lazy_static;

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

    pub const INNER: &str = "TInner";

    pub const IDX: &str = "\\HH\\idx";

    pub const IDXREADONLY: &str = "\\HH\\idx_readonly";

    pub const TYPE_STRUCTURE: &str = "\\HH\\TypeStructure";
}

pub mod hh {

    pub const MEMOIZE_OPTION: &str = "\\HH\\MemoizeOption";

    pub const CONTAINS: &str = "\\HH\\Lib\\C\\contains";

    pub const CONTAINS_KEY: &str = "\\HH\\Lib\\C\\contains_key";
}

pub mod readonly {
    pub const IDX: &str = "\\HH\\idx_readonly";

    pub const AS_MUT: &str = "\\HH\\Readonly\\as_mut";
}

pub mod coeffects {
    use std::fmt;

    use hash::HashSet;
    use lazy_static::lazy_static;
    use serde::Serialize;

    pub const DEFAULTS: &str = "defaults";

    pub const RX_LOCAL: &str = "rx_local";

    pub const RX_SHALLOW: &str = "rx_shallow";

    pub const RX: &str = "rx";

    pub const WRITE_THIS_PROPS: &str = "write_this_props";

    pub const WRITE_PROPS: &str = "write_props";

    pub const LEAK_SAFE_LOCAL: &str = "leak_safe_local";

    pub const LEAK_SAFE_SHALLOW: &str = "leak_safe_shallow";

    pub const LEAK_SAFE: &str = "leak_safe";

    pub const ZONED_LOCAL: &str = "zoned_local";

    pub const ZONED_SHALLOW: &str = "zoned_shallow";

    pub const ZONED: &str = "zoned";

    pub const ZONED_WITH: &str = "zoned_with";

    pub const PURE: &str = "pure";

    pub const READ_GLOBALS: &str = "read_globals";

    pub const GLOBALS: &str = "globals";

    pub const BACKDOOR: &str = "86backdoor";

    pub const BACKDOOR_GLOBALS_LEAK_SAFE: &str = "86backdoor_globals_leak_safe";

    pub const CONTEXTS: &str = "HH\\Contexts";

    pub const UNSAFE_CONTEXTS: &str = "HH\\Contexts\\Unsafe";

    pub const CAPABILITIES: &str = "HH\\Capabilities";

    pub fn is_any_zoned(x: &str) -> bool {
        lazy_static! {
            static ref ZONED_SET: HashSet<&'static str> =
                vec![ZONED, ZONED_WITH].into_iter().collect();
        }
        ZONED_SET.contains(x)
    }

    pub fn is_any_zoned_or_defaults(x: &str) -> bool {
        lazy_static! {
            static ref ZONED_SET: HashSet<&'static str> =
                vec![ZONED, ZONED_WITH, ZONED_LOCAL, ZONED_SHALLOW, DEFAULTS]
                    .into_iter()
                    .collect();
        }
        ZONED_SET.contains(x)
    }

    // context does not provide the capability to fetch the IC even indirectly
    pub fn is_any_without_implicit_policy_or_unsafe(x: &str) -> bool {
        lazy_static! {
            static ref LEAK_SAFE_GLOBALS_SET: HashSet<&'static str> =
                vec![LEAK_SAFE, GLOBALS, READ_GLOBALS, WRITE_PROPS]
                    .into_iter()
                    .collect();
        }
        LEAK_SAFE_GLOBALS_SET.contains(x)
    }

    #[derive(PartialEq, Eq, Hash, Debug, Copy, Clone, Serialize)]
    #[repr(C)]
    pub enum Ctx {
        Defaults,

        // Shared
        WriteThisProps,
        WriteProps,

        // Rx hierarchy
        RxLocal,
        RxShallow,
        Rx,

        // Zoned hierarchy
        ZonedWith,
        ZonedLocal,
        ZonedShallow,
        Zoned,

        LeakSafeLocal,
        LeakSafeShallow,
        LeakSafe,

        ReadGlobals,
        Globals,

        // Pure
        Pure,
    }

    impl fmt::Display for Ctx {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            use Ctx::*;
            match self {
                Defaults => write!(f, "{}", DEFAULTS),
                Pure => write!(f, "{}", PURE),
                RxLocal => write!(f, "{}", RX_LOCAL),
                RxShallow => write!(f, "{}", RX_SHALLOW),
                Rx => write!(f, "{}", RX),
                WriteThisProps => write!(f, "{}", WRITE_THIS_PROPS),
                WriteProps => write!(f, "{}", WRITE_PROPS),
                ZonedWith => write!(f, "{}", ZONED_WITH),
                ZonedLocal => write!(f, "{}", ZONED_LOCAL),
                ZonedShallow => write!(f, "{}", ZONED_SHALLOW),
                Zoned => write!(f, "{}", ZONED),
                LeakSafeLocal => write!(f, "{}", LEAK_SAFE_LOCAL),
                LeakSafeShallow => write!(f, "{}", LEAK_SAFE_SHALLOW),
                LeakSafe => write!(f, "{}", LEAK_SAFE),
                ReadGlobals => write!(f, "{}", READ_GLOBALS),
                Globals => write!(f, "{}", GLOBALS),
            }
        }
    }

    write_bytes::display_bytes_using_display!(Ctx);

    pub fn ctx_str_to_enum(s: &str) -> Option<Ctx> {
        match s {
            DEFAULTS => Some(Ctx::Defaults),
            PURE => Some(Ctx::Pure),
            RX_LOCAL => Some(Ctx::RxLocal),
            RX_SHALLOW => Some(Ctx::RxShallow),
            RX => Some(Ctx::Rx),
            WRITE_THIS_PROPS => Some(Ctx::WriteThisProps),
            WRITE_PROPS => Some(Ctx::WriteProps),
            ZONED_WITH => Some(Ctx::ZonedWith),
            ZONED_LOCAL => Some(Ctx::ZonedLocal),
            ZONED_SHALLOW => Some(Ctx::ZonedShallow),
            ZONED => Some(Ctx::Zoned),
            LEAK_SAFE_LOCAL => Some(Ctx::LeakSafeLocal),
            LEAK_SAFE_SHALLOW => Some(Ctx::LeakSafeShallow),
            LEAK_SAFE => Some(Ctx::LeakSafe),
            GLOBALS => Some(Ctx::Globals),
            READ_GLOBALS => Some(Ctx::ReadGlobals),
            _ => None,
        }
    }

    pub fn capability_matches_name(c: &Ctx, name: &Ctx) -> bool {
        // Either the capability we are searching for matches exactly, is a subtype
        // of, or is contained in the capability/context we are visiting
        if c == name
            || self::capability_contained_in_ctx(c, name)
            || self::capability_is_subtype_of_capability(c, name)
        {
            return true;
        }
        false
    }

    pub fn capability_contained_in_ctx(c: &Ctx, ctx: &Ctx) -> bool {
        match ctx {
            Ctx::Defaults => self::capability_in_defaults_ctx(c),
            Ctx::Rx | Ctx::RxLocal | Ctx::RxShallow => self::capability_in_rx_ctx(c),
            Ctx::LeakSafe | Ctx::LeakSafeLocal | Ctx::LeakSafeShallow => {
                self::capability_in_controlled_ctx(c)
            }
            Ctx::Zoned | Ctx::ZonedLocal | Ctx::ZonedShallow | Ctx::ZonedWith => {
                self::capability_in_policied_ctx(c)
            }
            // By definition, granular capabilities are not contexts and cannot
            // contain other capabilities. Also included in the fall through here
            // are pure, namespaced, polymorphic, and encapsulated contexts; oldrx related
            // contexts; and illformed symbols (e.g. primitive type used as a context)
            _ => false,
        }
    }

    pub fn capability_is_subtype_of_capability(s: &Ctx, c: &Ctx) -> bool {
        match c {
            Ctx::WriteProps => *s == Ctx::WriteThisProps,
            Ctx::Globals => *s == Ctx::ReadGlobals,
            // By definition, contexts which contain multiple capabilities are not
            // themselves granular capabilities and therefore not subject to this
            // logic. Also included in the fall through here are capabilities which do
            // not have "subtype" capabilities and illformed symbols (e.g. primitive
            // type used as a context)
            _ => false,
        }
    }

    pub fn capability_in_defaults_ctx(c: &Ctx) -> bool {
        lazy_static! {
            static ref DEFAULTS_SET: HashSet<Ctx> =
                vec![Ctx::WriteProps, Ctx::Globals,].into_iter().collect();
        }
        DEFAULTS_SET.contains(c)
    }

    pub fn capability_in_policied_ctx(c: &Ctx) -> bool {
        lazy_static! {
            static ref POLICIED_SET: HashSet<Ctx> = vec![Ctx::WriteProps, Ctx::ReadGlobals,]
                .into_iter()
                .collect();
        }
        POLICIED_SET.contains(c)
    }

    pub fn capability_in_rx_ctx(c: &Ctx) -> bool {
        lazy_static! {
            static ref RX_SET: HashSet<Ctx> = vec![Ctx::WriteProps,].into_iter().collect();
        }
        RX_SET.contains(c)
    }

    pub fn capability_in_controlled_ctx(c: &Ctx) -> bool {
        lazy_static! {
            static ref CONTROLLED_SET: HashSet<Ctx> = vec![Ctx::WriteProps, Ctx::ReadGlobals,]
                .into_iter()
                .collect();
        }
        CONTROLLED_SET.contains(c)
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
    use hash::HashSet;
    use lazy_static::lazy_static;

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

pub mod unstable_features {
    pub const COEFFECTS_PROVISIONAL: &str = "coeffects_provisional";
    pub const READONLY: &str = "readonly";
    pub const EXPRESSION_TREES: &str = "expression_trees";
    pub const MODULE_REFERENCES: &str = "module_references";
    pub const PACKAGE: &str = "package";
}

pub mod regex {
    pub const T_PATTERN: &str = "\\HH\\Lib\\Regex\\Pattern";
}

pub mod emitter_special_functions {
    pub const EVAL: &str = "\\eval";
    pub const SET_FRAME_METADATA: &str = "\\HH\\set_frame_metadata";
    pub const SET_PRODUCT_ATTRIBUTION_ID: &str = "\\HH\\set_product_attribution_id";
    pub const SET_PRODUCT_ATTRIBUTION_ID_DEFERRED: &str =
        "\\HH\\set_product_attribution_id_deferred";
    pub const SYSTEMLIB_REIFIED_GENERICS: &str = "\\__systemlib_reified_generics";
    pub const GENA: &str = "gena";
}

pub mod math {
    pub const NAN: &str = "NAN";
    pub const INF: &str = "INF";
    pub const NEG_INF: &str = "-INF";
}

pub mod expression_trees {
    pub const MAKE_TREE: &str = "makeTree";

    pub const INT_TYPE: &str = "intType";
    pub const FLOAT_TYPE: &str = "floatType";
    pub const BOOL_TYPE: &str = "boolType";
    pub const STRING_TYPE: &str = "stringType";
    pub const NULL_TYPE: &str = "nullType";
    pub const VOID_TYPE: &str = "voidType";
    pub const SYMBOL_TYPE: &str = "symbolType";
    pub const LAMBDA_TYPE: &str = "lambdaType";

    pub const VISIT_INT: &str = "visitInt";
    pub const VISIT_FLOAT: &str = "visitFloat";
    pub const VISIT_BOOL: &str = "visitBool";
    pub const VISIT_STRING: &str = "visitString";
    pub const VISIT_NULL: &str = "visitNull";
    pub const VISIT_BINOP: &str = "visitBinop";
    pub const VISIT_UNOP: &str = "visitUnop";
    pub const VISIT_LOCAL: &str = "visitLocal";
    pub const VISIT_LAMBDA: &str = "visitLambda";
    pub const VISIT_GLOBAL_FUNCTION: &str = "visitGlobalFunction";
    pub const VISIT_STATIC_METHOD: &str = "visitStaticMethod";
    pub const VISIT_CALL: &str = "visitCall";
    pub const VISIT_ASSIGN: &str = "visitAssign";
    pub const VISIT_TERNARY: &str = "visitTernary";
    pub const VISIT_IF: &str = "visitIf";
    pub const VISIT_WHILE: &str = "visitWhile";
    pub const VISIT_RETURN: &str = "visitReturn";
    pub const VISIT_FOR: &str = "visitFor";
    pub const VISIT_BREAK: &str = "visitBreak";
    pub const VISIT_CONTINUE: &str = "visitContinue";
    pub const VISIT_PROPERTY_ACCESS: &str = "visitPropertyAccess";
    pub const VISIT_XHP: &str = "visitXhp";

    pub const SPLICE: &str = "splice";

    pub const DOLLARDOLLAR_TMP_VAR: &str = "$0dollardollar";
}

pub mod modules {
    pub const DEFAULT: &str = "default";
}

#[cfg(test)]
mod test {
    use crate::members::is_special_xhp_attribute;
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
