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

#include "thrift/compiler/ast/thrift_ast_debug_tree_utils.h"

#include <string>
#include <fmt/core.h>

#include "thrift/common/tree_printer.h"
#include "thrift/compiler/ast/t_program_bundle.h"

namespace apache::thrift::compiler {

namespace {
using ::apache::thrift::compiler::scope::program_scope;
using ::apache::thrift::tree_printer::scope;

scope& addChildForSourceRange(
    const std::string& prefix,
    const source_range& sourceRange,
    scope& parentScope) {
  scope& sourceRangeScope = parentScope.make_child("{} [source_range]", prefix);
  sourceRangeScope.make_child("begin (offset): {}", sourceRange.begin.offset());
  sourceRangeScope.make_child("end (offset): {}", sourceRange.end.offset());
  return sourceRangeScope;
}

// The addChildFor...Ptr() functions add information for AST node references
// (which may be empty - i.e., nullptr). They typically only include minimal
// information, used to lookup the full node somewhere else in the debug tree
// (eg. by name, or memory address).

scope& addChildForNamedPtr(
    const std::string& prefix, const t_named* namedPtr, scope& parentScope) {
  scope& namedScope = parentScope.make_child(
      "{} [t_named*] {:#x}", prefix, uintptr_t(namedPtr));
  if (namedPtr != nullptr) {
    namedScope.make_child("scoped_name: {}", namedPtr->get_scoped_name());
  }
  return namedScope;
}

scope& addChildForConstValuePtr(
    const std::string& prefix,
    const t_const_value* constValuePtr,
    scope& parentScope) {
  scope& constValueScope = parentScope.make_child(
      "{} [t_const_value*] {:#x}", prefix, uintptr_t(constValuePtr));
  if (constValuePtr != nullptr) {
    const t_const_value& constValue = *constValuePtr;
    std::optional<source_range> sourceRange = constValue.src_range();
    if (sourceRange.has_value()) {
      addChildForSourceRange("src_range", sourceRange.value(), constValueScope);
    } else {
      constValueScope.make_child("src_range: <absent>");
    }
    constValueScope.make_child("is_empty? {}", constValue.is_empty());
    constValueScope.make_child(
        "kind: {}", t_const_value::kind_to_string(constValue.kind()));
  }
  return constValueScope;
}

scope& addChildForStructuredPtr(
    const std::string& prefix,
    const t_structured* structuredPtr,
    scope& parentScope) {
  scope& structuredPtrScope = parentScope.make_child(
      "{} [t_structured*] {:#x}", prefix, uintptr_t(structuredPtr));
  if (structuredPtr != nullptr) {
    structuredPtrScope.make_child("name: {}", structuredPtr->name());
  }

  return structuredPtrScope;
}

scope& addChildForProgramPtr(
    const std::string& prefix,
    const t_program* programPtr,
    scope& parentScope) {
  scope& programPtrScope = parentScope.make_child(
      "{} [t_program*] {:#x}", prefix, uintptr_t(programPtr));
  if (programPtr != nullptr) {
    programPtrScope.make_child("name: {}", programPtr->name());
  }

  return programPtrScope;
}

scope& addChildForNode(
    const std::string& prefix, const t_node& node, scope& parentScope) {
  scope& nodeScope =
      parentScope.make_child("{} [t_node] @{:#x}", prefix, uintptr_t(&node));
  addChildForSourceRange("src_range", node.src_range(), nodeScope);
  const deprecated_annotation_map& unstructuredAnnotations =
      node.unstructured_annotations();
  scope& unstructuredAnnotationsScope = nodeScope.make_child(
      "unstructured_annotations (size: {})", unstructuredAnnotations.size());
  for (const auto& [name, value] : unstructuredAnnotations) {
    scope& unstructuredAnnotationScope =
        unstructuredAnnotationsScope.make_child("name: {}", name);
    addChildForSourceRange(
        "src_range", value.src_range, unstructuredAnnotationScope);
    unstructuredAnnotationScope.make_child("value: {}", value.value);
  }

  return nodeScope;
}

scope& addChildForTypeRef(
    const std::string& prefix, const t_type_ref& typeRef, scope& parentScope) {
  scope& typeRefScope = parentScope.make_child("{} [t_type_ref]", prefix);
  typeRefScope.make_child("empty? {}", typeRef.empty());
  const bool isResolved = typeRef.resolved();
  typeRefScope.make_child("resolved? {}", isResolved);
  addChildForSourceRange("src_range", typeRef.src_range(), typeRefScope);

  const t_type* typePtr = typeRef.get_type();
  scope& typeScope =
      typeRefScope.make_child("type: [t_type*] {:#x}", uintptr_t(typePtr));
  if (typePtr != nullptr) {
    typeScope.make_child("full_name: {}", typePtr->get_full_name());
  }

  const t_placeholder_typedef* unresolvedTypePtr = typeRef.unresolved_type();
  scope& unresolvedTypeScope = typeRefScope.make_child(
      "unresolved_type: [t_placeholder_typedef*] {:#x}",
      uintptr_t(unresolvedTypePtr));
  if (unresolvedTypePtr != nullptr) {
    unresolvedTypeScope.make_child(
        "full_name: {}", unresolvedTypePtr->get_full_name());
  }

  return typeRefScope;
}

scope& addChildForNamed(
    const std::string& prefix, const t_named& named, scope& parentScope);

scope& addChildForConst(
    const std::string& prefix, const t_const& constParam, scope& parentScope) {
  scope& constScope = parentScope.make_child("{} [t_const]", prefix);
  addChildForNamed("(base)", constParam, constScope);
  addChildForTypeRef("type_ref", constParam.type_ref(), constScope);
  addChildForConstValuePtr("value", constParam.value(), constScope);
  return constScope;
}

scope& addChildForNamed(
    const std::string& prefix, const t_named& named, scope& parentScope) {
  scope& namedScope =
      parentScope.make_child("{} [t_named] @{:#x}", prefix, uintptr_t(&named));
  addChildForNode("(base)", named, namedScope);
  namedScope.make_child("name: {}", named.name());
  namedScope.make_child("scoped_name: {}", named.get_scoped_name());
  namedScope.make_child("uri: {}", named.uri());
  namedScope.make_child("explicit_uri? {}", named.explicit_uri());
  namedScope.make_child("generated? {}", named.generated());

  node_list_view<const t_const> structuredAnnotations =
      named.structured_annotations();
  scope& structuredAnnotationsScope = namedScope.make_child(
      "structured_annotations (size: {})", structuredAnnotations.size());
  for (std::size_t i = 0; i < structuredAnnotations.size(); ++i) {
    addChildForConst(
        fmt::format("structured_annotations[{}]", i),
        structuredAnnotations[i],
        structuredAnnotationsScope);
  }

  namedScope.make_child("has_doc? {}", named.has_doc());
  addChildForSourceRange("doc_range", named.doc_range(), namedScope);

  std::optional<source_range> nameRange = named.name_range();
  if (nameRange.has_value()) {
    addChildForSourceRange("name_range", nameRange.value(), namedScope);
  } else {
    namedScope.make_child("name_range: <absent>");
  }

  return namedScope;
}

scope& addChildForInclude(
    const std::string& prefix, const t_include& include, scope& parentScope) {
  scope& includeScope = parentScope.make_child(
      "{} [t_include] @{:#x}", prefix, uintptr_t(&include));
  addChildForNode("(base)", include, includeScope);
  includeScope.make_child("raw_path: {}", include.raw_path());

  std::optional<std::string_view> alias = include.alias();
  includeScope.make_child(
      "alias: {}", alias.has_value() ? alias.value() : "<absent>");
  addChildForSourceRange("str_range", include.str_range(), includeScope);
  return includeScope;
}

scope& addChildForType(
    const std::string& prefix, const t_type& type, scope& parentScope) {
  scope& typeScope =
      parentScope.make_child("{} [t_type] @{:#x}", prefix, uintptr_t(&type));
  addChildForNamed("(base)", type, typeScope);
  typeScope.make_child("full_name: {}", type.get_full_name());

  const t_type* trueType = type.get_true_type();
  scope& trueTypeScope =
      typeScope.make_child("true_type: [t_type*] {:#x}", uintptr_t(trueType));
  if (trueType != nullptr) {
    trueTypeScope.make_child("full_name: {}", trueType->get_full_name());
  }
  return typeScope;
}

scope& addChildForContainer(
    const std::string& prefix,
    const t_container& container,
    scope& parentScope) {
  scope& containerScope = parentScope.make_child(
      "{} [t_container] @{:#x}", prefix, uintptr_t(&container));
  addChildForType("(base)", container, containerScope);
  return containerScope;
}

scope& addChildForTypedef(
    const std::string& prefix,
    const t_typedef& typedefAst,
    scope& parentScope) {
  scope& typedefScope = parentScope.make_child(
      "{} [t_typedef] @{:#x}", prefix, uintptr_t(&typedefAst));
  addChildForType("(base)", typedefAst, typedefScope);
  addChildForTypeRef("type", typedefAst.type(), typedefScope);
  typedefScope.make_child("typedef_kind: {}", typedefAst.typedef_kind());
  return typedefScope;
}

scope& addChildForEnumValue(
    const std::string& prefix,
    const t_enum_value& enumValue,
    scope& parentScope) {
  scope& enumValueScope = parentScope.make_child(
      "{} [t_enum_value] @{:#x}", prefix, uintptr_t(&enumValue));
  addChildForNamed("(base)", enumValue, enumValueScope);
  enumValueScope.make_child("value: {}", enumValue.get_value());
  enumValueScope.make_child("has_value: {}", enumValue.has_value());
  return enumValueScope;
}

scope& addChildForEnum(
    const std::string& prefix, const t_enum& enumAst, scope& parentScope) {
  scope& enumScope =
      parentScope.make_child("{} [t_enum] @{:#x}", prefix, uintptr_t(&enumAst));
  addChildForType("(base)", enumAst, enumScope);

  node_list_view<const t_enum_value> values = enumAst.values();
  scope& valuesScope = enumScope.make_child("values (size: {})", values.size());
  for (std::size_t i = 0; i < values.size(); ++i) {
    addChildForEnumValue(fmt::format("values[{}]", i), values[i], valuesScope);
  }

  enumScope.make_child("unused: {}", enumAst.unused());

  node_list_view<const t_const> consts = enumAst.consts();
  scope& constsScope = enumScope.make_child("consts (size: {})", consts.size());
  for (std::size_t i = 0; i < consts.size(); ++i) {
    addChildForConst(fmt::format("consts[{}]", i), consts[i], constsScope);
  }

  return enumScope;
}

scope& addChildForField(
    const std::string& prefix, const t_field& field, scope& parentScope) {
  scope& fieldScope =
      parentScope.make_child("{} [t_field] @{:#x}", prefix, uintptr_t(&field));
  addChildForNamed("(base)", field, fieldScope);

  fieldScope.make_child("qualifier: {}", field.qualifier());
  fieldScope.make_child("id: {}", field.id());
  std::optional<t_field_id> explicitId = field.explicit_id();
  if (explicitId.has_value()) {
    fieldScope.make_child("explicit_id: {}", explicitId.value());
  } else {
    fieldScope.make_child("explicit_id: <absent>");
  }
  addChildForTypeRef("type", field.type(), fieldScope);
  fieldScope.make_child("is_injected? {}", field.is_injected());
  addChildForConstValuePtr("default_value", field.default_value(), fieldScope);

  return fieldScope;
}

scope& addChildForStructured(
    const std::string& prefix,
    const t_structured& structured,
    scope& parentScope) {
  scope& structuredScope = parentScope.make_child(
      "{} [t_structured] @{:#x}", prefix, uintptr_t(&structured));
  addChildForType("(base)", structured, structuredScope);

  node_list_view<const t_field> fields = structured.fields();
  scope& fieldsScope =
      structuredScope.make_child("fields (size: {})", fields.size());
  for (std::size_t i = 0; i < fields.size(); ++i) {
    addChildForField(fmt::format("fields[{}]", i), fields[i], fieldsScope);
  }

  return structuredScope;
}

scope& addChildForException(
    const std::string& prefix,
    const t_exception& exception,
    scope& parentScope) {
  scope& exceptionScope = parentScope.make_child(
      "{} [t_exception] @{:#x}", prefix, uintptr_t(&exception));

  addChildForStructured("(base)", exception, exceptionScope);
  exceptionScope.make_child("kind: {}", exception.kind());
  exceptionScope.make_child("blame: {}", exception.blame());
  exceptionScope.make_child("safety: {}", exception.safety());

  exceptionScope.make_child(
      "message_field: [t_field*] {:#x}",
      uintptr_t(exception.get_message_field()));

  return exceptionScope;
}

scope& addChildForParamlist(
    const std::string& prefix,
    const t_paramlist& paramlist,
    scope& parentScope) {
  scope& paramlistScope = parentScope.make_child(
      "{} [t_paramlist] @{:#x}", prefix, uintptr_t(&paramlist));
  addChildForStructured("(base)", paramlist, paramlistScope);
  return paramlistScope;
}

scope& addChildForThrows(
    const std::string& prefix, const t_throws& throws, scope& parentScope) {
  scope& throwsScope = parentScope.make_child(
      "{} [t_throws] @{:#x}", prefix, uintptr_t(&throws));
  addChildForStructured("(base)", throws, throwsScope);
  return throwsScope;
}

scope& addChildForFunction(
    const std::string& prefix, const t_function& function, scope& parentScope) {
  scope& functionScope = parentScope.make_child(
      "{} [t_function] @{:#x}", prefix, uintptr_t(&function));
  addChildForNamed("(base)", function, functionScope);
  addChildForTypeRef("return_type", function.return_type(), functionScope);
  addChildForParamlist("params", function.params(), functionScope);

  const t_throws* exceptions = function.exceptions();
  if (exceptions != nullptr) {
    addChildForThrows("exceptions", *exceptions, functionScope);
  } else {
    functionScope.make_child("exceptions: N/A");
  }

  functionScope.make_child("qualifier: {}", function.qualifier());

  const t_node* sinkOrStream = function.sink_or_stream();
  if (sinkOrStream != nullptr) {
    addChildForNode("sink_or_stream", *sinkOrStream, functionScope);
  } else {
    functionScope.make_child("sink_or_stream: N/A");
  }

  functionScope.make_child(
      "is_interaction_constructor? {}", function.is_interaction_constructor());
  addChildForTypeRef("interaction", function.interaction(), functionScope);
  return functionScope;
}

scope& addChildForInterface(
    const std::string& prefix,
    const t_interface& interface,
    scope& parentScope) {
  scope& interfaceScope = parentScope.make_child(
      "{} [t_interface] {:#x}", prefix, uintptr_t(&interface));
  addChildForType("(base)", interface, interfaceScope);

  node_list_view<const t_function> functions = interface.functions();
  scope& functionsScope =
      interfaceScope.make_child("functions (size: {})", functions.size());

  for (std::size_t i = 0; i < functions.size(); ++i) {
    addChildForFunction(
        fmt::format("functions[{}]", i), functions[i], functionsScope);
  }

  return interfaceScope;
}

scope& addChildForService(
    const std::string& prefix, const t_service& service, scope& parentScope) {
  scope& serviceScope = parentScope.make_child(
      "{} [t_service] @{:#x}", prefix, uintptr_t(&service));
  addChildForInterface("(base)", service, serviceScope);
  if (const t_service* extends = service.extends(); extends != nullptr) {
    addChildForService("extends", *extends, serviceScope);
  } else {
    serviceScope.make_child("extends: N/A");
  }
  addChildForSourceRange(
      "extends_range", service.extends_range(), serviceScope);
  return serviceScope;
}

scope& addChildForPackage(
    const std::string& prefix, const t_package& package, scope& parentScope) {
  scope& packageScope = parentScope.make_child(
      "{} [t_package] @{:#x}", prefix, uintptr_t(&package));

  packageScope.make_child("name: {}", package.name());
  packageScope.make_child("is_explicit? {}", package.is_explicit());
  packageScope.make_child("empty? {}", package.empty());
  return packageScope;
}

scope& addChildForResolutionMismatch(
    const std::string& prefix,
    const ResolutionMismatch& resolutionMismatch,
    scope& parentScope) {
  scope& resolutionMismatchScope = parentScope.make_child(
      "{} [ResolutionMismatch] @{:#x}", prefix, uintptr_t(&resolutionMismatch));

  resolutionMismatchScope.make_child("id", resolutionMismatch.id);
  addChildForSourceRange(
      "id_loc", resolutionMismatch.id_loc, resolutionMismatchScope);
  resolutionMismatchScope.make_child(
      "program: [t_program*] {:#x}", uintptr_t(resolutionMismatch.program));
  addChildForNamedPtr(
      "local_node", resolutionMismatch.local_node, resolutionMismatchScope);
  addChildForNamedPtr(
      "global_node", resolutionMismatch.global_node, resolutionMismatchScope);

  return resolutionMismatchScope;
}
scope& addChildForGlobalScope(
    const std::string& prefix,
    const t_global_scope& globalScope,
    scope& parentScope) {
  scope& globalScopeScope = parentScope.make_child(
      "{} [t_global_scope] @{:#x}", prefix, uintptr_t(&globalScope));

  // placeholder_typedefs
  node_list_view<const t_placeholder_typedef> placeholderTypedefs =
      globalScope.placeholder_typedefs();
  scope& placeholderTypedefsScope = globalScopeScope.make_child(
      "placeholder_typedefs (size: {})", placeholderTypedefs.size());
  for (std::size_t i = 0; i < placeholderTypedefs.size(); ++i) {
    const t_placeholder_typedef& placeholderTypedef = placeholderTypedefs[i];
    scope& placeholderTypedefScope = placeholderTypedefsScope.make_child(
        "placeholder_typedefs[{}] [t_placeholder_typedef] @{:#x}",
        i,
        uintptr_t(&placeholderTypedef));
    addChildForTypedef("(base)", placeholderTypedef, placeholderTypedefScope);
  }

  // resolution_mismatches
  const t_global_scope::ResolutionMismatches& resolutionMismatches =
      globalScope.resolution_mismatches();
  scope& resolutionMismatchesScope = globalScopeScope.make_child(
      "resolution_mismatches (size: {})", resolutionMismatches.size());
  for (const ResolutionMismatch& resolutionMismatch : resolutionMismatches) {
    addChildForResolutionMismatch(
        "resolution_mismatches[?]",
        resolutionMismatch,
        resolutionMismatchesScope);
  }

  // program_scopes
  const t_global_scope::ProgramScopes& programScopes =
      globalScope.program_scopes();
  scope& programScopesScope = globalScopeScope.make_child(
      "program_scopes (size: {})", programScopes.size());
  for (const auto& [key, programScopePtrs] : programScopes) {
    scope& programScopeScope = programScopesScope.make_child(
        "program_scope[\"{}\"] (size: {})", key, programScopePtrs.size());
    for (std::size_t i = 0; i < programScopePtrs.size(); ++i) {
      programScopeScope.make_child(
          "{}: [program_scope*] {:#x}", i, uintptr_t(programScopePtrs[i]));
    }
  }

  return globalScopeScope;
}

scope& addChildForProgramScope(
    const std::string& prefix,
    const program_scope& programScope,
    scope& parentScope) {
  scope& programScopeScope = parentScope.make_child(
      "{} [program_scope] @{:#x}", prefix, uintptr_t(&programScope));

  return programScopeScope;
}

scope& addChildForProgram(
    const std::string& prefix, const t_program& program, scope& parentScope) {
  scope& programScope = parentScope.make_child(
      "{} [t_program] @{:#x}", prefix, uintptr_t(&program));
  addChildForNamed("(base)", program, programScope);

  programScope.make_child("path: {}", program.path());
  programScope.make_child("full_path: {}", program.full_path());
  programScope.make_child("include_prefix: {}", program.include_prefix());

  addChildForPackage("package", program.package(), programScope);

  const t_global_scope* globalScopePtr = program.global_scope();
  if (globalScopePtr != nullptr) {
    addChildForGlobalScope("global_scope", *globalScopePtr, programScope);
  }

  addChildForProgramScope(
      "program_scope", program.program_scope(), programScope);

  const std::vector<t_include*>& includes = program.includes();
  scope& includesScope =
      programScope.make_child("includes (size: {})", includes.size());
  for (std::size_t i = 0; i < includes.size(); ++i) {
    addChildForInclude(
        fmt::format("includes[{}]", i), *includes[i], includesScope);
  }

  const std::vector<t_program*>& includedPrograms =
      program.get_included_programs();
  scope& includedProgramsScope = programScope.make_child(
      "included_programs (size: {})", includedPrograms.size());
  for (std::size_t i = 0; i < includedPrograms.size(); ++i) {
    addChildForProgramPtr(
        fmt::format("included_program[{}]: ", i),
        includedPrograms[i],
        includedProgramsScope);
  }

  const std::vector<t_program*>& includesForCodegen =
      program.get_includes_for_codegen();
  scope& includesForCodegenScope = programScope.make_child(
      "includes_for_codegen (size: {})", includesForCodegen.size());
  for (std::size_t i = 0; i < includesForCodegen.size(); ++i) {
    addChildForProgramPtr(
        fmt::format("includes_for_codegen[{}]: ", i),
        includesForCodegen[i],
        includesForCodegenScope);
  }

  const std::size_t namespaceCount = program.namespaces().size();
  if (namespaceCount > 0) {
    scope& namespaceScope =
        programScope.make_child("namespaces (size: {})", namespaceCount);
    for (const auto& [k, v] : program.namespaces()) {
      namespaceScope.make_child("{}: {}", k, v->ns());
    }
  }

  const std::unordered_map<std::string, std::vector<std::string>>&
      languagesIncludes = program.language_includes();
  scope& languageIncludesScope = programScope.make_child(
      "language_includes (size: {})", languagesIncludes.size());
  for (const auto& [language, languageIncludes] : languagesIncludes) {
    scope& languageScope =
        languageIncludesScope.make_child("language: {}", language);
    for (const std::string& languageInclude : languageIncludes) {
      languageScope.make_child("include: {}", languageInclude);
    }
  }

  const std::vector<t_typedef*>& typedefs = program.typedefs();
  scope& typedefsScope =
      programScope.make_child("typedefs (size: {})", typedefs.size());
  for (std::size_t i = 0; i < typedefs.size(); ++i) {
    addChildForTypedef(
        fmt::format("typedefs[{}]", i), *typedefs[i], typedefsScope);
  }

  const std::vector<t_enum*>& enums = program.enums();
  scope& enumsScope = programScope.make_child("enums (size: {})", enums.size());
  for (std::size_t i = 0; i < enums.size(); ++i) {
    addChildForEnum(fmt::format("enums[{}]", i), *enums[i], enumsScope);
  }

  const std::vector<t_const*>& consts = program.consts();
  scope& constsScope =
      programScope.make_child("consts (size: {})", consts.size());
  for (std::size_t i = 0; i < consts.size(); ++i) {
    addChildForConst(fmt::format("consts[{}]", i), *consts[i], constsScope);
  }

  const std::vector<t_structured*>& structsAndUnions =
      program.structs_and_unions();
  scope& structsAndUnionsScope = programScope.make_child(
      "structs_and_unions (size: {})", structsAndUnions.size());
  for (std::size_t i = 0; i < structsAndUnions.size(); ++i) {
    addChildForStructured(
        fmt::format("structs_and_unions[{}]", i),
        *structsAndUnions[i],
        structsAndUnionsScope);
  }

  const std::vector<t_exception*>& exceptions = program.exceptions();
  scope& exceptionsScope =
      programScope.make_child("exceptions (size: {})", exceptions.size());
  for (std::size_t i = 0; i < exceptions.size(); ++i) {
    addChildForException(
        fmt::format("exceptions[{}]", i), *exceptions[i], exceptionsScope);
  }

  const std::vector<t_service*>& services = program.services();
  scope& servicesScope =
      programScope.make_child("services (size: {})", services.size());
  for (std::size_t i = 0; i < services.size(); ++i) {
    addChildForService(
        fmt::format("services[{}]", i), *services[i], servicesScope);
  }

  const std::vector<t_interaction*>& interactions = program.interactions();
  scope& interactionsScope =
      programScope.make_child("interactions (size: {})", interactions.size());
  for (std::size_t i = 0; i < interactions.size(); ++i) {
    addChildForService(
        fmt::format("interactions[{}]", i),
        *interactions[i],
        interactionsScope);
  }

  node_list_view<const t_named> definitions = program.definitions();
  scope& definitionsScope =
      programScope.make_child("definitions (size: {})", definitions.size());
  for (std::size_t i = 0; i < definitions.size(); ++i) {
    addChildForNamedPtr(
        fmt::format("definitions[{}]:", i), &definitions[i], definitionsScope);
  }

  const std::vector<t_structured*>& structuredDefinitions =
      program.structured_definitions();
  scope& structuredDefinitionsScope = programScope.make_child(
      "structured_definitions (size: {})", structuredDefinitions.size());
  for (std::size_t i = 0; i < structuredDefinitions.size(); ++i) {
    addChildForStructuredPtr(
        fmt::format("structured_definitions[{}]:", i),
        structuredDefinitions[i],
        structuredDefinitionsScope);
  }

  node_list_view<const t_container> typeInstantiations =
      program.type_instantiations();
  scope& typeInstantiationsScope = programScope.make_child(
      "type_instantiations (size: {})", typeInstantiations.size());
  for (std::size_t i = 0; i < typeInstantiations.size(); ++i) {
    addChildForContainer(
        fmt::format("type_instantiations[{}]", i),
        typeInstantiations[i],
        typeInstantiationsScope);
  }

  return programScope;
}

} // namespace

// public static
apache::thrift::tree_printer::scope
ThriftAstDebugTreeUtils::createTreeForProgramBundle(
    const t_program_bundle& programBundle) {
  using apache::thrift::tree_printer::scope;
  scope programBundleScope = scope::make_root("[t_program_bundle]");
  node_list_view<const t_program> programs = programBundle.programs();
  scope& programsScope =
      programBundleScope.make_child("programs (size: {})", programs.size());
  for (std::size_t i = 0; i < programs.size(); ++i) {
    addChildForProgram(
        fmt::format("programs[{}]{}", i, (i == 0 ? " (root_program)" : "")),
        programs[i],
        programsScope);
  }
  return programBundleScope;
}

} // namespace apache::thrift::compiler
