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

#include <cstdint>
#include <vector>

#include <glog/logging.h>

#include <folly/hash/Hash.h>
#include <thrift/lib/cpp/DistinctTable.h>
#include <thrift/lib/thrift/gen-cpp2/frozen_types.h>

#define THRIFT_DECLARE_HASH(T)           \
  namespace std {                        \
  template <>                            \
  struct hash<T> {                       \
    size_t operator()(const T& x) const; \
  };                                     \
  }
#define THRIFT_IMPL_HASH(T)                      \
  namespace std {                                \
  size_t hash<T>::operator()(const T& x) const { \
    return x.hash();                             \
  }                                              \
  }

namespace apache {
namespace thrift {
namespace frozen {
namespace schema {

class MemoryField;
class MemoryLayoutBase;
class MemoryLayout;
class MemorySchema;

} // namespace schema
} // namespace frozen
} // namespace thrift
} // namespace apache

THRIFT_DECLARE_HASH(apache::thrift::frozen::schema::MemoryField)
THRIFT_DECLARE_HASH(apache::thrift::frozen::schema::MemoryLayoutBase)
THRIFT_DECLARE_HASH(apache::thrift::frozen::schema::MemoryLayout)
THRIFT_DECLARE_HASH(apache::thrift::frozen::schema::MemorySchema)

namespace apache {
namespace thrift {
namespace frozen {
namespace schema {

// Trivially copyable, hashed bytewise.
class MemoryField {
 public:
  MemoryField() = default;

  inline size_t hash() const {
    return folly::hash::hash_combine(id, layoutId, offset);
  }

  inline bool operator==(const MemoryField& other) const {
    return id == other.id && layoutId == other.layoutId &&
        offset == other.offset;
  }

  inline void setId(int16_t i) { id = i; }

  inline int16_t getId() const { return id; }

  inline void setLayoutId(int16_t lid) { layoutId = lid; }

  inline int16_t getLayoutId() const { return layoutId; }

  inline void setOffset(int16_t o) { offset = o; }

  inline int16_t getOffset() const { return offset; }

 private:
  // Thrift field index
  int16_t id;

  // Index into MemorySchema::layouts
  int16_t layoutId;

  // field offset:
  //  < 0: -(bit offset)
  //  >= 0: byte offset
  int16_t offset;
};

static_assert(
    sizeof(MemoryField) == 3 * sizeof(int16_t), "Memory Field is not padded.");

class MemoryLayoutBase {
 public:
  MemoryLayoutBase() = default;
  virtual ~MemoryLayoutBase() = default;

  inline size_t hash() const { return folly::hash::hash_combine(bits, size); }

  inline bool operator==(const MemoryLayoutBase& other) const {
    return bits == other.bits && size == other.size;
  }

  inline void setSize(int32_t s) { size = s; }

  inline int32_t getSize() const { return size; }

  inline void setBits(int16_t b) { bits = b; }

  inline int16_t getBits() const { return bits; }

 private:
  int32_t size;
  int16_t bits;
};

class MemoryLayout : public MemoryLayoutBase {
 public:
  using MemoryLayoutBase::MemoryLayoutBase;

  inline size_t hash() const {
    return folly::hash::hash_combine(
        MemoryLayoutBase::hash(),
        folly::hash::hash_range(fields.begin(), fields.end()));
  }

  inline bool operator==(const MemoryLayout& other) const {
    return MemoryLayoutBase::operator==(other) && fields == other.fields;
  }

  inline void addField(MemoryField&& field) {
    fields.push_back(std::move(field));
  }

  inline void setFields(std::vector<MemoryField>&& fs) {
    fields = std::move(fs);
  }

  inline const std::vector<MemoryField>& getFields() const { return fields; }

 private:
  std::vector<MemoryField> fields;
};

class Schema;
class MemorySchema {
 public:
  MemorySchema() = default;
  ~MemorySchema() = default;

  inline size_t hash() const {
    return folly::hash::hash_combine(
        folly::hash::hash_range(layouts.begin(), layouts.end()), rootLayout);
  }

  inline bool operator==(const MemorySchema& other) const {
    return layouts == other.layouts;
  }

  inline void setRootLayoutId(int16_t rootId) {
    DCHECK(rootId < static_cast<int16_t>(layouts.size()));
    rootLayout = rootId;
  }

  inline int16_t getRootLayoutId() const { return rootLayout; }

  inline const MemoryLayout& getRootLayout() const {
    return layouts.at(rootLayout);
  }

  inline const MemoryLayout& getLayoutForField(const MemoryField& field) const {
    return layouts.at(field.getLayoutId());
  }

  inline const std::vector<MemoryLayout>& getLayouts() const { return layouts; }

  class Helper {
    // Add helper structures here to help minimize size of schema during
    // save() operations.
   public:
    explicit Helper(MemorySchema& schema) : layoutTable_(&schema.layouts) {}

    int16_t add(MemoryLayout&& layout);

   private:
    DistinctTable<MemoryLayout> layoutTable_;
  };

  void initFromSchema(Schema&& schema);

 private:
  std::vector<MemoryLayout> layouts;
  int16_t rootLayout;
};

struct SchemaInfo {
  using Field = MemoryField;
  using Layout = MemoryLayout;
  using Schema = MemorySchema;
  using Helper = MemorySchema::Helper;
};

void convert(Schema&& schema, MemorySchema& memSchema);
void convert(const MemorySchema& memSchema, Schema& schema);

} // namespace schema
} // namespace frozen
} // namespace thrift
} // namespace apache
