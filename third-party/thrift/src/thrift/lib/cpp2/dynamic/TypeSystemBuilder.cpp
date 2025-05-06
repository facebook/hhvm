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

#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <folly/Overload.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>
#include <folly/lang/Assume.h>
#include <folly/lang/SafeAssert.h>

#include <cstddef>
#include <functional>
#include <new>
#include <stdexcept>
#include <utility>
#include <variant>

namespace apache::thrift::dynamic {

namespace {

/**
 * LazyInitPtr<T> is a smart pointer that allocates memory for an object of type
 * `T` without immediately constructing it. This allows lazy in-place
 * initialization of `T`at a later time, while providing a stable pointer to the
 * allocated memory even before construction.
 *
 * This is technically undefined behavior, but allows us to implementation the
 * semantics required for LazyInitDefinition mentioned below.
 *
 * On creation, LazyInitPtr<T> allocates sufficient memory for a `T` object but
 * does not call `T`'s constructor. The actual construction is deferred until
 * there is an explicit call to `emplace(...)`.
 *
 * The key feature of LazyInitPtr<T> is that it allows obtaining a raw pointer
 * to the allocated memory (via `get()`) even before `T` is constructed. This is
 * different from `std::unique_ptr<T>` which does not perform any allocation
 * (and thus returns nullptr) unless an object is constructed.
 *
 * The pointer returned by `get()` must not be dereferenced until the object is
 * constructed (see `emplace(...)` and `isInitialized(...)`). This kind of
 * deferred initialization is useful when setting up circular references.
 *
 * LazyInitPtr<T> models Movable but not Copyable.
 */
template <typename T>
class LazyInitPtr final {
  static_assert(std::is_nothrow_move_constructible_v<T>);
  static_assert(std::is_nothrow_destructible_v<T>);

 public:
  LazyInitPtr() : storage_(allocateUninitializedStorage()) {}
  ~LazyInitPtr() noexcept { destroyObjectAndFreeStorage(); }

  LazyInitPtr(const LazyInitPtr&) = delete;
  LazyInitPtr& operator=(const LazyInitPtr&) = delete;

  LazyInitPtr(LazyInitPtr&& other) noexcept
      : storage_(std::exchange(other.storage_, nullptr)),
        initialized_(std::exchange(other.initialized_, false)) {}

  LazyInitPtr& operator=(LazyInitPtr&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    destroyObjectAndFreeStorage();
    storage_ = std::exchange(other.storage_, nullptr);
    initialized_ = std::exchange(other.initialized_, false);
  }

  bool isInitialized() const noexcept { return initialized_; }
  /**
   * Returns a pointer to the (possibly uninitialized) memory for the contained
   * object.
   *
   * The returned pointer must not be deferenced unless isInitialized() returns
   * true. The lifetime of the object is valid for the lifetime of the owning
   * LazyInitPtr<T>, including if this object is moved (similar to other smart
   * pointers).
   */
  T* get() const noexcept { return std::launder(storage_); }

  /**
   * Constructs an object in-place using the provided arguments.
   *
   * Pre-conditions:
   *   - isInitialized() == false
   *   - This object has not been moved-from
   */
  template <typename... Args>
  T& emplace(Args&&... args) {
    if (initialized_) {
      throw std::logic_error("LazyInitPtr: object is already constructed");
    }
    new (storage_) T(std::forward<Args>(args)...);
    initialized_ = true;
    return *get();
  }

 private:
  static T* allocateUninitializedStorage() {
    return static_cast<T*>(
        ::operator new(sizeof(T), std::align_val_t{alignof(T)}));
  }

  void destroyObjectAndFreeStorage() noexcept {
    if (initialized_) {
      FOLLY_SAFE_DCHECK(storage_ != nullptr);
      storage_->~T();
      initialized_ = false;
    }
    if (storage_ != nullptr) {
      ::operator delete(storage_, sizeof(T), std::align_val_t{alignof(T)});
      storage_ = nullptr;
    }
  }

