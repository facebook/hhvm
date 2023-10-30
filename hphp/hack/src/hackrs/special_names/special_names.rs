// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// These use the same casing as naming_special_names.ml for now.
#![allow(non_upper_case_globals)]

use hash::HashSet;
use naming_special_names_rust as sn;
use once_cell::sync::Lazy;
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
        Lazy::new(|| $value.into())
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

    pub static cParent: Lazy<TypeName> = lazy!(sn::classes::PARENT);
    pub static cStatic: Lazy<TypeName> = lazy!(sn::classes::STATIC);
    pub static cSelf: Lazy<TypeName> = lazy!(sn::classes::SELF);
    pub static cUnknown: Lazy<TypeName> = lazy!(sn::classes::UNKNOWN);
    pub static cAwaitable: Lazy<TypeName> = lazy!(sn::classes::AWAITABLE);
    pub static cGenerator: Lazy<TypeName> = lazy!(sn::classes::GENERATOR);
    pub static cAsyncGenerator: Lazy<TypeName> = lazy!(sn::classes::ASYNC_GENERATOR);
    pub static cHHFormatString: Lazy<TypeName> = lazy!(sn::classes::HH_FORMAT_STRING);
    pub static cHH_BuiltinEnum: Lazy<TypeName> = lazy!(sn::classes::HH_BUILTIN_ENUM);
    pub static cHH_BuiltinEnumClass: Lazy<TypeName> = lazy!(sn::classes::HH_BUILTIN_ENUM_CLASS);
    pub static cHH_BuiltinAbstractEnumClass: Lazy<TypeName> =
        lazy!(sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS);
    pub static cThrowable: Lazy<TypeName> = lazy!(sn::classes::THROWABLE);
    pub static cStdClass: Lazy<TypeName> = lazy!(sn::classes::STD_CLASS);
    pub static cDateTime: Lazy<TypeName> = lazy!(sn::classes::DATE_TIME);
    pub static cDateTimeImmutable: Lazy<TypeName> = lazy!(sn::classes::DATE_TIME_IMMUTABLE);
    pub static cAsyncIterator: Lazy<TypeName> = lazy!(sn::classes::ASYNC_ITERATOR);
    pub static cAsyncKeyedIterator: Lazy<TypeName> = lazy!(sn::classes::ASYNC_KEYED_ITERATOR);
    pub static cStringish: Lazy<TypeName> = lazy!(sn::classes::STRINGISH);
    pub static cStringishObject: Lazy<TypeName> = lazy!(sn::classes::STRINGISH_OBJECT);
    pub static cXHPChild: Lazy<TypeName> = lazy!(sn::classes::XHP_CHILD);
    pub static cIMemoizeParam: Lazy<TypeName> = lazy!(sn::classes::IMEMOIZE_PARAM);
    pub static cUNSAFESingletonMemoizeParam: Lazy<TypeName> =
        lazy!(sn::classes::UNSAFE_SINGLETON_MEMOIZE_PARAM);
    pub static cClassname: Lazy<TypeName> = lazy!(sn::classes::CLASS_NAME);
    pub static cTypename: Lazy<TypeName> = lazy!(sn::classes::TYPE_NAME);
    pub static cIDisposable: Lazy<TypeName> = lazy!(sn::classes::IDISPOSABLE);
    pub static cIAsyncDisposable: Lazy<TypeName> = lazy!(sn::classes::IASYNC_DISPOSABLE);
    pub static cMemberOf: Lazy<TypeName> = lazy!(sn::classes::MEMBER_OF);
    pub static cEnumClassLabel: Lazy<TypeName> = lazy!(sn::classes::ENUM_CLASS_LABEL);
    pub static cSpliceable: Lazy<TypeName> = lazy!(sn::classes::SPLICEABLE);
    pub static cSupportDyn: Lazy<TypeName> = lazy!(sn::classes::SUPPORT_DYN);

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
    pub static cVector: Lazy<TypeName> = lazy!(sn::collections::VECTOR);
    pub static cMutableVector: Lazy<TypeName> = lazy!(sn::collections::MUTABLE_VECTOR);
    pub static cImmVector: Lazy<TypeName> = lazy!(sn::collections::IMM_VECTOR);
    pub static cSet: Lazy<TypeName> = lazy!(sn::collections::SET);
    pub static cConstSet: Lazy<TypeName> = lazy!(sn::collections::CONST_SET);
    pub static cMutableSet: Lazy<TypeName> = lazy!(sn::collections::MUTABLE_SET);
    pub static cImmSet: Lazy<TypeName> = lazy!(sn::collections::IMM_SET);
    pub static cMap: Lazy<TypeName> = lazy!(sn::collections::MAP);
    pub static cMutableMap: Lazy<TypeName> = lazy!(sn::collections::MUTABLE_MAP);
    pub static cImmMap: Lazy<TypeName> = lazy!(sn::collections::IMM_MAP);
    pub static cPair: Lazy<TypeName> = lazy!(sn::collections::PAIR);

    // interfaces
    pub static cContainer: Lazy<TypeName> = lazy!(sn::collections::CONTAINER);
    pub static cKeyedContainer: Lazy<TypeName> = lazy!(sn::collections::KEYED_CONTAINER);
    pub static cTraversable: Lazy<TypeName> = lazy!(sn::collections::TRAVERSABLE);
    pub static cKeyedTraversable: Lazy<TypeName> = lazy!(sn::collections::KEYED_TRAVERSABLE);
    pub static cCollection: Lazy<TypeName> = lazy!(sn::collections::COLLECTION);
    pub static cConstVector: Lazy<TypeName> = lazy!(sn::collections::CONST_VECTOR);
    pub static cConstMap: Lazy<TypeName> = lazy!(sn::collections::CONST_MAP);
    pub static cConstCollection: Lazy<TypeName> = lazy!(sn::collections::CONST_COLLECTION);
    pub static cAnyArray: Lazy<TypeName> = lazy!(sn::collections::ANY_ARRAY);
    pub static cDict: Lazy<TypeName> = lazy!(sn::collections::DICT);
    pub static cVec: Lazy<TypeName> = lazy!(sn::collections::VEC);
    pub static cKeyset: Lazy<TypeName> = lazy!(sn::collections::KEYSET);

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

    pub static mGetInstanceKey: Lazy<MethodName> = lazy!(sn::members::M_GET_INSTANCE_KEY);
    pub static mClass: Lazy<ClassConstName> = lazy!(sn::members::M_CLASS);
    pub static __construct: Lazy<MethodName> = lazy!(sn::members::__CONSTRUCT);
    pub static __destruct: Lazy<MethodName> = lazy!(sn::members::__DESTRUCT);
    pub static __call: Lazy<MethodName> = lazy!(sn::members::__CALL);
    pub static __callStatic: Lazy<MethodName> = lazy!(sn::members::__CALL_STATIC);
    pub static __clone: Lazy<MethodName> = lazy!(sn::members::__CLONE);
    pub static __debugInfo: Lazy<MethodName> = lazy!(sn::members::__DEBUG_INFO);
    pub static __dispose: Lazy<MethodName> = lazy!(sn::members::__DISPOSE);
    pub static __disposeAsync: Lazy<MethodName> = lazy!(sn::members::__DISPOSE_ASYNC);
    pub static __get: Lazy<MethodName> = lazy!(sn::members::__GET);
    pub static __invoke: Lazy<MethodName> = lazy!(sn::members::__INVOKE);
    pub static __isset: Lazy<MethodName> = lazy!(sn::members::__ISSET);
    pub static __set: Lazy<MethodName> = lazy!(sn::members::__SET);
    pub static __set_state: Lazy<MethodName> = lazy!(sn::members::__SET_STATE);
    pub static __sleep: Lazy<MethodName> = lazy!(sn::members::__SLEEP);
    pub static __toString: Lazy<MethodName> = lazy!(sn::members::__TO_STRING);
    pub static __unset: Lazy<MethodName> = lazy!(sn::members::__UNSET);
    pub static __wakeup: Lazy<MethodName> = lazy!(sn::members::__WAKEUP);

    /// Not really a PropName, but it's treated as one in deferred_init_members
    /// of folded decls.
    pub static parentConstruct: Lazy<PropName> = lazy!(concat("parent::", *__construct));
}

