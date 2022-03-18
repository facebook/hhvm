// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// These use the same casing as naming_special_names.ml for now.
#![allow(non_snake_case)]

use naming_special_names_rust as sn;
use pos::{ClassConstName, Symbol, TypeConstName, TypeName};
use std::collections::HashSet;

#[derive(Debug)]
pub struct SpecialNames {
    pub classes: Classes,
    pub collections: Collections,
    pub members: Members,
    pub attribute_kinds: AttributeKinds,
    pub user_attributes: UserAttributes,
    pub special_functions: SpecialFunctions,
    pub autoimported_functions: AutoimportedFunctions,
    pub special_idents: SpecialIdents,
    pub pseudo_functions: PseudoFunctions,
    pub stdlib_functions: StdlibFunctions,
    pub typehints: Typehints,
    pub pseudo_consts: PseudoConsts,
    pub fb: Fb,
    pub hh: Hh,
    pub shapes: Shapes,
    pub superglobals: Superglobals,
    pub regex: Regex,
    pub emitter_special_functions: EmitterSpecialFunctions,
    pub xhp: Xhp,
    pub unstable_features: UnstableFeatures,
    pub coeffects: Coeffects,
    pub readonly: Readonly,
    pub capabilities: Capabilities,
    pub expression_trees: ExpressionTrees,
}

impl SpecialNames {
    pub fn new() -> &'static Self {
        let coeffects = Coeffects::new();
        let capabilities = Capabilities::new(&coeffects);
        Box::leak(Box::new(Self {
            classes: Classes::new(),
            collections: Collections::new(),
            members: Members::new(),
            attribute_kinds: AttributeKinds::new(),
            user_attributes: UserAttributes::new(),
            special_functions: SpecialFunctions::new(),
            autoimported_functions: AutoimportedFunctions::new(),
            special_idents: SpecialIdents::new(),
            pseudo_functions: PseudoFunctions::new(),
            stdlib_functions: StdlibFunctions::new(),
            typehints: Typehints::new(),
            pseudo_consts: PseudoConsts::new(),
            fb: Fb::new(),
            hh: Hh::new(),
            shapes: Shapes::new(),
            superglobals: Superglobals::new(),
            regex: Regex::new(),
            emitter_special_functions: EmitterSpecialFunctions::new(),
            xhp: Xhp::new(),
            unstable_features: UnstableFeatures::new(),
            coeffects,
            readonly: Readonly::new(),
            capabilities,
            expression_trees: ExpressionTrees::new(),
        }))
    }
}

#[derive(Debug)]
pub struct Classes {
    pub cParent: TypeName,
    pub cStatic: TypeName,
    pub cSelf: TypeName,
    pub cUnknown: TypeName,

    // Used for dynamic classnames, e.g. new $foo();
    pub cAwaitable: TypeName,
    pub cGenerator: TypeName,
    pub cAsyncGenerator: TypeName,
    pub cHHFormatString: TypeName,
    pub cHH_BuiltinEnum: TypeName,
    pub cHH_BuiltinEnumClass: TypeName,
    pub cHH_BuiltinAbstractEnumClass: TypeName,
    pub cThrowable: TypeName,
    pub cStdClass: TypeName,
    pub cDateTime: TypeName,
    pub cDateTimeImmutable: TypeName,
    pub cAsyncIterator: TypeName,
    pub cAsyncKeyedIterator: TypeName,
    pub cStringish: TypeName,
    pub cStringishObject: TypeName,
    pub cXHPChild: TypeName,
    pub cIMemoizeParam: TypeName,
    pub cUNSAFESingletonMemoizeParam: TypeName,
    pub cClassname: TypeName,
    pub cTypename: TypeName,
    pub cIDisposable: TypeName,
    pub cIAsyncDisposable: TypeName,
    pub cMemberOf: TypeName,
    pub cEnumClassLabel: TypeName,

    /// Classes that can be spliced into ExpressionTrees
    pub cSpliceable: TypeName,
    pub cSupportDyn: TypeName,
}

#[derive(Debug)]
pub struct Collections {
    // concrete classes
    pub cVector: Symbol,
    pub cMutableVector: Symbol,
    pub cImmVector: Symbol,
    pub cSet: Symbol,
    pub cConstSet: Symbol,
    pub cMutableSet: Symbol,
    pub cImmSet: Symbol,
    pub cMap: Symbol,
    pub cMutableMap: Symbol,
    pub cImmMap: Symbol,
    pub cPair: Symbol,

    // interfaces
    pub cContainer: Symbol,
    pub cKeyedContainer: Symbol,
    pub cTraversable: Symbol,
    pub cKeyedTraversable: Symbol,
    pub cCollection: Symbol,
    pub cConstVector: Symbol,
    pub cConstMap: Symbol,
    pub cConstCollection: Symbol,
    pub cAnyArray: Symbol,
    pub cDict: Symbol,
    pub cVec: Symbol,
    pub cKeyset: Symbol,
}

#[derive(Debug)]
pub struct Members {
    pub mGetInstanceKey: Symbol,
    pub mClass: ClassConstName,
    pub parentConstruct: Symbol,
    pub __construct: Symbol,
    pub __destruct: Symbol,
    pub __call: Symbol,
    pub __callStatic: Symbol,
    pub __clone: Symbol,
    pub __debugInfo: Symbol,
    pub __dispose: Symbol,
    pub __disposeAsync: Symbol,
    pub __get: Symbol,
    pub __invoke: Symbol,
    pub __isset: Symbol,
    pub __set: Symbol,
    pub __set_state: Symbol,
    pub __sleep: Symbol,
    pub __toString: Symbol,
    pub __unset: Symbol,
    pub __wakeup: Symbol,
}

