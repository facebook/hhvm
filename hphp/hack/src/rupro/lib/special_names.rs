// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// These use the same casing as naming_special_names.ml for now.
#![allow(non_snake_case)]

use crate::alloc::GlobalAllocator;
use crate::pos::Symbol;

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
    pub fb: FB,
    pub hh: HH,
    pub shapes: Shapes,
    pub superglobals: Superglobals,
    pub regex: Regex,
    pub emitter_special_functions: EmitterSpecialFunctions,
    pub xhp: XHP,
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
            fb: FB::new(alloc),
            hh: HH::new(alloc),
            shapes: Shapes::new(alloc),
            superglobals: Superglobals::new(alloc),
            regex: Regex::new(alloc),
            emitter_special_functions: EmitterSpecialFunctions::new(alloc),
            xhp: XHP::new(alloc),
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
    pub cClassname: Symbol,
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
    pub uaOverride: Symbol,
    pub uaConsistentConstruct: Symbol,
    pub uaConst: Symbol,
    pub uaDeprecated: Symbol,
    pub uaEntryPoint: Symbol,
    pub uaMemoize: Symbol,
    pub uaMemoizeLSB: Symbol,
    pub uaPolicyShardedMemoize: Symbol,
    pub uaPolicyShardedMemoizeLSB: Symbol,
    pub uaPHPStdLib: Symbol,
    pub uaAcceptDisposable: Symbol,
    pub uaReturnDisposable: Symbol,
    pub uaLSB: Symbol,
    pub uaSealed: Symbol,
    pub uaLateInit: Symbol,
    pub uaNewable: Symbol,
    pub uaEnforceable: Symbol,
    pub uaExplicit: Symbol,
    pub uaNonDisjoint: Symbol,
    pub uaSoft: Symbol,
    pub uaWarn: Symbol,
    pub uaMockClass: Symbol,
    pub uaProvenanceSkipFrame: Symbol,
    pub uaDynamicallyCallable: Symbol,
    pub uaDynamicallyConstructible: Symbol,
    pub uaReifiable: Symbol,
    pub uaNeverInline: Symbol,
    pub uaDisableTypecheckerInternal: Symbol,
    pub uaHasTopLevelCode: Symbol,
    pub uaIsFoldable: Symbol,
    pub uaNative: Symbol,
    pub uaOutOnly: Symbol,
    pub uaAlwaysInline: Symbol,
    pub uaEnableUnstableFeatures: Symbol,
    pub uaEnumClass: Symbol,
    pub uaPolicied: Symbol,
    pub uaInferFlows: Symbol,
    pub uaExternal: Symbol,
    pub uaCanCall: Symbol,
    pub uaSupportDynamicType: Symbol,
    pub uaRequireDynamic: Symbol,
    pub uaModule: Symbol,
    pub uaInternal: Symbol,
    pub uaEnableMethodTraitDiamond: Symbol,
    pub uaIgnoreReadonlyLocalErrors: Symbol,
    pub uaIgnoreCoeffectLocalErrors: Symbol,
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
    pub is_null: Symbol,
    pub get_class: Symbol,
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
pub struct FB {
    pub cEnum: Symbol,
    pub tInner: Symbol,
    pub idx: Symbol,
    pub cTypeStructure: Symbol,
    pub cIncorrectType: Symbol,
}