pub mod attribute_kinds {
    use super::*;

    pub static cls: Lazy<TypeName> = lazy!(sn::attribute_kinds::CLS);
    pub static clscst: Lazy<TypeName> = lazy!(sn::attribute_kinds::CLS_CST);
    pub static enum_: Lazy<TypeName> = lazy!(sn::attribute_kinds::ENUM);
    pub static typealias: Lazy<TypeName> = lazy!(sn::attribute_kinds::TYPE_ALIAS);
    pub static fn_: Lazy<TypeName> = lazy!(sn::attribute_kinds::FN);
    pub static mthd: Lazy<TypeName> = lazy!(sn::attribute_kinds::MTHD);
    pub static instProperty: Lazy<TypeName> = lazy!(sn::attribute_kinds::INST_PROPERTY);
    pub static staticProperty: Lazy<TypeName> = lazy!(sn::attribute_kinds::STATIC_PROPERTY);
    pub static parameter: Lazy<TypeName> = lazy!(sn::attribute_kinds::PARAMETER);
    pub static typeparam: Lazy<TypeName> = lazy!(sn::attribute_kinds::TYPE_PARAM);
    pub static file: Lazy<TypeName> = lazy!(sn::attribute_kinds::FILE);
    pub static typeconst: Lazy<TypeName> = lazy!(sn::attribute_kinds::TYPE_CONST);
    pub static lambda: Lazy<TypeName> = lazy!(sn::attribute_kinds::LAMBDA);
    pub static enumcls: Lazy<TypeName> = lazy!(sn::attribute_kinds::ENUM_CLS);

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

