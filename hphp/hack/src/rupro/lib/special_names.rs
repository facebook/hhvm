// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// These use the same casing as naming_special_names.ml for now.
#![allow(non_snake_case)]

use crate::alloc::GlobalAllocator;
use naming_special_names_rust as sn;
use pos::{Symbol, TypeName};

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
    pub fn new(alloc: &GlobalAllocator) -> &'static Self {
        let coeffects = Coeffects::new(alloc);
        let capabilities = Capabilities::new(alloc, &coeffects);
        Box::leak(Box::new(Self {
            classes: Classes::new(alloc),
            collections: Collections::new(alloc),
            members: Members::new(alloc),
            attribute_kinds: AttributeKinds::new(alloc),
            user_attributes: UserAttributes::new(alloc),
            special_functions: SpecialFunctions::new(alloc),
            autoimported_functions: AutoimportedFunctions::new(alloc),
            special_idents: SpecialIdents::new(alloc),
            pseudo_functions: PseudoFunctions::new(alloc),
            stdlib_functions: StdlibFunctions::new(alloc),
            typehints: Typehints::new(alloc),
            pseudo_consts: PseudoConsts::new(alloc),
            fb: Fb::new(alloc),
            hh: Hh::new(alloc),
            shapes: Shapes::new(alloc),
            superglobals: Superglobals::new(alloc),
            regex: Regex::new(alloc),
            emitter_special_functions: EmitterSpecialFunctions::new(alloc),
            xhp: Xhp::new(alloc),
            unstable_features: UnstableFeatures::new(alloc),
            coeffects,
            readonly: Readonly::new(alloc),
            capabilities,
            expression_trees: ExpressionTrees::new(alloc),
        }))
    }
}

#[derive(Debug)]
pub struct Classes {
    pub cParent: Symbol,
    pub cStatic: Symbol,
    pub cSelf: Symbol,
    pub cUnknown: Symbol,

    // Used for dynamic classnames, e.g. new $foo();
    pub cAwaitable: Symbol,
    pub cGenerator: Symbol,
    pub cAsyncGenerator: Symbol,
    pub cHHFormatString: Symbol,
    pub cHH_BuiltinEnum: Symbol,
    pub cHH_BuiltinEnumClass: Symbol,
    pub cHH_BuiltinAbstractEnumClass: Symbol,
    pub cThrowable: Symbol,
    pub cStdClass: Symbol,
    pub cDateTime: Symbol,
    pub cDateTimeImmutable: Symbol,
    pub cAsyncIterator: Symbol,
    pub cAsyncKeyedIterator: Symbol,
    pub cStringish: Symbol,
    pub cStringishObject: Symbol,
    pub cXHPChild: Symbol,
    pub cIMemoizeParam: Symbol,
    pub cUNSAFESingletonMemoizeParam: Symbol,
    pub cClassname: TypeName,
    pub cTypename: Symbol,
    pub cIDisposable: Symbol,
    pub cIAsyncDisposable: Symbol,
    pub cMemberOf: Symbol,
    pub cEnumClassLabel: Symbol,