#[derive(Debug)]
pub struct AttributeKinds {
    pub cls: Symbol,
    pub clscst: Symbol,
    pub enum_: Symbol,
    pub typealias: Symbol,
    pub fn_: Symbol,
    pub mthd: Symbol,
    pub instProperty: Symbol,
    pub staticProperty: Symbol,
    pub parameter: Symbol,
    pub typeparam: Symbol,
    pub file: Symbol,
    pub typeconst: Symbol,
    pub lambda: Symbol,
    pub enumcls: Symbol,
}

#[derive(Debug)]
pub struct UserAttributes {
    pub uaOverride: TypeName,
    pub uaConsistentConstruct: TypeName,
    pub uaConst: TypeName,
    pub uaDeprecated: TypeName,
    pub uaEntryPoint: TypeName,
    pub uaMemoize: TypeName,
    pub uaMemoizeLSB: TypeName,
    pub uaPolicyShardedMemoize: TypeName,
    pub uaPolicyShardedMemoizeLSB: TypeName,
    pub uaPHPStdLib: TypeName,
    pub uaAcceptDisposable: TypeName,
    pub uaReturnDisposable: TypeName,
    pub uaLSB: TypeName,
    pub uaSealed: TypeName,
    pub uaLateInit: TypeName,
    pub uaNewable: TypeName,
    pub uaEnforceable: TypeName,
    pub uaExplicit: TypeName,
    pub uaNonDisjoint: TypeName,
    pub uaSoft: TypeName,
    pub uaWarn: TypeName,
    pub uaMockClass: TypeName,
    pub uaProvenanceSkipFrame: TypeName,
    pub uaDynamicallyCallable: TypeName,
    pub uaDynamicallyConstructible: TypeName,
    pub uaReifiable: TypeName,
    pub uaNeverInline: TypeName,
    pub uaDisableTypecheckerInternal: TypeName,
    pub uaHasTopLevelCode: TypeName,
    pub uaIsFoldable: TypeName,
    pub uaNative: TypeName,
    pub uaOutOnly: TypeName,
    pub uaAlwaysInline: TypeName,
    pub uaEnableUnstableFeatures: TypeName,
    pub uaEnumClass: TypeName,
    pub uaPolicied: TypeName,
    pub uaInferFlows: TypeName,
    pub uaExternal: TypeName,
    pub uaCanCall: TypeName,
    pub uaSupportDynamicType: TypeName,
    pub uaRequireDynamic: TypeName,
    pub uaModule: TypeName,
    pub uaInternal: TypeName,
    pub uaEnableMethodTraitDiamond: TypeName,
    pub uaIgnoreReadonlyLocalErrors: TypeName,
    pub uaIgnoreCoeffectLocalErrors: TypeName,
}

/// Tested before \\-prepending name-canonicalization
#[derive(Debug)]
pub struct SpecialFunctions {
    pub echo: Symbol,
    pub hhas_adata: Symbol,
}

/// There are a number of functions that are automatically imported into the
/// namespace. The full list can be found in hh_autoimport.ml.
#[derive(Debug)]
pub struct AutoimportedFunctions {
    pub invariant_violation: Symbol,
    pub invariant: Symbol,
    pub fun_: Symbol,
    pub inst_meth: Symbol,
    pub class_meth: Symbol,
    pub meth_caller: Symbol,
}

#[derive(Debug)]
pub struct SpecialIdents {
    pub this: Symbol,
    pub placeholder: Symbol,
    pub dollardollar: Symbol,
    /// Intentionally uses an invalid variable name to ensure it's translated
    pub tmp_var_prefix: Symbol,
}

/// PseudoFunctions are functions (or items that are parsed like functions)
/// that are treated like builtins that do not have a public HHI or interface.
#[derive(Debug)]
pub struct PseudoFunctions {
    pub isset: Symbol,
    pub unset: Symbol,
    pub hh_show: Symbol,
    pub hh_expect: Symbol,
    pub hh_expect_equivalent: Symbol,
    pub hh_show_env: Symbol,
    pub hh_log_level: Symbol,
    pub hh_force_solve: Symbol,
    pub hh_loop_forever: Symbol,
    pub echo: Symbol,
    pub empty: Symbol,
    pub exit: Symbol,
    pub die: Symbol,
    pub unsafe_cast: Symbol,
    pub enforced_cast: Symbol,
}

#[derive(Debug)]
pub struct StdlibFunctions {
    pub is_array: Symbol,
    pub is_null: Symbol,
    pub get_class: Symbol,
    pub array_filter: Symbol,
    pub call_user_func: Symbol,
    pub type_structure: Symbol,
    pub array_mark_legacy: Symbol,
    pub array_unmark_legacy: Symbol,
    pub is_php_array: Symbol,
    pub is_any_array: Symbol,
    pub is_dict_or_darray: Symbol,
    pub is_vec_or_varray: Symbol,
}

