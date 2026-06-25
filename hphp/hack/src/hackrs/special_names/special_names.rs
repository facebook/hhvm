// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// These use the same casing as naming_special_names.ml for now.
#![allow(non_upper_case_globals)]

use std::sync::LazyLock;

use hash::HashSet;
use naming_special_names_rust as sn;
use pos::ClassConstName;
use pos::ConstName;
use pos::FunName;
use pos::MethodName;
use pos::PropName;
use pos::Symbol;
use pos::TypeConstName;
use pos::TypeName;

macro_rules! lazy {
    ($value:expr) => {
        LazyLock::new(|| $value.into())
    };
}

fn concat<S1: AsRef<str>, S2: AsRef<str>>(s1: S1, s2: S2) -> String {
    format!("{}{}", s1.as_ref(), s2.as_ref())
}

pub fn types() -> impl Iterator<Item = TypeName> {
    classes::iter()
        .chain(collections::iter())
        .chain(attribute_kinds::iter())
        .chain(user_attributes::iter())
        .chain(fb::types())
        .chain(shapes::types())
        .chain(regex::types())
        .chain(coeffects::types())
        .chain(capabilities::iter())
}

pub fn functions() -> impl Iterator<Item = FunName> {
    autoimported_functions::iter()
        .chain(pseudo_functions::iter())
        .chain(stdlib_functions::iter())
        .chain(fb::functions())
        .chain(hh::iter())
}

pub mod classes {
    use super::*;

    pub static cParent: LazyLock<TypeName> = lazy!(sn::classes::PARENT);
    pub static cStatic: LazyLock<TypeName> = lazy!(sn::classes::STATIC);
    pub static cSelf: LazyLock<TypeName> = lazy!(sn::classes::SELF);
    pub static cUnknown: LazyLock<TypeName> = lazy!(sn::classes::UNKNOWN);
    pub static cAwaitable: LazyLock<TypeName> = lazy!(sn::classes::AWAITABLE);
    pub static cGenerator: LazyLock<TypeName> = lazy!(sn::classes::GENERATOR);
    pub static cAsyncGenerator: LazyLock<TypeName> = lazy!(sn::classes::ASYNC_GENERATOR);
    pub static cHHFormatString: LazyLock<TypeName> = lazy!(sn::classes::HH_FORMAT_STRING);
    pub static cHH_BuiltinEnum: LazyLock<TypeName> = lazy!(sn::classes::HH_BUILTIN_ENUM);
    pub static cHH_BuiltinEnumClass: LazyLock<TypeName> = lazy!(sn::classes::HH_BUILTIN_ENUM_CLASS);
    pub static cHH_BuiltinAbstractEnumClass: LazyLock<TypeName> =
        lazy!(sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS);
    pub static cThrowable: LazyLock<TypeName> = lazy!(sn::classes::THROWABLE);
    pub static cStdClass: LazyLock<TypeName> = lazy!(sn::classes::STD_CLASS);
    pub static cDateTime: LazyLock<TypeName> = lazy!(sn::classes::DATE_TIME);
    pub static cDateTimeImmutable: LazyLock<TypeName> = lazy!(sn::classes::DATE_TIME_IMMUTABLE);
    pub static cAsyncIterator: LazyLock<TypeName> = lazy!(sn::classes::ASYNC_ITERATOR);
    pub static cAsyncKeyedIterator: LazyLock<TypeName> = lazy!(sn::classes::ASYNC_KEYED_ITERATOR);
    pub static cStringish: LazyLock<TypeName> = lazy!(sn::classes::STRINGISH);
    pub static cStringishObject: LazyLock<TypeName> = lazy!(sn::classes::STRINGISH_OBJECT);
    pub static cXHPChild: LazyLock<TypeName> = lazy!(sn::classes::XHP_CHILD);
    pub static cIMemoizeParam: LazyLock<TypeName> = lazy!(sn::classes::IMEMOIZE_PARAM);
    pub static cUNSAFESingletonMemoizeParam: LazyLock<TypeName> =
        lazy!(sn::classes::UNSAFE_SINGLETON_MEMOIZE_PARAM);
    pub static cClassname: LazyLock<TypeName> = lazy!(sn::classes::CLASS_NAME);
    pub static cConcrete: LazyLock<TypeName> = lazy!(sn::classes::CONCRETE);
    pub static cTypename: LazyLock<TypeName> = lazy!(sn::classes::TYPE_NAME);
    pub static cIDisposable: LazyLock<TypeName> = lazy!(sn::classes::IDISPOSABLE);
    pub static cIAsyncDisposable: LazyLock<TypeName> = lazy!(sn::classes::IASYNC_DISPOSABLE);
    pub static cMemberOf: LazyLock<TypeName> = lazy!(sn::classes::MEMBER_OF);
    pub static cEnumClassLabel: LazyLock<TypeName> = lazy!(sn::classes::ENUM_CLASS_LABEL);
    pub static cSpliceable: LazyLock<TypeName> = lazy!(sn::classes::SPLICEABLE);
    pub static cSupportDyn: LazyLock<TypeName> = lazy!(sn::classes::SUPPORT_DYN);

