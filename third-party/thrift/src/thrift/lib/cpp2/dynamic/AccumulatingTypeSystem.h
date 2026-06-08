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

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include <fmt/core.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

namespace apache::thrift::type_system {

/**
 * A TypeSystem implementation that is built up incrementally by folding in
 * Types/TypeSystems over time.
 *
 * Each call to addTypes() materializes any new definitions into a single, flat
 * underlying store. Unlike the TypeSystem layering per batch (which produces
 * an overlay chain of O(n) depth, after adding n typesystems), this keeps
 * lookups flat.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Reference stability + immutability
 * ──────────────────────────────────────────────────────────────────────────
 * Definitions are stored in a node-stable container and are considered
 * immutable after being added. Therefore any DefinitionRef or TypeRef
 * previously returned from this instance remains valid across subsequent
 * addTypes() calls, for the lifetime of this instance.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Deduplication & conflicts
 * ──────────────────────────────────────────────────────────────────────────
 * When an incoming definition shares a URI with one already present (either in
 * a prior batch or in the base TypeSystem), it is compared per `MergePolicy`:
 *   - equal      -> deduplicated (the incoming definition is dropped)
 *   - not equal  -> MergeResolution::Error throws; TakeFirst keeps the former
 *
 * A new definition may reference types defined earlier in the same batch
 * (including cyclically), types added in a previous batch, or types in the
 * base. It may NOT reference a type that is only added in a later batch.
 *
 * ──────────────────────────────────────────────────────────────────────────
 * Thread safety
 * ──────────────────────────────────────────────────────────────────────────
 * This class performs NO internal synchronization. Callers that share an
 * instance across threads must coordinate externally. Reference stability
 * guarantees hold across threads.
 */
class AccumulatingTypeSystem final : public TypeSystem {
 public:
  /**
   * Thrown when an incoming definition conflicts with an existing one (same
   * URI, unequal under the policy's digest mode) while MergeResolution::Error
   * is in effect.
   */
  class MergeConflictError : public std::runtime_error {
   public:
    explicit MergeConflictError(Uri uri)
        : std::runtime_error(
              fmt::format(
                  "Conflicting definitions for URI '{}' during TypeSystem merge",
                  uri)),
          uri_(std::move(uri)) {}

    /** The URI of the definition that conflicted. */
    const Uri& uri() const { return uri_; }

   private:
    Uri uri_;
  };

  /**
   * What to do when two definitions that share a URI are NOT equal (per the
   * MergePolicy's digest mode).
   */
  enum class MergeResolution {
    /** Throw MergeConflictError on conflict. */
    Error,
    /** Keep whichever definition was added first; ignore the conflicting one.
     */
    TakeFirst,
  };

  struct MergePolicy {
    // How two definitions sharing a URI are compared for equality.
    // `DigestMode::Full` requires them to be fully identical.
    // `DigestMode::Structural` ignores annotations and custom defaults.
    DigestMode digestMode = DigestMode::Full;
    MergeResolution resolution = MergeResolution::Error;
  };

  /** Constructs an empty instance with the default merge policy and no base. */
  AccumulatingTypeSystem();

  /**
   * Constructs an instance, optionally layered on a base TypeSystem. Types in
   * the base are treated as already defined: an incoming type sharing a URI
   * with a base type is deduplicated (or conflicts per MergePolicy) and is
   * never added to this instance's working set; new types may reference base
   * types. The base is fixed for the lifetime of the instance.
   */
  explicit AccumulatingTypeSystem(
      MergePolicy policy, std::shared_ptr<const TypeSystem> base = nullptr);
  ~AccumulatingTypeSystem() override;

  AccumulatingTypeSystem(const AccumulatingTypeSystem&) = delete;
  AccumulatingTypeSystem& operator=(const AccumulatingTypeSystem&) = delete;
  AccumulatingTypeSystem(AccumulatingTypeSystem&&) = delete;
  AccumulatingTypeSystem& operator=(AccumulatingTypeSystem&&) = delete;

  /**
   * Folds the definitions of a SerializableTypeSystem into this instance.
   *
   * New (non-duplicate) definitions are appended; existing references stay
   * valid. See the class documentation for deduplication, conflict, and
   * reference-stability semantics.
   *
   * Exception safety: strong. If anything in the batch fails — a conflict
   * (MergeResolution::Error), a structurally invalid definition, an
   * unresolvable type reference, or a duplicate source identifier — this
   * instance is left unchanged: the batch is validated and deduplicated before
   * materialization, and materialization itself rolls back on failure.
   *
   * Throws:
   *   - MergeConflictError on a MergeResolution::Error conflict.
   *   - InvalidTypeError on a structurally invalid definition or unresolvable
   *     type reference.
   */
  void addTypes(const SerializableTypeSystem& typeSystem);

  /**
   * Folds a deep copy of every type in a runtime TypeSystem into this instance.
   *
   * The source's definitions are serialized and re-materialized into this
   * instance's own store, so the resulting nodes are owned here and outlive the
   * source. Deduplication, conflict, and reference-stability semantics match
   * addTypes(const SerializableTypeSystem&).
   *
   * Throws:
   *   - InvalidTypeError if the source does not enumerate its URIs
   *     (getKnownUris() == std::nullopt).
   *   - MergeConflictError on a MergeResolution::Error conflict.
   *   - InvalidTypeError on a structurally invalid definition or unresolvable
   *     type reference.
   */
  void addTypes(const TypeSystem& typeSystem);

  /**
   * Folds a single definition into this instance. Equivalent to addTypes() with
   * a one-element SerializableTypeSystem.
   */
  void addType(Uri uri, const SerializableTypeDefinitionEntry& entry);

  /**
   * Folds a single runtime definition (and its transitive closure) from
   * `source` into this instance. `def` must be a user-defined type defined in
   * `source`; it is serialized and re-materialized into this instance's own
   * store — a deep copy, like addTypes(const TypeSystem&).
   *
   * Throws the same exceptions as addTypes(const TypeSystem&); in particular
   * InvalidTypeError if `def`'s URI is not defined in `source`.
   */
  void addType(const TypeSystem& source, DefinitionRef def);

  // TypeSystem interface implementation.
  std::optional<DefinitionRef> getUserDefinedType(UriView uri) const override;
  std::optional<folly::F14FastSet<Uri>> getKnownUris() const override;
  std::optional<DefinitionRef> getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView sourceIdentifier) const override;
  std::optional<SourceIdentifierView> getSourceIdentiferForUserDefinedType(
      DefinitionRef def) const override;
  NameToDefinitionsMap getUserDefinedTypesAtLocation(
      std::string_view location) const override;

  /** Number of types added to this instance (excludes the base). */
  std::size_t size() const;
  /** True if no types have been added to this instance (excludes the base). */
  bool empty() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace apache::thrift::type_system