    pub static uaOverride: Lazy<TypeName> = lazy!(sn::user_attributes::OVERRIDE);
    pub static uaConsistentConstruct: Lazy<TypeName> =
        lazy!(sn::user_attributes::CONSISTENT_CONSTRUCT);
    pub static uaConst: Lazy<TypeName> = lazy!(sn::user_attributes::CONST);
    pub static uaDeprecated: Lazy<TypeName> = lazy!(sn::user_attributes::DEPRECATED);
    pub static uaEntryPoint: Lazy<TypeName> = lazy!(sn::user_attributes::ENTRY_POINT);
    pub static uaMemoize: Lazy<TypeName> = lazy!(sn::user_attributes::MEMOIZE);
    pub static uaMemoizeLSB: Lazy<TypeName> = lazy!(sn::user_attributes::MEMOIZE_LSB);
    pub static uaPHPStdLib: Lazy<TypeName> = lazy!(sn::user_attributes::PHP_STD_LIB);
    pub static uaAcceptDisposable: Lazy<TypeName> = lazy!(sn::user_attributes::ACCEPT_DISPOSABLE);
    pub static uaReturnDisposable: Lazy<TypeName> = lazy!(sn::user_attributes::RETURN_DISPOSABLE);
    pub static uaLSB: Lazy<TypeName> = lazy!(sn::user_attributes::LSB);
    pub static uaSealed: Lazy<TypeName> = lazy!(sn::user_attributes::SEALED);
    pub static uaLateInit: Lazy<TypeName> = lazy!(sn::user_attributes::LATE_INIT);
    pub static uaNewable: Lazy<TypeName> = lazy!(sn::user_attributes::NEWABLE);
    pub static uaEnforceable: Lazy<TypeName> = lazy!(sn::user_attributes::ENFORCEABLE);
    pub static uaExplicit: Lazy<TypeName> = lazy!(sn::user_attributes::EXPLICIT);
    pub static uaNonDisjoint: Lazy<TypeName> = lazy!(sn::user_attributes::NON_DISJOINT);
    pub static uaSoft: Lazy<TypeName> = lazy!(sn::user_attributes::SOFT);
    pub static uaWarn: Lazy<TypeName> = lazy!(sn::user_attributes::WARN);
    pub static uaMockClass: Lazy<TypeName> = lazy!(sn::user_attributes::MOCK_CLASS);
    pub static uaProvenanceSkipFrame: Lazy<TypeName> =
        lazy!(sn::user_attributes::PROVENANCE_SKIP_FRAME);
    pub static uaDynamicallyCallable: Lazy<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_CALLABLE);
    pub static uaDynamicallyConstructible: Lazy<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_CONSTRUCTIBLE);
    pub static uaDynamicallyReferenced: Lazy<TypeName> =
        lazy!(sn::user_attributes::DYNAMICALLY_REFERENCED);
    pub static uaReifiable: Lazy<TypeName> = lazy!(sn::user_attributes::REIFIABLE);
    pub static uaNeverInline: Lazy<TypeName> = lazy!(sn::user_attributes::NEVER_INLINE);
    pub static uaDisableTypecheckerInternal: Lazy<TypeName> =
        lazy!(sn::user_attributes::DISABLE_TYPECHECKER_INTERNAL);
    pub static uaHasTopLevelCode: Lazy<TypeName> = lazy!(sn::user_attributes::HAS_TOP_LEVEL_CODE);
    pub static uaIsFoldable: Lazy<TypeName> = lazy!(sn::user_attributes::IS_FOLDABLE);
    pub static uaNative: Lazy<TypeName> = lazy!(sn::user_attributes::NATIVE);
    pub static uaOutOnly: Lazy<TypeName> = lazy!(sn::user_attributes::OUT_ONLY);
    pub static uaAlwaysInline: Lazy<TypeName> = lazy!(sn::user_attributes::ALWAYS_INLINE);
    pub static uaEnableUnstableFeatures: Lazy<TypeName> =
        lazy!(sn::user_attributes::ENABLE_UNSTABLE_FEATURES);
    pub static uaEnumClass: Lazy<TypeName> = lazy!(sn::user_attributes::ENUM_CLASS);
    pub static uaPolicied: Lazy<TypeName> = lazy!(sn::user_attributes::POLICIED);
    pub static uaInferFlows: Lazy<TypeName> = lazy!(sn::user_attributes::INFERFLOWS);
    pub static uaExternal: Lazy<TypeName> = lazy!(sn::user_attributes::EXTERNAL);
    pub static uaCanCall: Lazy<TypeName> = lazy!(sn::user_attributes::CAN_CALL);
    pub static uaSupportDynamicType: Lazy<TypeName> =
        lazy!(sn::user_attributes::SUPPORT_DYNAMIC_TYPE);
    pub static uaNoAutoDynamic: Lazy<TypeName> = lazy!(sn::user_attributes::NO_AUTO_DYNAMIC);
    pub static uaNoAutoBound: Lazy<TypeName> = lazy!(sn::user_attributes::NO_AUTO_BOUND);
    pub static uaRequireDynamic: Lazy<TypeName> = lazy!(sn::user_attributes::REQUIRE_DYNAMIC);
    pub static uaEnableMethodTraitDiamond: Lazy<TypeName> =
        lazy!(sn::user_attributes::ENABLE_METHOD_TRAIT_DIAMOND);
    pub static uaIgnoreReadonlyLocalErrors: Lazy<TypeName> =
        lazy!(sn::user_attributes::IGNORE_READONLY_LOCAL_ERRORS);
    pub static uaIgnoreCoeffectLocalErrors: Lazy<TypeName> =
        lazy!(sn::user_attributes::IGNORE_COEFFECT_LOCAL_ERRORS);
    pub static uaModuleLevelTrait: Lazy<TypeName> = lazy!(sn::user_attributes::MODULE_LEVEL_TRAIT);

    pub fn iter() -> impl Iterator<Item = TypeName> {
        [
            *uaOverride,
            *uaConsistentConstruct,
            *uaConst,
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

pub mod special_functions {
    use super::*;

    pub static echo: Lazy<FunName> = lazy!(sn::special_functions::ECHO);
}

pub mod autoimported_functions {
    use super::*;

    pub static invariant_violation: Lazy<FunName> =
        lazy!(sn::autoimported_functions::INVARIANT_VIOLATION);
    pub static invariant: Lazy<FunName> = lazy!(sn::autoimported_functions::INVARIANT);
    pub static meth_caller: Lazy<FunName> = lazy!(sn::autoimported_functions::METH_CALLER);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [*invariant_violation, *invariant, *meth_caller].into_iter()
    }
}

pub mod special_idents {
    use super::*;

    pub static this: Lazy<Symbol> = lazy!(sn::special_idents::THIS);
    pub static placeholder: Lazy<Symbol> = lazy!(sn::special_idents::PLACEHOLDER);
    pub static dollardollar: Lazy<Symbol> = lazy!(sn::special_idents::DOLLAR_DOLLAR);
    pub static tmp_var_prefix: Lazy<Symbol> = lazy!(sn::special_idents::TMP_VAR_PREFIX);
}

pub mod pseudo_functions {
    use super::*;

    pub static isset: Lazy<FunName> = lazy!(sn::pseudo_functions::ISSET);
    pub static unset: Lazy<FunName> = lazy!(sn::pseudo_functions::UNSET);
    pub static hh_show: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_SHOW);
    pub static hh_expect: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_EXPECT);
    pub static hh_expect_equivalent: Lazy<FunName> =
        lazy!(sn::pseudo_functions::HH_EXPECT_EQUIVALENT);
    pub static hh_show_env: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_SHOW_ENV);
    pub static hh_log_level: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_LOG_LEVEL);
    pub static hh_force_solve: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_FORCE_SOLVE);
    pub static hh_loop_forever: Lazy<FunName> = lazy!(sn::pseudo_functions::HH_LOOP_FOREVER);
    pub static echo: Lazy<FunName> = lazy!(sn::pseudo_functions::ECHO);
    pub static empty: Lazy<FunName> = lazy!(sn::pseudo_functions::EMPTY);
    pub static exit: Lazy<FunName> = lazy!(sn::pseudo_functions::EXIT);
    pub static die: Lazy<FunName> = lazy!(sn::pseudo_functions::DIE);
    pub static unsafe_cast: Lazy<FunName> = lazy!(sn::pseudo_functions::UNSAFE_CAST);
    pub static unsafe_nonnull_cast: Lazy<FunName> =
        lazy!(sn::pseudo_functions::UNSAFE_NONNULL_CAST);
    pub static enforced_cast: Lazy<FunName> = lazy!(sn::pseudo_functions::ENFORCED_CAST);

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
            *echo,
            *empty,
            *exit,
            *die,
            *unsafe_cast,
            *unsafe_nonnull_cast,
            *enforced_cast,
        ]
        .into_iter()
    }
}

