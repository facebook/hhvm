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
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/lib/cpp2/util.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

class mstch_base;
struct mstch_context;

constexpr auto kOnInvalidUtf8 = "onInvalidUtf8";
constexpr auto kEnumType = "type";

struct mstch_element_position {
  mstch_element_position() = default;
  mstch_element_position(size_t index, size_t size)
      : index(index), first(index == 0), last(index == size - 1) {}

  int32_t index = 0;
  bool first = false;
  bool last = false;
};

struct field_generator_context {
  const t_struct* strct = nullptr;
  const t_field* serialization_prev = nullptr;
  const t_field* serialization_next = nullptr;
  int isset_index = -1;
};

enum CodingErrorAction {
  Legacy = 0,
  Report = 1,
};

enum EnumType {
  LegacyEnum = 0,
  Open = 1,
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
using mstch_function_factory = mstch_factory<t_function>;
using mstch_type_factory = mstch_factory<t_type>;
using mstch_typedef_factory = mstch_factory<t_typedef>;
using mstch_struct_factory = mstch_factory<t_struct>;
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
  auto& get(t_struct*) { return struct_factory; }
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
  std::map<std::string, std::string> options;

  std::unordered_map<std::string, std::shared_ptr<mstch_base>> enum_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> struct_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> service_cache;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> program_cache;
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
    return make_mstch_annotations(annotated->annotations());
  }

  mstch::node structured_annotations(const t_named* annotated) {
    return make_mstch_array(
        annotated->structured_annotations(),
        *context_.structured_annotation_factory);
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
          containing_service,
          args...);
    }
    return make_mstch_array(container, *context_.service_factory, args...);
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
    std::string elem_id = id + item->get_name();
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
            {"program:name", &mstch_program::name},
            {"program:autogen_path", &mstch_program::autogen_path},
            {"program:includePrefix", &mstch_program::include_prefix},
            {"program:structs", &mstch_program::structs},
            {"program:enums", &mstch_program::enums},
            {"program:services", &mstch_program::services},
            {"program:typedefs", &mstch_program::typedefs},
            {"program:constants", &mstch_program::constants},
            {"program:enums?", &mstch_program::has_enums},
            {"program:structs?", &mstch_program::has_structs},
            {"program:unions?", &mstch_program::has_unions},
            {"program:services?", &mstch_program::has_services},
            {"program:typedefs?", &mstch_program::has_typedefs},
            {"program:constants?", &mstch_program::has_constants},
            {"program:thrift_uris?", &mstch_program::has_thrift_uris},
            {"program:interned_values?", &mstch_program::has_interned_values},
            {"program:num_interned_values",
             &mstch_program::num_interned_values},
            {"program:interned_values", &mstch_program::interned_values},
        });
    register_has_option("program:frozen?", "frozen");
    register_has_option("program:json?", "json");
    register_has_option("program:any?", "any");
    register_has_option(
        "program:unstructured_annotations_in_metadata?",
        "deprecated_unstructured_annotations_in_metadata");
  }

  virtual std::string get_program_namespace(const t_program*) { return {}; }

  mstch::node name() { return program_->name(); }
  mstch::node autogen_path() {
    std::string path = program_->path();
    // use posix path separators, even on windows, for autogen comment
    // to avoid spurious fixture regen diffs
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
  }
  mstch::node include_prefix() { return program_->include_prefix(); }
  mstch::node has_enums() { return !program_->enums().empty(); }
  mstch::node has_structs() {
    return !program_->structs_and_unions().empty() ||
        !program_->xceptions().empty();
  }
  mstch::node has_services() { return !program_->services().empty(); }
  mstch::node has_typedefs() { return !program_->typedefs().empty(); }
  mstch::node has_constants() {
    return !program_->consts().empty() || context_.options.count("interning");
  }
  mstch::node has_unions() {
    auto& structs = program_->structs_and_unions();
    return std::any_of(
        structs.cbegin(), structs.cend(), std::mem_fn(&t_struct::is_union));
  }

  mstch::node has_thrift_uris();
  mstch::node structs();
  mstch::node enums();
  mstch::node services();
  mstch::node typedefs();
  mstch::node constants();

  mstch::node has_interned_values() { return !program_->intern_list().empty(); }
  mstch::node num_interned_values() {
    return static_cast<int>(program_->intern_list().size());
  }
  mstch::node interned_values();

 protected:
  const t_program* program_;
};