    pub fn iter_keywords() -> impl Iterator<Item = TypeName> {
        [*cParent, *cStatic, *cSelf].into_iter()
    }

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *cUnknown,
            *cAwaitable,
            *cGenerator,
            *cAsyncGenerator,
            *cHHFormatString,
            *cHH_BuiltinEnum,
            *cHH_BuiltinEnumClass,
            *cHH_BuiltinAbstractEnumClass,
            *cThrowable,
            *cStdClass,
            *cDateTime,
            *cDateTimeImmutable,
            *cAsyncIterator,
            *cAsyncKeyedIterator,
            *cStringish,
            *cStringishObject,
            *cXHPChild,
            *cIMemoizeParam,
            *cUNSAFESingletonMemoizeParam,
            *cClassname,
            *cTypename,
            *cIDisposable,
            *cIAsyncDisposable,
            *cMemberOf,
            *cEnumClassLabel,
            *cSpliceable,
            *cSupportDyn,
        ]
        .into_iter()
    }
}

pub mod collections {
    use super::*;

    // concrete classes
    pub static cVector: LazyLock<TypeName> = lazy!(sn::collections::VECTOR);
    pub static cMutableVector: LazyLock<TypeName> = lazy!(sn::collections::MUTABLE_VECTOR);
    pub static cImmVector: LazyLock<TypeName> = lazy!(sn::collections::IMM_VECTOR);
    pub static cSet: LazyLock<TypeName> = lazy!(sn::collections::SET);
    pub static cConstSet: LazyLock<TypeName> = lazy!(sn::collections::CONST_SET);
    pub static cMutableSet: LazyLock<TypeName> = lazy!(sn::collections::MUTABLE_SET);
    pub static cImmSet: LazyLock<TypeName> = lazy!(sn::collections::IMM_SET);
    pub static cMap: LazyLock<TypeName> = lazy!(sn::collections::MAP);
    pub static cMutableMap: LazyLock<TypeName> = lazy!(sn::collections::MUTABLE_MAP);
    pub static cImmMap: LazyLock<TypeName> = lazy!(sn::collections::IMM_MAP);
    pub static cPair: LazyLock<TypeName> = lazy!(sn::collections::PAIR);

    // interfaces
    pub static cContainer: LazyLock<TypeName> = lazy!(sn::collections::CONTAINER);
    pub static cKeyedContainer: LazyLock<TypeName> = lazy!(sn::collections::KEYED_CONTAINER);
    pub static cTraversable: LazyLock<TypeName> = lazy!(sn::collections::TRAVERSABLE);
    pub static cKeyedTraversable: LazyLock<TypeName> = lazy!(sn::collections::KEYED_TRAVERSABLE);
    pub static cCollection: LazyLock<TypeName> = lazy!(sn::collections::COLLECTION);
    pub static cConstVector: LazyLock<TypeName> = lazy!(sn::collections::CONST_VECTOR);
    pub static cConstMap: LazyLock<TypeName> = lazy!(sn::collections::CONST_MAP);
    pub static cConstCollection: LazyLock<TypeName> = lazy!(sn::collections::CONST_COLLECTION);
    pub static cAnyArray: LazyLock<TypeName> = lazy!(sn::collections::ANY_ARRAY);
    pub static cDict: LazyLock<TypeName> = lazy!(sn::collections::DICT);
    pub static cVec: LazyLock<TypeName> = lazy!(sn::collections::VEC);
    pub static cKeyset: LazyLock<TypeName> = lazy!(sn::collections::KEYSET);

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *cVector,
            *cMutableVector,
            *cImmVector,
            *cSet,
            *cConstSet,
            *cMutableSet,
            *cImmSet,
            *cMap,
            *cMutableMap,
            *cImmMap,
            *cPair,
            *cContainer,
            *cKeyedContainer,
            *cTraversable,
            *cKeyedTraversable,
            *cCollection,
            *cConstVector,
            *cConstMap,
            *cConstCollection,
            *cAnyArray,
            *cDict,
            *cVec,
            *cKeyset,
        ]
        .into_iter()
    }
}

pub mod members {
    use super::*;