#[derive(Debug)]
pub struct Typehints {
    pub null: Symbol,
    pub void: Symbol,
    pub resource: Symbol,
    pub num: Symbol,
    pub arraykey: Symbol,
    pub noreturn: Symbol,
    pub mixed: Symbol,
    pub nonnull: Symbol,
    pub this: Symbol,
    pub dynamic: Symbol,
    pub supportdynamic: Symbol,
    pub nothing: Symbol,
    pub int: Symbol,
    pub bool: Symbol,
    pub float: Symbol,
    pub string: Symbol,
    pub darray: Symbol,
    pub varray: Symbol,
    pub varray_or_darray: Symbol,
    pub vec_or_dict: Symbol,
    pub callable: Symbol,
    pub object_cast: Symbol,
    pub supportdyn: Symbol,
    pub hh_sypportdyn: Symbol,
    pub wildcard: Symbol,

    pub reserved_typehints: HashSet<Symbol>,
}

#[derive(Debug)]
pub struct PseudoConsts {
    pub g__LINE__: Symbol,
    pub g__CLASS__: Symbol,
    pub g__TRAIT__: Symbol,
    pub g__FILE__: Symbol,
    pub g__DIR__: Symbol,
    pub g__FUNCTION__: Symbol,
    pub g__METHOD__: Symbol,
    pub g__NAMESPACE__: Symbol,
    pub g__COMPILER_FRONTEND__: Symbol,
    pub g__FUNCTION_CREDENTIAL__: Symbol,

    // exit and die are not pseudo consts, but they are currently parsed as such.
    // Would be more correct to parse them as special statements like return
    pub exit: Symbol,
    pub die: Symbol,
}

#[derive(Debug)]
pub struct Fb {
    pub cEnum: TypeName,
    pub tInner: TypeConstName,
    pub idx: Symbol,
    pub cTypeStructure: TypeName,
    pub cIncorrectType: TypeName,
}

#[derive(Debug)]
pub struct Hh {
    pub contains: Symbol,
    pub contains_key: Symbol,
}

#[derive(Debug)]
pub struct Shapes {
    pub cShapes: Symbol,
    pub idx: Symbol,
    pub at: Symbol,
    pub keyExists: Symbol,
    pub removeKey: Symbol,
    pub toArray: Symbol,
    pub toDict: Symbol,
}

#[derive(Debug)]
pub struct Superglobals {
    pub globals: Symbol,
}

#[derive(Debug)]
pub struct Regex {
    pub tPattern: Symbol,
}

/// These are functions treated by the emitter specially. They are not
/// autoimported (see hh_autoimport.ml) nor are they consider PseudoFunctions
/// so they can be overridden by namespacing (at least currently)
#[derive(Debug)]
pub struct EmitterSpecialFunctions {
    pub eval: Symbol,
    pub set_frame_metadata: Symbol,
    pub systemlib_reified_generics: Symbol,
}

#[derive(Debug)]
pub struct Xhp {
    pub pcdata: Symbol,
    pub any: Symbol,
    pub empty: Symbol,
}

/// This should be a subset of rust_parser_errors::UnstableFeatures that is relevant
/// to the typechecker
#[derive(Debug)]
pub struct UnstableFeatures {
    pub coeffects_provisional: Symbol,
    pub ifc: Symbol,
    pub readonly: Symbol,
    pub expression_trees: Symbol,
    pub modules: Symbol,
}

#[derive(Debug)]
pub struct Coeffects {
    pub capability: Symbol,
    pub local_capability: Symbol,
    pub contexts: Symbol,
    pub unsafe_contexts: Symbol,
    pub generated_generic_prefix: Symbol,
}

#[derive(Debug)]
pub struct Readonly {
    pub idx: Symbol,
    pub as_mut: Symbol,
}

#[derive(Debug)]
pub struct Capabilities {
    pub defaults: Symbol,
    pub write_props: Symbol,
    pub writeProperty: Symbol,
    pub accessGlobals: Symbol,
    pub readGlobals: Symbol,
    pub system: Symbol,
    pub implicitPolicy: Symbol,
    pub implicitPolicyLocal: Symbol,
    pub io: Symbol,
    pub rx: Symbol,
    pub rxLocal: Symbol,
}

#[derive(Debug)]
pub struct ExpressionTrees {
    pub makeTree: Symbol,
    pub intType: Symbol,
    pub floatType: Symbol,
    pub boolType: Symbol,
    pub stringType: Symbol,
    pub nullType: Symbol,
    pub voidType: Symbol,
    pub symbolType: Symbol,
    pub visitInt: Symbol,
    pub visitFloat: Symbol,
    pub visitBool: Symbol,
    pub visitString: Symbol,
    pub visitNull: Symbol,
    pub visitBinop: Symbol,
    pub visitUnop: Symbol,
    pub visitLocal: Symbol,
    pub visitLambda: Symbol,
    pub visitGlobalFunction: Symbol,
    pub visitStaticMethod: Symbol,
    pub visitCall: Symbol,
    pub visitAssign: Symbol,
    pub visitTernary: Symbol,
    pub visitIf: Symbol,
    pub visitWhile: Symbol,
    pub visitReturn: Symbol,
    pub visitFor: Symbol,
    pub visitBreak: Symbol,
    pub visitContinue: Symbol,
    pub splice: Symbol,
    pub dollardollarTmpVar: Symbol,
}

