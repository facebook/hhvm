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

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/whisker/mstch_compat.h>

namespace apache::thrift::compiler {

class mstch_base;
struct mstch_context;

constexpr auto kOnInvalidUtf8 = "onInvalidUtf8";

struct mstch_element_position {
  mstch_element_position() = default;
  mstch_element_position(size_t index, size_t size)
      : index(index), first(index == 0), last(index == size - 1) {}

  int32_t index = 0;
  bool first = false;
  bool last = false;
};

struct field_generator_context {
  const t_structured* strct = nullptr;
  const t_field* serialization_prev = nullptr;
  const t_field* serialization_next = nullptr;
  int isset_index = -1;
};

enum CodingErrorAction {
  Legacy = 0,
  Report = 1,
};

// A factory creating mstch objects wrapping Thrift AST nodes.
// Node: A Thrift AST node type to be wrapped.
// Args: Additional arguments passed to an mstch object constructor.
template <typename Node, typename... Args>
class mstch_factory {
 public:
  using node_type = Node; // Thrift AST node type.

  virtual ~mstch_factory() = default;

  // Creates an mstch object that wraps a Thrift AST node and provides access
  // to its properties. The `ctx` object is owned by `t_mstch_generator` and
  // persists throughout the generation phase.
  virtual std::shared_ptr<mstch_base> make_mstch_object(
      const Node* node,
      mstch_context& ctx,
      mstch_element_position pos = {},
      Args...) const = 0;

  std::shared_ptr<mstch_base> make_mstch_object(
      const Node& node,
      mstch_context& ctx,
      mstch_element_position pos = {},
      Args... args) const {
    return make_mstch_object(&node, ctx, pos, args...);
  }
};

using mstch_program_factory = mstch_factory<t_program>;
using mstch_service_factory = mstch_factory<t_service>;
using mstch_interaction_factory =
    mstch_factory<t_interaction, const t_service*>;
using mstch_function_factory = mstch_factory<t_function, const t_interface*>;
using mstch_type_factory = mstch_factory<t_type>;
using mstch_typedef_factory = mstch_factory<t_typedef>;
using mstch_struct_factory = mstch_factory<t_structured>;
using mstch_field_factory =
    mstch_factory<t_field, const field_generator_context*>;
using mstch_enum_factory = mstch_factory<t_enum>;
using mstch_enum_value_factory = mstch_factory<t_enum_value>;
using mstch_const_factory =
    mstch_factory<t_const, const t_const*, const t_type*, const t_field*>;
using mstch_const_value_factory =
    mstch_factory<t_const_value, const t_const*, const t_type*>;
using mstch_const_map_element_factory = mstch_factory<
    std::pair<t_const_value*, t_const_value*>,
    const t_const*,
    const std::pair<const t_type*, const t_type*>&>;
using mstch_structured_annotation_factory = mstch_factory<t_const>;
using mstch_deprecated_annotation_factory = mstch_factory<t_annotation>;
using mstch_stream_factory = mstch_factory<t_stream>;

namespace detail {
// Structured annotations don't have a separate AST node type yet so use a
// tag type to distinguish it from t_const.
struct structured_annotation_tag {};
} // namespace detail

class mstch_factories {
 public:
  std::unique_ptr<mstch_program_factory> program_factory;
  std::unique_ptr<mstch_service_factory> service_factory;
  std::unique_ptr<mstch_interaction_factory> interaction_factory;
  std::unique_ptr<mstch_function_factory> function_factory;
  std::unique_ptr<mstch_type_factory> type_factory;
  std::unique_ptr<mstch_typedef_factory> typedef_factory;
  std::unique_ptr<mstch_struct_factory> struct_factory;
  std::unique_ptr<mstch_field_factory> field_factory;
  std::unique_ptr<mstch_enum_factory> enum_factory;
  std::unique_ptr<mstch_enum_value_factory> enum_value_factory;
  std::unique_ptr<mstch_const_factory> const_factory;
  std::unique_ptr<mstch_const_value_factory> const_value_factory;
  std::unique_ptr<mstch_const_map_element_factory> const_map_element_factory;
  std::unique_ptr<mstch_structured_annotation_factory>
      structured_annotation_factory;
  std::unique_ptr<mstch_deprecated_annotation_factory>
      deprecated_annotation_factory;

  mstch_factories();

  struct no_data {};