  T* storage_ = nullptr;
  bool initialized_ = false;
};

/**
 * LazyInitDefinition exists thanks to 2 conflicting requirements:
 *   1. TypeRef access should be performant — when a TypeRef points to a
 *      StructNode, it should store a pointer to the StructNode directly. There
 *      should be no "lazy resolution".
 *   2. Thrift type references can have cycles.
 *
 * Consider the simple case with a self referential struct:
 *
 *     struct Foo {
 *       1: optional Foo obj;
 *     }
 *
 * We expect that the FieldNode corresponding to `obj` will have a TypeRef to
 * `Foo` (and thus a StructNode pointer for `Foo`). However, we cannot construct
 * the StructNode for `Foo` without first creating the FieldNode for `obj`!
 *
 * To break this cycle, the approach we take is:
 *   1. Perform an initial pass where we create uninitialized nodes for each
 *      definition.
 *   2. Create the TypeSystem graph with pointers to uninitialized nodes.
 *   3. Perform a second pass and to initialize the node objects.
 */
class LazyInitDefinition final {
 public:
  using Alternative = std::variant<
      LazyInitPtr<StructNode>,
      LazyInitPtr<UnionNode>,
      LazyInitPtr<EnumNode>,
      LazyInitPtr<OpaqueAliasNode>>;

  explicit LazyInitDefinition(Alternative definition) noexcept
      : definition_(std::move(definition)) {}

  template <typename T>
  LazyInitPtr<T>& asType() {
    return std::get<LazyInitPtr<T>>(definition_);
  }

  bool isInitialized() const {
    return folly::variant_match(
        definition_, [](const auto& lazyInitPtr) -> bool {
          return lazyInitPtr.isInitialized();
        });
  }

  DefinitionRef toDefinitionRef() const {
    return folly::variant_match(
        definition_, [](const auto& lazyInitPtr) -> DefinitionRef {
          return DefinitionRef(lazyInitPtr.get());
        });
  }

  TypeRef toTypeRef() const {
    return TypeRef::fromDefinition(toDefinitionRef());
  }

 private:
  Alternative definition_;
};

class TypeSystemImpl final : public TypeSystem {
 private:
  struct UriHeterogeneousHash {
    using is_transparent = void;

    std::size_t operator()(const Uri& uri) const noexcept {
      return std::hash<UriView>{}(uri);
    }
    std::size_t operator()(const UriView& uri) const noexcept {
      return std::hash<UriView>{}(uri);
    }
  };

 public:
  using DefinitionsMap = folly::F14FastMap<
      Uri,
      LazyInitDefinition,
      UriHeterogeneousHash,
      std::equal_to<>>;
  DefinitionsMap definitions;

  DefinitionRef getUserDefinedType(UriView uri) final {
    if (auto def = definitions.find(uri); def != definitions.end()) {
      FOLLY_SAFE_DCHECK(def->second.isInitialized());
      return def->second.toDefinitionRef();
    }
    throw InvalidTypeError(
        fmt::format("Definition for uri '{}' was not found", uri));
  }