impl Classes {
    fn new() -> Self {
        Self {
            cParent: TypeName::new(sn::classes::PARENT),
            cStatic: TypeName::new(sn::classes::STATIC),
            cSelf: TypeName::new(sn::classes::SELF),
            cUnknown: TypeName::new(sn::classes::UNKNOWN),
            cAwaitable: TypeName::new(sn::classes::AWAITABLE),
            cGenerator: TypeName::new(sn::classes::GENERATOR),
            cAsyncGenerator: TypeName::new(sn::classes::ASYNC_GENERATOR),
            cHHFormatString: TypeName::new(sn::classes::HH_FORMAT_STRING),
            cHH_BuiltinEnum: TypeName::new(sn::classes::HH_BUILTIN_ENUM),
            cHH_BuiltinEnumClass: TypeName::new(sn::classes::HH_BUILTIN_ENUM_CLASS),
            cHH_BuiltinAbstractEnumClass: TypeName::new(
                sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS,
            ),
            cThrowable: TypeName::new(sn::classes::THROWABLE),
            cStdClass: TypeName::new(sn::classes::STD_CLASS),
            cDateTime: TypeName::new(sn::classes::DATE_TIME),
            cDateTimeImmutable: TypeName::new(sn::classes::DATE_TIME_IMMUTABLE),
            cAsyncIterator: TypeName::new(sn::classes::ASYNC_ITERATOR),
            cAsyncKeyedIterator: TypeName::new(sn::classes::ASYNC_KEYED_ITERATOR),
            cStringish: TypeName::new(sn::classes::STRINGISH),
            cStringishObject: TypeName::new(sn::classes::STRINGISH_OBJECT),
            cXHPChild: TypeName::new(sn::classes::XHP_CHILD),
            cIMemoizeParam: TypeName::new(sn::classes::IMEMOIZE_PARAM),
            cUNSAFESingletonMemoizeParam: TypeName::new(
                sn::classes::UNSAFE_SINGLETON_MEMOIZE_PARAM,
            ),
            cClassname: TypeName::new(sn::classes::CLASS_NAME),
            cTypename: TypeName::new(sn::classes::TYPE_NAME),
            cIDisposable: TypeName::new(sn::classes::IDISPOSABLE),
            cIAsyncDisposable: TypeName::new(sn::classes::IASYNC_DISPOSABLE),
            cMemberOf: TypeName::new(sn::classes::MEMBER_OF),
            cEnumClassLabel: TypeName::new(sn::classes::ENUM_CLASS_LABEL),
            cSpliceable: TypeName::new(sn::classes::SPLICEABLE),
            cSupportDyn: TypeName::new(sn::classes::SUPPORT_DYN),
        }
    }
}

impl Collections {
    fn new() -> Self {
        Self {
            // concrete classes
            cVector: Symbol::new(sn::collections::VECTOR),
            cMutableVector: Symbol::new(sn::collections::MUTABLE_VECTOR),
            cImmVector: Symbol::new(sn::collections::IMM_VECTOR),
            cSet: Symbol::new(sn::collections::SET),
            cConstSet: Symbol::new(sn::collections::CONST_SET),
            cMutableSet: Symbol::new(sn::collections::MUTABLE_SET),
            cImmSet: Symbol::new(sn::collections::IMM_SET),
            cMap: Symbol::new(sn::collections::MAP),
            cMutableMap: Symbol::new(sn::collections::MUTABLE_MAP),
            cImmMap: Symbol::new(sn::collections::IMM_MAP),
            cPair: Symbol::new(sn::collections::PAIR),

            // interfaces
            cContainer: Symbol::new(sn::collections::CONTAINER),
            cKeyedContainer: Symbol::new(sn::collections::KEYED_CONTAINER),
            cTraversable: Symbol::new(sn::collections::TRAVERSABLE),
            cKeyedTraversable: Symbol::new(sn::collections::KEYED_TRAVERSABLE),
            cCollection: Symbol::new(sn::collections::COLLECTION),
            cConstVector: Symbol::new(sn::collections::CONST_VECTOR),
            cConstMap: Symbol::new(sn::collections::CONST_MAP),
            cConstCollection: Symbol::new(sn::collections::CONST_COLLECTION),
            cAnyArray: Symbol::new(sn::collections::ANY_ARRAY),
            cDict: Symbol::new(sn::collections::DICT),
            cVec: Symbol::new(sn::collections::VEC),
            cKeyset: Symbol::new(sn::collections::KEYSET),
        }
    }
}

impl Members {
    fn new() -> Self {
        Self {
            mGetInstanceKey: Symbol::new(sn::members::M_GET_INSTANCE_KEY),
            mClass: ClassConstName::new(sn::members::M_CLASS),
            parentConstruct: Symbol::new("parent::".to_owned() + sn::members::__CONSTRUCT),
            __construct: Symbol::new(sn::members::__CONSTRUCT),
            __destruct: Symbol::new(sn::members::__DESTRUCT),
            __call: Symbol::new(sn::members::__CALL),
            __callStatic: Symbol::new(sn::members::__CALL_STATIC),
            __clone: Symbol::new(sn::members::__CLONE),
            __debugInfo: Symbol::new(sn::members::__DEBUG_INFO),
            __dispose: Symbol::new(sn::members::__DISPOSE),
            __disposeAsync: Symbol::new(sn::members::__DISPOSE_ASYNC),
            __get: Symbol::new(sn::members::__GET),
            __invoke: Symbol::new(sn::members::__INVOKE),
            __isset: Symbol::new(sn::members::__ISSET),
            __set: Symbol::new(sn::members::__SET),
            __set_state: Symbol::new(sn::members::__SET_STATE),
            __sleep: Symbol::new(sn::members::__SLEEP),
            __toString: Symbol::new(sn::members::__TO_STRING),
            __unset: Symbol::new(sn::members::__UNSET),
            __wakeup: Symbol::new(sn::members::__WAKEUP),
        }
    }
}