class mstch_service : public mstch_base {
 public:
  using ast_type = t_service;

  mstch_service(
      const t_service* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), service_(s) {
    register_methods(
        this,
        {
            {"service:name", &mstch_service::name},
            {"service:functions", &mstch_service::functions},
            {"service:functions?", &mstch_service::has_functions},
            {"service:extends", &mstch_service::extends},
            {"service:extends?", &mstch_service::has_extends},
            {"service:streams?", &mstch_service::has_streams},
            {"service:sinks?", &mstch_service::has_sinks},
            {"service:annotations", &mstch_service::annotations},
            {"service:thrift_uri", &mstch_service::thrift_uri},
            {"service:parent", &mstch_service::parent},
            {"service:interaction?", &mstch_service::is_interaction},
            {"service:interactions", &mstch_service::interactions},
            {"service:interactions?", &mstch_service::has_interactions},
            {"service:structured_annotations?",
             &mstch_service::has_structured_annotations},
            {"service:structured_annotations",
             &mstch_service::structured_annotations},
            {"interaction:serial?", &mstch_service::is_serial_interaction},
            {"interaction:eb?", &mstch_service::is_event_base_interaction},
        });

    // Collect performed interactions and cache them
    for (const auto* function : get_functions()) {
      if (function->return_type()->is_service()) {
        interactions_.insert(
            dynamic_cast<const t_interaction*>(function->return_type()));
      } else if (auto interaction = function->returned_interaction()) {
        interactions_.insert(
            dynamic_cast<const t_interaction*>(interaction->get_true_type()));
      }
    }
    assert(interactions_.count(nullptr) == 0);
  }

  virtual std::string get_service_namespace(const t_program*) { return {}; }

  mstch::node name() { return service_->get_name(); }
  mstch::node has_functions() { return !get_functions().empty(); }
  mstch::node has_extends() { return service_->get_extends() != nullptr; }
  mstch::node functions();
  mstch::node extends();
  mstch::node annotations() { return mstch_base::annotations(service_); }
  mstch::node thrift_uri() { return service_->uri(); }

  mstch::node parent() { return context_.options.at("parent_service_name"); }

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
    if (!service_->is_interaction()) {
      // For Python interactions:
      context_.options["parent_service_name"] = service_->get_name();
      context_.options["parent_service_cpp_name"] = cpp2::get_name(service_);
    }
    return make_mstch_interactions(interactions_, service_);
  }
  mstch::node has_structured_annotations() {
    return !service_->structured_annotations().empty();
  }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(service_);
  }
  mstch::node is_interaction() { return service_->is_interaction(); }
  mstch::node is_serial_interaction() {
    return service_->is_serial_interaction();
  }
  mstch::node is_event_base_interaction() {
    return service_->has_annotation("process_in_event_base");
  }

  virtual ~mstch_service() = default;

 protected:
  const t_service* service_;
  std::set<const t_interaction*> interactions_;

  mstch::node make_mstch_extended_service_cached(const t_service* service);
  virtual const std::vector<t_function*>& get_functions() const {
    return service_->get_functions();
  }
};

class mstch_function : public mstch_base {
 public:
  using ast_type = t_function;