pub mod stdlib_functions {
    use super::*;

    pub static is_array: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_ARRAY);
    pub static is_null: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_NULL);
    pub static get_class: Lazy<FunName> = lazy!(sn::std_lib_functions::GET_CLASS);
    pub static array_filter: Lazy<FunName> = lazy!(sn::std_lib_functions::ARRAY_FILTER);
    pub static call_user_func: Lazy<FunName> = lazy!(sn::std_lib_functions::CALL_USER_FUNC);
    pub static type_structure: Lazy<FunName> = lazy!(sn::std_lib_functions::TYPE_STRUCTURE);
    pub static array_mark_legacy: Lazy<FunName> = lazy!(sn::std_lib_functions::ARRAY_MARK_LEGACY);
    pub static array_unmark_legacy: Lazy<FunName> =
        lazy!(sn::std_lib_functions::ARRAY_UNMARK_LEGACY);
    pub static is_php_array: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_PHP_ARRAY);
    pub static is_any_array: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_ANY_ARRAY);
    pub static is_dict_or_darray: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_DICT_OR_DARRAY);
    pub static is_vec_or_varray: Lazy<FunName> = lazy!(sn::std_lib_functions::IS_VEC_OR_VARRAY);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [
            *is_array,
            *is_null,
            *get_class,
            *array_filter,
            *call_user_func,
            *type_structure,
            *array_mark_legacy,
            *array_unmark_legacy,
            *is_php_array,
            *is_any_array,
            *is_dict_or_darray,
            *is_vec_or_varray,
        ]
        .into_iter()
    }
}

