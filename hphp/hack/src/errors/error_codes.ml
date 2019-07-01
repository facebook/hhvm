(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
  Please change the error map file if you have changed the error codes.
  The file is at hphp/hack/test/errors/error_map.ml
 *)

(*****************************************************************************)
(* Error codes.
 * Each error has a unique number associated with it. The following modules
 * define the error code associated with each kind of error.
 * It is ok to extend the codes with new values, it is NOT OK to change the
 * value of an existing error to a different error code!
 * I added some comments to make that extra clear :-)
 *)
(*****************************************************************************)

(* Errors in the Temporary range are for errors that will disappear in the
 * future. *)
module Temporary = struct
  let darray_not_supported = 1
  let varray_not_supported = 2
  (* DEPRECATED let unknown_fields_not_supported = 3 *)
  let varray_or_darray_not_supported = 4
  (* DEPRECATED let goto_not_supported = 5 *)
  let nonnull_not_supported = 6
end

module Parsing = struct
  type t =
  | FixmeFormat [@value 1001]
  | ParsingError
  | UnexpectedEofDEPRECATED
  | UnterminatedCommentDEPRECATED
  | UnterminatedXhpCommentDEPRECATED
  | CallTimePassByReferenceDEPRECATED
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
  [@@ deriving enum, show { with_path = false } ]
  let err_code = to_enum
end

module Naming                               = struct
  type t =
  | AddATypehint [@value 2001]
  | TypeparamAlokDEPRECATED
  | AssertArity
  | PrimitiveInvalidAlias
  | CyclicConstraintDEPRECATED
  | DidYouMeanNaming
  | DifferentScopeDEPRECATED
  | DisallowedXhpType
  | DoubleInsteadOfFloatDEPRECATED
  | DynamicClassDEPRECATED
  | LvarInObjGet
  | ErrorNameAlreadyBound
  | ExpectedCollection
  | ExpectedVariable
  | FdNameAlreadyBound
  | GenArrayRecArityDEPRECATED
  | GenArrayVaRecArityDEPRECATED
  | GenaArityDEPRECATED
  | GenericClassVarDEPRECATED
  | GenvaArity
  | IllegalClass
  | IllegalClassMeth
  | IllegalConstant
  | IllegalFun
  | IllegalInstMeth
  | IllegalMethCaller
  | IllegalMethFun
  | IntegerInsteadOfIntDEPRECATED
  | InvalidReqExtends
  | InvalidReqImplements
  | LocalConstDEPRECATED
  | LowercaseThis
  | MethodNameAlreadyBound
  | MissingArrow
  | MissingTypehint
  | NameAlreadyBound
  | NamingTooFewArguments
  | NamingTooManyArguments
  | PrimitiveToplevel
  | RealInsteadOfFloatDEPRECATED
  | ShadowedTypeParam
  | StartWith_T
  | ThisMustBeReturn
  | ThisNoArgument
  | ThisHintOutsideClass
  | ThisReserved
  | TparamWithTparam
  | TypedefConstraintDEPRECATED
  | UnboundName
  | Undefined
  | UnexpectedArrow
  | UnexpectedTypedef
  | UsingInternalClass
  | VoidCast
  | ObjectCast
  | UnsetCast
  | NullsafePropertyAccessDEPRECATED
  | IllegalTrait
  | ShapeTypehintDEPRECATED
  | DynamicNewInStrictMode
  | InvalidTypeAccessRoot
  | DuplicateUserAttribute
  | ReturnOnlyTypehint
  | UnexpectedTypeArguments
  | TooManyTypeArguments
  | ClassnameParam
  | InvalidInstanceof
  | NameIsReserved
  | DollardollarUnused
  | IllegalMemberVariableClass
  | TooFewTypeArguments
  | GotoLabelAlreadyDefined
  | GotoLabelUndefined
  | GotoLabelDefinedInFinally
  | GotoInvokedInFinally
  | DynamicClassPropertyNameInStrictModeDEPRECATED
  | ThisAsLexicalVariable
  | DynamicClassNameInStrictMode
  | XhpOptionalRequiredAttr
  | XhpRequiredWithDefault
  | VariableVariablesDisallowedDEPRECATED
  | ArrayTypehintsDisallowed
  | ArrayLiteralsDisallowed
  | WildcardDisallowed
  | AttributeClassNameConflict
  | MethodNeedsVisibility
  | ReferenceInStrictMode
  | ReferenceInRx
  | DeclareStatementDEPRECATED
  | MisplacedRxOfScopeDEPRECATED
  | RxOfScopeAndExplicitRxDEPRECATED
  | UnsupportedFeatureDEPRECATED
  | TraitInterfaceConstructorPromoDEPRECATED
  | NonstaticPropertyWithLSB
  | ReferenceInAnonUseClauseDEPRECATED
  | RxMoveInvalidLocation
  | MisplacedMutabilityHint
  | MutabilityHintInNonRx
  | InvalidReturnMutableHint
  | NoTparamsOnTypeConstsDEPRECATED
  | PocketUniversesDuplication
  | UnsupportedTraitUseAs
  | UnsupportedInsteadOf
  | InvalidTraitUseAsVisibility
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
  [@@ deriving enum, show { with_path = false } ]
  let err_code = to_enum