#[derive(Debug)]
pub struct HH {
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
pub struct XHP {
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
            cParent: alloc.symbol("parent"),
            cStatic: alloc.symbol("static"),
            cSelf: alloc.symbol("self"),
            cUnknown: alloc.symbol("\\*Unknown*"),
            cAwaitable: alloc.symbol("\\HH\\Awaitable"),
            cGenerator: alloc.symbol("\\Generator"),
            cAsyncGenerator: alloc.symbol("\\HH\\AsyncGenerator"),
            cHHFormatString: alloc.symbol("\\HH\\FormatString"),
            cHH_BuiltinEnum: alloc.symbol("\\HH\\BuiltinEnum"),
            cHH_BuiltinEnumClass: alloc.symbol("\\HH\\BuiltinEnumClass"),
            cHH_BuiltinAbstractEnumClass: alloc.symbol("\\HH\\BuiltinAbstractEnumClass"),
            cThrowable: alloc.symbol("\\Throwable"),
            cStdClass: alloc.symbol("\\stdClass"),
            cDateTime: alloc.symbol("\\DateTime"),
            cDateTimeImmutable: alloc.symbol("\\DateTimeImmutable"),
            cAsyncIterator: alloc.symbol("\\HH\\AsyncIterator"),
            cAsyncKeyedIterator: alloc.symbol("\\HH\\AsyncKeyedIterator"),
            cStringish: alloc.symbol("\\Stringish"),
            cStringishObject: alloc.symbol("\\StringishObject"),
            cXHPChild: alloc.symbol("\\XHPChild"),
            cIMemoizeParam: alloc.symbol("\\HH\\IMemoizeParam"),
            cUNSAFESingletonMemoizeParam: alloc.symbol("\\HH\\UNSAFESingletonMemoizeParam"),
            cClassname: alloc.symbol("\\HH\\classname"),
            cTypename: alloc.symbol("\\HH\\typename"),
            cIDisposable: alloc.symbol("\\IDisposable"),
            cIAsyncDisposable: alloc.symbol("\\IAsyncDisposable"),
            cMemberOf: alloc.symbol("\\HH\\MemberOf"),
            cEnumClassLabel: alloc.symbol("\\HH\\EnumClass\\Label"),
            cSpliceable: alloc.symbol("\\Spliceable"),
            cSupportDyn: alloc.symbol("\\HH\\supportdyn"),
        }
    }
}

impl Collections {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            // concrete classes
            cVector: alloc.symbol("\\HH\\Vector"),
            cMutableVector: alloc.symbol("\\MutableVector"),
            cImmVector: alloc.symbol("\\HH\\ImmVector"),
            cSet: alloc.symbol("\\HH\\Set"),
            cConstSet: alloc.symbol("\\ConstSet"),
            cMutableSet: alloc.symbol("\\MutableSet"),
            cImmSet: alloc.symbol("\\HH\\ImmSet"),
            cMap: alloc.symbol("\\HH\\Map"),
            cMutableMap: alloc.symbol("\\MutableMap"),
            cImmMap: alloc.symbol("\\HH\\ImmMap"),
            cPair: alloc.symbol("\\HH\\Pair"),

            // interfaces
            cContainer: alloc.symbol("\\HH\\Container"),
            cKeyedContainer: alloc.symbol("\\HH\\KeyedContainer"),
            cTraversable: alloc.symbol("\\HH\\Traversable"),
            cKeyedTraversable: alloc.symbol("\\HH\\KeyedTraversable"),
            cCollection: alloc.symbol("\\Collection"),
            cConstVector: alloc.symbol("\\ConstVector"),
            cConstMap: alloc.symbol("\\ConstMap"),
            cConstCollection: alloc.symbol("\\ConstCollection"),
            cAnyArray: alloc.symbol("\\HH\\AnyArray"),
            cDict: alloc.symbol("\\HH\\dict"),
            cVec: alloc.symbol("\\HH\\vec"),
            cKeyset: alloc.symbol("\\HH\\keyset"),
        }
    }
}

impl Members {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            mGetInstanceKey: alloc.symbol("getInstanceKey"),
            mClass: alloc.symbol("class"),
            __construct: alloc.symbol("__construct"),
            __destruct: alloc.symbol("__destruct"),
            __call: alloc.symbol("__call"),
            __callStatic: alloc.symbol("__callStatic"),
            __clone: alloc.symbol("__clone"),
            __debugInfo: alloc.symbol("__debugInfo"),
            __dispose: alloc.symbol("__dispose"),
            __disposeAsync: alloc.symbol("__disposeAsync"),
            __get: alloc.symbol("__get"),
            __invoke: alloc.symbol("__invoke"),
            __isset: alloc.symbol("__isset"),
            __set: alloc.symbol("__set"),
            __set_state: alloc.symbol("__set_state"),
            __sleep: alloc.symbol("__sleep"),
            __toString: alloc.symbol("__toString"),
            __unset: alloc.symbol("__unset"),
            __wakeup: alloc.symbol("__wakeup"),
        }
    }
}