  // NOTE: This function should ONLY be called within TypeSystemBuilder::build.
  // It is defined here instead of as a local lambda because the implementation
  // is recursive and such lambdas as a pain until C++23's "deducing this".
  TypeRef typeOf(const TypeId& typeId) {
    return typeId.visit(
        [&](const Uri& uri) -> TypeRef {
          if (auto def = definitions.find(uri); def != definitions.end()) {
            return def->second.toTypeRef();
          }
          throw InvalidTypeError(
              fmt::format("Definition for uri '{}' was not found", uri));
        },
        [&](const TypeId::List& list) -> TypeRef {
          return TypeRef(TypeRef::List(typeOf(list.elementType())));
        },
        [&](const TypeId::Set& set) -> TypeRef {
          return TypeRef(TypeRef::Set(typeOf(set.elementType())));
        },
        [&](const TypeId::Map& map) -> TypeRef {
          return TypeRef(
              TypeRef::Map(typeOf(map.keyType()), typeOf(map.valueType())));
        },
        [](const auto& primitive) -> TypeRef { return TypeRef(primitive); });
  }
};

} // namespace

std::unique_ptr<TypeSystem> TypeSystemBuilder::build() && {
  auto typeSystem = std::make_unique<TypeSystemImpl>();

  // Fill in definitions with uninitialized stubs
  for (auto& entry : definitions_) {
    const Uri& uri = entry.first;
    SerializableTypeDefinition& def = entry.second;
    auto uninitDef = std::invoke([&]() -> LazyInitDefinition {
      switch (def.getType()) {
        case SerializableTypeDefinition::Type::structDef:
          return LazyInitDefinition(LazyInitPtr<StructNode>());
        case SerializableTypeDefinition::Type::unionDef:
          return LazyInitDefinition(LazyInitPtr<UnionNode>());
        case SerializableTypeDefinition::Type::enumDef:
          return LazyInitDefinition(LazyInitPtr<EnumNode>());
        case SerializableTypeDefinition::Type::opaqueAliasDef:
          return LazyInitDefinition(LazyInitPtr<OpaqueAliasNode>());
        default:
          break;
      }
      folly::assume_unreachable();
    });
    FOLLY_SAFE_DCHECK(!uninitDef.isInitialized());
    typeSystem->definitions.emplace(uri, std::move(uninitDef));
  }

  const auto makeFields = [&](std::vector<SerializableFieldDefinition> fields)
      -> std::vector<FieldNode> {
    std::vector<FieldNode> result;
    result.reserve(fields.size());
    for (auto& field : fields) {
      result.emplace_back(
          std::move(*field.identity()),
          *field.presence(),
          typeSystem->typeOf(*field.type()),
          field.customDefaultValue().has_value()
              ? std::optional{std::move(*field.customDefaultValue())}
              : std::nullopt);
    }
    return result;
  };

  for (auto& entry : definitions_) {
    const Uri& uri = entry.first;
    SerializableTypeDefinition& def = entry.second;
    // We created uninitialized stubs above so we can assume they exist
    LazyInitDefinition& uninitDef = typeSystem->definitions.find(uri)->second;
    FOLLY_SAFE_DCHECK(!uninitDef.isInitialized());

    switch (def.getType()) {
      case SerializableTypeDefinition::Type::structDef: {
        SerializableStructDefinition& structDef = *def.structDef_ref();
        uninitDef.asType<StructNode>().emplace(
            uri,
            makeFields(std::move(*structDef.fields())),
            *structDef.isSealed());
      } break;
      case SerializableTypeDefinition::Type::unionDef: {
        SerializableUnionDefinition& unionDef = *def.unionDef_ref();
        uninitDef.asType<UnionNode>().emplace(
            uri,
            makeFields(std::move(*unionDef.fields())),
            *unionDef.isSealed());
      } break;
      case SerializableTypeDefinition::Type::enumDef: {
        SerializableEnumDefinition& enumDef = *def.enumDef_ref();
        std::vector<EnumNode::Value> values;
        values.reserve(enumDef.values()->size());
        for (SerializableEnumValueDefinition& mapping : *enumDef.values()) {
          values.emplace_back(
              EnumNode::Value(std::move(*mapping.name()), *mapping.datum()));
        }
        uninitDef.asType<EnumNode>().emplace(uri, std::move(values));
      } break;
      case SerializableTypeDefinition::Type::opaqueAliasDef: {
        SerializableOpaqueAliasDefinition& opaqueAliasDef =
            *def.opaqueAliasDef_ref();
        uninitDef.asType<OpaqueAliasNode>().emplace(
            uri, typeSystem->typeOf(*opaqueAliasDef.targetType()));
      } break;
      default:
        break;
    }
  }

  return typeSystem;
}

namespace {

/**
 * For structured types, both field ids AND names must be unique.
 */
void validateIdentitiesAreUnique(
    UriView uri, folly::span<const SerializableFieldDefinition> fields) {
  folly::F14FastSet<FieldId> seenIds;
  folly::F14FastSet<FieldName> seenNames;

  for (const auto& field : fields) {
    if (seenIds.contains(field.identity()->id())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate field id '{}' in structured type '{}'",
          field.identity()->id(),
          uri));
    }
    seenIds.insert(field.identity()->id());