    pub static mGetInstanceKey: LazyLock<MethodName> = lazy!(sn::members::M_GET_INSTANCE_KEY);
    pub static mClass: LazyLock<ClassConstName> = lazy!(sn::members::M_CLASS);
    pub static __construct: LazyLock<MethodName> = lazy!(sn::members::__CONSTRUCT);
    pub static __destruct: LazyLock<MethodName> = lazy!(sn::members::__DESTRUCT);
    pub static __call: LazyLock<MethodName> = lazy!(sn::members::__CALL);
    pub static __callStatic: LazyLock<MethodName> = lazy!(sn::members::__CALL_STATIC);
    pub static __clone: LazyLock<MethodName> = lazy!(sn::members::__CLONE);
    pub static __debugInfo: LazyLock<MethodName> = lazy!(sn::members::__DEBUG_INFO);
    pub static __dispose: LazyLock<MethodName> = lazy!(sn::members::__DISPOSE);
    pub static __disposeAsync: LazyLock<MethodName> = lazy!(sn::members::__DISPOSE_ASYNC);
    pub static __get: LazyLock<MethodName> = lazy!(sn::members::__GET);
    pub static __invoke: LazyLock<MethodName> = lazy!(sn::members::__INVOKE);
    pub static __isset: LazyLock<MethodName> = lazy!(sn::members::__ISSET);
    pub static __set: LazyLock<MethodName> = lazy!(sn::members::__SET);
    pub static __set_state: LazyLock<MethodName> = lazy!(sn::members::__SET_STATE);
    pub static __sleep: LazyLock<MethodName> = lazy!(sn::members::__SLEEP);
    pub static __toString: LazyLock<MethodName> = lazy!(sn::members::__TO_STRING);
    pub static __unset: LazyLock<MethodName> = lazy!(sn::members::__UNSET);
    pub static __wakeup: LazyLock<MethodName> = lazy!(sn::members::__WAKEUP);

    /// Not really a PropName, but it's treated as one in deferred_init_members
    /// of folded decls.
    pub static parentConstruct: LazyLock<PropName> = lazy!(concat("parent::", *__construct));
}

pub mod attribute_kinds {
    use super::*;

    pub static cls: LazyLock<TypeName> = lazy!(sn::attribute_kinds::CLS);
    pub static clscst: LazyLock<TypeName> = lazy!(sn::attribute_kinds::CLS_CST);
    pub static enum_: LazyLock<TypeName> = lazy!(sn::attribute_kinds::ENUM);
    pub static typealias: LazyLock<TypeName> = lazy!(sn::attribute_kinds::TYPE_ALIAS);
    pub static fn_: LazyLock<TypeName> = lazy!(sn::attribute_kinds::FN);
    pub static mthd: LazyLock<TypeName> = lazy!(sn::attribute_kinds::MTHD);
    pub static instProperty: LazyLock<TypeName> = lazy!(sn::attribute_kinds::INST_PROPERTY);
    pub static staticProperty: LazyLock<TypeName> = lazy!(sn::attribute_kinds::STATIC_PROPERTY);
    pub static parameter: LazyLock<TypeName> = lazy!(sn::attribute_kinds::PARAMETER);
    pub static typeparam: LazyLock<TypeName> = lazy!(sn::attribute_kinds::TYPE_PARAM);
    pub static file: LazyLock<TypeName> = lazy!(sn::attribute_kinds::FILE);
    pub static typeconst: LazyLock<TypeName> = lazy!(sn::attribute_kinds::TYPE_CONST);
    pub static lambda: LazyLock<TypeName> = lazy!(sn::attribute_kinds::LAMBDA);
    pub static enumcls: LazyLock<TypeName> = lazy!(sn::attribute_kinds::ENUM_CLS);

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *cls,
            *clscst,
            *enum_,
            *typealias,
            *fn_,
            *mthd,
            *instProperty,
            *staticProperty,
            *parameter,
            *typeparam,
            *file,
            *typeconst,
            *lambda,
            *enumcls,
        ]
        .into_iter()
    }
}

pub mod user_attributes {
    use super::*;

