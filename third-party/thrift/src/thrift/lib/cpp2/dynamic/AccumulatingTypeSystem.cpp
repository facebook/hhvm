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

#include <thrift/lib/cpp2/dynamic/AccumulatingTypeSystem.h>

#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>
#include <thrift/lib/cpp2/dynamic/detail/TypeSystemImpl.h>

#include <folly/container/F14Map.h>
#include <folly/lang/Assume.h>

#include <utility>

namespace apache::thrift::type_system {

namespace {

// Resolves a conflict between an incoming definition and an existing one,
// according to the `MergeResolution` policy.
void resolveConflict(MergeResolution resolution, UriView uri) {
  switch (resolution) {
    case MergeResolution::Error:
      throw MergeConflictError(Uri(uri));
    case MergeResolution::TakeFirst:
      return; // keep the existing definition
  }
  folly::assume_unreachable();
}

} // namespace

struct AccumulatingTypeSystem::Impl {
  Impl(MergePolicy p, std::shared_ptr<const TypeSystem> base)
      : policy(p), ts(std::move(base)) {}

  MergePolicy policy;
  // The flat, owning, reference-stable store. Only ever appended to.
  detail::TypeSystemImpl ts;
  // URIs added to this instance (excludes base) -> their policy digest, used to
  // deduplicate/detect conflicts on subsequent additions.
  folly::F14FastMap<Uri, TypeSystemDigest> digests;
};

AccumulatingTypeSystem::AccumulatingTypeSystem(
    MergePolicy policy, std::shared_ptr<const TypeSystem> base)
    : impl_(std::make_unique<Impl>(policy, std::move(base))) {}

AccumulatingTypeSystem::~AccumulatingTypeSystem() = default;

void AccumulatingTypeSystem::addTypes(
    const SerializableTypeSystem& typeSystem) {
  // Validate, deduplicate, and conflict-check the entire batch BEFORE mutating
  // any state, so that a conflict or invalid definition leaves this instance
  // unchanged.
  folly::F14FastMap<Uri, detail::DefinitionEntry> survivors;
  folly::F14FastMap<Uri, TypeSystemDigest> survivorDigests;
  const TypeSystem* base = impl_->ts.base();

  for (const auto& [uri, entry] : *typeSystem.types()) {
    const SerializableTypeDefinition& def = *entry.definition();

    if (base != nullptr) {
      if (auto baseRef = base->getUserDefinedType(uri)) {
        const auto newDigest = TypeSystemHasher{impl_->policy.digestMode}(def);
        const auto baseDigest = TypeSystemHasher{impl_->policy.digestMode}(
            TypeRef::fromDefinition(*baseRef));
        if (newDigest != baseDigest) {
          resolveConflict(impl_->policy.resolution, uri);
        }
        // Present in the base: deduplicated (equal) or resolved (TakeFirst);
        // never added to this instance's working set.
        continue;
      }
    }

    const TypeSystemDigest digest =
        TypeSystemHasher{impl_->policy.digestMode}(def);

    // Already present in this instance's working set (a prior addTypes()).
    if (auto it = impl_->digests.find(uri); it != impl_->digests.end()) {
      if (digest != it->second) {
        resolveConflict(impl_->policy.resolution, uri);
      }
      continue;
    }

    // A genuinely new type: validate its structure and stage it.
    detail::validateDefinition(uri, def);
    survivors.emplace(
        uri, detail::DefinitionEntry{def, entry.sourceInfo().to_optional()});
    survivorDigests.emplace(uri, digest);
  }

  if (survivors.empty()) {
    return;
  }

  // Commit: append the new definitions to the persistent store. Existing nodes
  // are untouched, so previously-returned references remain valid.
  impl_->ts.insertDefinitions(std::move(survivors));
  for (auto& [uri, digest] : survivorDigests) {
    impl_->digests.emplace(uri, digest);
  }
}

void AccumulatingTypeSystem::addTypes(const TypeSystem& typeSystem) {
  auto uris = typeSystem.getKnownUris();
  if (!uris.has_value()) {
    throw InvalidTypeError(
        "Cannot add types from a TypeSystem that does not enumerate its URIs");
  }
  // Serialize the source's definitions (each addDefinition pulls in its
  // transitive closure; the builder deduplicates), then re-materialize them
  // into this instance's own store via the SerializableTypeSystem path.
  auto builder = SerializableTypeSystemBuilder::withSourceInfo(typeSystem);
  for (const auto& uri : *uris) {
    builder.addDefinition(uri);
  }
  addTypes(*std::move(builder).build());
}

void AccumulatingTypeSystem::addType(
    Uri uri, const SerializableTypeDefinitionEntry& entry) {
  SerializableTypeSystem typeSystem;
  typeSystem.types()[std::move(uri)] = entry;
  addTypes(typeSystem);
}

void AccumulatingTypeSystem::addType(
    const TypeSystem& source, DefinitionRef def) {
  // Serialize the single definition (addDefinition pulls in its transitive
  // closure), then re-materialize it into this instance's own store.
  auto builder = SerializableTypeSystemBuilder::withSourceInfo(source);
  builder.addDefinition(def.uri());
  addTypes(*std::move(builder).build());
}

std::optional<DefinitionRef> AccumulatingTypeSystem::getUserDefinedType(
    UriView uri) const {
  return impl_->ts.getUserDefinedType(uri);
}

std::optional<folly::F14FastSet<Uri>> AccumulatingTypeSystem::getKnownUris()
    const {
  return impl_->ts.getKnownUris();
}

std::optional<DefinitionRef>
AccumulatingTypeSystem::getUserDefinedTypeBySourceIdentifier(
    SourceIdentifierView sourceIdentifier) const {
  return impl_->ts.getUserDefinedTypeBySourceIdentifier(sourceIdentifier);
}

std::optional<SourceIdentifierView>
AccumulatingTypeSystem::getSourceIdentiferForUserDefinedType(
    DefinitionRef def) const {
  return impl_->ts.getSourceIdentiferForUserDefinedType(def);
}

TypeSystem::NameToDefinitionsMap
AccumulatingTypeSystem::getUserDefinedTypesAtLocation(
    std::string_view location) const {
  return impl_->ts.getUserDefinedTypesAtLocation(location);
}

std::size_t AccumulatingTypeSystem::size() const {
  return impl_->digests.size();
}

bool AccumulatingTypeSystem::empty() const {
  return impl_->digests.empty();
}

} // namespace apache::thrift::type_system