pub mod typehints {
    use super::*;

    pub static null: Lazy<TypeName> = lazy!(sn::typehints::NULL);
    pub static void: Lazy<TypeName> = lazy!(sn::typehints::VOID);
    pub static resource: Lazy<TypeName> = lazy!(sn::typehints::RESOURCE);
    pub static num: Lazy<TypeName> = lazy!(sn::typehints::NUM);
    pub static arraykey: Lazy<TypeName> = lazy!(sn::typehints::ARRAYKEY);
    pub static noreturn: Lazy<TypeName> = lazy!(sn::typehints::NORETURN);
    pub static mixed: Lazy<TypeName> = lazy!(sn::typehints::MIXED);
    pub static nonnull: Lazy<TypeName> = lazy!(sn::typehints::NONNULL);
    pub static this: Lazy<TypeName> = lazy!(sn::typehints::THIS);
    pub static dynamic: Lazy<TypeName> = lazy!(sn::typehints::DYNAMIC);
    pub static nothing: Lazy<TypeName> = lazy!(sn::typehints::NOTHING);
    pub static int: Lazy<TypeName> = lazy!(sn::typehints::INT);
    pub static bool: Lazy<TypeName> = lazy!(sn::typehints::BOOL);
    pub static float: Lazy<TypeName> = lazy!(sn::typehints::FLOAT);
    pub static string: Lazy<TypeName> = lazy!(sn::typehints::STRING);
    pub static darray: Lazy<TypeName> = lazy!(sn::typehints::DARRAY);
    pub static varray: Lazy<TypeName> = lazy!(sn::typehints::VARRAY);
    pub static varray_or_darray: Lazy<TypeName> = lazy!(sn::typehints::VARRAY_OR_DARRAY);
    pub static vec_or_dict: Lazy<TypeName> = lazy!(sn::typehints::VEC_OR_DICT);
    pub static callable: Lazy<TypeName> = lazy!(sn::typehints::CALLABLE);
    pub static object_cast: Lazy<TypeName> = lazy!(sn::typehints::OBJECT_CAST);
    pub static supportdyn: Lazy<TypeName> = lazy!(sn::typehints::SUPPORTDYN);
    pub static hh_sypportdyn: Lazy<TypeName> = lazy!(sn::typehints::HH_SUPPORTDYN);
    pub static wildcard: Lazy<TypeName> = lazy!(sn::typehints::WILDCARD);