    pub static uaUnsafeAllowMultipleInstantiations: LazyLock<TypeName> =
        lazy!(sn::user_attributes::UNSAFE_ALLOW_MULTIPLE_INSTANTIATIONS);
    pub static uaOverride: LazyLock<TypeName> = lazy!(sn::user_attributes::OVERRIDE);
    pub static uaConsistentConstruct: LazyLock<TypeName> =
        lazy!(sn::user_attributes::CONSISTENT_CONSTRUCT);
    pub static uaConst: LazyLock<TypeName> = lazy!(sn::user_attributes::CONST);
    pub static uaTestsBypassVisibility: LazyLock<TypeName> =
        lazy!(sn::user_attributes::TESTS_BYPASS_VISIBILITY);
    pub static uaDeprecated: LazyLock<TypeName> = lazy!(sn::user_attributes::DEPRECATED);
    pub static uaEntryPoint: LazyLock<TypeName> = lazy!(sn::user_attributes::ENTRY_POINT);
    pub static uaMemoize: LazyLock<TypeName> = lazy!(sn::user_attributes::MEMOIZE);
    pub static uaMemoizeLSB: LazyLock<TypeName> = lazy!(sn::user_attributes::MEMOIZE_LSB);
    pub static uaPHPStdLib: LazyLock<TypeName> = lazy!(sn::user_attributes::PHP_STD_LIB);
    pub static uaAcceptDisposable: LazyLock<TypeName> =
        lazy!(sn::user_attributes::ACCEPT_DISPOSABLE);
    pub static uaReturnDisposable: LazyLock<TypeName> =
        lazy!(sn::user_attributes::RETURN_DISPOSABLE);
    pub static uaLSB: LazyLock<TypeName> = lazy!(sn::user_attributes::LSB);
    pub static uaSealed: LazyLock<TypeName> = lazy!(sn::user_attributes::SEALED);
    pub static uaOverlapping: LazyLock<TypeName> = lazy!(sn::user_attributes::OVERLAPPING);
    pub static uaLateInit: LazyLock<TypeName> = lazy!(sn::user_attributes::LATE_INIT);
    pub static uaNewable: LazyLock<TypeName> = lazy!(sn::user_attributes::NEWABLE);
    pub static uaEnforceable: LazyLock<TypeName> = lazy!(sn::user_attributes::ENFORCEABLE);
    pub static uaExplicit: LazyLock<TypeName> = lazy!(sn::user_attributes::EXPLICIT);
    pub static uaNonDisjoint: LazyLock<TypeName> = lazy!(sn::user_attributes::NON_DISJOINT);
    pub static uaSoft: LazyLock<TypeName> = lazy!(sn::user_attributes::SOFT);
    pub static uaWarn: LazyLock<TypeName> = lazy!(sn::user_attributes::WARN);
    pub static uaMockClass: LazyLock<TypeName> = lazy!(sn::user_attributes::MOCK_CLASS);
    pub static uaProvenanceSkipFrame: LazyLock<TypeName> =
        lazy!(sn::user_attributes::PROVENANCE_SKIP_FRAME);
    pub static uaDynamicallyCallable: LazyLock<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_CALLABLE);
    pub static uaDynamicallyConstructible: LazyLock<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_CONSTRUCTIBLE);
    pub static uaDynamicallyReferenced: LazyLock<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_REFERENCED);
    pub static uaReifiable: LazyLock<TypeName> = lazy!(sn::user_attributes::REIFIABLE);
    pub static uaNeverInline: LazyLock<TypeName> = lazy!(sn::user_attributes::NEVER_INLINE);
    pub static uaDisableTypecheckerInternal: LazyLock<TypeName> =
        lazy!(sn::user_attributes::DISABLE_TYPECHECKER_INTERNAL);
    pub static uaHasTopLevelCode: LazyLock<TypeName> =
        lazy!(sn::user_attributes::HAS_TOP_LEVEL_CODE);
    pub static uaIsFoldable: LazyLock<TypeName> = lazy!(sn::user_attributes::IS_FOLDABLE);
    pub static uaNative: LazyLock<TypeName> = lazy!(sn::user_attributes::NATIVE);
    pub static uaOutOnly: LazyLock<TypeName> = lazy!(sn::user_attributes::OUT_ONLY);
    pub static uaAlwaysInline: LazyLock<TypeName> = lazy!(sn::user_attributes::ALWAYS_INLINE);
    pub static uaEnableUnstableFeatures: LazyLock<TypeName> =
        lazy!(sn::user_attributes::ENABLE_UNSTABLE_FEATURES);
    pub static uaEnumClass: LazyLock<TypeName> = lazy!(sn::user_attributes::ENUM_CLASS);
    pub static uaPolicied: LazyLock<TypeName> = lazy!(sn::user_attributes::POLICIED);
    pub static uaInferFlows: LazyLock<TypeName> = lazy!(sn::user_attributes::INFERFLOWS);
    pub static uaExternal: LazyLock<TypeName> = lazy!(sn::user_attributes::EXTERNAL);
    pub static uaCanCall: LazyLock<TypeName> = lazy!(sn::user_attributes::CAN_CALL);
    pub static uaSupportDynamicType: LazyLock<TypeName> =
        lazy!(sn::user_attributes::SUPPORT_DYNAMIC_TYPE);
    pub static uaNoAutoDynamic: LazyLock<TypeName> = lazy!(sn::user_attributes::NO_AUTO_DYNAMIC);
    pub static uaNoAutoBound: LazyLock<TypeName> = lazy!(sn::user_attributes::NO_AUTO_BOUND);
    pub static uaRequireDynamic: LazyLock<TypeName> = lazy!(sn::user_attributes::REQUIRE_DYNAMIC);
    pub static uaEnableMethodTraitDiamond: LazyLock<TypeName> =
        lazy!(sn::user_attributes::ENABLE_METHOD_TRAIT_DIAMOND);
    pub static uaIgnoreReadonlyLocalErrors: LazyLock<TypeName> =
        lazy!(sn::user_attributes::IGNORE_READONLY_LOCAL_ERRORS);
    pub static uaIgnoreCoeffectLocalErrors: LazyLock<TypeName> =
        lazy!(sn::user_attributes::IGNORE_COEFFECT_LOCAL_ERRORS);
    pub static uaModuleLevelTrait: LazyLock<TypeName> =
        lazy!(sn::user_attributes::MODULE_LEVEL_TRAIT);

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *uaUnsafeAllowMultipleInstantiations,
            *uaOverride,
            *uaConsistentConstruct,
            *uaConst,
            *uaTestsBypassVisibility,
            *uaDeprecated,
            *uaEntryPoint,
            *uaMemoize,
            *uaMemoizeLSB,
            *uaPHPStdLib,
            *uaAcceptDisposable,
            *uaReturnDisposable,
            *uaLSB,
            *uaSealed,
            *uaLateInit,
            *uaNewable,
            *uaEnforceable,
            *uaExplicit,
            *uaNonDisjoint,
            *uaOverlapping,
            *uaSoft,
            *uaWarn,
            *uaMockClass,
            *uaProvenanceSkipFrame,
            *uaDynamicallyCallable,
            *uaDynamicallyConstructible,
            *uaDynamicallyReferenced,
            *uaReifiable,
            *uaNeverInline,
            *uaDisableTypecheckerInternal,
            *uaHasTopLevelCode,
            *uaIsFoldable,
            *uaNative,
            *uaOutOnly,
            *uaAlwaysInline,
            *uaEnableUnstableFeatures,
            *uaEnumClass,
            *uaPolicied,
            *uaInferFlows,
            *uaExternal,
            *uaCanCall,
            *uaSupportDynamicType,
            *uaRequireDynamic,
            *uaEnableMethodTraitDiamond,
            *uaIgnoreReadonlyLocalErrors,
            *uaIgnoreCoeffectLocalErrors,
        ]
        .into_iter()
    }
}

