/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

namespace apache {
namespace thrift {
namespace compiler {

// thrift
inline constexpr auto kBitmaskEnumUri =
    "facebook.com/thrift/annotation/BitmaskEnum";
inline constexpr auto kBoxUri = "facebook.com/thrift/annotation/Box";
inline constexpr auto kInternBoxUri =
    "facebook.com/thrift/annotation/InternBox";
inline constexpr auto kExperimentalUri =
    "facebook.com/thrift/annotation/Experimental";
inline constexpr auto kExceptionMessageUri =
    "facebook.com/thrift/annotation/ExceptionMessage";
inline constexpr auto kGenerateRuntimeSchemaUri =
    "facebook.com/thrift/annotation/GenerateRuntimeSchema";
inline constexpr auto kInjectMetadataFieldsUri =
    "facebook.com/thrift/annotation/InjectMetadataFields";
inline constexpr auto kMixinUri = "facebook.com/thrift/annotation/Mixin";
inline constexpr auto kNoLegacyUri = "facebook.com/thrift/annotation/NoLegacy";
inline constexpr auto kReserveIdsUri =
    "facebook.com/thrift/annotation/ReserveIds";
inline constexpr auto kSerializeInFieldIdOrderUri =
    "facebook.com/thrift/annotation/SerializeInFieldIdOrder";
inline constexpr auto kSetGeneratedUri =
    "facebook.com/thrift/annotation/SetGenerated";
inline constexpr auto kSchemaAnnotationUri =
    "facebook.com/thrift/annotation/Schema";
inline constexpr auto kTerseWriteUri =
    "facebook.com/thrift/annotation/TerseWrite";
inline constexpr auto kTransitiveUri =
    "facebook.com/thrift/annotation/Transitive";
inline constexpr auto kSerialUri = "facebook.com/thrift/annotation/Serial";
inline constexpr auto kUriUri = "facebook.com/thrift/annotation/Uri";
inline constexpr auto kPriorityUri = "facebook.com/thrift/annotation/Priority";
inline constexpr auto kDeprecatedUnvalidatedAnnotationsUri =
    "facebook.com/thrift/annotation/DeprecatedUnvalidatedAnnotations";

// scope
inline constexpr auto kScopeProgramUri =
    "facebook.com/thrift/annotation/Program";
inline constexpr auto kScopeStructUri = "facebook.com/thrift/annotation/Struct";
inline constexpr auto kScopeUnionUri = "facebook.com/thrift/annotation/Union";
inline constexpr auto kScopeExceptionUri =
    "facebook.com/thrift/annotation/Exception";
inline constexpr auto kScopeFieldUri = "facebook.com/thrift/annotation/Field";
inline constexpr auto kScopeTypedefUri =
    "facebook.com/thrift/annotation/Typedef";
inline constexpr auto kScopeServiceUri =
    "facebook.com/thrift/annotation/Service";
inline constexpr auto kScopeInteractionUri =
    "facebook.com/thrift/annotation/Interaction";
inline constexpr auto kScopeFunctionUri =
    "facebook.com/thrift/annotation/Function";
inline constexpr auto kScopeEnumUri = "facebook.com/thrift/annotation/Enum";
inline constexpr auto kScopeEnumValueUri =
    "facebook.com/thrift/annotation/EnumValue";
inline constexpr auto kScopeConstUri = "facebook.com/thrift/annotation/Const";

// compatibility
inline constexpr auto kStringsUri = "facebook.com/thrift/annotation/Strings";
inline constexpr auto kEnumsUri = "facebook.com/thrift/annotation/Enums";

// cpp
inline constexpr auto kCppAdapterUri =
    "facebook.com/thrift/annotation/cpp/Adapter";
inline constexpr auto kCppDisableLazyChecksumUri =
    "facebook.com/thrift/annotation/cpp/DisableLazyChecksum";
inline constexpr auto kCppEnumTypeUri =
    "facebook.com/thrift/annotation/cpp/EnumType";
inline constexpr auto kCppFieldInterceptorUri =
    "facebook.com/thrift/annotation/cpp/FieldInterceptor";
inline constexpr auto kCppGenerateTypedInterceptor =
    "facebook.com/thrift/annotation/cpp/GenerateTypedInterceptor";
inline constexpr auto kCppLazyUri = "facebook.com/thrift/annotation/cpp/Lazy";
inline constexpr auto kCppMinimizePaddingUri =
    "facebook.com/thrift/annotation/cpp/MinimizePadding";
inline constexpr auto kCppPackIssetUri =
    "facebook.com/thrift/annotation/cpp/PackIsset";
inline constexpr auto kCppRefUri = "facebook.com/thrift/annotation/cpp/Ref";
inline constexpr auto kCppScopedEnumAsUnionTypeUri =
    "facebook.com/thrift/annotation/cpp/ScopedEnumAsUnionType";
inline constexpr auto kCppTriviallyRelocatableUri =
    "facebook.com/thrift/annotation/cpp/TriviallyRelocatable";
inline constexpr auto kCppUseOpEncodeUri =
    "facebook.com/thrift/annotation/cpp/UseOpEncode";
inline constexpr auto kCppFrozen2ExcludeUri =
    "facebook.com/thrift/annotation/cpp/Frozen2Exclude";
inline constexpr auto kCppRuntimeAnnotation =
    "facebook.com/thrift/annotation/cpp/RuntimeAnnotation";
inline constexpr auto kCppTypeUri = "facebook.com/thrift/annotation/cpp/Type";
inline constexpr auto kCppNameUri = "facebook.com/thrift/annotation/cpp/Name";
inline constexpr auto kCppProcessInEbThreadUri =
    "facebook.com/thrift/annotation/cpp/ProcessInEbThreadUnsafe";

// java
inline constexpr auto kJavaMutableUri =
    "facebook.com/thrift/annotation/java/Mutable";
inline constexpr auto kJavaAnnotationUri =
    "facebook.com/thrift/annotation/java/Annotation";
inline constexpr auto kJavaRecursiveUri =
    "facebook.com/thrift/annotation/java/Recursive";
inline constexpr auto kJavaAdapterUri =
    "facebook.com/thrift/annotation/java/Adapter";
inline constexpr auto kJavaWrapperUri =
    "facebook.com/thrift/annotation/java/Wrapper";

// python
inline constexpr auto kPythonAdapterUri =
    "facebook.com/thrift/annotation/python/Adapter";
inline constexpr auto kPythonPy3HiddenUri =
    "facebook.com/thrift/annotation/python/Py3Hidden";
inline constexpr auto kPythonFlagsUri =
    "facebook.com/thrift/annotation/python/Flags";
inline constexpr auto kPythonNameUri =
    "facebook.com/thrift/annotation/python/Name";
inline constexpr auto kUseCAPIUri =
    "facebook.com/thrift/annotation/python/UseCAPI";

// hack
inline constexpr auto kHackAdapterUri =
    "facebook.com/thrift/annotation/hack/Adapter";
inline constexpr auto kHackAttributeUri =
    "facebook.com/thrift/annotation/hack/Attributes";
inline constexpr auto kHackFieldWrapperUri =
    "facebook.com/thrift/annotation/hack/FieldWrapper";
inline constexpr auto kHackNameUri = "facebook.com/thrift/annotation/hack/Name";
inline constexpr auto kHackSkipCodegenUri =
    "facebook.com/thrift/annotation/hack/SkipCodegen";
inline constexpr auto kHackStructAsTraitUri =
    "facebook.com/thrift/annotation/hack/StructAsTrait";
inline constexpr auto kHackStructTraitUri =
    "facebook.com/thrift/annotation/hack/StructTrait";
inline constexpr auto kHackUnionEnumAttributesUri =
    "facebook.com/thrift/annotation/hack/UnionEnumAttributes";
inline constexpr auto kHackWrapperUri =
    "facebook.com/thrift/annotation/hack/Wrapper";
inline constexpr auto kHackModuleInternalUri =
    "facebook.com/thrift/annotation/hack/ModuleInternal";

// go
inline constexpr auto kGoNameUri = "facebook.com/thrift/annotation/go/Name";
inline constexpr auto kGoTagUri = "facebook.com/thrift/annotation/go/Tag";

inline constexpr auto kGeneratePatchUri =
    "facebook.com/thrift/op/GeneratePatch";
inline constexpr auto kAssignOnlyPatchUri =
    "facebook.com/thrift/op/AssignOnlyPatch";

} // namespace compiler
} // namespace thrift
} // namespace apache