    if (seenNames.contains(field.identity()->name())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate field name '{}' in structured type '{}'",
          field.identity()->name(),
          uri));
    }
    seenNames.insert(field.identity()->name());
  }
}

/**
 * For unions, all fields must be optional.
 */
void validateFieldsAreOptional(
    UriView uri, const SerializableUnionDefinition& unionDef) {
  for (const SerializableFieldDefinition& field : *unionDef.fields()) {
    if (field.presence() != PresenceQualifier::OPTIONAL) {
      throw InvalidTypeError(fmt::format(
          "field '{}' must be optional in union '{}'",
          field.identity()->name(),
          uri));
    }
  }
}

/**
 * For enums, both enum names AND values must be unique.
 */
void validateEnumMappingsAreUnique(
    UriView uri, const SerializableEnumDefinition& enumDef) {
  folly::F14FastSet<std::string_view> seenNames;
  folly::F14FastSet<std::int32_t> seenValues;

  for (const SerializableEnumValueDefinition& entry : *enumDef.values()) {
    if (seenNames.contains(*entry.name())) {
      throw InvalidTypeError(
          fmt::format("Duplicate name '{}' in enum '{}'", *entry.name(), uri));
    }
    seenNames.insert(*entry.name());

    if (seenValues.contains(*entry.datum())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate value '{}' in enum '{}'", *entry.datum(), uri));
    }
    seenValues.insert(*entry.datum());
  }
}

/**
 * Opaque aliases are not allowed to have a target type which is a user-defined
 * type.
 */
void validateOpaqueAliasIsNotUserDefined(
    UriView uri, const SerializableOpaqueAliasDefinition& opaqueAliasDef) {
  if (opaqueAliasDef.targetType()->kind() == TypeId::Kind::URI) {
    throw InvalidTypeError(fmt::format(
        "Opaque alias '{}' cannot have target type of a user-defined type '{}'",
        uri,
        *opaqueAliasDef.targetType()));
  }
}

} // namespace

void TypeSystemBuilder::addType(
    Uri uri, SerializableStructDefinition structDef) {
  validateIdentitiesAreUnique(uri, *structDef.fields());

  SerializableTypeDefinition def;
  def.set_structDef(std::move(structDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(Uri uri, SerializableUnionDefinition unionDef) {
  validateIdentitiesAreUnique(uri, *unionDef.fields());
  validateFieldsAreOptional(uri, unionDef);

  SerializableTypeDefinition def;
  def.set_unionDef(std::move(unionDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(Uri uri, SerializableEnumDefinition enumDef) {
  validateEnumMappingsAreUnique(uri, enumDef);

  SerializableTypeDefinition def;
  def.set_enumDef(std::move(enumDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(
    Uri uri, SerializableOpaqueAliasDefinition opaqueAliasDef) {
  validateOpaqueAliasIsNotUserDefined(uri, opaqueAliasDef);

  SerializableTypeDefinition def;
  def.set_opaqueAliasDef(std::move(opaqueAliasDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addTypes(SerializableTypeSystem typeSystemDef) {
  for (auto& [uri, def] : *typeSystemDef.types()) {
    switch (def.getType()) {
      case SerializableTypeDefinition::Type::structDef:
        addType(uri, std::move(*def.structDef_ref()));
        break;
      case SerializableTypeDefinition::Type::unionDef:
        addType(uri, std::move(*def.unionDef_ref()));
        break;
      case SerializableTypeDefinition::Type::enumDef:
        addType(uri, std::move(*def.enumDef_ref()));
        break;
      case SerializableTypeDefinition::Type::opaqueAliasDef:
        addType(uri, std::move(*def.opaqueAliasDef_ref()));
        break;
      default:
        break;
    }
    folly::assume_unreachable();
  }
}

void TypeSystemBuilder::tryEmplace(Uri uri, SerializableTypeDefinition&& def) {
  auto [_, inserted] = definitions_.emplace(uri, std::move(def));
  if (!inserted) {
    throw InvalidTypeError(
        fmt::format("Duplicate definition for Uri '{}'", uri));
  }
}

} // namespace apache::thrift::dynamic