  mstch_function(
      const t_function* f, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), function_(f) {
    register_methods(
        this,
        {
            {"function:name", &mstch_function::name},
            {"function:oneway?", &mstch_function::oneway},
            {"function:return_type", &mstch_function::return_type},
            {"function:exceptions", &mstch_function::exceptions},
            {"function:exceptions?", &mstch_function::has_exceptions},
            {"function:args", &mstch_function::arg_list},
            {"function:args?", &mstch_function::has_args},
            {"function:comma", &mstch_function::arg_comma},
            {"function:priority", &mstch_function::priority},
            {"function:annotations", &mstch_function::annotations},
            {"function:starts_interaction?",
             &mstch_function::starts_interaction},
            {"function:structured_annotations?",
             &mstch_function::has_structured_annotations},
            {"function:structured_annotations",
             &mstch_function::structured_annotations},
            {"function:qualifier", &mstch_function::qualifier},
            {"function:creates_interaction?",
             &mstch_function::creates_interaction},
            {"function:in_or_creates_interaction?",
             &mstch_function::in_or_creates_interaction},
            {"function:void?", &mstch_function::is_void},

            // Sink methods:
            {"function:sink?", &mstch_function::has_sink},
            {"function:sink_has_first_response?",
             &mstch_function::sink_has_first_response},
            {"function:sink_first_response_type",
             &mstch_function::sink_first_response_type},
            {"function:sink_elem_type", &mstch_function::sink_elem_type},
            {"function:sink_exceptions?", &mstch_function::has_sink_exceptions},
            {"function:sink_exceptions", &mstch_function::sink_exceptions},
            {"function:sink_final_response_type",
             &mstch_function::sink_final_reponse_type},
            {"function:sink_final_response_exceptions?",
             &mstch_function::has_sink_final_response_exceptions},
            {"function:sink_final_response_exceptions",
             &mstch_function::sink_final_response_exceptions},

            // Stream methods:
            {"function:stream?", &mstch_function::has_stream},
            {"function:stream_has_first_response?",
             &mstch_function::stream_has_first_response},
            {"function:stream_first_response_type",
             &mstch_function::stream_first_response_type},
            {"function:stream_elem_type", &mstch_function::stream_elem_type},
            {"function:stream_exceptions?",
             &mstch_function::has_stream_exceptions},
            {"function:stream_exceptions", &mstch_function::stream_exceptions},
        });
  }

  mstch::node name() { return function_->get_name(); }
  mstch::node oneway() {
    return function_->qualifier() == t_function_qualifier::oneway;
  }
  mstch::node has_exceptions() {
    return !get_elems(function_->exceptions()).empty();
  }
  mstch::node has_args() { return has_args_(); }
  mstch::node arg_comma() {
    return has_args_() ? std::string(", ") : std::string();
  }
  mstch::node priority() {
    return function_->get_annotation("priority", "NORMAL");
  }
  mstch::node annotations() { return mstch_base::annotations(function_); }

  mstch::node return_type();
  mstch::node exceptions();