  // Adds a factory for the mstch object type MstchType.
  // data: Optional data to be passed by value as the last argument to an
  //       mstch object constructor.
  //
  // Example:
  //   factories.set<my_mstch_program>();
  //
  // where my_mstch_program is a subclass of mstch_program.
  template <typename MstchType, typename Data = no_data>
  void add(Data data = {}) {
    typename MstchType::ast_type* tag = nullptr;
    make<MstchType>(get(tag), data);
  }

 protected:
  ~mstch_factories() = default;

 private:
  template <typename Data, typename MstchType, typename Node, typename... Args>
  class mstch_factory_impl : public mstch_factory<Node, Args...> {
   private:
    Data data_;

   public:
    explicit mstch_factory_impl(Data data) : data_(data) {}

    std::shared_ptr<mstch_base> make_mstch_object(
        const Node* node,
        mstch_context& ctx,
        mstch_element_position pos,
        Args... args) const override {
      return std::make_shared<MstchType>(node, ctx, pos, args..., data_);
    }
  };

  template <typename MstchType, typename Node, typename... Args>
  class mstch_factory_impl<no_data, MstchType, Node, Args...>
      : public mstch_factory<Node, Args...> {
   public:
    explicit mstch_factory_impl(no_data) {}

    std::shared_ptr<mstch_base> make_mstch_object(
        const Node* node,
        mstch_context& ctx,
        mstch_element_position pos,
        Args... args) const override {
      return std::make_shared<MstchType>(node, ctx, pos, args...);
    }
  };

  // Returns a factory using a pointer to AST type for tag dispatch.
  auto& get(t_program*) { return program_factory; }
  auto& get(t_service*) { return service_factory; }
  auto& get(t_interaction*) { return interaction_factory; }
  auto& get(t_function*) { return function_factory; }
  auto& get(t_type*) { return type_factory; }
  auto& get(t_typedef*) { return typedef_factory; }
  auto& get(t_structured*) { return struct_factory; }
  auto& get(t_field*) { return field_factory; }
  auto& get(t_enum*) { return enum_factory; }
  auto& get(t_enum_value*) { return enum_value_factory; }
  auto& get(t_const*) { return const_factory; }
  auto& get(t_const_value*) { return const_value_factory; }
  auto& get(std::pair<t_const_value*, t_const_value*>*) {
    return const_map_element_factory;
  }
  auto& get(detail::structured_annotation_tag*) {
    return structured_annotation_factory;
  }
  auto& get(t_annotation*) { return deprecated_annotation_factory; }

  template <typename MstchType, typename Node, typename... Args, typename Data>
  void make(std::unique_ptr<mstch_factory<Node, Args...>>& factory, Data data) {
    using factory_impl = mstch_factory_impl<Data, MstchType, Node, Args...>;
    factory = std::make_unique<factory_impl>(data);
  }
};

// Mstch object construction context.
struct mstch_context : mstch_factories {
  using compiler_options_map = std::map<std::string, std::string, std::less<>>;
  compiler_options_map options;

  std::unordered_map<std::string, std::shared_ptr<mstch_base>> enum_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> struct_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> service_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> program_cache;

  node_metadata_cache metadata_cache;

  const whisker::prototype_database* prototypes = nullptr;

  /**
   * Sets or erases the option with the given `key` depending on the
   * `condition`.
   *
   * If `condition` is true, `options[key]` will be set to the given `value`.
   * Otherwise, the entry for `key` in `options` (if any) is removed.
   *
   * @return `this` (for chaining).
   */
  mstch_context& set_or_erase_option(
      bool condition, const std::string& key, const std::string& value);

  node_metadata_cache& cache() { return metadata_cache; }
};

std::shared_ptr<mstch_base> make_mstch_program_cached(
    const t_program* program,
    mstch_context& ctx,
    mstch_element_position pos = {});

std::shared_ptr<mstch_base> make_mstch_service_cached(
    const t_program* program,
    const t_service* service,
    mstch_context& ctx,
    mstch_element_position pos = {});

// A base class for mstch object types that wrap Thrift AST nodes. It is called
// mstch_base rather than mstch_node to avoid confusion with mstch::node.
class mstch_base : public mstch::object {
 protected:
  // A range of t_field* to avoid copying between std::vector<t_field*>
  // and std::vector<const t_field*>.
  class field_range {
   public:
    /* implicit */ field_range(const std::vector<t_field*>& fields) noexcept
        : begin_(const_cast<const t_field* const*>(fields.data())),
          end_(const_cast<const t_field* const*>(
              fields.data() + fields.size())) {}
    /* implicit */ field_range(
        const std::vector<const t_field*>& fields) noexcept
        : begin_(fields.data()), end_(fields.data() + fields.size()) {}
    constexpr size_t size() const noexcept { return end_ - begin_; }
    constexpr const t_field* const* begin() const noexcept { return begin_; }
    constexpr const t_field* const* end() const noexcept { return end_; }

