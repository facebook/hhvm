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

#include <thrift/lib/cpp2/frozen/schema/MemorySchema.h>

#include <limits>
#include <type_traits>

#include <folly/Utility.h>
#include <folly/container/Enumerate.h>

THRIFT_IMPL_HASH(apache::thrift::frozen::schema::MemoryField)
THRIFT_IMPL_HASH(apache::thrift::frozen::schema::MemoryLayoutBase)
THRIFT_IMPL_HASH(apache::thrift::frozen::schema::MemoryLayout)
THRIFT_IMPL_HASH(apache::thrift::frozen::schema::MemorySchema)

namespace apache {
namespace thrift {
namespace frozen {
namespace schema {

int16_t MemorySchema::Helper::add(MemoryLayout&& layout) {
  // Add distinct layout, bounds check layoutId
  size_t layoutId = layoutTable_.add(std::move(layout));
  CHECK_LE(layoutId, folly::to_unsigned(std::numeric_limits<int16_t>::max()))
      << "Layout overflow";
  return static_cast<int16_t>(layoutId);
}

void MemorySchema::initFromSchema(Schema&& schema) {
  if (!schema.layouts()->empty()) {
    layouts.resize(schema.layouts()->size());

    for (const auto& layoutKvp : *schema.layouts()) {
      const auto id = layoutKvp.first;
      const auto& layout = layoutKvp.second;

      // Note: This will throw if there are any id >=
      // schema.layouts.size().
      auto& memLayout = layouts.at(id);

      memLayout.setSize(*layout.size());
      memLayout.setBits(*layout.bits());

      std::vector<MemoryField> fields;
      fields.reserve(layout.fields()->size());
      for (const auto& fieldKvp : *layout.fields()) {
        MemoryField& memField = fields.emplace_back();
        const auto& fieldId = fieldKvp.first;
        const auto& field = fieldKvp.second;

        memField.setId(fieldId);
        memField.setLayoutId(*field.layoutId());
        memField.setOffset(*field.offset());
      }
      memLayout.setFields(std::move(fields));
    }
  }
  setRootLayoutId(*schema.rootLayout());
}

void convert(Schema&& schema, MemorySchema& memSchema) {
  memSchema.initFromSchema(std::move(schema));
}

void convert(const MemorySchema& memSchema, Schema& schema) {
  using LayoutsType = std::decay_t<decltype(*schema.layouts())>;
  LayoutsType::container_type layouts;
  for (const auto&& memLayout : folly::enumerate(memSchema.getLayouts())) {
    Layout newLayout;
    newLayout.size() = memLayout->getSize();
    newLayout.bits() = memLayout->getBits();

    using FieldsType = std::decay_t<decltype(*newLayout.fields())>;
    FieldsType::container_type fields;
    for (const auto& field : memLayout->getFields()) {
      Field newField;
      newField.layoutId() = field.getLayoutId();
      newField.offset() = field.getOffset();
      fields.emplace_back(field.getId(), std::move(newField));
    }
    newLayout.fields() = FieldsType{std::move(fields)};

    layouts.emplace_back(memLayout.index, std::move(newLayout));
  }
  schema.layouts() = LayoutsType{std::move(layouts)};

  //
  // Type information is discarded when transforming from memSchema to
  // schema, so force this bit to true.
  //
  *schema.relaxTypeChecks() = true;
  *schema.rootLayout() = memSchema.getRootLayoutId();
}

} // namespace schema
} // namespace frozen
} // namespace thrift
} // namespace apache