impl AttributeKinds {
    fn new() -> Self {
        Self {
            cls: Symbol::new(sn::attribute_kinds::CLS),
            clscst: Symbol::new(sn::attribute_kinds::CLS_CST),
            enum_: Symbol::new(sn::attribute_kinds::ENUM),
            typealias: Symbol::new(sn::attribute_kinds::TYPE_ALIAS),
            fn_: Symbol::new(sn::attribute_kinds::FN),
            mthd: Symbol::new(sn::attribute_kinds::MTHD),
            instProperty: Symbol::new(sn::attribute_kinds::INST_PROPERTY),
            staticProperty: Symbol::new(sn::attribute_kinds::STATIC_PROPERTY),
            parameter: Symbol::new(sn::attribute_kinds::PARAMETER),
            typeparam: Symbol::new(sn::attribute_kinds::TYPE_PARAM),
            file: Symbol::new(sn::attribute_kinds::FILE),
            typeconst: Symbol::new(sn::attribute_kinds::TYPE_CONST),
            lambda: Symbol::new(sn::attribute_kinds::LAMBDA),
            enumcls: Symbol::new(sn::attribute_kinds::ENUM_CLS),
        }
    }
}

impl UserAttributes {
    fn new() -> Self {
        Self {
            uaOverride: TypeName(Symbol::new(sn::user_attributes::OVERRIDE)),
            uaConsistentConstruct: TypeName(Symbol::new(sn::user_attributes::CONSISTENT_CONSTRUCT)),
            uaConst: TypeName(Symbol::new(sn::user_attributes::CONST)),
            uaDeprecated: TypeName(Symbol::new(sn::user_attributes::DEPRECATED)),
            uaEntryPoint: TypeName(Symbol::new(sn::user_attributes::ENTRY_POINT)),
            uaMemoize: TypeName(Symbol::new(sn::user_attributes::MEMOIZE)),
            uaMemoizeLSB: TypeName(Symbol::new(sn::user_attributes::MEMOIZE_LSB)),
            uaPolicyShardedMemoize: TypeName(Symbol::new(
                sn::user_attributes::POLICY_SHARDED_MEMOIZE,
            )),
            uaPolicyShardedMemoizeLSB: TypeName(Symbol::new(
                sn::user_attributes::POLICY_SHARDED_MEMOIZE_LSB,
            )),
            uaPHPStdLib: TypeName(Symbol::new(sn::user_attributes::PHP_STD_LIB)),
            uaAcceptDisposable: TypeName(Symbol::new(sn::user_attributes::ACCEPT_DISPOSABLE)),
            uaReturnDisposable: TypeName(Symbol::new(sn::user_attributes::RETURN_DISPOSABLE)),
            uaLSB: TypeName(Symbol::new(sn::user_attributes::LSB)),
            uaSealed: TypeName(Symbol::new(sn::user_attributes::SEALED)),
            uaLateInit: TypeName(Symbol::new(sn::user_attributes::LATE_INIT)),
            uaNewable: TypeName(Symbol::new(sn::user_attributes::NEWABLE)),
            uaEnforceable: TypeName(Symbol::new(sn::user_attributes::ENFORCEABLE)),
            uaExplicit: TypeName(Symbol::new(sn::user_attributes::EXPLICIT)),
            uaNonDisjoint: TypeName(Symbol::new(sn::user_attributes::NON_DISJOINT)),
            uaSoft: TypeName(Symbol::new(sn::user_attributes::SOFT)),
            uaWarn: TypeName(Symbol::new(sn::user_attributes::WARN)),
            uaMockClass: TypeName(Symbol::new(sn::user_attributes::MOCK_CLASS)),
            uaProvenanceSkipFrame: TypeName(Symbol::new(
                sn::user_attributes::PROVENANCE_SKIP_FRAME,
            )),
            uaDynamicallyCallable: TypeName(Symbol::new(sn::user_attributes::DYNAMICALLY_CALLABLE)),
            uaDynamicallyConstructible: TypeName(Symbol::new(
                sn::user_attributes::DYNAMICALLY_CONSTRUCTIBLE,
            )),
            uaReifiable: TypeName(Symbol::new(sn::user_attributes::REIFIABLE)),
            uaNeverInline: TypeName(Symbol::new(sn::user_attributes::NEVER_INLINE)),
            uaDisableTypecheckerInternal: TypeName(Symbol::new(
                sn::user_attributes::DISABLE_TYPECHECKER_INTERNAL,
            )),
            uaHasTopLevelCode: TypeName(Symbol::new(sn::user_attributes::HAS_TOP_LEVEL_CODE)),
            uaIsFoldable: TypeName(Symbol::new(sn::user_attributes::IS_FOLDABLE)),
            uaNative: TypeName(Symbol::new(sn::user_attributes::NATIVE)),
            uaOutOnly: TypeName(Symbol::new(sn::user_attributes::OUT_ONLY)),
            uaAlwaysInline: TypeName(Symbol::new(sn::user_attributes::ALWAYS_INLINE)),
            uaEnableUnstableFeatures: TypeName(Symbol::new(
                sn::user_attributes::ENABLE_UNSTABLE_FEATURES,
            )),
            uaEnumClass: TypeName(Symbol::new(sn::user_attributes::ENUM_CLASS)),
            uaPolicied: TypeName(Symbol::new(sn::user_attributes::POLICIED)),
            uaInferFlows: TypeName(Symbol::new(sn::user_attributes::INFERFLOWS)),
            uaExternal: TypeName(Symbol::new(sn::user_attributes::EXTERNAL)),
            uaCanCall: TypeName(Symbol::new(sn::user_attributes::CAN_CALL)),
            uaSupportDynamicType: TypeName(Symbol::new(sn::user_attributes::SUPPORT_DYNAMIC_TYPE)),
            uaRequireDynamic: TypeName(Symbol::new(sn::user_attributes::REQUIRE_DYNAMIC)),
            uaModule: TypeName(Symbol::new(sn::user_attributes::MODULE)),
            uaInternal: TypeName(Symbol::new(sn::user_attributes::INTERNAL)),
            uaEnableMethodTraitDiamond: TypeName(Symbol::new(
                sn::user_attributes::ENABLE_METHOD_TRAIT_DIAMOND,
            )),
            uaIgnoreReadonlyLocalErrors: TypeName(Symbol::new(
                sn::user_attributes::IGNORE_READONLY_LOCAL_ERRORS,
            )),
            uaIgnoreCoeffectLocalErrors: TypeName(Symbol::new(
                sn::user_attributes::IGNORE_COEFFECT_LOCAL_ERRORS,
            )),
        }
    }
}