   private:
    const t_field* const* begin_;
    const t_field* const* end_;
  };

 public:
  mstch_base(mstch_context& ctx, mstch_element_position pos)
      : context_(ctx), pos_(pos) {
    register_methods(
        this,
        {
            {"first?", &mstch_base::first},
            {"last?", &mstch_base::last},
            {"is_struct?", &mstch_base::is_struct},
        });
  }
  virtual ~mstch_base() = default;

  mstch::node first() { return pos_.first; }
  mstch::node last() { return pos_.last; }
  mstch::node is_struct();

  mstch::node annotations(const t_named* annotated) {
    return make_mstch_annotations(annotated->unstructured_annotations());
  }

  mstch::node structured_annotations(const t_named* annotated) {
    return make_mstch_array(
        annotated->structured_annotations(),
        *context_.structured_annotation_factory);
  }

  template <typename T>
  whisker::object make_self(const T& node) {
    return whisker::object(whisker::native_handle<T>(
        whisker::manage_as_static(node), context_.prototypes->of<T>()));
  }

  template <typename Container, typename Factory, typename... Args>
  mstch::array make_mstch_array(
      const Container& container, const Factory& factory, const Args&... args) {
    mstch::array a;
    size_t i = 0;
    for (auto&& element : container) {
      auto pos = mstch_element_position(i, container.size());
      a.push_back(factory.make_mstch_object(element, context_, pos, args...));
      ++i;
    }
    return a;
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_services(const C& container, const Args&... args) {
    return make_mstch_array(container, context_.service_factory.get(), args...);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_interactions(
      const C& container,
      const t_service* containing_service,
      const Args&... args) {
    if (context_.interaction_factory) {
      return make_mstch_array(
          container,
          *context_.interaction_factory,
          args...,
          containing_service);
    }
    return make_mstch_array(container, *context_.service_factory);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_annotations(const C& container, const Args&... args) {
    return make_mstch_array(
        container, *context_.deprecated_annotation_factory, args...);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_enum_values(const C& container, const Args&... args) {
    return make_mstch_array(container, *context_.enum_value_factory, args...);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_consts(const C& container, const Args&... args) {
    return make_mstch_array(container, *context_.const_value_factory, args...);
  }

  virtual mstch::array make_mstch_fields(const field_range& fields) {
    return make_mstch_array(fields, *context_.field_factory, nullptr);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_functions(const C& container, const Args&... args) {
    return make_mstch_array(container, *context_.function_factory, args...);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_typedefs(const C& container, const Args&... args) {
    return make_mstch_array(container, *context_.typedef_factory, args...);
  }

  template <typename C, typename... Args>
  mstch::array make_mstch_types(const C& container, const Args&... args) {
    return make_mstch_array(container, *context_.type_factory, args...);
  }

  template <typename Item, typename Factory, typename Cache>
  mstch::node make_mstch_element_cached(
      const Item& item,
      const Factory& factory,
      Cache& cache,
      const std::string& id,
      size_t element_index,
      size_t element_count) {
    std::string elem_id = id + item->name();
    auto itr = cache.find(elem_id);
    if (itr == cache.end()) {
      auto pos = mstch_element_position(element_index, element_count);
      itr = cache.emplace_hint(
          itr, elem_id, factory.make_mstch_object(item, context_, pos));
    }
    return itr->second;
  }

  template <typename Container, typename Factory, typename Cache>
  mstch::array make_mstch_array_cached(
      const Container& container,
      const Factory& factory,
      Cache& cache,
      const std::string& id) {
    mstch::array a;
    for (size_t i = 0; i < container.size(); ++i) {
      a.push_back(make_mstch_element_cached(
          container[i], factory, cache, id, i, container.size()));
    }
    return a;
  }

  bool has_option(const std::string& option) const;
  std::string get_option(const std::string& option) const;

  // Registers has_option(option) under the given name.
  void register_has_option(std::string key, std::string option);

 protected:
  mstch_context& context_;
  const mstch_element_position pos_;
};

class mstch_program : public mstch_base {
 public:
  using ast_type = t_program;

  mstch_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), program_(p) {
    register_methods(
        this,
        {
            {"program:self", &mstch_program::self},
            {"program:structs", &mstch_program::structs},
            {"program:enums", &mstch_program::enums},
            {"program:services", &mstch_program::services},
            {"program:interactions", &mstch_program::interactions},
            {"program:typedefs", &mstch_program::typedefs},
            {"program:constants", &mstch_program::constants},
        });
  }

  virtual std::string get_program_namespace(const t_program*) { return {}; }

  whisker::object self() { return make_self(*program_); }

  mstch::node structs();
  mstch::node enums();
  mstch::node services();
  mstch::node interactions();
  mstch::node typedefs();
  mstch::node constants();

 protected:
  const t_program* program_;
};

class mstch_service : public mstch_base {
 public:
  using ast_type = t_service;

  mstch_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service = nullptr)
      : mstch_base(ctx, pos),
        service_(s),
        containing_service_(containing_service) {
    assert(containing_service_ == nullptr || service_->is_interaction());

    register_methods(
        this,
        {
            {"service:self", &mstch_service::self},
            {"service:functions", &mstch_service::functions},
            {"service:functions?", &mstch_service::has_functions},
            {"service:extends", &mstch_service::extends},
            {"service:extends?", &mstch_service::has_extends},
            {"service:streams?", &mstch_service::has_streams},
            {"service:sinks?", &mstch_service::has_sinks},
            {"service:annotations", &mstch_service::annotations},
            {"service:parent_service_name",
             &mstch_service::parent_service_name},
            {"service:interactions", &mstch_service::interactions},
            {"service:interactions?", &mstch_service::has_interactions},
            {"service:structured_annotations",
             &mstch_service::structured_annotations},
        });

    if (service_->is_interaction()) {
      register_methods(
          this,
          {
              {"interaction:serial?", &mstch_service::is_serial_interaction},
              {"interaction:eb?", &mstch_service::is_event_base_interaction},
          });
    }

    // Collect performed interactions and cache them.
    std::set<const t_interaction*> seen;
    for (const auto* function : get_functions()) {
      if (const auto& interaction = function->interaction()) {
        auto* ptr = dynamic_cast<const t_interaction*>(interaction.get_type());
        if (!seen.insert(ptr).second) {
          continue; // Already seen this interaction.
        }
        interactions_.push_back(ptr);
      }
    }
  }

  virtual std::string get_service_namespace(const t_program*) { return {}; }

  whisker::object self() { return make_self(*service_); }
  mstch::node has_functions() { return !get_functions().empty(); }
  mstch::node has_extends() { return service_->extends() != nullptr; }
  mstch::node functions();
  mstch::node extends();
  mstch::node annotations() { return mstch_base::annotations(service_); }

  mstch::node parent_service_name() { return parent_service()->name(); }

  mstch::node has_streams() {
    auto& funcs = get_functions();
    return std::any_of(funcs.cbegin(), funcs.cend(), [](const auto& func) {
      return func->stream() != nullptr;
    });
  }

  mstch::node has_sinks() {
    auto& funcs = get_functions();
    return std::any_of(funcs.cbegin(), funcs.cend(), [](const auto& func) {
      return func->sink() != nullptr;
    });
  }

  mstch::node has_interactions() { return !interactions_.empty(); }
  mstch::node interactions() {
    return make_mstch_interactions(interactions_, service_);
  }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(service_);
  }
  mstch::node is_interaction() { return service_->is_interaction(); }
  mstch::node is_serial_interaction() {
    return service_->is_serial_interaction();
  }
  mstch::node is_event_base_interaction() {
    return service_->has_unstructured_annotation("process_in_event_base") ||
        service_->has_structured_annotation(kCppProcessInEbThreadUri);
  }

  ~mstch_service() override = default;

 protected:
  const t_service* service_;
  std::vector<const t_interaction*> interactions_;

  // If `service_` is really an interaction, `containing_service_` is the
  // service it belongs to.
  const t_service* containing_service_ = nullptr;

  mstch::node make_mstch_extended_service_cached(const t_service* service);
  virtual const std::vector<t_function*>& get_functions() const {
    return service_->get_functions();
  }
  const t_service* parent_service() const {
    return service_->is_interaction() ? containing_service_ : service_;
  }
};

class mstch_function : public mstch_base {
 public:
  using ast_type = t_function;

  mstch_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_interface* iface)
      : mstch_base(ctx, pos), function_(f), interface_(iface) {
    register_methods(
        this,
        {
            {"function:self", &mstch_function::self},
            {"function:return_type", &mstch_function::return_type},
            {"function:exceptions", &mstch_function::exceptions},
            {"function:args", &mstch_function::arg_list},
            {"function:args?", &mstch_function::has_args},
            {"function:comma", &mstch_function::arg_comma},
            {"function:annotations", &mstch_function::annotations},
            {"function:structured_annotations",
             &mstch_function::structured_annotations},
            {"function:in_or_creates_interaction?",
             &mstch_function::in_or_creates_interaction},

            // Sink methods:
            {"function:sink_first_response_type",
             &mstch_function::sink_first_response_type},
            {"function:sink_elem_type", &mstch_function::sink_elem_type},
            {"function:sink_exceptions", &mstch_function::sink_exceptions},
            {"function:sink_final_response_type",
             &mstch_function::sink_final_reponse_type},
            {"function:sink_final_response_exceptions",
             &mstch_function::sink_final_response_exceptions},

            // Stream methods:
            {"function:stream_first_response_type",
             &mstch_function::stream_first_response_type},
            {"function:stream_elem_type", &mstch_function::stream_elem_type},
            {"function:stream_exceptions", &mstch_function::stream_exceptions},
        });
  }

  whisker::object self() { return make_self(*function_); }
  mstch::node has_args() { return has_args_(); }
  mstch::node arg_comma() {
    return has_args_() ? std::string(", ") : std::string();
  }
  mstch::node annotations() { return mstch_base::annotations(function_); }

  mstch::node return_type();
  mstch::node exceptions();

  mstch::node arg_list();
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(function_);
  }

  mstch::node in_or_creates_interaction() {
    return function_->interaction() || is_interaction_member();
  }
  mstch::node sink_first_response_type();
  mstch::node sink_elem_type();
  mstch::node sink_exceptions();
  mstch::node sink_final_reponse_type();
  mstch::node sink_final_response_exceptions();

  mstch::node stream_elem_type();
  mstch::node stream_first_response_type();
  mstch::node stream_exceptions();

 protected:
  const t_function* function_;
  const t_interface* interface_;

  bool has_args_() { return function_->params().has_fields(); }

  bool is_interaction_member() const {
    // TODO(T230131540): Whisker migration requires back-reference from function
    // to parent interface
    return dynamic_cast<const t_interaction*>(interface_) != nullptr;
  }

  mstch::node make_exceptions(const t_throws* exceptions) {
    return exceptions ? make_mstch_fields(exceptions->get_members())
                      : mstch::node();
  }
};

class mstch_type : public mstch_base {
 public:
  using ast_type = t_type;

  mstch_type(const t_type* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), type_(t), resolved_type_(t->get_true_type()) {
    register_methods(
        this,
        {
            {"type:self", &mstch_type::self},
            {"type:numeric_or_void?", &mstch_type::is_numeric_or_void},
            {"type:structured", &mstch_type::get_structured},
            {"type:enum", &mstch_type::get_enum},
            {"type:list_elem_type", &mstch_type::get_list_type},
            {"type:set_elem_type", &mstch_type::get_set_type},
            {"type:key_type", &mstch_type::get_key_type},
            {"type:value_type", &mstch_type::get_value_type},
            {"type:typedef_type", &mstch_type::get_typedef_type},
            {"type:typedef", &mstch_type::get_typedef},
        });
  }

  whisker::object self() { return make_self(*type_); }

  mstch::node is_numeric_or_void() {
    return resolved_type_->is_void() || resolved_type_->is_bool() ||
        resolved_type_->is_byte() || resolved_type_->is_i16() ||
        resolved_type_->is_i32() || resolved_type_->is_i64() ||
        resolved_type_->is_double() || resolved_type_->is_float();
  }
  virtual std::string get_type_namespace(const t_program*) { return ""; }
  mstch::node get_structured();
  mstch::node get_enum();
  mstch::node get_list_type();
  mstch::node get_set_type();
  mstch::node get_key_type();
  mstch::node get_value_type();
  mstch::node get_typedef_type();
  mstch::node get_typedef();

 protected:
  const t_type* type_;
  const t_type* resolved_type_;
};

class mstch_typedef : public mstch_base {
 public:
  using ast_type = t_typedef;

  mstch_typedef(
      const t_typedef* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), typedef_(t) {
    register_methods(
        this,
        {
            {"typedef:self", &mstch_typedef::self},
            {"typedef:type", &mstch_typedef::type},
            {"typedef:structured_annotations",
             &mstch_typedef::structured_annotations},
        });
  }

  whisker::object self() { return make_self(*typedef_); }
  mstch::node type();
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(typedef_);
  }

 protected:
  const t_typedef* typedef_;
};

class mstch_struct : public mstch_base {
 public:
  using ast_type = t_structured;