impl AttributeKinds {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cls: alloc.symbol("\\HH\\ClassAttribute"),
            clscst: alloc.symbol("\\HH\\ClassConstantAttribute"),
            enum_: alloc.symbol("\\HH\\EnumAttribute"),
            typealias: alloc.symbol("\\HH\\TypeAliasAttribute"),
            fn_: alloc.symbol("\\HH\\FunctionAttribute"),
            mthd: alloc.symbol("\\HH\\MethodAttribute"),
            instProperty: alloc.symbol("\\HH\\InstancePropertyAttribute"),
            staticProperty: alloc.symbol("\\HH\\StaticPropertyAttribute"),
            parameter: alloc.symbol("\\HH\\ParameterAttribute"),
            typeparam: alloc.symbol("\\HH\\TypeParameterAttribute"),
            file: alloc.symbol("\\HH\\FileAttribute"),
            typeconst: alloc.symbol("\\HH\\TypeConstantAttribute"),
            lambda: alloc.symbol("\\HH\\LambdaAttribute"),
            enumcls: alloc.symbol("\\HH\\EnumClassAttribute"),
        }
    }
}

impl UserAttributes {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            uaOverride: alloc.symbol("__Override"),
            uaConsistentConstruct: alloc.symbol("__ConsistentConstruct"),
            uaConst: alloc.symbol("__Const"),
            uaDeprecated: alloc.symbol("__Deprecated"),
            uaEntryPoint: alloc.symbol("__EntryPoint"),
            uaMemoize: alloc.symbol("__Memoize"),
            uaMemoizeLSB: alloc.symbol("__MemoizeLSB"),
            uaPolicyShardedMemoize: alloc.symbol("__PolicyShardedMemoize"),
            uaPolicyShardedMemoizeLSB: alloc.symbol("__PolicyShardedMemoizeLSB"),
            uaPHPStdLib: alloc.symbol("__PHPStdLib"),
            uaAcceptDisposable: alloc.symbol("__AcceptDisposable"),
            uaReturnDisposable: alloc.symbol("__ReturnDisposable"),
            uaLSB: alloc.symbol("__LSB"),
            uaSealed: alloc.symbol("__Sealed"),
            uaLateInit: alloc.symbol("__LateInit"),
            uaNewable: alloc.symbol("__Newable"),
            uaEnforceable: alloc.symbol("__Enforceable"),
            uaExplicit: alloc.symbol("__Explicit"),
            uaNonDisjoint: alloc.symbol("__NonDisjoint"),
            uaSoft: alloc.symbol("__Soft"),
            uaWarn: alloc.symbol("__Warn"),
            uaMockClass: alloc.symbol("__MockClass"),
            uaProvenanceSkipFrame: alloc.symbol("__ProvenanceSkipFrame"),
            uaDynamicallyCallable: alloc.symbol("__DynamicallyCallable"),
            uaDynamicallyConstructible: alloc.symbol("__DynamicallyConstructible"),
            uaReifiable: alloc.symbol("__Reifiable"),
            uaNeverInline: alloc.symbol("__NEVER_INLINE"),
            uaDisableTypecheckerInternal: alloc.symbol("__DisableTypecheckerInternal"),
            uaHasTopLevelCode: alloc.symbol("__HasTopLevelCode"),
            uaIsFoldable: alloc.symbol("__IsFoldable"),
            uaNative: alloc.symbol("__Native"),
            uaOutOnly: alloc.symbol("__OutOnly"),
            uaAlwaysInline: alloc.symbol("__ALWAYS_INLINE"),
            uaEnableUnstableFeatures: alloc.symbol("__EnableUnstableFeatures"),
            uaEnumClass: alloc.symbol("__EnumClass"),
            uaPolicied: alloc.symbol("__Policied"),
            uaInferFlows: alloc.symbol("__InferFlows"),
            uaExternal: alloc.symbol("__External"),
            uaCanCall: alloc.symbol("__CanCall"),
            uaSupportDynamicType: alloc.symbol("__SupportDynamicType"),
            uaRequireDynamic: alloc.symbol("__RequireDynamic"),
            uaModule: alloc.symbol("__Module"),
            uaInternal: alloc.symbol("__Internal"),
            uaEnableMethodTraitDiamond: alloc.symbol("__EnableMethodTraitDiamond"),
            uaIgnoreReadonlyLocalErrors: alloc.symbol("__IgnoreReadonlyLocalErrors"),
            uaIgnoreCoeffectLocalErrors: alloc.symbol("__IgnoreCoeffectLocalErrors"),
        }
    }
}

