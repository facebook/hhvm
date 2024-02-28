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

/**
 * Macros to be used by codegen'd X_types.h and X_types.cpp files, intended to
 * reduce the coupling between code generation and library code
 */
#define FROZEN_FIELD(NAME, ID, /*TYPE*/...) Field<__VA_ARGS__> NAME##Field;
#define FROZEN_FIELD_OPT(NAME, ID, /*TYPE*/...) \
  Field<folly::Optional<__VA_ARGS__>> NAME##Field;
#define FROZEN_FIELD_REQ FROZEN_FIELD
#define FROZEN_FIELD_REF FROZEN_FIELD

#define FROZEN_VIEW_FIELD(NAME, /*TYPE*/...)                   \
  auto NAME() const {                                          \
    return this->layout_->NAME##Field.layout.view(             \
        this->position_(this->layout_->NAME##Field.pos));      \
  }                                                            \
  auto NAME##_ref() const {                                    \
    return FieldView<typename Layout<__VA_ARGS__>::View>(      \
        this->layout_->NAME##Field.layout.view(                \
            this->position_(this->layout_->NAME##Field.pos))); \
  }                                                            \
  auto get_##NAME() const {                                    \
    return this->layout_->NAME##Field.layout.view(             \
        this->position_(this->layout_->NAME##Field.pos));      \
  }
#define FROZEN_VIEW_FIELD_OPT(NAME, /*TYPE*/...)          \
  auto NAME() const {                                     \
    return this->layout_->NAME##Field.layout.view(        \
        this->position_(this->layout_->NAME##Field.pos)); \
  }
#define FROZEN_VIEW_FIELD_REQ FROZEN_VIEW_FIELD
#define FROZEN_VIEW_FIELD_REF FROZEN_VIEW_FIELD

#define FROZEN_VIEW(...)                                     \
  struct View : public ViewBase<View, LayoutSelf, T> {       \
    View() {}                                                \
    View(const LayoutSelf* layout, ViewPosition position)    \
        : ViewBase<View, LayoutSelf, T>(layout, position) {} \
    bool operator==(const View& other) const;                \
    bool operator<(const View& other) const;                 \
    __VA_ARGS__                                              \
  };                                                         \
  View view(ViewPosition self) const { return View(this, self); }

#define FROZEN_TYPE(TYPE, ...)                                               \
  template <>                                                                \
  struct Layout<TYPE, void> : public LayoutBase {                            \
    typedef LayoutBase Base;                                                 \
    typedef Layout LayoutSelf;                                               \
    Layout();                                                                \
    typedef TYPE T;                                                          \
    FieldPosition maximize();                                                \
    FieldPosition layout(LayoutRoot& root, const T& x, LayoutPosition self); \
    void freeze(FreezeRoot& root, const T& x, FreezePosition self) const;    \
    void thaw(ViewPosition self, T& out) const;                              \
    void print(std::ostream& os, int level) const final;                     \
    void clear() final;                                                      \
    struct View;                                                             \
    static size_t hash(const TYPE&);                                         \
    static size_t hash(const View&);                                         \
    __VA_ARGS__                                                              \
  }

#define FROZEN_CTOR_FIELD(NAME, ID) , NAME##Field(ID, #NAME)
#define FROZEN_CTOR_FIELD_OPT(NAME, ID) , NAME##Field(ID, #NAME)
#define FROZEN_CTOR_FIELD_REQ FROZEN_CTOR_FIELD
#define FROZEN_CTOR_FIELD_REF FROZEN_CTOR_FIELD
#define FROZEN_CTOR(TYPE, ...) \
  Layout<TYPE>::Layout() : LayoutBase(typeid(TYPE)) __VA_ARGS__ {}

#define FROZEN_MAXIMIZE_FIELD(NAME) pos = maximizeField(pos, this->NAME##Field);

#define FROZEN_MAXIMIZE(TYPE, ...)            \
  FieldPosition Layout<TYPE>::maximize() {    \
    FieldPosition pos = startFieldPosition(); \
    __VA_ARGS__;                              \
    return pos;                               \
  }

#define FROZEN_LAYOUT_FIELD(NAME) \
  pos = root.layoutField(self, pos, this->NAME##Field, *x.NAME##_ref());
#define FROZEN_LAYOUT_FIELD_OPT(NAME) \
  pos = root.layoutOptionalField(self, pos, this->NAME##Field, x.NAME##_ref());
#define FROZEN_LAYOUT_FIELD_REQ(NAME) \
  pos = root.layoutField(self, pos, this->NAME##Field, *x.NAME##_ref());
#define FROZEN_LAYOUT_FIELD_REF(NAME) \
  pos = root.layoutField(self, pos, this->NAME##Field, x.NAME##_ref());
#define FROZEN_LAYOUT(TYPE, ...)              \
  FieldPosition Layout<TYPE>::layout(         \
      [[maybe_unused]] LayoutRoot& root,      \
      [[maybe_unused]] const T& x,            \
      [[maybe_unused]] LayoutPosition self) { \
    FieldPosition pos = startFieldPosition(); \
    __VA_ARGS__;                              \
    return pos;                               \
  }

#define FROZEN_FREEZE_FIELD(NAME) \
  root.freezeField(self, this->NAME##Field, *x.NAME##_ref());
#define FROZEN_FREEZE_FIELD_OPT(NAME) \
  root.freezeOptionalField(self, this->NAME##Field, x.NAME##_ref());
#define FROZEN_FREEZE_FIELD_REQ(NAME) \
  root.freezeField(self, this->NAME##Field, *x.NAME##_ref());
#define FROZEN_FREEZE_FIELD_REF(NAME) \
  root.freezeField(self, this->NAME##Field, x.NAME##_ref());

#define FROZEN_FREEZE(TYPE, ...)                    \
  void Layout<TYPE>::freeze(                        \
      [[maybe_unused]] FreezeRoot& root,            \
      [[maybe_unused]] const T& x,                  \
      [[maybe_unused]] FreezePosition self) const { \
    __VA_ARGS__;                                    \
  }

#define FROZEN_THAW_FIELD(NAME)                                  \
  thawField(self, this->NAME##Field, out.NAME##_ref().ensure()); \
  if (this->NAME##Field.layout.empty()) {                        \
    ::apache::thrift::unset_unsafe_deprecated(out.NAME##_ref()); \
  }
#define FROZEN_THAW_FIELD_OPT(NAME) \
  thawField(self, this->NAME##Field, out.NAME##_ref());
#define FROZEN_THAW_FIELD_REQ(NAME) \
  thawField(self, this->NAME##Field, *out.NAME##_ref());
#define FROZEN_THAW_FIELD_REF(NAME) \
  thawField(self, this->NAME##Field, out.NAME##_ref());

#define FROZEN_THAW(TYPE, ...)                                             \
  void Layout<TYPE>::thaw(                                                 \
      [[maybe_unused]] ViewPosition self, [[maybe_unused]] T& out) const { \
    __VA_ARGS__;                                                           \
  }
#define FROZEN_DEBUG_FIELD(NAME) this->NAME##Field.print(os, level + 1);
#define FROZEN_DEBUG(TYPE, ...)                                 \
  void Layout<TYPE>::print(std::ostream& os, int level) const { \
    LayoutBase::print(os, level);                               \
    os << #TYPE;                                                \
    __VA_ARGS__                                                 \
  }
#define FROZEN_CLEAR_FIELD(NAME) this->NAME##Field.clear();
#define FROZEN_CLEAR(TYPE, ...) \
  void Layout<TYPE>::clear() {  \
    LayoutBase::clear();        \
    __VA_ARGS__                 \
  }

#define FROZEN_SAVE_FIELD(NAME) \
  this->NAME##Field.template save<SchemaInfo>(schema, _layout, helper);

#define FROZEN_SAVE_BODY(...)                               \
  Base::template save<SchemaInfo>(schema, _layout, helper); \
  __VA_ARGS__

#define FROZEN_SAVE_INLINE(...)                    \
  template <typename SchemaInfo>                   \
  inline void save(                                \
      typename SchemaInfo::Schema& schema,         \
      typename SchemaInfo::Layout& _layout,        \
      typename SchemaInfo::Helper& helper) const { \
    FROZEN_SAVE_BODY(__VA_ARGS__)                  \
  }

#define FROZEN_LOAD_FIELD(NAME, ID)                                   \
  case ID:                                                            \
    this->NAME##Field.template load<SchemaInfo>(schema, field, root); \
    break;

#define FROZEN_LOAD_BODY(...)                             \
  Base::template load<SchemaInfo>(schema, _layout, root); \
  for (const auto& field : _layout.getFields()) {         \
    switch (field.getId()) { __VA_ARGS__ }                \
  }

#define FROZEN_LOAD_INLINE(...)                   \
  template <typename SchemaInfo>                  \
  inline void load(                               \
      const typename SchemaInfo::Schema& schema,  \
      const typename SchemaInfo::Layout& _layout, \
      LoadRoot& root) {                           \
    FROZEN_LOAD_BODY(__VA_ARGS__)                 \
  }

#define FROZEN_EXCLUDE_TYPE(...) \
  template <>                    \
  struct IsExcluded<__VA_ARGS__> : std::true_type {};