    /// Classes that can be spliced into ExpressionTrees
    pub cSpliceable: Symbol,
    pub cSupportDyn: Symbol,
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
    pub mClass: Symbol,
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
    pub cEnum: Symbol,
    pub tInner: Symbol,
    pub idx: Symbol,
    pub cTypeStructure: Symbol,
    pub cIncorrectType: Symbol,
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
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cParent: alloc.symbol(sn::classes::PARENT),
            cStatic: alloc.symbol(sn::classes::STATIC),
            cSelf: alloc.symbol(sn::classes::SELF),
            cUnknown: alloc.symbol(sn::classes::UNKNOWN),
            cAwaitable: alloc.symbol(sn::classes::AWAITABLE),
            cGenerator: alloc.symbol(sn::classes::GENERATOR),
            cAsyncGenerator: alloc.symbol(sn::classes::ASYNC_GENERATOR),
            cHHFormatString: alloc.symbol(sn::classes::HH_FORMAT_STRING),
            cHH_BuiltinEnum: alloc.symbol(sn::classes::HH_BUILTIN_ENUM),
            cHH_BuiltinEnumClass: alloc.symbol(sn::classes::HH_BUILTIN_ENUM_CLASS),
            cHH_BuiltinAbstractEnumClass: alloc.symbol(sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS),
            cThrowable: alloc.symbol(sn::classes::THROWABLE),
            cStdClass: alloc.symbol(sn::classes::STD_CLASS),
            cDateTime: alloc.symbol(sn::classes::DATE_TIME),
            cDateTimeImmutable: alloc.symbol(sn::classes::DATE_TIME_IMMUTABLE),
            cAsyncIterator: alloc.symbol(sn::classes::ASYNC_ITERATOR),
            cAsyncKeyedIterator: alloc.symbol(sn::classes::ASYNC_KEYED_ITERATOR),
            cStringish: alloc.symbol(sn::classes::STRINGISH),
            cStringishObject: alloc.symbol(sn::classes::STRINGISH_OBJECT),
            cXHPChild: alloc.symbol(sn::classes::XHP_CHILD),
            cIMemoizeParam: alloc.symbol(sn::classes::IMEMOIZE_PARAM),
            cUNSAFESingletonMemoizeParam: alloc.symbol(sn::classes::UNSAFE_SINGLETON_MEMOIZE_PARAM),
            cClassname: TypeName(alloc.symbol(sn::classes::CLASS_NAME)),
            cTypename: alloc.symbol(sn::classes::TYPE_NAME),
            cIDisposable: alloc.symbol(sn::classes::IDISPOSABLE),
            cIAsyncDisposable: alloc.symbol(sn::classes::IASYNC_DISPOSABLE),
            cMemberOf: alloc.symbol(sn::classes::MEMBER_OF),
            cEnumClassLabel: alloc.symbol(sn::classes::ENUM_CLASS_LABEL),
            cSpliceable: alloc.symbol(sn::classes::SPLICEABLE),
            cSupportDyn: alloc.symbol(sn::classes::SUPPORT_DYN),
        }
    }
}

impl Collections {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            // concrete classes
            cVector: alloc.symbol(sn::collections::VECTOR),
            cMutableVector: alloc.symbol(sn::collections::MUTABLE_VECTOR),
            cImmVector: alloc.symbol(sn::collections::IMM_VECTOR),
            cSet: alloc.symbol(sn::collections::SET),
            cConstSet: alloc.symbol(sn::collections::CONST_SET),
            cMutableSet: alloc.symbol(sn::collections::MUTABLE_SET),
            cImmSet: alloc.symbol(sn::collections::IMM_SET),
            cMap: alloc.symbol(sn::collections::MAP),
            cMutableMap: alloc.symbol(sn::collections::MUTABLE_MAP),
            cImmMap: alloc.symbol(sn::collections::IMM_MAP),
            cPair: alloc.symbol(sn::collections::PAIR),

            // interfaces
            cContainer: alloc.symbol(sn::collections::CONTAINER),
            cKeyedContainer: alloc.symbol(sn::collections::KEYED_CONTAINER),
            cTraversable: alloc.symbol(sn::collections::TRAVERSABLE),
            cKeyedTraversable: alloc.symbol(sn::collections::KEYED_TRAVERSABLE),
            cCollection: alloc.symbol(sn::collections::COLLECTION),
            cConstVector: alloc.symbol(sn::collections::CONST_VECTOR),
            cConstMap: alloc.symbol(sn::collections::CONST_MAP),
            cConstCollection: alloc.symbol(sn::collections::CONST_COLLECTION),
            cAnyArray: alloc.symbol(sn::collections::ANY_ARRAY),
            cDict: alloc.symbol(sn::collections::DICT),
            cVec: alloc.symbol(sn::collections::VEC),
            cKeyset: alloc.symbol(sn::collections::KEYSET),
        }
    }
}