impl SpecialFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            echo: alloc.symbol("echo"),
            hhas_adata: alloc.symbol("__hhas_adata"),
        }
    }
}

impl AutoimportedFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            invariant_violation: alloc.symbol("\\HH\\invariant_violation"),
            invariant: alloc.symbol("\\HH\\invariant"),
            fun_: alloc.symbol("\\HH\\fun"),
            inst_meth: alloc.symbol("\\HH\\inst_meth"),
            class_meth: alloc.symbol("\\HH\\class_meth"),
            meth_caller: alloc.symbol("\\HH\\meth_caller"),
        }
    }
}

impl SpecialIdents {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            this: alloc.symbol("$this"),
            placeholder: alloc.symbol("$_"),
            dollardollar: alloc.symbol("$$"),
            tmp_var_prefix: alloc.symbol("__tmp$"),
        }
    }
}

impl PseudoFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            isset: alloc.symbol("\\isset"),
            unset: alloc.symbol("\\unset"),
            hh_show: alloc.symbol("\\hh_show"),
            hh_expect: alloc.symbol("\\hh_expect"),
            hh_expect_equivalent: alloc.symbol("\\hh_expect_equivalent"),
            hh_show_env: alloc.symbol("\\hh_show_env"),
            hh_log_level: alloc.symbol("\\hh_log_level"),
            hh_force_solve: alloc.symbol("\\hh_force_solve"),
            hh_loop_forever: alloc.symbol("\\hh_loop_forever"),
            echo: alloc.symbol("\\echo"),
            empty: alloc.symbol("\\empty"),
            exit: alloc.symbol("\\exit"),
            die: alloc.symbol("\\die"),
            unsafe_cast: alloc.symbol("\\HH\\FIXME\\UNSAFE_CAST"),
            enforced_cast: alloc.symbol("\\HH\\FIXME\\ENFORCED_CAST"),
        }
    }
}

impl StdlibFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            is_null: alloc.symbol("\\is_null"),
            get_class: alloc.symbol("\\get_class"),
            call_user_func: alloc.symbol("\\call_user_func"),
            type_structure: alloc.symbol("\\HH\\type_structure"),
            array_mark_legacy: alloc.symbol("\\HH\\array_mark_legacy"),
            array_unmark_legacy: alloc.symbol("\\HH\\array_unmark_legacy"),
            is_php_array: alloc.symbol("\\HH\\is_php_array"),
            is_any_array: alloc.symbol("\\HH\\is_any_array"),
            is_dict_or_darray: alloc.symbol("\\HH\\is_dict_or_darray"),
            is_vec_or_varray: alloc.symbol("\\HH\\is_vec_or_varray"),
        }
    }
}

impl Typehints {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            null: alloc.symbol("null"),
            void: alloc.symbol("void"),
            resource: alloc.symbol("resource"),
            num: alloc.symbol("num"),
            arraykey: alloc.symbol("arraykey"),
            noreturn: alloc.symbol("noreturn"),
            mixed: alloc.symbol("mixed"),
            nonnull: alloc.symbol("nonnull"),
            this: alloc.symbol("this"),
            dynamic: alloc.symbol("dynamic"),
            supportdynamic: alloc.symbol("supportdynamic"),
            nothing: alloc.symbol("nothing"),
            int: alloc.symbol("int"),
            bool: alloc.symbol("bool"),
            float: alloc.symbol("float"),
            string: alloc.symbol("string"),
            darray: alloc.symbol("darray"),
            varray: alloc.symbol("varray"),
            varray_or_darray: alloc.symbol("varray_or_darray"),
            vec_or_dict: alloc.symbol("vec_or_dict"),
            callable: alloc.symbol("callable"),
            wildcard: alloc.symbol("_"),
        }
    }
}

impl PseudoConsts {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            g__LINE__: alloc.symbol("\\__LINE__"),
            g__CLASS__: alloc.symbol("\\__CLASS__"),
            g__TRAIT__: alloc.symbol("\\__TRAIT__"),
            g__FILE__: alloc.symbol("\\__FILE__"),
            g__DIR__: alloc.symbol("\\__DIR__"),
            g__FUNCTION__: alloc.symbol("\\__FUNCTION__"),
            g__METHOD__: alloc.symbol("\\__METHOD__"),
            g__NAMESPACE__: alloc.symbol("\\__NAMESPACE__"),
            g__COMPILER_FRONTEND__: alloc.symbol("\\__COMPILER_FRONTEND__"),
            g__FUNCTION_CREDENTIAL__: alloc.symbol("\\__FUNCTION_CREDENTIAL__"),
            exit: alloc.symbol("\\exit"),
            die: alloc.symbol("\\die"),
        }
    }
}

