// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::pos::Pos;
use ocamlrep_derive::OcamlRep;

#[derive(Clone, Debug, OcamlRep)]
pub struct Error(usize, Vec<(Pos, String)>);

#[derive(Clone, Debug, OcamlRep)]
pub struct Errors(Vec<Error>);

impl Errors {
    pub fn empty() -> Self {
        Errors(vec![])
    }
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
#[repr(usize)]
pub enum Naming {
    AddATypehint = 2001,
    TypeparamAlokDEPRECATED,
    AssertArity,
    PrimitiveInvalidAlias,
    CyclicConstraintDEPRECATED,
    DidYouMeanNaming,
    DifferentScopeDEPRECATED,
    DisallowedXhpType,
    DoubleInsteadOfFloatDEPRECATED,
    DynamicClassDEPRECATED,
    LvarInObjGet,
    ErrorNameAlreadyBound,
    ExpectedCollection,
    ExpectedVariable,
    FdNameAlreadyBound,
    GenArrayRecArityDEPRECATED,
    GenArrayVaRecArityDEPRECATED,
    GenaArityDEPRECATED,
    GenericClassVarDEPRECATED,
    GenvaArityDEPRECATED,
    IllegalClass,
    IllegalClassMeth,
    IllegalConstant,
    IllegalFun,
    IllegalInstMeth,
    IllegalMethCaller,
    IllegalMethFun,
    IntegerInsteadOfIntDEPRECATED,
    InvalidReqExtends,
    InvalidReqImplements,
    LocalConstDEPRECATED,
    LowercaseThis,
    MethodNameAlreadyBound,
    MissingArrow,
    MissingTypehint,
    NameAlreadyBound,
    NamingTooFewArguments,
    NamingTooManyArguments,
    PrimitiveToplevel,
    RealInsteadOfFloatDEPRECATED,
    ShadowedTypeParam,
    StartWithT,
    ThisMustBeReturn,
    ThisNoArgument,
    ThisHintOutsideClass,
    ThisReserved,
    TparamWithTparam,
    TypedefConstraintDEPRECATED,
    UnboundName,
    Undefined,
    UnexpectedArrow,
    UnexpectedTypedef,
    UsingInternalClassDEPRECATED,
    VoidCast,
    ObjectCast,
    UnsetCast,
    NullsafePropertyAccessDEPRECATED,
    IllegalTrait,
    ShapeTypehintDEPRECATED,
    DynamicNewInStrictMode,
    InvalidTypeAccessRoot,
    DuplicateUserAttribute,
    ReturnOnlyTypehint,
    UnexpectedTypeArguments,
    TooManyTypeArguments,
    ClassnameParam,
    InvalidInstanceofDEPRECATED,
    NameIsReserved,
    DollardollarUnused,
    IllegalMemberVariableClass,
    TooFewTypeArguments,
    GotoLabelAlreadyDefined,
    GotoLabelUndefined,
    GotoLabelDefinedInFinally,
    GotoInvokedInFinally,
    DynamicClassPropertyNameInStrictModeDEPRECATED,
    ThisAsLexicalVariable,
    DynamicClassNameInStrictMode,
    XhpOptionalRequiredAttr,
    XhpRequiredWithDefault,
    VariableVariablesDisallowedDEPRECATED,
    ArrayTypehintsDisallowed,
    ArrayLiteralsDisallowed,
    WildcardDisallowed,
    AttributeClassNameConflict,
    MethodNeedsVisibility,
    ReferenceInStrictMode,
    ReferenceInRx,
    DeclareStatementDEPRECATED,
    MisplacedRxOfScopeDEPRECATED,
    RxOfScopeAndExplicitRxDEPRECATED,
    UnsupportedFeatureDEPRECATED,
    TraitInterfaceConstructorPromoDEPRECATED,
    NonstaticPropertyWithLSB,
    ReferenceInAnonUseClauseDEPRECATED,
    RxMoveInvalidLocation,
    MisplacedMutabilityHint,
    MutabilityHintInNonRx,
    InvalidReturnMutableHint,
    NoTparamsOnTypeConstsDEPRECATED,
    PocketUniversesDuplication,
    UnsupportedTraitUseAs,
    UnsupportedInsteadOf,
    InvalidTraitUseAsVisibility,
    InvalidFunPointer,
    IllegalUseOfDynamicallyCallable,
    PocketUniversesNotInClass,
    PocketUniversesAtomMissing,
    PocketUniversesAtomUnknown,
}

impl Naming {
    pub fn fd_name_already_bound(p: Pos) -> Error {
        Error(
            Self::FdNameAlreadyBound as usize,
            vec![(p, "Field name already bound".into())],
        )
    }

    pub fn method_needs_visibility(p: Pos) -> Error {
        Error(
            Self::MethodNeedsVisibility as usize,
            vec![(
                p,
                "Methods need to be marked public, private, or protected.".into(),
            )],
        )
    }