    pub static reserved_typehints: Lazy<HashSet<TypeName>> = Lazy::new(|| {
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
            *string,
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

    pub static g__LINE__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__LINE__);
    pub static g__CLASS__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__CLASS__);
    pub static g__TRAIT__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__TRAIT__);
    pub static g__FILE__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__FILE__);
    pub static g__DIR__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__DIR__);
    pub static g__FUNCTION__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__FUNCTION__);
    pub static g__METHOD__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__METHOD__);
    pub static g__NAMESPACE__: Lazy<ConstName> = lazy!(sn::pseudo_consts::G__NAMESPACE__);
    pub static g__COMPILER_FRONTEND__: Lazy<ConstName> =
        lazy!(sn::pseudo_consts::G__COMPILER_FRONTEND__);
    pub static g__FUNCTION_CREDENTIAL__: Lazy<ConstName> =
        lazy!(sn::pseudo_consts::G__FUNCTION_CREDENTIAL__);
    pub static exit: Lazy<ConstName> = lazy!(sn::pseudo_consts::EXIT);
    pub static die: Lazy<ConstName> = lazy!(sn::pseudo_consts::DIE);
}

pub mod fb {
    use super::*;

    pub static cEnum: Lazy<TypeName> = lazy!(sn::fb::ENUM);
    pub static tInner: Lazy<TypeConstName> = lazy!(sn::fb::INNER);
    pub static idx: Lazy<FunName> = lazy!(sn::fb::IDX);
    pub static cTypeStructure: Lazy<TypeName> = lazy!(sn::fb::TYPE_STRUCTURE);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*cEnum, *cTypeStructure].into_iter()
    }

    pub fn functions() -> impl Iterator<Item = FunName> {
        [*idx].into_iter()
    }
}