impl FB {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cEnum: alloc.symbol("\\Enum"),
            tInner: alloc.symbol("TInner"),
            idx: alloc.symbol("\\HH\\idx"),
            cTypeStructure: alloc.symbol("\\HH\\TypeStructure"),
            cIncorrectType: alloc.symbol("\\HH\\INCORRECT_TYPE"),
        }
    }
}

impl HH {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            contains: alloc.symbol("\\HH\\Lib\\C\\contains"),
            contains_key: alloc.symbol("\\HH\\Lib\\C\\contains_key"),
        }
    }
}

impl Shapes {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            cShapes: alloc.symbol("\\HH\\Shapes"),
            idx: alloc.symbol("idx"),
            at: alloc.symbol("at"),
            keyExists: alloc.symbol("keyExists"),
            removeKey: alloc.symbol("removeKey"),
            toArray: alloc.symbol("toArray"),
            toDict: alloc.symbol("toDict"),
        }
    }
}

impl Superglobals {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            globals: alloc.symbol("$GLOBALS"),
        }
    }
}

impl Regex {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            tPattern: alloc.symbol("\\HH\\Lib\\Regex\\Pattern"),
        }
    }
}

impl EmitterSpecialFunctions {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            eval: alloc.symbol("\\eval"),
            set_frame_metadata: alloc.symbol("\\HH\\set_frame_metadata"),
            systemlib_reified_generics: alloc.symbol("\\__systemlib_reified_generics"),
        }
    }
}

impl XHP {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            pcdata: alloc.symbol("pcdata"),
            any: alloc.symbol("any"),
            empty: alloc.symbol("empty"),
        }
    }
}

impl UnstableFeatures {
    fn new(alloc: &GlobalAllocator) -> Self {
        Self {
            coeffects_provisional: alloc.symbol("coeffects_provisional"),
            ifc: alloc.symbol("ifc"),
            readonly: alloc.symbol("readonly"),
            expression_trees: alloc.symbol("expression_trees"),
            modules: alloc.symbol("modules"),
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
            idx: alloc.symbol("\\HH\\idx_readonly"),
            as_mut: alloc.symbol("\\HH\\Readonly\\as_mut"),
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
            makeTree: alloc.symbol("makeTree"),
            intType: alloc.symbol("intType"),
            floatType: alloc.symbol("floatType"),
            boolType: alloc.symbol("boolType"),
            stringType: alloc.symbol("stringType"),
            nullType: alloc.symbol("nullType"),
            voidType: alloc.symbol("voidType"),
            symbolType: alloc.symbol("symbolType"),
            visitInt: alloc.symbol("visitInt"),
            visitFloat: alloc.symbol("visitFloat"),
            visitBool: alloc.symbol("visitBool"),
            visitString: alloc.symbol("visitString"),
            visitNull: alloc.symbol("visitNull"),
            visitBinop: alloc.symbol("visitBinop"),
            visitUnop: alloc.symbol("visitUnop"),
            visitLocal: alloc.symbol("visitLocal"),
            visitLambda: alloc.symbol("visitLambda"),
            visitGlobalFunction: alloc.symbol("visitGlobalFunction"),
            visitStaticMethod: alloc.symbol("visitStaticMethod"),
            visitCall: alloc.symbol("visitCall"),
            visitAssign: alloc.symbol("visitAssign"),
            visitTernary: alloc.symbol("visitTernary"),
            visitIf: alloc.symbol("visitIf"),
            visitWhile: alloc.symbol("visitWhile"),
            visitReturn: alloc.symbol("visitReturn"),
            visitFor: alloc.symbol("visitFor"),
            visitBreak: alloc.symbol("visitBreak"),
            visitContinue: alloc.symbol("visitContinue"),
            splice: alloc.symbol("splice"),
            dollardollarTmpVar: alloc.symbol("$0dollardollar"),
        }
    }
}