pub mod pre_namespaced_functions {
    use super::*;

    pub static echo: LazyLock<FunName> = lazy!(sn::pre_namespaced_functions::ECHO);
}

pub mod autoimported_functions {
    use super::*;

    pub static invariant_violation: LazyLock<FunName> =
        lazy!(sn::autoimported_functions::INVARIANT_VIOLATION);
    pub static invariant: LazyLock<FunName> = lazy!(sn::autoimported_functions::INVARIANT);
    pub static meth_caller: LazyLock<FunName> = lazy!(sn::autoimported_functions::METH_CALLER);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [*invariant_violation, *invariant, *meth_caller].into_iter()
    }
}

pub mod special_idents {
    use super::*;

    pub static this: LazyLock<Symbol> = lazy!(sn::special_idents::THIS);
    pub static placeholder: LazyLock<Symbol> = lazy!(sn::special_idents::PLACEHOLDER);
    pub static dollardollar: LazyLock<Symbol> = lazy!(sn::special_idents::DOLLAR_DOLLAR);
    pub static tmp_var_prefix: LazyLock<Symbol> = lazy!(sn::special_idents::TMP_VAR_PREFIX);
}

pub mod pseudo_functions {
    use super::*;

    pub static isset: LazyLock<FunName> = lazy!(sn::pseudo_functions::ISSET);
    pub static unset: LazyLock<FunName> = lazy!(sn::pseudo_functions::UNSET);
    pub static hh_show: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_SHOW);
    pub static hh_expect: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_EXPECT);
    pub static hh_expect_equivalent: LazyLock<FunName> =
        lazy!(sn::pseudo_functions::HH_EXPECT_EQUIVALENT);
    pub static hh_show_env: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_SHOW_ENV);
    pub static hh_log_level: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_LOG_LEVEL);
    pub static hh_force_solve: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_FORCE_SOLVE);
    pub static hh_loop_forever: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_LOOP_FOREVER);
    pub static hh_sleep: LazyLock<FunName> = lazy!(sn::pseudo_functions::HH_SLEEP);
    pub static echo: LazyLock<FunName> = lazy!(sn::pseudo_functions::ECHO);
    pub static empty: LazyLock<FunName> = lazy!(sn::pseudo_functions::EMPTY);
    pub static exit: LazyLock<FunName> = lazy!(sn::pseudo_functions::EXIT);
    pub static unsafe_cast: LazyLock<FunName> = lazy!(sn::pseudo_functions::UNSAFE_CAST);
    pub static unsafe_nonnull_cast: LazyLock<FunName> =
        lazy!(sn::pseudo_functions::UNSAFE_NONNULL_CAST);
    pub static enforced_cast: LazyLock<FunName> = lazy!(sn::pseudo_functions::ENFORCED_CAST);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [
            *isset,
            *unset,
            *hh_show,
            *hh_expect,
            *hh_expect_equivalent,
            *hh_show_env,
            *hh_log_level,
            *hh_force_solve,
            *hh_loop_forever,
            *hh_sleep,
            *echo,
            *empty,
            *exit,
            *unsafe_cast,
            *unsafe_nonnull_cast,
            *enforced_cast,
        ]
        .into_iter()
    }
}