pub mod hh {
    use super::*;

    pub static contains: Lazy<FunName> = lazy!(sn::hh::CONTAINS);
    pub static contains_key: Lazy<FunName> = lazy!(sn::hh::CONTAINS_KEY);

    pub fn iter() -> impl Iterator<Item = FunName> {
        [*contains, *contains_key].into_iter()
    }
}

pub mod shapes {
    use super::*;

    pub static cShapes: Lazy<TypeName> = lazy!(sn::shapes::SHAPES);
    pub static idx: Lazy<MethodName> = lazy!(sn::shapes::IDX);
    pub static at: Lazy<MethodName> = lazy!(sn::shapes::AT);
    pub static keyExists: Lazy<MethodName> = lazy!(sn::shapes::KEY_EXISTS);
    pub static removeKey: Lazy<MethodName> = lazy!(sn::shapes::REMOVE_KEY);
    pub static toArray: Lazy<MethodName> = lazy!(sn::shapes::TO_ARRAY);
    pub static toDict: Lazy<MethodName> = lazy!(sn::shapes::TO_DICT);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*cShapes].into_iter()
    }
}

pub mod superglobals {
    use super::*;

    pub static globals: Lazy<Symbol> = lazy!(sn::superglobals::GLOBALS);
}

pub mod regex {
    use super::*;

    pub static tPattern: Lazy<TypeName> = lazy!(sn::regex::T_PATTERN);

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*tPattern].into_iter()
    }
}

pub mod emitter_special_functions {
    use super::*;

    pub static eval: Lazy<FunName> = lazy!(sn::emitter_special_functions::EVAL);
    pub static set_frame_metadata: Lazy<FunName> =
        lazy!(sn::emitter_special_functions::SET_FRAME_METADATA);
    pub static systemlib_reified_generics: Lazy<FunName> =
        lazy!(sn::emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS);
}

pub mod xhp {
    use super::*;

    pub static pcdata: Lazy<Symbol> = lazy!(sn::xhp::PCDATA);
    pub static any: Lazy<Symbol> = lazy!(sn::xhp::ANY);
    pub static empty: Lazy<Symbol> = lazy!(sn::xhp::EMPTY);
}

pub mod unstable_features {
    use super::*;

    pub static coeffects_provisional: Lazy<Symbol> =
        lazy!(sn::unstable_features::COEFFECTS_PROVISIONAL);
    pub static readonly: Lazy<Symbol> = lazy!(sn::unstable_features::READONLY);
    pub static expression_trees: Lazy<Symbol> = lazy!(sn::unstable_features::EXPRESSION_TREES);
    pub static modules: Lazy<Symbol> = lazy!(sn::unstable_features::MODULES);
}

pub mod coeffects {
    use super::*;

    pub static capability: Lazy<Symbol> = lazy!("$#capability");
    pub static local_capability: Lazy<Symbol> = lazy!("$#local_capability");
    pub static contexts: Lazy<TypeName> = lazy!("\\HH\\Contexts");
    pub static unsafe_contexts: Lazy<TypeName> = lazy!(concat(*contexts, "\\Unsafe"));
    pub static generated_generic_prefix: Lazy<Symbol> = lazy!("T/");

    pub fn types() -> impl Iterator<Item = TypeName> {
        [*contexts, *unsafe_contexts].into_iter()
    }

    pub fn is_generated_generic(x: impl AsRef<str>) -> bool {
        x.as_ref().starts_with(generated_generic_prefix.as_str())
    }
}