  mstch_struct(
      const t_structured* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), struct_(s) {
    register_methods(
        this,
        {
            {"struct:self", &mstch_struct::self},
            {"struct:fields?", &mstch_struct::has_fields},
            {"struct:fields", &mstch_struct::fields},
            {"struct:fields_in_serialization_order",
             &mstch_struct::fields_in_serialization_order},
            {"struct:exception?", &mstch_struct::is_exception},
            {"struct:plain?", &mstch_struct::is_plain},
            {"struct:annotations", &mstch_struct::annotations},
            {"struct:structured_annotations",
             &mstch_struct::structured_annotations},
            {"struct:exception_kind", &mstch_struct::exception_kind},
            {"struct:exception_safety", &mstch_struct::exception_safety},
            {"struct:exception_blame", &mstch_struct::exception_blame},
        });

    // Populate field_context_generator for each field.
    auto field_ctx = field_generator_context();
    field_ctx.strct = struct_;
    for (const t_field& field : s->fields()) {
      if (cpp2::field_has_isset(&field)) {
        field_ctx.isset_index++;
      }
      context_map[&field] = field_ctx;
    }

    const t_field* prev = nullptr;
    for (const t_field* curr : get_members_in_serialization_order()) {
      if (prev) {
        context_map[prev].serialization_next = curr;
        context_map[curr].serialization_prev = prev;
      }
      prev = curr;
    }
  }