pub mod stdlib_functions {
    use super::*;

    pub static get_class: LazyLock<FunName> = lazy!(sn::std_lib_functions::GET_CLASS);
    pub static type_structure: LazyLock<FunName> = lazy!(sn::std_lib_functions::TYPE_STRUCTURE);
    pub static array_mark_legacy: LazyLock<FunName> =
        lazy!(sn::std_lib_functions::ARRAY_MARK_LEGACY);
    pub static array_unmark_legacy: LazyLock<FunName> =
        lazy!(sn::std_lib_functions::ARRAY_UNMARK_LEGACY);
    pub static is_php_array: LazyLock<FunName> = lazy!(sn::std_lib_functions::IS_PHP_ARRAY);
    pub static is_any_array: LazyLock<FunName> = lazy!(sn::std_lib_functions::IS_ANY_ARRAY);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [
            *get_class,
            *type_structure,
            *array_mark_legacy,
            *array_unmark_legacy,
            *is_php_array,
            *is_any_array,
        ]
        .into_iter()
    }
}

pub mod typehints {
    use super::*;

    pub static null: LazyLock<TypeName> = lazy!(sn::typehints::NULL);
    pub static void: LazyLock<TypeName> = lazy!(sn::typehints::VOID);
    pub static resource: LazyLock<TypeName> = lazy!(sn::typehints::RESOURCE);
    pub static num: LazyLock<TypeName> = lazy!(sn::typehints::NUM);
    pub static arraykey: LazyLock<TypeName> = lazy!(sn::typehints::ARRAYKEY);
    pub static noreturn: LazyLock<TypeName> = lazy!(sn::typehints::NORETURN);
    pub static mixed: LazyLock<TypeName> = lazy!(sn::typehints::MIXED);
    pub static nonnull: LazyLock<TypeName> = lazy!(sn::typehints::NONNULL);
    pub static this: LazyLock<TypeName> = lazy!(sn::typehints::THIS);
    pub static dynamic: LazyLock<TypeName> = lazy!(sn::typehints::DYNAMIC);
    pub static nothing: LazyLock<TypeName> = lazy!(sn::typehints::NOTHING);
    pub static int: LazyLock<TypeName> = lazy!(sn::typehints::INT);
    pub static bool: LazyLock<TypeName> = lazy!(sn::typehints::BOOL);
    pub static float: LazyLock<TypeName> = lazy!(sn::typehints::FLOAT);
    pub static darray: LazyLock<TypeName> = lazy!(sn::typehints::DARRAY);
    pub static varray: LazyLock<TypeName> = lazy!(sn::typehints::VARRAY);
    pub static varray_or_darray: LazyLock<TypeName> = lazy!(sn::typehints::VARRAY_OR_DARRAY);
    pub static vec_or_dict: LazyLock<TypeName> = lazy!(sn::typehints::VEC_OR_DICT);
    pub static callable: LazyLock<TypeName> = lazy!(sn::typehints::CALLABLE);
    pub static object_cast: LazyLock<TypeName> = lazy!(sn::typehints::OBJECT_CAST);
    pub static supportdyn: LazyLock<TypeName> = lazy!(sn::typehints::SUPPORTDYN);
    pub static hh_sypportdyn: LazyLock<TypeName> = lazy!(sn::typehints::HH_SUPPORTDYN);
    pub static wildcard: LazyLock<TypeName> = lazy!(sn::typehints::WILDCARD);
    pub static hh_string: LazyLock<TypeName> = lazy!(sn::typehints::HH_STRING);

    pub static reserved_typehints: LazyLock<HashSet<TypeName>> = LazyLock::new(|| {
        [
            *null,
            *void,
            *resource,
            *num,
            *arraykey,
            *noreturn,
            *mixed,
            *nonnull,
            *this,
            *dynamic,
            *nothing,
            *int,
            *bool,
            *float,
            *darray,
            *varray,
            *varray_or_darray,
            *vec_or_dict,
            *callable,
            *wildcard,
        ]
        .into_iter()
        .collect()
    });
}

pub mod pseudo_consts {
    use super::*;