impl Members {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            mGetInstanceKey: alloc.symbol(sn::members::M_GET_INSTANCE_KEY),
            mClass: alloc.symbol(sn::members::M_CLASS),
            __construct: alloc.symbol(sn::members::__CONSTRUCT),
            __destruct: alloc.symbol(sn::members::__DESTRUCT),
            __call: alloc.symbol(sn::members::__CALL),
            __callStatic: alloc.symbol(sn::members::__CALL_STATIC),
            __clone: alloc.symbol(sn::members::__CLONE),
            __debugInfo: alloc.symbol(sn::members::__DEBUG_INFO),
            __dispose: alloc.symbol(sn::members::__DISPOSE),
            __disposeAsync: alloc.symbol(sn::members::__DISPOSE_ASYNC),
            __get: alloc.symbol(sn::members::__GET),
            __invoke: alloc.symbol(sn::members::__INVOKE),
            __isset: alloc.symbol(sn::members::__ISSET),
            __set: alloc.symbol(sn::members::__SET),
            __set_state: alloc.symbol(sn::members::__SET_STATE),
            __sleep: alloc.symbol(sn::members::__SLEEP),
            __toString: alloc.symbol(sn::members::__TO_STRING),
            __unset: alloc.symbol(sn::members::__UNSET),
            __wakeup: alloc.symbol(sn::members::__WAKEUP),
        }
    }
}

impl AttributeKinds {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cls: alloc.symbol(sn::attribute_kinds::CLS),
            clscst: alloc.symbol(sn::attribute_kinds::CLS_CST),
            enum_: alloc.symbol(sn::attribute_kinds::ENUM),
            typealias: alloc.symbol(sn::attribute_kinds::TYPE_ALIAS),
            fn_: alloc.symbol(sn::attribute_kinds::FN),
            mthd: alloc.symbol(sn::attribute_kinds::MTHD),
            instProperty: alloc.symbol(sn::attribute_kinds::INST_PROPERTY),
            staticProperty: alloc.symbol(sn::attribute_kinds::STATIC_PROPERTY),
            parameter: alloc.symbol(sn::attribute_kinds::PARAMETER),
            typeparam: alloc.symbol(sn::attribute_kinds::TYPE_PARAM),
            file: alloc.symbol(sn::attribute_kinds::FILE),
            typeconst: alloc.symbol(sn::attribute_kinds::TYPE_CONST),
            lambda: alloc.symbol(sn::attribute_kinds::LAMBDA),
            enumcls: alloc.symbol(sn::attribute_kinds::ENUM_CLS),
        }
    }
}