  mstch::node arg_list();
  mstch::node starts_interaction() {
    return function_->return_type()->is_service();
  }
  mstch::node has_structured_annotations() {
    return !function_->structured_annotations().empty();
  }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(function_);
  }

  mstch::node qualifier() {
    auto q = function_->qualifier();
    switch (q) {
      case t_function_qualifier::oneway:
        return std::string("OneWay");
      case t_function_qualifier::idempotent:
        return std::string("Idempotent");
      case t_function_qualifier::readonly:
        return std::string("ReadOnly");
      default:
        return std::string("Unspecified");
    }
  }

  mstch::node creates_interaction() {
    return !function_->returned_interaction().empty();
  }
  mstch::node in_or_creates_interaction() {
    return !function_->returned_interaction().empty() ||
        function_->is_interaction_member();
  }
  mstch::node is_void() {
    return function_->return_type()->is_void() &&
        !function_->returned_interaction() && !function_->sink_or_stream();
  }

  mstch::node has_sink() { return function_->sink() != nullptr; }
  mstch::node sink_has_first_response() {
    const t_sink* sink = function_->sink();
    return sink && sink->sink_has_first_response();
  }
  mstch::node sink_first_response_type();
  mstch::node sink_elem_type();
  mstch::node has_sink_exceptions() {
    const t_sink* sink = function_->sink();
    const t_throws* exceptions = sink ? sink->sink_exceptions() : nullptr;
    return exceptions && exceptions->has_fields();
  }
  mstch::node sink_exceptions();
  mstch::node sink_final_reponse_type();
  mstch::node has_sink_final_response_exceptions() {
    const t_sink* sink = function_->sink();
    const t_throws* exceptions =
        sink ? sink->final_response_exceptions() : nullptr;
    return exceptions && exceptions->has_fields();
  }
  mstch::node sink_final_response_exceptions();

  mstch::node has_stream() { return function_->stream() != nullptr; }
  mstch::node stream_elem_type();
  mstch::node stream_has_first_response() {
    const t_stream_response* stream = function_->stream();
    return stream && !stream->first_response_type().empty();
  }
  mstch::node stream_first_response_type();
  mstch::node has_stream_exceptions() {
    const t_stream_response* stream = function_->stream();
    const t_throws* exceptions = stream ? stream->exceptions() : nullptr;
    return exceptions && exceptions->has_fields();
  }
  mstch::node stream_exceptions();

 protected:
  const t_function* function_;

  bool has_args_() { return function_->get_paramlist()->has_fields(); }

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
            {"type:name", &mstch_type::name},
            {"type:void?", &mstch_type::is_void},
            {"type:string?", &mstch_type::is_string},
            {"type:binary?", &mstch_type::is_binary},
            {"type:primitive?", &mstch_type::is_primitive},
            {"type:bool?", &mstch_type::is_bool},
            {"type:byte?", &mstch_type::is_byte},
            {"type:i16?", &mstch_type::is_i16},
            {"type:i32?", &mstch_type::is_i32},
            {"type:i64?", &mstch_type::is_i64},
            {"type:double?", &mstch_type::is_double},
            {"type:float?", &mstch_type::is_float},
            {"type:floating_point?", &mstch_type::is_floating_point},
            {"type:struct?", &mstch_type::is_struct},
            {"type:union?", &mstch_type::is_union},
            {"type:enum?", &mstch_type::is_enum},
            {"type:service?", &mstch_type::is_service},
            {"type:base?", &mstch_type::is_base},
            {"type:container?", &mstch_type::is_container},
            {"type:list?", &mstch_type::is_list},
            {"type:set?", &mstch_type::is_set},
            {"type:map?", &mstch_type::is_map},
            {"type:typedef?", &mstch_type::is_typedef},
            {"type:struct", &mstch_type::get_struct},
            {"type:enum", &mstch_type::get_enum},
            {"type:list_elem_type", &mstch_type::get_list_type},
            {"type:set_elem_type", &mstch_type::get_set_type},
            {"type:key_type", &mstch_type::get_key_type},
            {"type:value_type", &mstch_type::get_value_type},
            {"type:typedef_type", &mstch_type::get_typedef_type},
            {"type:typedef", &mstch_type::get_typedef},
            {"type:interaction?", &mstch_type::is_interaction},
        });
  }
  mstch::node name() { return type_->get_name(); }
  mstch::node is_void() { return resolved_type_->is_void(); }
  mstch::node is_string() { return resolved_type_->is_string(); }
  mstch::node is_binary() { return resolved_type_->is_binary(); }
  mstch::node is_bool() { return resolved_type_->is_bool(); }
  mstch::node is_byte() { return resolved_type_->is_byte(); }
  mstch::node is_i16() { return resolved_type_->is_i16(); }
  mstch::node is_i32() { return resolved_type_->is_i32(); }
  mstch::node is_i64() { return resolved_type_->is_i64(); }
  mstch::node is_double() { return resolved_type_->is_double(); }
  mstch::node is_float() { return resolved_type_->is_float(); }
  mstch::node is_primitive() {
    return resolved_type_->is_void() || resolved_type_->is_bool() ||
        resolved_type_->is_byte() || resolved_type_->is_i16() ||
        resolved_type_->is_i32() || resolved_type_->is_i64() ||
        resolved_type_->is_double() || resolved_type_->is_float();
  }
  mstch::node is_floating_point() {
    return resolved_type_->is_floating_point();
  }
  mstch::node is_struct() {
    return resolved_type_->is_struct() || resolved_type_->is_exception();
  }
  mstch::node is_union() { return resolved_type_->is_union(); }
  mstch::node is_enum() { return resolved_type_->is_enum(); }
  mstch::node is_service() { return resolved_type_->is_service(); }
  mstch::node is_base() { return resolved_type_->is_base_type(); }
  mstch::node is_container() { return resolved_type_->is_container(); }
  mstch::node is_list() { return resolved_type_->is_list(); }
  mstch::node is_set() { return resolved_type_->is_set(); }
  mstch::node is_map() { return resolved_type_->is_map(); }
  mstch::node is_typedef() { return type_->is_typedef(); }
  virtual std::string get_type_namespace(const t_program*) { return ""; }
  mstch::node get_struct();
  mstch::node get_enum();
  mstch::node get_list_type();
  mstch::node get_set_type();
  mstch::node get_key_type();
  mstch::node get_value_type();
  mstch::node get_typedef_type();
  mstch::node get_typedef();
  mstch::node is_interaction() { return type_->is_service(); }

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
            {"typedef:type", &mstch_typedef::type},
            {"typedef:is_same_type", &mstch_typedef::is_same_type},
            {"typedef:name", &mstch_typedef::name},
            {"typedef:structured_annotations?",
             &mstch_typedef::has_structured_annotations},
            {"typedef:structured_annotations",
             &mstch_typedef::structured_annotations},
        });
  }
  mstch::node type();
  mstch::node name() { return typedef_->name(); }
  mstch::node is_same_type() {
    return typedef_->get_name() == typedef_->get_type()->get_name();
  }
  mstch::node has_structured_annotations() {
    return !typedef_->structured_annotations().empty();
  }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(typedef_);
  }

 protected:
  const t_typedef* typedef_;
};