    pub static g__LINE__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__LINE__);
    pub static g__CLASS__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__CLASS__);
    pub static g__TRAIT__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__TRAIT__);
    pub static g__FILE__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__FILE__);
    pub static g__DIR__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__DIR__);
    pub static g__FUNCTION__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__FUNCTION__);
    pub static g__METHOD__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__METHOD__);
    pub static g__NAMESPACE__: LazyLock<ConstName> = lazy!(sn::pseudo_consts::G__NAMESPACE__);
    pub static g__COMPILER_FRONTEND__: LazyLock<ConstName> =
        lazy!(sn::pseudo_consts::G__COMPILER_FRONTEND__);
    pub static g__FUNCTION_CREDENTIAL__: LazyLock<ConstName> =
        lazy!(sn::pseudo_consts::G__FUNCTION_CREDENTIAL__);
    pub static exit: LazyLock<ConstName> = lazy!(sn::pseudo_consts::EXIT);

    pub fn iter() -> impl Iterator<Item = ConstName> {
        [
            *g__LINE__,
            *g__CLASS__,
            *g__TRAIT__,
            *g__FILE__,
            *g__DIR__,
            *g__FUNCTION__,
            *g__METHOD__,
            *g__NAMESPACE__,
            *g__COMPILER_FRONTEND__,
            *g__FUNCTION_CREDENTIAL__,
            *exit,
        ]
        .into_iter()
    }
}

pub mod fb {
    use super::*;

    pub static cEnum: LazyLock<TypeName> = lazy!(sn::fb::ENUM);
    pub static tInner: LazyLock<TypeConstName> = lazy!(sn::fb::INNER);
    pub static idx: LazyLock<FunName> = lazy!(sn::fb::IDX);
    pub static cTypeStructure: LazyLock<TypeName> = lazy!(sn::fb::TYPE_STRUCTURE);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*cEnum, *cTypeStructure].into_iter()
    }

    pub fn functions() -> impl Iterator<Item = FunName> {
        [*idx].into_iter()
    }
}

pub mod hh {
    use super::*;

    pub static contains: LazyLock<FunName> = lazy!(sn::hh::CONTAINS);
    pub static contains_key: LazyLock<FunName> = lazy!(sn::hh::CONTAINS_KEY);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [*contains, *contains_key].into_iter()
    }
}

pub mod shapes {
    use super::*;

    pub static cShapes: LazyLock<TypeName> = lazy!(sn::shapes::SHAPES);
    pub static idx: LazyLock<MethodName> = lazy!(sn::shapes::IDX);
    pub static at: LazyLock<MethodName> = lazy!(sn::shapes::AT);
    pub static keyExists: LazyLock<MethodName> = lazy!(sn::shapes::KEY_EXISTS);
    pub static removeKey: LazyLock<MethodName> = lazy!(sn::shapes::REMOVE_KEY);
    pub static toArray: LazyLock<MethodName> = lazy!(sn::shapes::TO_ARRAY);
    pub static toDict: LazyLock<MethodName> = lazy!(sn::shapes::TO_DICT);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*cShapes].into_iter()
    }
}

pub mod superglobals {
    use super::*;

    pub static globals: LazyLock<Symbol> = lazy!(sn::superglobals::GLOBALS);
}

pub mod regex {
    use super::*;

    pub static tPattern: LazyLock<TypeName> = lazy!(sn::regex::T_PATTERN);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*tPattern].into_iter()
    }
}

pub mod emitter_special_functions {
    use super::*;

    pub static eval: LazyLock<FunName> = lazy!(sn::emitter_special_functions::EVAL);
    pub static set_frame_metadata: LazyLock<FunName> =
        lazy!(sn::emitter_special_functions::SET_FRAME_METADATA);
    pub static systemlib_reified_generics: LazyLock<FunName> =
        lazy!(sn::emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS);
}

pub mod xhp {
    use super::*;

    pub static pcdata: LazyLock<Symbol> = lazy!(sn::xhp::PCDATA);
    pub static any: LazyLock<Symbol> = lazy!(sn::xhp::ANY);
    pub static empty: LazyLock<Symbol> = lazy!(sn::xhp::EMPTY);
}

pub mod unstable_features {
    use super::*;

    pub static coeffects_provisional: LazyLock<Symbol> =
        lazy!(sn::unstable_features::COEFFECTS_PROVISIONAL);
    pub static readonly: LazyLock<Symbol> = lazy!(sn::unstable_features::READONLY);
    pub static expression_trees: LazyLock<Symbol> = lazy!(sn::unstable_features::EXPRESSION_TREES);
    pub static named_parameters: LazyLock<Symbol> = lazy!(sn::unstable_features::NAMED_PARAMETERS);
}

pub mod coeffects {
    use super::*;