impl UserAttributes {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            uaOverride: TypeName(alloc.symbol(sn::user_attributes::OVERRIDE)),
            uaConsistentConstruct: TypeName(
                alloc.symbol(sn::user_attributes::CONSISTENT_CONSTRUCT),
            ),
            uaConst: TypeName(alloc.symbol(sn::user_attributes::CONST)),
            uaDeprecated: TypeName(alloc.symbol(sn::user_attributes::DEPRECATED)),
            uaEntryPoint: TypeName(alloc.symbol(sn::user_attributes::ENTRY_POINT)),
            uaMemoize: TypeName(alloc.symbol(sn::user_attributes::MEMOIZE)),
            uaMemoizeLSB: TypeName(alloc.symbol(sn::user_attributes::MEMOIZE_LSB)),
            uaPolicyShardedMemoize: TypeName(
                alloc.symbol(sn::user_attributes::POLICY_SHARDED_MEMOIZE),
            ),
            uaPolicyShardedMemoizeLSB: TypeName(
                alloc.symbol(sn::user_attributes::POLICY_SHARDED_MEMOIZE_LSB),
            ),
            uaPHPStdLib: TypeName(alloc.symbol(sn::user_attributes::PHP_STD_LIB)),
            uaAcceptDisposable: TypeName(alloc.symbol(sn::user_attributes::ACCEPT_DISPOSABLE)),
            uaReturnDisposable: TypeName(alloc.symbol(sn::user_attributes::RETURN_DISPOSABLE)),
            uaLSB: TypeName(alloc.symbol(sn::user_attributes::LSB)),
            uaSealed: TypeName(alloc.symbol(sn::user_attributes::SEALED)),
            uaLateInit: TypeName(alloc.symbol(sn::user_attributes::LATE_INIT)),
            uaNewable: TypeName(alloc.symbol(sn::user_attributes::NEWABLE)),
            uaEnforceable: TypeName(alloc.symbol(sn::user_attributes::ENFORCEABLE)),
            uaExplicit: TypeName(alloc.symbol(sn::user_attributes::EXPLICIT)),
            uaNonDisjoint: TypeName(alloc.symbol(sn::user_attributes::NON_DISJOINT)),
            uaSoft: TypeName(alloc.symbol(sn::user_attributes::SOFT)),
            uaWarn: TypeName(alloc.symbol(sn::user_attributes::WARN)),
            uaMockClass: TypeName(alloc.symbol(sn::user_attributes::MOCK_CLASS)),
            uaProvenanceSkipFrame: TypeName(
                alloc.symbol(sn::user_attributes::PROVENANCE_SKIP_FRAME),
            ),
            uaDynamicallyCallable: TypeName(
                alloc.symbol(sn::user_attributes::DYNAMICALLY_CALLABLE),
            ),
            uaDynamicallyConstructible: TypeName(
                alloc.symbol(sn::user_attributes::DYNAMICALLY_CONSTRUCTIBLE),
            ),
            uaReifiable: TypeName(alloc.symbol(sn::user_attributes::REIFIABLE)),
            uaNeverInline: TypeName(alloc.symbol(sn::user_attributes::NEVER_INLINE)),
            uaDisableTypecheckerInternal: TypeName(
                alloc.symbol(sn::user_attributes::DISABLE_TYPECHECKER_INTERNAL),
            ),
            uaHasTopLevelCode: TypeName(alloc.symbol(sn::user_attributes::HAS_TOP_LEVEL_CODE)),
            uaIsFoldable: TypeName(alloc.symbol(sn::user_attributes::IS_FOLDABLE)),
            uaNative: TypeName(alloc.symbol(sn::user_attributes::NATIVE)),
            uaOutOnly: TypeName(alloc.symbol(sn::user_attributes::OUT_ONLY)),
            uaAlwaysInline: TypeName(alloc.symbol(sn::user_attributes::ALWAYS_INLINE)),
            uaEnableUnstableFeatures: TypeName(
                alloc.symbol(sn::user_attributes::ENABLE_UNSTABLE_FEATURES),
            ),
            uaEnumClass: TypeName(alloc.symbol(sn::user_attributes::ENUM_CLASS)),
            uaPolicied: TypeName(alloc.symbol(sn::user_attributes::POLICIED)),
            uaInferFlows: TypeName(alloc.symbol(sn::user_attributes::INFERFLOWS)),
            uaExternal: TypeName(alloc.symbol(sn::user_attributes::EXTERNAL)),
            uaCanCall: TypeName(alloc.symbol(sn::user_attributes::CAN_CALL)),
            uaSupportDynamicType: TypeName(alloc.symbol(sn::user_attributes::SUPPORT_DYNAMIC_TYPE)),
            uaRequireDynamic: TypeName(alloc.symbol(sn::user_attributes::REQUIRE_DYNAMIC)),
            uaModule: TypeName(alloc.symbol(sn::user_attributes::MODULE)),
            uaInternal: TypeName(alloc.symbol(sn::user_attributes::INTERNAL)),
            uaEnableMethodTraitDiamond: TypeName(
                alloc.symbol(sn::user_attributes::ENABLE_METHOD_TRAIT_DIAMOND),
            ),
            uaIgnoreReadonlyLocalErrors: TypeName(
                alloc.symbol(sn::user_attributes::IGNORE_READONLY_LOCAL_ERRORS),
            ),
            uaIgnoreCoeffectLocalErrors: TypeName(
                alloc.symbol(sn::user_attributes::IGNORE_COEFFECT_LOCAL_ERRORS),
            ),
        }
    }
}

impl SpecialFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            echo: alloc.symbol(sn::special_functions::ECHO),
            hhas_adata: alloc.symbol(sn::special_functions::HHAS_ADATA),
        }
    }
}

impl AutoimportedFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            invariant_violation: alloc.symbol(sn::autoimported_functions::INVARIANT_VIOLATION),
            invariant: alloc.symbol(sn::autoimported_functions::INVARIANT),
            fun_: alloc.symbol(sn::autoimported_functions::FUN_),
            inst_meth: alloc.symbol(sn::autoimported_functions::INST_METH),
            class_meth: alloc.symbol(sn::autoimported_functions::CLASS_METH),
            meth_caller: alloc.symbol(sn::autoimported_functions::METH_CALLER),
        }
    }
}