end

module NastCheck                            = struct
  type t =
  | AbstractBody [@value 3001]
  | AbstractWithBody
  | AwaitInSyncFunction
  | CallBeforeInit
  | CaseFallthrough
  | ContinueInSwitch
  | DangerousMethodName
  | DefaultFallthrough
  | InterfaceWithMemberVariable
  | InterfaceWithStaticMemberVariable
  | Magic
  | NoConstructParent
  | NonInterface
  | NotAbstractWithoutBody
  | NotInitialized
  | NotPublicInterface
  | RequiresNonClass
  | ReturnInFinally
  | ReturnInGen
  | ToStringReturnsString
  | ToStringVisibility
  | ToplevelBreak
  | ToplevelContinue
  | UsesNonTrait
  | IllegalFunctionName
  | NotAbstractWithoutTypeconst
  | TypeconstDependsOnExternalTparam
  | TypeconstAssignedTparamDEPRECATED
  | AbstractWithTypeconstDEPRECATED
  | ConstructorRequired
  | InterfaceWithPartialTypeconst
  | MultipleXhpCategory
  | OptionalShapeFieldsNotSupportedDEPRECATED
  | AwaitNotAllowed
  | AsyncInInterfaceDEPRECATED
  | AwaitInCoroutine
  | YieldInCoroutine
  | SuspendOutsideOfCoroutine
  | SuspendInFinally
  | BreakContinueNNotSupported
  | StaticMemoizedFunction
  | InoutParamsOutsideOfSync
  | InoutParamsSpecial
  | InoutParamsMixByref
  | InoutParamsMemoize
  | InoutParamsRetByRefDEPRECATED
  | ReadingFromAppend
  | ConstAttributeProhibited
  | RetiredError3049DEPRECATED
  | InoutArgumentBadExpr
  | MutableParamsOutsideOfSyncDEPRECATED
  | MutableAsyncMethodDEPRECATED
  | MutableMethodsMustBeReactive
  | MutableAttributeOnFunction
  | MutableReturnAnnotatedDeclsMustBeReactive
  | IllegalDestructor
  | ConditionallyReactiveFunctionDEPRECATED
  | MultipleConditionallyReactiveAnnotations
  | ConditionallyReactiveAnnotationInvalidArguments
  | MissingReactivityForConditionDEPRECATED
  | MultipleReactivityAnnotationsDEPRECATED
  | RxIsEnabledInvalidLocation
  | MaybeRxInvalidLocation
  | NoOnlyrxIfRxfuncForRxIfArgs
  | CoroutineInConstructor
  | IllegalReturnByRefDEPRECATED
  | IllegalByRefExpr
  | VariadicByRefParam
  | MaybeMutableAttributeOnFunction
  | ConflictingMutableAndMaybeMutableAttributes
  | MaybeMutableMethodsMustBeReactive
  | RequiresFinalClass
  | InterfaceUsesTrait
  | NonstaticMethodInAbstractFinalClass
  | MutableOnStaticDEPRECATED
  | ClassnameConstInstanceOf
  | ByRefParamOnConstruct
  | ByRefDynamicCall
  | ByRefProperty
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
  [@@ deriving enum, show { with_path = false } ]
  let err_code = to_enum