    pub static capability: LazyLock<Symbol> = lazy!("$#capability");
    pub static local_capability: LazyLock<Symbol> = lazy!("$#local_capability");
    pub static contexts: LazyLock<TypeName> = lazy!("\\HH\\Contexts");
    pub static unsafe_contexts: LazyLock<TypeName> = lazy!(concat(*contexts, "\\Unsafe"));
    pub static generated_generic_prefix: LazyLock<Symbol> = lazy!("T/");

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*contexts, *unsafe_contexts].into_iter()
    }

    pub fn is_generated_generic(x: impl AsRef<str>) -> bool {
        x.as_ref().starts_with(generated_generic_prefix.as_str())
    }
}

pub mod readonly {
    use super::*;

    pub static idx: LazyLock<FunName> = lazy!(sn::readonly::IDX);
    pub static as_mut: LazyLock<FunName> = lazy!(sn::readonly::AS_MUT);
}

pub mod capabilities {
    use super::*;

    pub static defaults: LazyLock<TypeName> = lazy!(concat(*coeffects::contexts, "\\defaults"));
    pub static write_props: LazyLock<TypeName> =
        lazy!(concat(*coeffects::contexts, "\\write_props"));

    const prefix: &str = "\\HH\\Capabilities\\";
    pub static writeProperty: LazyLock<TypeName> = lazy!(concat(prefix, "WriteProperty"));
    pub static accessGlobals: LazyLock<TypeName> = lazy!(concat(prefix, "AccessGlobals"));
    pub static readGlobals: LazyLock<TypeName> = lazy!(concat(prefix, "ReadGlobals"));
    pub static system: LazyLock<TypeName> = lazy!(concat(prefix, "System"));
    pub static systemLocal: LazyLock<TypeName> = lazy!(concat(prefix, "SystemLocal"));
    pub static implicitPolicy: LazyLock<TypeName> = lazy!(concat(prefix, "ImplicitPolicy"));
    pub static implicitPolicyLocal: LazyLock<TypeName> =
        lazy!(concat(prefix, "ImplicitPolicyLocal"));
    pub static io: LazyLock<TypeName> = lazy!(concat(prefix, "IO"));
    pub static rx: LazyLock<TypeName> = lazy!(concat(prefix, "Rx"));
    pub static rxLocal: LazyLock<TypeName> = lazy!(concat(*rx, "Local"));

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *defaults,
            *write_props,
            *writeProperty,
            *accessGlobals,
            *readGlobals,
            *system,
            *systemLocal,
            *implicitPolicy,
            *implicitPolicyLocal,
            *io,
            *rx,
            *rxLocal,
        ]
        .into_iter()
    }
}

pub mod expression_trees {
    use super::*;

    pub static makeTree: LazyLock<MethodName> = lazy!(sn::expression_trees::MAKE_TREE);
    pub static intType: LazyLock<MethodName> = lazy!(sn::expression_trees::INT_TYPE);
    pub static floatType: LazyLock<MethodName> = lazy!(sn::expression_trees::FLOAT_TYPE);
    pub static boolType: LazyLock<MethodName> = lazy!(sn::expression_trees::BOOL_TYPE);
    pub static stringType: LazyLock<MethodName> = lazy!(sn::expression_trees::STRING_TYPE);
    pub static voidType: LazyLock<MethodName> = lazy!(sn::expression_trees::VOID_TYPE);
    pub static symbolType: LazyLock<MethodName> = lazy!(sn::expression_trees::SYMBOL_TYPE);
    pub static visitInt: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_INT);
    pub static visitFloat: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_FLOAT);
    pub static visitBool: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_BOOL);
    pub static visitString: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_STRING);
    pub static visitNull: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_NULL);
    pub static visitBinop: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_BINOP);
    pub static visitUnop: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_UNOP);
    pub static visitLocal: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_LOCAL);
    pub static visitLambda: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_LAMBDA);
    pub static visitGlobalFunction: LazyLock<MethodName> =
        lazy!(sn::expression_trees::VISIT_GLOBAL_FUNCTION);
    pub static visitStaticMethod: LazyLock<MethodName> =
        lazy!(sn::expression_trees::VISIT_STATIC_METHOD);
    pub static visitCall: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_CALL);
    pub static visitAssign: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_ASSIGN);
    pub static visitTernary: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_TERNARY);
    pub static visitIf: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_IF);
    pub static visitWhile: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_WHILE);
    pub static visitReturn: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_RETURN);
    pub static visitFor: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_FOR);
    pub static visitBreak: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_BREAK);
    pub static visitContinue: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_CONTINUE);
    pub static visitSubscript: LazyLock<MethodName> = lazy!(sn::expression_trees::VISIT_SUBSCRIPT);
    pub static visitSubscriptAssign: LazyLock<MethodName> =
        lazy!(sn::expression_trees::VISIT_SUBSCRIPT_ASSIGN);
    pub static splice: LazyLock<MethodName> = lazy!(sn::expression_trees::SPLICE);

    pub static dollardollarTmpVar: LazyLock<Symbol> =
        lazy!(sn::expression_trees::DOLLARDOLLAR_TMP_VAR);
}