impl SpecialIdents {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            this: alloc.symbol(sn::special_idents::THIS),
            placeholder: alloc.symbol(sn::special_idents::PLACEHOLDER),
            dollardollar: alloc.symbol(sn::special_idents::DOLLAR_DOLLAR),
            tmp_var_prefix: alloc.symbol(sn::special_idents::TMP_VAR_PREFIX),
        }
    }
}

impl PseudoFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            isset: alloc.symbol(sn::pseudo_functions::ISSET),
            unset: alloc.symbol(sn::pseudo_functions::UNSET),
            hh_show: alloc.symbol(sn::pseudo_functions::HH_SHOW),
            hh_expect: alloc.symbol(sn::pseudo_functions::HH_EXPECT),
            hh_expect_equivalent: alloc.symbol(sn::pseudo_functions::HH_EXPECT_EQUIVALENT),
            hh_show_env: alloc.symbol(sn::pseudo_functions::HH_SHOW_ENV),
            hh_log_level: alloc.symbol(sn::pseudo_functions::HH_LOG_LEVEL),
            hh_force_solve: alloc.symbol(sn::pseudo_functions::HH_FORCE_SOLVE),
            hh_loop_forever: alloc.symbol(sn::pseudo_functions::HH_LOOP_FOREVER),
            echo: alloc.symbol(sn::pseudo_functions::ECHO),
            empty: alloc.symbol(sn::pseudo_functions::EMPTY),
            exit: alloc.symbol(sn::pseudo_functions::EXIT),
            die: alloc.symbol(sn::pseudo_functions::DIE),
            unsafe_cast: alloc.symbol(sn::pseudo_functions::UNSAFE_CAST),
            enforced_cast: alloc.symbol(sn::pseudo_functions::ENFORCED_CAST),
        }
    }
}

impl StdlibFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            is_array: alloc.symbol(sn::std_lib_functions::IS_ARRAY),
            is_null: alloc.symbol(sn::std_lib_functions::IS_NULL),
            get_class: alloc.symbol(sn::std_lib_functions::GET_CLASS),
            array_filter: alloc.symbol(sn::std_lib_functions::ARRAY_FILTER),
            call_user_func: alloc.symbol(sn::std_lib_functions::CALL_USER_FUNC),
            type_structure: alloc.symbol(sn::std_lib_functions::TYPE_STRUCTURE),
            array_mark_legacy: alloc.symbol(sn::std_lib_functions::ARRAY_MARK_LEGACY),
            array_unmark_legacy: alloc.symbol(sn::std_lib_functions::ARRAY_UNMARK_LEGACY),
            is_php_array: alloc.symbol(sn::std_lib_functions::IS_PHP_ARRAY),
            is_any_array: alloc.symbol(sn::std_lib_functions::IS_ANY_ARRAY),
            is_dict_or_darray: alloc.symbol(sn::std_lib_functions::IS_DICT_OR_DARRAY),
            is_vec_or_varray: alloc.symbol(sn::std_lib_functions::IS_VEC_OR_VARRAY),
        }
    }
}

impl Typehints {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            null: alloc.symbol(sn::typehints::NULL),
            void: alloc.symbol(sn::typehints::VOID),
            resource: alloc.symbol(sn::typehints::RESOURCE),
            num: alloc.symbol(sn::typehints::NUM),
            arraykey: alloc.symbol(sn::typehints::ARRAYKEY),
            noreturn: alloc.symbol(sn::typehints::NORETURN),
            mixed: alloc.symbol(sn::typehints::MIXED),
            nonnull: alloc.symbol(sn::typehints::NONNULL),
            this: alloc.symbol(sn::typehints::THIS),
            dynamic: alloc.symbol(sn::typehints::DYNAMIC),
            supportdynamic: alloc.symbol(sn::typehints::SUPPORTDYNAMIC),
            nothing: alloc.symbol(sn::typehints::NOTHING),
            int: alloc.symbol(sn::typehints::INT),
            bool: alloc.symbol(sn::typehints::BOOL),
            float: alloc.symbol(sn::typehints::FLOAT),
            string: alloc.symbol(sn::typehints::STRING),
            darray: alloc.symbol(sn::typehints::DARRAY),
            varray: alloc.symbol(sn::typehints::VARRAY),
            varray_or_darray: alloc.symbol(sn::typehints::VARRAY_OR_DARRAY),
            vec_or_dict: alloc.symbol(sn::typehints::VEC_OR_DICT),
            callable: alloc.symbol(sn::typehints::CALLABLE),
            object_cast: alloc.symbol(sn::typehints::OBJECT_CAST),
            supportdyn: alloc.symbol(sn::typehints::SUPPORTDYN),
            hh_sypportdyn: alloc.symbol(sn::typehints::HH_SUPPORTDYN),
            wildcard: alloc.symbol(sn::typehints::WILDCARD),
        }
    }
}