class mstch_struct : public mstch_base {
 public:
  using ast_type = t_struct;

  mstch_struct(
      const t_struct* s, mstch_context& ctx, mstch_element_position pos)
      : mstch_base(ctx, pos), struct_(s) {
    register_methods(
        this,
        {
            {"struct:name", &mstch_struct::name},
            {"struct:fields?", &mstch_struct::has_fields},
            {"struct:fields", &mstch_struct::fields},
            {"struct:fields_in_serialization_order",
             &mstch_struct::fields_in_serialization_order},
            {"struct:exception?", &mstch_struct::is_exception},
            {"struct:union?", &mstch_struct::is_union},
            {"struct:plain?", &mstch_struct::is_plain},
            {"struct:annotations", &mstch_struct::annotations},
            {"struct:thrift_uri", &mstch_struct::thrift_uri},
            {"struct:structured_annotations?",
             &mstch_struct::has_structured_annotations},
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
  mstch::node name() { return struct_->get_name(); }
  mstch::node has_fields() { return struct_->has_fields(); }
  mstch::node fields();
  mstch::node is_exception() { return struct_->is_exception(); }
  mstch::node is_union() { return struct_->is_union(); }
  mstch::node is_plain() {
    return !struct_->is_exception() && !struct_->is_union();
  }
  mstch::node has_structured_annotations() {
    return !struct_->structured_annotations().empty();
  }
  mstch::node annotations() { return mstch_base::annotations(struct_); }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(struct_);
  }
  mstch::node thrift_uri() { return struct_->uri(); }

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
    if (struct_->find_structured_annotation_or_null(
            kSerializeInFieldIdOrderUri)) {
      return get_members_in_key_order();
    }

    return struct_->get_members();
  }

  mstch::node fields_in_serialization_order() {
    return make_mstch_fields(get_members_in_serialization_order());
  }

 protected:
  const t_struct* struct_;
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
            {"field:name", &mstch_field::name},
            {"field:key", &mstch_field::key},
            {"field:value", &mstch_field::value},
            {"field:type", &mstch_field::type},
            {"field:index", &mstch_field::index},
            {"field:required?", &mstch_field::is_required},
            {"field:terse?", &mstch_field::is_terse},
            {"field:optional?", &mstch_field::is_optional},
            {"field:opt_in_req_out?", &mstch_field::is_optInReqOut},
            {"field:annotations", &mstch_field::annotations},
            {"field:structured_annotations?",
             &mstch_field::has_structured_annotations},
            {"field:structured_annotations",
             &mstch_field::structured_annotations},
            {"field:strings_compat?", &mstch_field::is_strings_compat},
            {"field:coding_error_action_legacy?",
             &mstch_field::is_coding_error_action_legacy},
            {"field:coding_error_action_report?",
             &mstch_field::is_coding_error_action_report},
        });
  }
  mstch::node name() { return field_->get_name(); }
  mstch::node key() { return std::to_string(field_->get_key()); }
  mstch::node value();
  mstch::node type();
  mstch::node index() { return std::to_string(pos_.index); }
  mstch::node is_terse() {
    return field_->qualifier() == t_field_qualifier::terse;
  }
  mstch::node is_required() {
    return field_->get_req() == t_field::e_req::required;
  }
  mstch::node is_optional() { return is_optional_(); }
  mstch::node is_optInReqOut() {
    return field_->get_req() == t_field::e_req::opt_in_req_out;
  }
  mstch::node annotations() { return mstch_base::annotations(field_); }
  mstch::node has_structured_annotations() {
    return !field_->structured_annotations().empty();
  }
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
    if (field_->find_structured_annotation_or_null(uri) != nullptr) {
      return true;
    }
    auto type = field_->get_type();
    if (type->is_typedef()) {
      if (t_typedef::get_first_structured_annotation_or_null(type, uri) !=
          nullptr) {
        return true;
      }
    }
    if (field_context_ != nullptr && field_context_->strct != nullptr) {
      if (field_context_->strct->find_structured_annotation_or_null(uri) !=
          nullptr) {
        return true;
      }
      if (field_context_->strct->program()->find_structured_annotation_or_null(
              uri) != nullptr) {
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
    if (type->is_typedef()) {
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
    if (action == def && annotation->value()->get_map().size() == 0) {
      return true;
    }

    return false;
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
            {"enum:name", &mstch_enum::name},
            {"enum:values", &mstch_enum::values},
            {"enum:structured_annotations?",
             &mstch_enum::has_structured_annotations},
            {"enum:structured_annotations",
             &mstch_enum::structured_annotations},
            {"enum:enums_compat?", &mstch_enum::is_enums_compat},
            {"enum:enum_type_legacy?", &mstch_enum::is_enum_type_legacy},
            {"enum:enum_type_open?", &mstch_enum::is_enum_type_open},
            {"enum:unused", &mstch_enum::unused_value},
            {"enum:thrift_uri", &mstch_enum::thrift_uri},
        });
  }

  mstch::node name() { return enum_->get_name(); }
  mstch::node values();
  mstch::node unused_value() { return enum_->unused(); }
  mstch::node has_structured_annotations() {
    return !enum_->structured_annotations().empty();
  }
  mstch::node structured_annotations() {
    return mstch_base::structured_annotations(enum_);
  }
  mstch::node is_enums_compat() { return has_compat_annotation(kEnumsUri); }
  bool has_compat_annotation(const char* uri) {
    if (enum_->find_structured_annotation_or_null(uri) != nullptr) {
      return true;
    }
    if (enum_->program()->find_structured_annotation_or_null(uri) != nullptr) {
      return true;
    }

    return false;
  }
  mstch::node is_enum_type_legacy() {
    return has_compat_annotation(
        kEnumsUri, kEnumType, EnumType::LegacyEnum, EnumType::Open);
  }
  mstch::node is_enum_type_open() {
    return has_compat_annotation(
        kEnumsUri, kEnumType, EnumType::Open, EnumType::Open);
  }
  bool has_compat_annotation(
      const char* uri, const char* field, EnumType action, EnumType def) {
    if (auto annotation = enum_->find_structured_annotation_or_null(uri)) {
      return has_compat_action(annotation, field, action, def);
    }
    if (auto annotation =
            enum_->program()->find_structured_annotation_or_null(uri)) {
      return has_compat_action(annotation, field, action, def);
    }
    return false;
  }
  bool has_compat_action(
      const t_const* annotation,
      const char* field,
      EnumType action,
      EnumType def) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == field) {
        return item.second->get_integer() == action;
      }
    }
    if (action == def && annotation->value()->get_map().size() == 0) {
      return true;
    }

    return false;
  }
  mstch::node thrift_uri() { return enum_->uri(); }

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
            {"enum_value:name", &mstch_enum_value::name},
            {"enum_value:value", &mstch_enum_value::value},
        });
  }
  mstch::node name() { return enum_value_->get_name(); }
  mstch::node value() { return std::to_string(enum_value_->get_value()); }

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
            {"constant:name", &mstch_const::name},
            {"constant:index", &mstch_const::index},
            {"constant:type", &mstch_const::type},
            {"constant:value", &mstch_const::value},
            {"constant:program", &mstch_const::program},
            {"constant:field", &mstch_const::field},
        });
  }
  mstch::node name() { return const_->get_name(); }
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
  using cv = t_const_value::t_const_value_type;

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
        type_(cv->get_type()) {
    register_methods(
        this,
        {
            {"value:bool?", &mstch_const_value::is_bool},
            {"value:double?", &mstch_const_value::is_double},
            {"value:integer?", &mstch_const_value::is_integer},
            {"value:enum?", &mstch_const_value::is_enum},
            {"value:enum_value?", &mstch_const_value::has_enum_value},
            {"value:string?", &mstch_const_value::is_string},
            {"value:base?", &mstch_const_value::is_base},
            {"value:map?", &mstch_const_value::is_map},
            {"value:list?", &mstch_const_value::is_list},
            {"value:container?", &mstch_const_value::is_container},
            {"value:empty_container?", &mstch_const_value::is_empty_container},
            {"value:value", &mstch_const_value::value},
            {"value:integer_value", &mstch_const_value::integer_value},
            {"value:double_value", &mstch_const_value::double_value},
            {"value:bool_value", &mstch_const_value::bool_value},
            {"value:enum_value", &mstch_const_value::enum_value},
            {"value:nonzero?", &mstch_const_value::is_non_zero},
            {"value:enum_name", &mstch_const_value::enum_name},
            {"value:string_value", &mstch_const_value::string_value},
            {"value:list_elements", &mstch_const_value::list_elems},
            {"value:map_elements", &mstch_const_value::map_elems},
            {"value:const_struct", &mstch_const_value::const_struct},
            {"value:const_struct?", &mstch_const_value::is_const_struct},
            {"value:const_struct_type", &mstch_const_value::const_struct_type},
            {"value:referenceable?", &mstch_const_value::referenceable},
            {"value:owning_const", &mstch_const_value::owning_const},
            {"value:enable_referencing",
             &mstch_const_value::enable_referencing},
        });
  }

  mstch::node is_bool() { return type_ == cv::CV_BOOL; }
  mstch::node is_double() { return type_ == cv::CV_DOUBLE; }
  mstch::node is_integer() {
    return type_ == cv::CV_INTEGER && !const_value_->is_enum();
  }
  mstch::node is_enum() {
    return type_ == cv::CV_INTEGER && const_value_->is_enum();
  }
  mstch::node has_enum_value() {
    return const_value_->get_enum_value() != nullptr;
  }
  mstch::node is_string() { return type_ == cv::CV_STRING; }
  mstch::node is_base() {
    return type_ == cv::CV_BOOL || type_ == cv::CV_DOUBLE ||
        type_ == cv::CV_INTEGER || type_ == cv::CV_STRING;
  }
  mstch::node is_map() { return type_ == cv::CV_MAP; }
  mstch::node is_list() { return type_ == cv::CV_LIST; }
  mstch::node is_container() {
    return type_ == cv::CV_MAP || type_ == cv::CV_LIST;
  }
  mstch::node is_empty_container() {
    return (type_ == cv::CV_MAP && const_value_->get_map().empty()) ||
        (type_ == cv::CV_LIST && const_value_->get_list().empty());
  }
  mstch::node value();
  mstch::node integer_value();
  mstch::node double_value();
  mstch::node bool_value();
  mstch::node enum_value();
  mstch::node is_non_zero();
  mstch::node enum_name();
  mstch::node string_value();
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
  mstch::node is_const_struct();
  mstch::node const_struct_type();

 protected:
  const t_const_value* const_value_;
  const t_const* current_const_;
  const t_type* expected_type_;
  const cv type_;

  virtual bool same_type_as_expected() const { return false; }
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
    return const_.type()->get_true_type()->is_struct();
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
  const annotation_value val_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