pub mod readonly {
    use super::*;

    pub static idx: Lazy<FunName> = lazy!(sn::readonly::IDX);
    pub static as_mut: Lazy<FunName> = lazy!(sn::readonly::AS_MUT);
}

pub mod capabilities {
    use super::*;

    pub static defaults: Lazy<TypeName> = lazy!(concat(*coeffects::contexts, "\\defaults"));
    pub static write_props: Lazy<TypeName> = lazy!(concat(*coeffects::contexts, "\\write_props"));

    const prefix: &str = "\\HH\\Capabilities\\";
    pub static writeProperty: Lazy<TypeName> = lazy!(concat(prefix, "WriteProperty"));
    pub static accessGlobals: Lazy<TypeName> = lazy!(concat(prefix, "AccessGlobals"));
    pub static readGlobals: Lazy<TypeName> = lazy!(concat(prefix, "ReadGlobals"));
    pub static system: Lazy<TypeName> = lazy!(concat(prefix, "System"));
    pub static systemLocal: Lazy<TypeName> = lazy!(concat(prefix, "SystemLocal"));
    pub static implicitPolicy: Lazy<TypeName> = lazy!(concat(prefix, "ImplicitPolicy"));
    pub static implicitPolicyLocal: Lazy<TypeName> = lazy!(concat(prefix, "ImplicitPolicyLocal"));
    pub static io: Lazy<TypeName> = lazy!(concat(prefix, "IO"));
    pub static rx: Lazy<TypeName> = lazy!(concat(prefix, "Rx"));
    pub static rxLocal: Lazy<TypeName> = lazy!(concat(*rx, "Local"));

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

    pub static makeTree: Lazy<MethodName> = lazy!(sn::expression_trees::MAKE_TREE);
    pub static intType: Lazy<MethodName> = lazy!(sn::expression_trees::INT_TYPE);
    pub static floatType: Lazy<MethodName> = lazy!(sn::expression_trees::FLOAT_TYPE);
    pub static boolType: Lazy<MethodName> = lazy!(sn::expression_trees::BOOL_TYPE);
    pub static stringType: Lazy<MethodName> = lazy!(sn::expression_trees::STRING_TYPE);
    pub static nullType: Lazy<MethodName> = lazy!(sn::expression_trees::NULL_TYPE);
    pub static voidType: Lazy<MethodName> = lazy!(sn::expression_trees::VOID_TYPE);
    pub static symbolType: Lazy<MethodName> = lazy!(sn::expression_trees::SYMBOL_TYPE);
    pub static visitInt: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_INT);
    pub static visitFloat: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_FLOAT);
    pub static visitBool: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_BOOL);
    pub static visitString: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_STRING);
    pub static visitNull: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_NULL);
    pub static visitBinop: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_BINOP);
    pub static visitUnop: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_UNOP);
    pub static visitLocal: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_LOCAL);
    pub static visitLambda: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_LAMBDA);
    pub static visitGlobalFunction: Lazy<MethodName> =
        lazy!(sn::expression_trees::VISIT_GLOBAL_FUNCTION);
    pub static visitStaticMethod: Lazy<MethodName> =
        lazy!(sn::expression_trees::VISIT_STATIC_METHOD);
    pub static visitCall: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_CALL);
    pub static visitAssign: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_ASSIGN);
    pub static visitTernary: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_TERNARY);
    pub static visitIf: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_IF);
    pub static visitWhile: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_WHILE);
    pub static visitReturn: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_RETURN);
    pub static visitFor: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_FOR);
    pub static visitBreak: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_BREAK);
    pub static visitContinue: Lazy<MethodName> = lazy!(sn::expression_trees::VISIT_CONTINUE);
    pub static splice: Lazy<MethodName> = lazy!(sn::expression_trees::SPLICE);

    pub static dollardollarTmpVar: Lazy<Symbol> = lazy!(sn::expression_trees::DOLLARDOLLAR_TMP_VAR);
}