impl PseudoConsts {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            g__LINE__: alloc.symbol(sn::pseudo_consts::G__LINE__),
            g__CLASS__: alloc.symbol(sn::pseudo_consts::G__CLASS__),
            g__TRAIT__: alloc.symbol(sn::pseudo_consts::G__TRAIT__),
            g__FILE__: alloc.symbol(sn::pseudo_consts::G__FILE__),
            g__DIR__: alloc.symbol(sn::pseudo_consts::G__DIR__),
            g__FUNCTION__: alloc.symbol(sn::pseudo_consts::G__FUNCTION__),
            g__METHOD__: alloc.symbol(sn::pseudo_consts::G__METHOD__),
            g__NAMESPACE__: alloc.symbol(sn::pseudo_consts::G__NAMESPACE__),
            g__COMPILER_FRONTEND__: alloc.symbol(sn::pseudo_consts::G__COMPILER_FRONTEND__),
            g__FUNCTION_CREDENTIAL__: alloc.symbol(sn::pseudo_consts::G__FUNCTION_CREDENTIAL__),
            exit: alloc.symbol(sn::pseudo_consts::EXIT),
            die: alloc.symbol(sn::pseudo_consts::DIE),
        }
    }
}

impl Fb {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cEnum: alloc.symbol(sn::fb::ENUM),
            tInner: alloc.symbol(sn::fb::INNER),
            idx: alloc.symbol(sn::fb::IDX),
            cTypeStructure: alloc.symbol(sn::fb::TYPE_STRUCTURE),
            cIncorrectType: alloc.symbol(sn::fb::INCORRECT_TYPE),
        }
    }
}

impl Hh {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            contains: alloc.symbol(sn::hh::CONTAINS),
            contains_key: alloc.symbol(sn::hh::CONTAINS_KEY),
        }
    }
}

impl Shapes {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cShapes: alloc.symbol(sn::shapes::SHAPES),
            idx: alloc.symbol(sn::shapes::IDX),
            at: alloc.symbol(sn::shapes::AT),
            keyExists: alloc.symbol(sn::shapes::KEY_EXISTS),
            removeKey: alloc.symbol(sn::shapes::REMOVE_KEY),
            toArray: alloc.symbol(sn::shapes::TO_ARRAY),
            toDict: alloc.symbol(sn::shapes::TO_DICT),
        }
    }
}

impl Superglobals {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            globals: alloc.symbol(sn::superglobals::GLOBALS),
        }
    }
}

impl Regex {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            tPattern: alloc.symbol(sn::regex::T_PATTERN),
        }
    }
}

impl EmitterSpecialFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            eval: alloc.symbol(sn::emitter_special_functions::EVAL),
            set_frame_metadata: alloc.symbol(sn::emitter_special_functions::SET_FRAME_METADATA),
            systemlib_reified_generics: alloc
                .symbol(sn::emitter_special_functions::SYSTEMLIB_REIFIED_GENERICS),
        }
    }
}

impl Xhp {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            pcdata: alloc.symbol(sn::xhp::PCDATA),
            any: alloc.symbol(sn::xhp::ANY),
            empty: alloc.symbol(sn::xhp::EMPTY),
        }
    }
}

impl UnstableFeatures {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            coeffects_provisional: alloc.symbol(sn::unstable_features::COEFFECTS_PROVISIONAL),
            ifc: alloc.symbol(sn::unstable_features::IFC),
            readonly: alloc.symbol(sn::unstable_features::READONLY),
            expression_trees: alloc.symbol(sn::unstable_features::EXPRESSION_TREES),
            modules: alloc.symbol(sn::unstable_features::MODULES),
        }
    }
}