  whisker::object self() { return make_self(*struct_); }
  mstch::node has_fields() { return struct_->has_fields(); }
  mstch::node fields();
  mstch::node is_exception() { return struct_->is<t_exception>(); }
  mstch::node is_plain() {
    return !struct_->is<t_exception>() && !struct_->is<t_union>();
  }
  mstch::node annotations() { return mstch_base::annotations(struct_); }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(struct_);
  }

  mstch::node exception_safety();
  mstch::node exception_blame();
  mstch::node exception_kind();

  mstch::array make_mstch_fields(const field_range& fields) override {
    mstch::array a;
    size_t i = 0;
    for (const auto* field : fields) {
      auto pos = mstch_element_position(i, fields.size());
      a.push_back(context_.field_factory->make_mstch_object(
          field, context_, pos, &context_map[field]));
      ++i;
    }
    return a;
  }

  // Returns the struct members ordered by the key.
  const std::vector<const t_field*>& get_members_in_key_order();

  field_range get_members_in_serialization_order() {
    if (struct_->has_structured_annotation(kSerializeInFieldIdOrderUri)) {
      return get_members_in_key_order();
    }

    return struct_->get_members();
  }

  mstch::node fields_in_serialization_order() {
    return make_mstch_fields(get_members_in_serialization_order());
  }