impl SpecialFunctions {
    fn new() -> Self {
        Self {
            echo: Symbol::new(sn::special_functions::ECHO),
            hhas_adata: Symbol::new(sn::special_functions::HHAS_ADATA),
        }
    }
}

impl AutoimportedFunctions {
    fn new() -> Self {
        Self {
            invariant_violation: Symbol::new(sn::autoimported_functions::INVARIANT_VIOLATION),
            invariant: Symbol::new(sn::autoimported_functions::INVARIANT),
            fun_: Symbol::new(sn::autoimported_functions::FUN_),
            inst_meth: Symbol::new(sn::autoimported_functions::INST_METH),
            class_meth: Symbol::new(sn::autoimported_functions::CLASS_METH),
            meth_caller: Symbol::new(sn::autoimported_functions::METH_CALLER),
        }
    }
}

impl SpecialIdents {
    fn new() -> Self {
        Self {
            this: Symbol::new(sn::special_idents::THIS),
            placeholder: Symbol::new(sn::special_idents::PLACEHOLDER),
            dollardollar: Symbol::new(sn::special_idents::DOLLAR_DOLLAR),
            tmp_var_prefix: Symbol::new(sn::special_idents::TMP_VAR_PREFIX),
        }
    }
}

impl PseudoFunctions {
    fn new() -> Self {
        Self {
            isset: Symbol::new(sn::pseudo_functions::ISSET),
            unset: Symbol::new(sn::pseudo_functions::UNSET),
            hh_show: Symbol::new(sn::pseudo_functions::HH_SHOW),
            hh_expect: Symbol::new(sn::pseudo_functions::HH_EXPECT),
            hh_expect_equivalent: Symbol::new(sn::pseudo_functions::HH_EXPECT_EQUIVALENT),
            hh_show_env: Symbol::new(sn::pseudo_functions::HH_SHOW_ENV),
            hh_log_level: Symbol::new(sn::pseudo_functions::HH_LOG_LEVEL),
            hh_force_solve: Symbol::new(sn::pseudo_functions::HH_FORCE_SOLVE),
            hh_loop_forever: Symbol::new(sn::pseudo_functions::HH_LOOP_FOREVER),
            echo: Symbol::new(sn::pseudo_functions::ECHO),
            empty: Symbol::new(sn::pseudo_functions::EMPTY),
            exit: Symbol::new(sn::pseudo_functions::EXIT),
            die: Symbol::new(sn::pseudo_functions::DIE),
            unsafe_cast: Symbol::new(sn::pseudo_functions::UNSAFE_CAST),
            enforced_cast: Symbol::new(sn::pseudo_functions::ENFORCED_CAST),
        }
    }
}

impl StdlibFunctions {
    fn new() -> Self {
        Self {
            is_array: Symbol::new(sn::std_lib_functions::IS_ARRAY),
            is_null: Symbol::new(sn::std_lib_functions::IS_NULL),
            get_class: Symbol::new(sn::std_lib_functions::GET_CLASS),
            array_filter: Symbol::new(sn::std_lib_functions::ARRAY_FILTER),
            call_user_func: Symbol::new(sn::std_lib_functions::CALL_USER_FUNC),
            type_structure: Symbol::new(sn::std_lib_functions::TYPE_STRUCTURE),
            array_mark_legacy: Symbol::new(sn::std_lib_functions::ARRAY_MARK_LEGACY),
            array_unmark_legacy: Symbol::new(sn::std_lib_functions::ARRAY_UNMARK_LEGACY),
            is_php_array: Symbol::new(sn::std_lib_functions::IS_PHP_ARRAY),
            is_any_array: Symbol::new(sn::std_lib_functions::IS_ANY_ARRAY),
            is_dict_or_darray: Symbol::new(sn::std_lib_functions::IS_DICT_OR_DARRAY),
            is_vec_or_varray: Symbol::new(sn::std_lib_functions::IS_VEC_OR_VARRAY),
        }
    }
}