end

module Typing                               = struct
  type t =
  | AbstractClassFinalDEPRECATED [@value 4001]
  | UninstantiableClass
  | AnonymousRecursive
  | AnonymousRecursiveCall
  | ArrayAccess
  | ArrayAppend
  | ArrayCast
  | ArrayGetArity
  | BadCall
  | ClassArity
  | ConstMutation
  | ConstructorNoArgs
  | CyclicClassDef
  | CyclicTypedef
  | DiscardedAwaitable
  | IssetEmptyInStrict
  | DynamicYieldPrivateDEPRECATED
  | EnumConstantTypeBad
  | EnumSwitchNonexhaustive
  | EnumSwitchNotConst
  | EnumSwitchRedundant
  | EnumSwitchRedundantDefault
  | EnumSwitchWrongClass
  | EnumTypeBad
  | EnumTypeTypedefMixedDEPRECATED
  | ExpectedClass
  | ExpectedLiteralFormatString
  | ExpectedStaticIntDEPRECATED
  | ExpectedTparam
  | ExpectingReturnTypeHint
  | ExpectingReturnTypeHintSuggest
  | ExpectingTypeHint
  | ExpectingTypeHintSuggest
  | RetiredError4034DEPRECATED
  | ExtendFinal
  | FieldKinds
  | FieldMissingDEPRECATED
  | FormatString
  | FunArityMismatch
  | FunTooFewArgs
  | FunTooManyArgs
  | FunUnexpectedNonvariadic
  | FunVariadicityHhVsPhp56
  | GenaExpectsArrayDEPRECATED
  | GenericArrayStrict
  | GenericStatic
  | ImplementAbstract
  | InterfaceFinal
  | InvalidShapeFieldConst
  | InvalidShapeFieldLiteral
  | InvalidShapeFieldName
  | InvalidShapeFieldType
  | MemberNotFound
  | MemberNotImplemented
  | MissingAssign
  | MissingConstructor
  | MissingField
  | NegativeTupleIndexDEPRECATED
  | SelfOutsideClass
  | NewStaticInconsistent
  | StaticOutsideClass
  | NonObjectMember
  | NullContainer
  | NullMember
  | NullableParameterDEPRECATED
  | OptionReturnOnlyTypehint
  | ObjectString
  | OptionMixed
  | OverflowDEPRECATED
  | OverrideFinal
  | OverridePerTrait
  | PairArity
  | AbstractCall
  | ParentInTrait
  | ParentOutsideClass
  | ParentUndefined
  | PreviousDefault
  | PrivateClassMeth
  | PrivateInstMeth
  | PrivateOverride
  | ProtectedClassMeth
  | ProtectedInstMeth
  | ReadBeforeWrite
  | ReturnInVoid
  | ShapeFieldClassMismatch
  | ShapeFieldTypeMismatch
  | ShouldBeOverride
  | SketchyNullCheckDEPRECATED
  | SketchyNullCheckPrimitiveDEPRECATED
  | SmemberNotFound
  | StaticDynamic
  | StaticOverflowDEPRECATED
  | RetiredError4093DEPRECATED
  | ThisInStaticDEPRECATED
  | ThisVarOutsideClass
  | TraitFinal
  | TupleArityDEPRECATED
  | TupleArityMismatchDEPRECATED
  | TupleIndexTooLargeDEPRECATED
  | TupleSyntax
  | TypeArityMismatch
  | TypeParamArity
  | RetiredError4103DEPRECATED
  | TypingTooFewArgs
  | TypingTooManyArgs
  | UnboundGlobal
  | UnboundNameTyping
  | UndefinedField
  | UndefinedParent
  | UnifyError
  | UnsatisfiedReq
  | Visibility
  | VisibilityExtends
  | VoidParameterDEPRECATED
  | WrongExtendKind
  | GenericUnify
  | NullsafeNotNeeded
  | TrivialStrictEq
  | VoidUsage
  | DeclaredCovariant
  | DeclaredContravariant
  | UnsetInStrictDEPRECATED
  | StrictMembersNotKnown
  | ErasedGenericAtRuntime
  | DynamicClassDEPRECATED
  | AttributeTooManyArguments
  | AttributeParamType
  | DeprecatedUse
  | AbstractConstUsage
  | CannotDeclareConstant
  | CyclicTypeconst
  | NullsafePropertyWriteContext
  | NoreturnUsage
  | ThisLvalueDEPRECATED
  | UnsetNonidxInStrict
  | InvalidShapeFieldNameEmpty
  | InvalidShapeFieldNameNumberDEPRECATED
  | ShapeFieldsUnknown
  | InvalidShapeRemoveKey
  | MissingOptionalFieldDEPRECATED
  | ShapeFieldUnset
  | AbstractConcreteOverride
  | LocalVariableModifedAndUsed
  | LocalVariableModifedTwice
  | AssignDuringCase
  | CyclicEnumConstraint
  | UnpackingDisallowed
  | InvalidClassname
  | InvalidMemoizedParam
  | IllegalTypeStructure
  | NotNullableCompareNullTrivial
  | ClassPropertyOnlyStaticLiteralDEPRECATED
  | AttributeTooFewArguments
  | ReferenceExprDEPRECATED
  | UnificationCycle
  | KeysetSet
  | EqIncompatibleTypes
  | ContravariantThis
  | InstanceofAlwaysFalseDEPRECATED
  | InstanceofAlwaysTrueDEPRECATED
  | AmbiguousMember
  | InstanceofGenericClassname
  | RequiredFieldIsOptional
  | FinalProperty
  | ArrayGetWithOptionalField
  | UnknownFieldDisallowedInShape
  | NullableCast
  | PassByRefAnnotationMissing
  | NonCallArgumentInSuspend
  | NonCoroutineCallInSuspend
  | CoroutineCallOutsideOfSuspend
  | FunctionIsNotCoroutine
  | CoroutinnessMismatch
  | ExpectingAwaitableReturnTypeHint
  | ReffinessInvariant
  | DollardollarLvalue
  | StaticMethodOnInterfaceDEPRECATED
  | DuplicateUsingVar
  | IllegalDisposable
  | EscapingDisposable
  | PassByRefAnnotationUnexpected
  | InoutAnnotationMissing
  | InoutAnnotationUnexpected
  | InoutnessMismatch
  | StaticSyntheticMethod
  | TraitReuse
  | InvalidNewDisposable
  | EscapingDisposableParameter
  | AcceptDisposableInvariant
  | InvalidDisposableHint
  | XhpRequired
  | EscapingThis
  | IllegalXhpChild
  | MustExtendDisposable
  | InvalidIsAsExpressionHint
  | AssigningToConst
  | SelfConstParentNot
  | ParentConstSelfNot
  | PartiallyValidIsAsExpressionHintDEPRECATED
  | NonreactiveFunctionCall
  | NonreactiveIndexing
  | ObjSetReactive
  | FunReactivityMismatch
  | OverridingPropConstMismatch
  | InvalidReturnDisposable
  | InvalidDisposableReturnHint
  | ReturnDisposableMismatch
  | InoutArgumentBadType
  | InconsistentUnsetDEPRECATED
  | ReassignMutableVar
  | InvalidFreezeTarget
  | InvalidFreezeUse
  | FreezeInNonreactiveContext
  | MutableCallOnImmutable
  | MutableArgumentMismatch
  | InvalidMutableReturnResult
  | MutableReturnResultMismatch
  | NonreactiveCallFromShallow
  | EnumTypeTypedefNonnull
  | RxEnabledInNonRxContext
  | RxEnabledInLambdasDEPRECATED
  | AmbiguousLambda
  | EllipsisStrictMode
  | UntypedLambdaStrictMode
  | BindingRefInArray
  | EchoInReactiveContext
  | SuperglobalInReactiveContext
  | StaticPropertyInReactiveContext
  | StaticInReactiveContextDEPRECATED
  | GlobalInReactiveContextDEPRECATED
  | WrongExpressionKindAttribute
  | AttributeClassNoConstructorArgsDEPRECATED
  | InvalidTypeForOnlyrxIfRxfuncParameter
  | MissingAnnotationForOnlyrxIfRxfuncParameter
  | CannotReturnBorrowedValueAsImmutable
  | DeclOverrideMissingHint
  | InvalidConditionallyReactiveCall
  | ExtendSealed
  | SealedFinalDEPRECATED
  | ComparisonInvalidTypes
  | OptionVoidDEPRECATED
  | MutableInNonreactiveContext
  | InvalidArgumentOfRxMutableFunction
  | LetVarImmutabilityViolation
  | UnsealableDEPRECATED
  | ReturnVoidToRxMismatch
  | ReturnsVoidToRxAsNonExpressionStatement
  | NonawaitedAwaitableInReactiveContext
  | ShapesKeyExistsAlwaysTrue
  | ShapesKeyExistsAlwaysFalse
  | ShapesMethodAccessWithNonExistentField
  | NonClassMember
  | PassingArrayCellByRef
  | CallSiteReactivityMismatch
  | RxParameterConditionMismatch
  | AmbiguousObjectAccess
  | ExtendPPL
  | ReassignMaybeMutableVar
  | MaybeMutableArgumentMismatch
  | ImmutableArgumentMismatch
  | ImmutableCallOnMutable
  | InvalidCallMaybeMutable
  | MutabilityMismatch
  | InvalidPPLCall
  | InvalidPPLStaticCall
  | TypeTestInLambdaDEPRECATED
  | InvalidTraversableInRx
  | ReassignMutableThis
  | MutableExpressionAsMultipleMutableArguments
  | InvalidUnsetTargetInRx
  | CoroutineOutsideExperimental
  | PPLMethPointer
  | InvalidTruthinessTest
  | RePrefixedNonString
  | BadRegexPattern
  | SketchyTruthinessTest
  | LateInitWithDefault
  | OverrideMemoizeLSB
  | ClassVarTypeGenericParam
  | InvalidSwitchCaseValueType
  | StringCast
  | BadLateInitOverride
  | EscapingMutableObject
  | OverrideLSB
  | MultipleConcreteDefs
  | MoveInNonreactiveContext
  | InvalidMoveUse
  | InvalidMoveTarget
  | IgnoredResultOfFreezeDEPRECATED
  | IgnoredResultOfMoveDEPRECATED
  | UnexpectedTy
  | UnserializableType
  | InconsistentMutability
  | InvalidMutabilityFlavorInAssignment
  | OptionNull
  | UnknownObjectMember
  | UnknownType
  | InvalidArrayKey
  | ReferenceExprNotFunctionArg
  | RedundantRxCondition
  | RedeclaringMissingMethod
  | InvalidEnforceableTypeArgument
  | RequireArgsReify
  | TypecheckerTimeout
  | InvalidReifiedArgument
  | GenericsNotAllowed
  | InvalidNewableTypeArgument
  | InvalidNewableTypeParamConstraints
  | NewWithoutNewable
  | NewStaticClassReified
  | MemoizeReified
  | ConsistentConstructReified
  | MethodVariance
  | MissingXhpRequiredAttr
  | BadXhpAttrRequiredOverride
  | ReifiedTparamVariadic
  | UnresolvedTypeVariable
  | InvalidSubString
  | InvalidArrayKeyConstraint
  | OverrideNoDefaultTypeconst
  (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
  [@@ deriving enum, show { with_path = false } ]
  let err_code = to_enum
end

(* 5xxx: reserved for FB lint *)
(* 6xxx: reserved for FB ai *)
(* 7xxx: reserved for FB ai *)

module Init = struct
  type t =
  | ForwardCompatibilityNotCurrent [@value 8001]
  | ForwardCompatibilityBelowMinimum
  [@@ deriving enum, show { with_path = false } ]
  let err_code = to_enum
end

(* 9xxx: reserved for FB ai *)
(* 10xxx: reserved for FB ai *)