 protected:
  const t_structured* struct_;
  // Although mstch_fields can be generated in a different order than the IDL
  // order, field_generator_context should be always computed in the IDL order,
  // as the context does not change by reordering. Without the map, each
  // different reordering recomputes field_generator_context, and each
  // field takes O(N) to loop through node_list_view<t_field> or
  // std::vector<t_field*> to find the exact t_field to compute
  // field_generator_context.
  std::unordered_map<const t_field*, field_generator_context> context_map;

  std::vector<const t_field*> fields_in_key_order_;
};

class mstch_field : public mstch_base {
 public:
  using ast_type = t_field;

  mstch_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context)
      : mstch_base(ctx, pos), field_(f), field_context_(field_context) {
    register_methods(
        this,
        {
            {"field:self", &mstch_field::self},
            {"field:value", &mstch_field::value},
            {"field:type", &mstch_field::type},
            {"field:idl_type", &mstch_field::idl_type},
            {"field:index", &mstch_field::index},
            {"field:annotations", &mstch_field::annotations},
            {"field:structured_annotations",
             &mstch_field::structured_annotations},
            {"field:strings_compat?", &mstch_field::is_strings_compat},
            {"field:coding_error_action_legacy?",
             &mstch_field::is_coding_error_action_legacy},
            {"field:coding_error_action_report?",
             &mstch_field::is_coding_error_action_report},
        });
  }