impl Typehints {
    fn new() -> Self {
        let mut this = Self {
            null: Symbol::new(sn::typehints::NULL),
            void: Symbol::new(sn::typehints::VOID),
            resource: Symbol::new(sn::typehints::RESOURCE),
            num: Symbol::new(sn::typehints::NUM),
            arraykey: Symbol::new(sn::typehints::ARRAYKEY),
            noreturn: Symbol::new(sn::typehints::NORETURN),
            mixed: Symbol::new(sn::typehints::MIXED),
            nonnull: Symbol::new(sn::typehints::NONNULL),
            this: Symbol::new(sn::typehints::THIS),
            dynamic: Symbol::new(sn::typehints::DYNAMIC),
            supportdynamic: Symbol::new(sn::typehints::SUPPORTDYNAMIC),
            nothing: Symbol::new(sn::typehints::NOTHING),
            int: Symbol::new(sn::typehints::INT),
            bool: Symbol::new(sn::typehints::BOOL),
            float: Symbol::new(sn::typehints::FLOAT),
            string: Symbol::new(sn::typehints::STRING),
            darray: Symbol::new(sn::typehints::DARRAY),
            varray: Symbol::new(sn::typehints::VARRAY),
            varray_or_darray: Symbol::new(sn::typehints::VARRAY_OR_DARRAY),
            vec_or_dict: Symbol::new(sn::typehints::VEC_OR_DICT),
            callable: Symbol::new(sn::typehints::CALLABLE),
            object_cast: Symbol::new(sn::typehints::OBJECT_CAST),
            supportdyn: Symbol::new(sn::typehints::SUPPORTDYN),
            hh_sypportdyn: Symbol::new(sn::typehints::HH_SUPPORTDYN),
            wildcard: Symbol::new(sn::typehints::WILDCARD),

            reserved_typehints: HashSet::default(),
        };

        this.reserved_typehints = HashSet::from([
            this.null,
            this.void,
            this.resource,
            this.num,
            this.arraykey,
            this.noreturn,
            this.mixed,
            this.nonnull,
            this.this,
            this.dynamic,
            this.nothing,
            this.int,
            this.bool,
            this.float,
            this.string,
            this.darray,
            this.varray,
            this.varray_or_darray,
            this.vec_or_dict,
            this.callable,
            this.wildcard,
        ]);
        this
    }
}

impl PseudoConsts {
    fn new() -> Self {
        Self {
            g__LINE__: Symbol::new(sn::pseudo_consts::G__LINE__),
            g__CLASS__: Symbol::new(sn::pseudo_consts::G__CLASS__),
            g__TRAIT__: Symbol::new(sn::pseudo_consts::G__TRAIT__),
            g__FILE__: Symbol::new(sn::pseudo_consts::G__FILE__),
            g__DIR__: Symbol::new(sn::pseudo_consts::G__DIR__),
            g__FUNCTION__: Symbol::new(sn::pseudo_consts::G__FUNCTION__),
            g__METHOD__: Symbol::new(sn::pseudo_consts::G__METHOD__),
            g__NAMESPACE__: Symbol::new(sn::pseudo_consts::G__NAMESPACE__),
            g__COMPILER_FRONTEND__: Symbol::new(sn::pseudo_consts::G__COMPILER_FRONTEND__),
            g__FUNCTION_CREDENTIAL__: Symbol::new(sn::pseudo_consts::G__FUNCTION_CREDENTIAL__),
            exit: Symbol::new(sn::pseudo_consts::EXIT),
            die: Symbol::new(sn::pseudo_consts::DIE),
        }
    }
}

impl Fb {
    fn new() -> Self {
        Self {
            cEnum: TypeName::new(sn::fb::ENUM),
            tInner: TypeConstName::new(sn::fb::INNER),
            idx: Symbol::new(sn::fb::IDX),
            cTypeStructure: TypeName::new(sn::fb::TYPE_STRUCTURE),
            cIncorrectType: TypeName::new(sn::fb::INCORRECT_TYPE),
        }
    }
}

impl Hh {
    fn new() -> Self {
        Self {
            contains: Symbol::new(sn::hh::CONTAINS),
            contains_key: Symbol::new(sn::hh::CONTAINS_KEY),
        }
    }
}

impl Shapes {
    fn new() -> Self {
        Self {
            cShapes: Symbol::new(sn::shapes::SHAPES),
            idx: Symbol::new(sn::shapes::IDX),
            at: Symbol::new(sn::shapes::AT),
            keyExists: Symbol::new(sn::shapes::KEY_EXISTS),
            removeKey: Symbol::new(sn::shapes::REMOVE_KEY),
            toArray: Symbol::new(sn::shapes::TO_ARRAY),
            toDict: Symbol::new(sn::shapes::TO_DICT),
        }
    }
}

impl Superglobals {
    fn new() -> Self {
        Self {
            globals: Symbol::new(sn::superglobals::GLOBALS),
        }
    }
}

impl Regex {
    fn new() -> Self {
        Self {
            tPattern: Symbol::new(sn::regex::T_PATTERN),
        }
    }
}

impl EmitterSpecialFunctions {
    fn new() -> Self {
        Self {
            eval: Symbol::new(sn::emitter_special_functions::EVAL),
            set_frame_metadata: Symbol::new(sn::emitter_special_functions::SET_FRAME_METADATA),
            systemlib_reified_generics: Symbol::new(
                sn::emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS,
            ),
        }
    }
}