    pub fn unsupported_trait_use_as(p: Pos) -> Error {
        Error(
            Self::UnsupportedTraitUseAs as usize,
            vec![(
                p,
                "Trait use as is a PHP feature that is unsupported in Hack".into(),
            )],
        )
    }

    pub fn unsupported_instead_of(p: Pos) -> Error {
        Error(
            Self::UnsupportedInsteadOf as usize,
            vec![(
                p,
                "insteadof is a PHP feature that is unsupported in Hack".into(),
            )],
        )
    }

    pub fn invalid_trait_use_as_visibility(p: Pos) -> Error {
        Error(
            Self::InvalidTraitUseAsVisibility as usize,
            vec![(
                p,
                "Cannot redeclare trait method's visibility in this manner".into(),
            )],
        )
    }
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
#[repr(usize)]
pub enum NastCheck {
    AbstractBody = 3001,
    AbstractWithBody,
    AwaitInSyncFunction,
    CallBeforeInit,
    CaseFallthrough,
    ContinueInSwitch,
    DangerousMethodNameDEPRECATED,
    DefaultFallthrough,
    InterfaceWithMemberVariable,
    InterfaceWithStaticMemberVariable,
    Magic,
    NoConstructParent,
    NonInterface,
    NotAbstractWithoutBody,
    NotInitialized,
    NotPublicInterface,
    RequiresNonClass,
    ReturnInFinally,
    ReturnInGen,
    ToStringReturnsString,
    ToStringVisibility,
    ToplevelBreak,
    ToplevelContinue,
    UsesNonTrait,
    IllegalFunctionName,
    NotAbstractWithoutTypeconst,
    TypeconstDependsOnExternalTparam,
    TypeconstAssignedTparamDEPRECATED,
    AbstractWithTypeconstDEPRECATED,
    ConstructorRequired,
    InterfaceWithPartialTypeconst,
    MultipleXhpCategory,
    OptionalShapeFieldsNotSupportedDEPRECATED,
    AwaitNotAllowedDEPRECATED,
    AsyncInInterfaceDEPRECATED,
    AwaitInCoroutine,
    YieldInCoroutine,
    SuspendOutsideOfCoroutine,
    SuspendInFinally,
    BreakContinueNNotSupportedDEPRECATED,
    StaticMemoizedFunction,
    InoutParamsOutsideOfSync,
    InoutParamsSpecial,
    InoutParamsMixByref,
    InoutParamsMemoize,
    InoutParamsRetByRefDEPRECATED,
    ReadingFromAppend,
    ConstAttributeProhibitedDEPRECATED,
    RetiredError3049DEPRECATED,
    InoutArgumentBadExpr,
    MutableParamsOutsideOfSyncDEPRECATED,
    MutableAsyncMethodDEPRECATED,
    MutableMethodsMustBeReactive,
    MutableAttributeOnFunction,
    MutableReturnAnnotatedDeclsMustBeReactive,
    IllegalDestructor,
    ConditionallyReactiveFunctionDEPRECATED,
    MultipleConditionallyReactiveAnnotations,
    ConditionallyReactiveAnnotationInvalidArguments,
    MissingReactivityForConditionDEPRECATED,
    MultipleReactivityAnnotationsDEPRECATED,
    RxIsEnabledInvalidLocation,
    MaybeRxInvalidLocation,
    NoOnlyrxIfRxfuncForRxIfArgs,
    CoroutineInConstructor,
    IllegalReturnByRefDEPRECATED,
    IllegalByRefExpr,
    VariadicByRefParam,
    MaybeMutableAttributeOnFunction,
    ConflictingMutableAndMaybeMutableAttributes,
    MaybeMutableMethodsMustBeReactive,
    RequiresFinalClass,
    InterfaceUsesTrait,
    NonstaticMethodInAbstractFinalClass,
    MutableOnStaticDEPRECATED,
    ClassnameConstInstanceOfDEPRECATED,
    ByRefParamOnConstruct,
    ByRefDynamicCall,
    ByRefProperty,
    ByRefCall,
    SwitchNonTerminalDefault,
    SwitchMultipleDefault,
    RepeatedRecordFieldName,
    PhpLambdaDisallowed,
}

impl NastCheck {
    pub fn not_abstract_without_typeconst(p: Pos) -> Error {
        Error(
            Self::NotAbstractWithoutTypeconst as usize,
            vec![(
                p,
                "This type constant is not declared as abstract, it must have an assigned type"
                    .into(),
            )],
        )
    }

    pub fn multiple_xhp_category(p: Pos) -> Error {
        Error(
            Self::MultipleXhpCategory as usize,
            vec![(
                p,
                "XHP classes can only contain one category declaration".into(),
            )],
        )
    }
}
