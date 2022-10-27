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

// TODO(dokwon): Make uris inline when thrift compiler is C++17 ready.

// thrift
constexpr auto kBitmaskEnum = "facebook.com/thrift/annotation/BitmaskEnum";
constexpr auto kBoxUri = "facebook.com/thrift/annotation/Box";
constexpr auto kInternBoxUri = "facebook.com/thrift/annotation/InternBox";
constexpr auto kExceptionMessageUri =
    "facebook.com/thrift/annotation/ExceptionMessage";
constexpr auto kGenDefaultEnumValueUri =
    "facebook.com/thrift/annotation/GenDefaultEnumValue";
constexpr auto kGenerateRuntimeSchemaUri =
    "facebook.com/thrift/annotation/GenerateRuntimeSchema";
constexpr auto kInjectMetadataFieldsUri =
    "facebook.com/thrift/annotation/InjectMetadataFields";
constexpr auto kMixinUri = "facebook.com/thrift/annotation/Mixin";
constexpr auto kNoLegacyUri = "facebook.com/thrift/annotation/NoLegacy";
constexpr auto kReserveIdsUri = "facebook.com/thrift/annotation/ReserveIds";
constexpr auto kSerializeInFieldIdOrderUri =
    "facebook.com/thrift/annotation/SerializeInFieldIdOrder";
constexpr auto kSetGeneratedUri = "facebook.com/thrift/annotation/SetGenerated";
constexpr auto kSchemaAnnotationUri = "facebook.com/thrift/annotation/Schema";
constexpr auto kTerseWriteUri = "facebook.com/thrift/annotation/TerseWrite";
constexpr auto kTransitiveUri = "facebook.com/thrift/annotation/Transitive";

// scope
constexpr auto kScopeProgramUri = "facebook.com/thrift/annotation/Program";
constexpr auto kScopeStructUri = "facebook.com/thrift/annotation/Struct";
constexpr auto kScopeUnionUri = "facebook.com/thrift/annotation/Union";
constexpr auto kScopeExceptionUri = "facebook.com/thrift/annotation/Exception";
constexpr auto kScopeFieldUri = "facebook.com/thrift/annotation/Field";
constexpr auto kScopeTypedefUri = "facebook.com/thrift/annotation/Typedef";
constexpr auto kScopeServiceUri = "facebook.com/thrift/annotation/Service";
constexpr auto kScopeInteractionUri =
    "facebook.com/thrift/annotation/Interaction";
constexpr auto kScopeFunctionUri = "facebook.com/thrift/annotation/Function";
constexpr auto kScopeEnumUri = "facebook.com/thrift/annotation/Enum";
constexpr auto kScopeEnumValueUri = "facebook.com/thrift/annotation/EnumValue";
constexpr auto kScopeConstUri = "facebook.com/thrift/annotation/Const";

// compatibility
constexpr auto kStringsUri = "facebook.com/thrift/annotation/Strings";

// cpp
constexpr auto kCppAdapterUri = "facebook.com/thrift/annotation/cpp/Adapter";
constexpr auto kCppDisableLazyChecksumUri =
    "facebook.com/thrift/annotation/cpp/DisableLazyChecksum";
constexpr auto kCppEnumTypeUri = "facebook.com/thrift/annotation/cpp/EnumType";
constexpr auto kCppFieldInterceptorUri =
    "facebook.com/thrift/annotation/cpp/FieldInterceptor";
constexpr auto kCppLazyUri = "facebook.com/thrift/annotation/cpp/Lazy";
constexpr auto kCppMinimizePaddingUri =
    "facebook.com/thrift/annotation/cpp/MinimizePadding";
constexpr auto kCppPackIssetUri =
    "facebook.com/thrift/annotation/cpp/PackIsset";
constexpr auto kCppRefUri = "facebook.com/thrift/annotation/cpp/Ref";
constexpr auto kCppScopedEnumAsUnionTypeUri =
    "facebook.com/thrift/annotation/cpp/ScopedEnumAsUnionType";
constexpr auto kCppStrongTypeUri =
    "facebook.com/thrift/annotation/cpp/StrongType";
constexpr auto kCppTriviallyRelocatableUri =
    "facebook.com/thrift/annotation/cpp/TriviallyRelocatable";
constexpr auto kCppUseOpEncodeUri =
    "facebook.com/thrift/annotation/cpp/UseOpEncode";

// java
constexpr auto kJavaAdapterUri = "facebook.com/thrift/annotation/java/Adapter";

// python
constexpr auto kPythonAdapterUri =
    "facebook.com/thrift/annotation/python/Adapter";

// hack
constexpr auto kHackAdapterUri = "facebook.com/thrift/annotation/hack/Adapter";
constexpr auto kHackAttributeUri =
    "facebook.com/thrift/annotation/hack/Attributes";
constexpr auto kHackFieldWrapperUri =
    "facebook.com/thrift/annotation/hack/FieldWrapper";
constexpr auto kHackNameUri = "facebook.com/thrift/annotation/hack/Name";
constexpr auto kHackSkipCodegenUri =
    "facebook.com/thrift/annotation/hack/SkipCodegen";
constexpr auto kHackStructAsTraitUri =
    "facebook.com/thrift/annotation/hack/StructAsTrait";
constexpr auto kHackStructTraitUri =
    "facebook.com/thrift/annotation/hack/StructTrait";
constexpr auto kHackUnionEnumAttributesUri =
    "facebook.com/thrift/annotation/hack/UnionEnumAttributes";
constexpr auto kHackWrapperUri = "facebook.com/thrift/annotation/hack/Wrapper";

} // namespace compiler
} // namespace thrift
} // namespace apache