  whisker::object self() { return make_self(*field_); }
  mstch::node value();
  mstch::node type();
  /**
   * Integer corresponding to the Thrift IDL type of the field, as defined by
   * `enum BaseType`.
   *
   * This corresponds to the "true" IDL type (i.e., after resolving aliases) and
   * does not include type parameters (such as map key and values, container
   * element types, etc.).
   */
  mstch::node idl_type();
  mstch::node index() { return pos_.index; }
  mstch::node annotations() { return mstch_base::annotations(field_); }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(field_);
  }
  mstch::node is_strings_compat() { return has_compat_annotation(kStringsUri); }
  mstch::node is_coding_error_action_legacy() {
    return has_compat_annotation(
        kStringsUri,
        kOnInvalidUtf8,
        CodingErrorAction::Legacy,
        CodingErrorAction::Report);
  }
  mstch::node is_coding_error_action_report() {
    return has_compat_annotation(
        kStringsUri,
        kOnInvalidUtf8,
        CodingErrorAction::Report,
        CodingErrorAction::Report);
  }
  bool has_compat_annotation(const char* uri) {
    if (field_->has_structured_annotation(uri)) {
      return true;
    }
    auto type = field_->get_type();
    if (type->is<t_typedef>()) {
      if (t_typedef::get_first_structured_annotation_or_null(type, uri) !=
          nullptr) {
        return true;
      }
    }
    if (field_context_ != nullptr && field_context_->strct != nullptr) {
      if (field_context_->strct->has_structured_annotation(uri)) {
        return true;
      }
      if (field_context_->strct->program()->has_structured_annotation(uri)) {
        return true;
      }
    }
    return false;
  }
  bool has_compat_annotation(
      const char* uri,
      const char* field,
      CodingErrorAction action,
      CodingErrorAction def) {
    auto type = field_->get_type();
    if (type->is<t_typedef>()) {
      if (auto annotation =
              t_typedef::get_first_structured_annotation_or_null(type, uri)) {
        return has_compat_action(annotation, field, action, def);
      }
    }
    if (auto annotation = field_->find_structured_annotation_or_null(uri)) {
      return has_compat_action(annotation, field, action, def);
    }
    if (field_context_ == nullptr || field_context_->strct == nullptr) {
      return false;
    }
    if (auto annotation =
            field_context_->strct->find_structured_annotation_or_null(uri)) {
      return has_compat_action(annotation, field, action, def);
    }
    if (auto annotation = field_context_->strct->program()
                              ->find_structured_annotation_or_null(uri)) {
      return has_compat_action(annotation, field, action, def);
    }
    return false;
  }
  bool has_compat_action(
      const t_const* annotation,
      const char* field,
      CodingErrorAction action,
      CodingErrorAction def) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == field) {
        return item.second->get_integer() == action;
      }
    }
    return action == def && annotation->value()->get_map().empty();
  }

 protected:
  const t_field* field_;
  const field_generator_context* field_context_;

  bool is_optional_() const {
    return field_->get_req() == t_field::e_req::optional;
  }
};

class mstch_enum : public mstch_base {
 public:
  using ast_type = t_enum;

  mstch_enum(const t_enum* e, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), enum_(e) {
    register_methods(
        this,
        {
            {"enum:self", &mstch_enum::self},
            {"enum:values", &mstch_enum::values},
            {"enum:structured_annotations",
             &mstch_enum::structured_annotations},
        });
  }

  whisker::object self() { return make_self(*enum_); }
  mstch::node values();
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(enum_);
  }

 protected:
  const t_enum* enum_;
};

class mstch_enum_value : public mstch_base {
 public:
  using ast_type = t_enum_value;

  mstch_enum_value(
      const t_enum_value* ev, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), enum_value_(ev) {
    register_methods(
        this,
        {
            {"enum_value:self", &mstch_enum_value::self},
        });
  }
  whisker::object self() { return make_self(*enum_value_); }

 protected:
  const t_enum_value* enum_value_;
};

class mstch_const : public mstch_base {
 public:
  using ast_type = t_const;