impl Coeffects {
    fn new(alloc: &GlobalAllocator) -> Self {
        let contexts = alloc.symbol("\\HH\\Contexts");
        let unsafe_contexts = alloc.concat(&contexts, "\\Unsafe");
        Self {
            capability: alloc.symbol("$#capability"),
            local_capability: alloc.symbol("$#local_capability"),
            contexts,
            unsafe_contexts,
            generated_generic_prefix: alloc.symbol("T/"),
        }
    }
}

impl Readonly {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            idx: alloc.symbol(sn::readonly::IDX),
            as_mut: alloc.symbol(sn::readonly::AS_MUT),
        }
    }
}

impl Capabilities {
    fn new(alloc: &GlobalAllocator, coeffects: &Coeffects) -> Self {
        let defaults = alloc.concat(&coeffects.contexts, "\\defaults");
        let write_props = alloc.concat(&coeffects.contexts, "\\write_props");

        let prefix = alloc.symbol("\\HH\\Capabilities\\");
        let writeProperty = alloc.concat(&prefix, "WriteProperty");
        let accessGlobals = alloc.concat(&prefix, "AccessGlobals");
        let readGlobals = alloc.concat(&prefix, "ReadGlobals");
        let system = alloc.concat(&prefix, "System");
        let implicitPolicy = alloc.concat(&prefix, "ImplicitPolicy");
        let implicitPolicyLocal = alloc.concat(&prefix, "ImplicitPolicyLocal");
        let io = alloc.concat(&prefix, "IO");
        let rx = alloc.concat(&prefix, "Rx");
        let rxLocal = alloc.concat(&rx, "Local");

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
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            makeTree: alloc.symbol(sn::expression_trees::MAKE_TREE),
            intType: alloc.symbol(sn::expression_trees::INT_TYPE),
            floatType: alloc.symbol(sn::expression_trees::FLOAT_TYPE),
            boolType: alloc.symbol(sn::expression_trees::BOOL_TYPE),
            stringType: alloc.symbol(sn::expression_trees::STRING_TYPE),
            nullType: alloc.symbol(sn::expression_trees::NULL_TYPE),
            voidType: alloc.symbol(sn::expression_trees::VOID_TYPE),
            symbolType: alloc.symbol(sn::expression_trees::SYMBOL_TYPE),
            visitInt: alloc.symbol(sn::expression_trees::VISIT_INT),
            visitFloat: alloc.symbol(sn::expression_trees::VISIT_FLOAT),
            visitBool: alloc.symbol(sn::expression_trees::VISIT_BOOL),
            visitString: alloc.symbol(sn::expression_trees::VISIT_STRING),
            visitNull: alloc.symbol(sn::expression_trees::VISIT_NULL),
            visitBinop: alloc.symbol(sn::expression_trees::VISIT_BINOP),
            visitUnop: alloc.symbol(sn::expression_trees::VISIT_UNOP),
            visitLocal: alloc.symbol(sn::expression_trees::VISIT_LOCAL),
            visitLambda: alloc.symbol(sn::expression_trees::VISIT_LAMBDA),
            visitGlobalFunction: alloc.symbol(sn::expression_trees::VISIT_GLOBAL_FUNCTION),
            visitStaticMethod: alloc.symbol(sn::expression_trees::VISIT_STATIC_METHOD),
            visitCall: alloc.symbol(sn::expression_trees::VISIT_CALL),
            visitAssign: alloc.symbol(sn::expression_trees::VISIT_ASSIGN),
            visitTernary: alloc.symbol(sn::expression_trees::VISIT_TERNARY),
            visitIf: alloc.symbol(sn::expression_trees::VISIT_IF),
            visitWhile: alloc.symbol(sn::expression_trees::VISIT_WHILE),
            visitReturn: alloc.symbol(sn::expression_trees::VISIT_RETURN),
            visitFor: alloc.symbol(sn::expression_trees::VISIT_FOR),
            visitBreak: alloc.symbol(sn::expression_trees::VISIT_BREAK),
            visitContinue: alloc.symbol(sn::expression_trees::VISIT_CONTINUE),
            splice: alloc.symbol(sn::expression_trees::SPLICE),
            dollardollarTmpVar: alloc.symbol(sn::expression_trees::DOLLARDOLLAR_TMP_VAR),
        }
    }
}