impl Xhp {
    fn new() -> Self {
        Self {
            pcdata: Symbol::new(sn::xhp::PCDATA),
            any: Symbol::new(sn::xhp::ANY),
            empty: Symbol::new(sn::xhp::EMPTY),
        }
    }
}

impl UnstableFeatures {
    fn new() -> Self {
        Self {
            coeffects_provisional: Symbol::new(sn::unstable_features::COEFFECTS_PROVISIONAL),
            ifc: Symbol::new(sn::unstable_features::IFC),
            readonly: Symbol::new(sn::unstable_features::READONLY),
            expression_trees: Symbol::new(sn::unstable_features::EXPRESSION_TREES),
            modules: Symbol::new(sn::unstable_features::MODULES),
        }
    }
}

fn concat<S1: AsRef<str>, S2: AsRef<str>>(s1: S1, s2: S2) -> Symbol {
    let s1 = s1.as_ref();
    let s2 = s2.as_ref();
    Symbol::new(format!("{}{}", s1, s2))
}

impl Coeffects {
    fn new() -> Self {
        let contexts = Symbol::new("\\HH\\Contexts");
        let unsafe_contexts = concat(&contexts, "\\Unsafe");
        Self {
            capability: Symbol::new("$#capability"),
            local_capability: Symbol::new("$#local_capability"),
            contexts,
            unsafe_contexts,
            generated_generic_prefix: Symbol::new("T/"),
        }
    }
}

impl Readonly {
    fn new() -> Self {
        Self {
            idx: Symbol::new(sn::readonly::IDX),
            as_mut: Symbol::new(sn::readonly::AS_MUT),
        }
    }
}

impl Capabilities {
    fn new(coeffects: &Coeffects) -> Self {
        let defaults = concat(&coeffects.contexts, "\\defaults");
        let write_props = concat(&coeffects.contexts, "\\write_props");

        let prefix = Symbol::new("\\HH\\Capabilities\\");
        let writeProperty = concat(&prefix, "WriteProperty");
        let accessGlobals = concat(&prefix, "AccessGlobals");
        let readGlobals = concat(&prefix, "ReadGlobals");
        let system = concat(&prefix, "System");
        let implicitPolicy = concat(&prefix, "ImplicitPolicy");
        let implicitPolicyLocal = concat(&prefix, "ImplicitPolicyLocal");
        let io = concat(&prefix, "IO");
        let rx = concat(&prefix, "Rx");
        let rxLocal = concat(&rx, "Local");

        Self {
            defaults,
            write_props,
            writeProperty,
            accessGlobals,
            readGlobals,
            system,
            implicitPolicy,
            implicitPolicyLocal,
            io,
            rx,
            rxLocal,
        }
    }
}

impl ExpressionTrees {
    fn new() -> Self {
        Self {
            makeTree: Symbol::new(sn::expression_trees::MAKE_TREE),
            intType: Symbol::new(sn::expression_trees::INT_TYPE),
            floatType: Symbol::new(sn::expression_trees::FLOAT_TYPE),
            boolType: Symbol::new(sn::expression_trees::BOOL_TYPE),
            stringType: Symbol::new(sn::expression_trees::STRING_TYPE),
            nullType: Symbol::new(sn::expression_trees::NULL_TYPE),
            voidType: Symbol::new(sn::expression_trees::VOID_TYPE),
            symbolType: Symbol::new(sn::expression_trees::SYMBOL_TYPE),
            visitInt: Symbol::new(sn::expression_trees::VISIT_INT),
            visitFloat: Symbol::new(sn::expression_trees::VISIT_FLOAT),
            visitBool: Symbol::new(sn::expression_trees::VISIT_BOOL),
            visitString: Symbol::new(sn::expression_trees::VISIT_STRING),
            visitNull: Symbol::new(sn::expression_trees::VISIT_NULL),
            visitBinop: Symbol::new(sn::expression_trees::VISIT_BINOP),
            visitUnop: Symbol::new(sn::expression_trees::VISIT_UNOP),
            visitLocal: Symbol::new(sn::expression_trees::VISIT_LOCAL),
            visitLambda: Symbol::new(sn::expression_trees::VISIT_LAMBDA),
            visitGlobalFunction: Symbol::new(sn::expression_trees::VISIT_GLOBAL_FUNCTION),
            visitStaticMethod: Symbol::new(sn::expression_trees::VISIT_STATIC_METHOD),
            visitCall: Symbol::new(sn::expression_trees::VISIT_CALL),
            visitAssign: Symbol::new(sn::expression_trees::VISIT_ASSIGN),
            visitTernary: Symbol::new(sn::expression_trees::VISIT_TERNARY),
            visitIf: Symbol::new(sn::expression_trees::VISIT_IF),
            visitWhile: Symbol::new(sn::expression_trees::VISIT_WHILE),
            visitReturn: Symbol::new(sn::expression_trees::VISIT_RETURN),
            visitFor: Symbol::new(sn::expression_trees::VISIT_FOR),
            visitBreak: Symbol::new(sn::expression_trees::VISIT_BREAK),
            visitContinue: Symbol::new(sn::expression_trees::VISIT_CONTINUE),
            splice: Symbol::new(sn::expression_trees::SPLICE),
            dollardollarTmpVar: Symbol::new(sn::expression_trees::DOLLARDOLLAR_TMP_VAR),
        }
    }
}