  mstch_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field)
      : mstch_base(ctx, pos),
        const_(c),
        current_const_(current_const),
        expected_type_(expected_type),
        field_(field) {
    register_methods(
        this,
        {
            {"constant:self", &mstch_const::self},
            {"constant:index", &mstch_const::index},
            {"constant:type", &mstch_const::type},
            {"constant:value", &mstch_const::value},
            {"constant:program", &mstch_const::program},
            {"constant:field", &mstch_const::field},
        });
  }

  whisker::object self() { return make_self(*const_); }
  mstch::node index() { return pos_.index; }
  mstch::node type();
  mstch::node value();
  mstch::node program();
  mstch::node field();

 protected:
  const t_const* const_;
  const t_const* current_const_;
  const t_type* expected_type_;
  const t_field* field_;
};

class mstch_const_value : public mstch_base {
 public:
  using ast_type = t_const_value;
  using cv = t_const_value::t_const_value_kind;

  mstch_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type)
      : mstch_base(ctx, pos),
        const_value_(cv),
        current_const_(current_const),
        expected_type_(expected_type),
        type_(cv->kind()) {
    register_methods(
        this,
        {
            {"value:self", &mstch_const_value::self},
            {"value:enum_value", &mstch_const_value::enum_value},
            {"value:list_elements", &mstch_const_value::list_elems},
            {"value:map_elements", &mstch_const_value::map_elems},
            {"value:const_struct", &mstch_const_value::const_struct},
            {"value:const_struct_type", &mstch_const_value::const_struct_type},
            {"value:referenceable?", &mstch_const_value::referenceable},
            {"value:owning_const", &mstch_const_value::owning_const},
            {"value:enable_referencing",
             &mstch_const_value::enable_referencing},
        });
  }

  whisker::object self() { return make_self(*const_value_); }
  mstch::node enum_value();
  mstch::node list_elems();
  mstch::node map_elems();
  mstch::node const_struct();
  mstch::node referenceable() {
    return const_value_->get_owner() &&
        current_const_ != const_value_->get_owner() && same_type_as_expected();
  }
  mstch::node owning_const();
  mstch::node enable_referencing() {
    return mstch::map{{"value:enable_referencing?", true}};
  }
  mstch::node const_struct_type();

 protected:
  const t_const_value* const_value_;
  const t_const* current_const_;
  const t_type* expected_type_;
  const cv type_;

  virtual bool same_type_as_expected() const { return false; }

  bool is_empty_container() const {
    return (const_value_->kind() == cv::CV_MAP &&
            const_value_->get_map().empty()) ||
        (const_value_->kind() == cv::CV_LIST &&
         const_value_->get_list().empty());
  }
};

class mstch_const_map_element : public mstch_base {
 public:
  using ast_type = std::pair<t_const_value*, t_const_value*>;

  mstch_const_map_element(
      const ast_type* e,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const std::pair<const t_type*, const t_type*>& expected_types)
      : mstch_base(ctx, pos),
        element_(*e),
        current_const_(current_const),
        expected_types_(expected_types) {
    register_methods(
        this,
        {
            {"element:key", &mstch_const_map_element::element_key},
            {"element:value", &mstch_const_map_element::element_value},
        });
  }
  mstch::node element_key();
  mstch::node element_value();

 protected:
  const std::pair<t_const_value*, t_const_value*> element_;
  const t_const* current_const_;
  const std::pair<const t_type*, const t_type*> expected_types_;
};

class mstch_structured_annotation : public mstch_base {
 public:
  using ast_type = detail::structured_annotation_tag;

  mstch_structured_annotation(
      const t_const* c, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), const_(*c) {
    register_methods(
        this,
        {{"structured_annotation:const",
          &mstch_structured_annotation::constant},
         {"structured_annotation:const_struct?",
          &mstch_structured_annotation::is_const_struct}});
  }
  mstch::node constant() {
    return context_.const_factory->make_mstch_object(
        &const_,
        context_,
        pos_,
        &const_,
        const_.type()->get_true_type(),
        nullptr);
  }

  mstch::node is_const_struct() {
    return const_.type()->get_true_type()->is<t_struct>() ||
        const_.type()->get_true_type()->is<t_union>();
  }

 protected:
  const t_const& const_;
};

class mstch_deprecated_annotation : public mstch_base {
 public:
  using ast_type = t_annotation;

  mstch_deprecated_annotation(
      const t_annotation* a, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), key_(a->first), val_(a->second) {
    register_methods(
        this,
        {
            {"annotation:key", &mstch_deprecated_annotation::key},
            {"annotation:value", &mstch_deprecated_annotation::value},
        });
  }
  mstch::node key() { return key_; }
  mstch::node value() { return val_.value; }

 protected:
  const std::string key_;
  const deprecated_annotation_value val_;
};

} // namespace apache::thrift::compiler
