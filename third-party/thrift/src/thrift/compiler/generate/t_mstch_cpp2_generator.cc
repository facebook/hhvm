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

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/type_visitor.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/cpp/name_resolver.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/sema/ast_validator.h>
#include <thrift/compiler/sema/schematizer.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/sema/standard_validator.h>

using apache::thrift::compiler::detail::schematizer;

namespace apache::thrift::compiler {
namespace {

// A compiler counterpart of cpp.EnumUnderlyingType that avoids dependency on
// the generated code and follows the compiler naming conventions.
enum class enum_underlying_type {
  i8 = 0,
  u8 = 1,
  i16 = 2,
  u16 = 3,
  u32 = 4,
};

const std::string& get_cpp_template(const t_type* type) {
  return type->get_unstructured_annotation({"cpp.template", "cpp2.template"});
}

bool is_complex_return(const t_type* type) {
  return type->is<t_container>() || type->is_string_or_binary() ||
      type->is<t_structured>();
}

bool same_types(const t_type* a, const t_type* b) {
  if (!a || !b) {
    return false;
  }

  if (get_cpp_template(a) != get_cpp_template(b) ||
      cpp2::get_type(a) != cpp2::get_type(b)) {
    return false;
  }

  const auto* resolved_a = a->get_true_type();
  const auto* resolved_b = b->get_true_type();

  // Check if both types are the same kind and for primitives, the same type
  if (const t_primitive_type *prim_a = resolved_a->try_as<t_primitive_type>(),
      *prim_b = resolved_b->try_as<t_primitive_type>();
      prim_a != nullptr && prim_b != nullptr) {
    // Both are primitives, check they are the same primitive type
    if (prim_a->primitive_type() != prim_b->primitive_type()) {
      return false;
    }
  } else if (typeid(*resolved_a) != typeid(*resolved_b)) {
    // Use typeid for other types to check they are the same kind
    return false;
  }

  if (const t_list* list_a = resolved_a->try_as<t_list>()) {
    const auto* list_b = static_cast<const t_list*>(resolved_b);
    return same_types(
        list_a->elem_type().get_type(), list_b->elem_type().get_type());
  } else if (const t_set* set_a = resolved_a->try_as<t_set>()) {
    const auto* set_b = static_cast<const t_set*>(resolved_b);
    return same_types(
        set_a->elem_type().get_type(), set_b->elem_type().get_type());
  } else if (const t_map* map_a = resolved_a->try_as<t_map>()) {
    const auto* map_b = static_cast<const t_map*>(resolved_b);
    return same_types(&map_a->key_type().deref(), &map_b->key_type().deref()) &&
        same_types(&map_a->val_type().deref(), &map_b->val_type().deref());
  }
  return true;
}

std::string get_out_dir_base(
    const t_mstch_generator::compiler_options_map& options) {
  return options.find("py3cpp") != options.end() ? "gen-py3cpp" : "gen-cpp2";
}

std::string mangle_field_name(const std::string& name) {
  return "__fbthrift_field_" + name;
}

bool should_mangle_field_storage_name_in_struct(const t_structured& s) {
  // We don't mangle field name if cpp.methods exist
  return !s.has_unstructured_annotation("cpp.methods");
}

bool resolves_to_container_or_struct(const t_type* type) {
  return type->is<t_container>() || type->is<t_structured>();
}

bool generate_reduced_client(const t_interface& i) {
  return i.is<t_interaction>();
}

template <class Comp>
const std::string& get_extremal_union_member(const t_union& u) {
  Comp comp;
  auto iter = std::min_element(
      u.fields().cbegin(),
      u.fields().cend(),
      [&comp](const t_field& a, const t_field& b) {
        return comp(a.id(), b.id());
      });
  if (iter == u.fields().cend()) {
    throw std::runtime_error("empty union struct");
  }
  return cpp2::get_name(&*iter);
}

// Compute the set of types that appear anywhere in the service
// definition as input or output types. This presents maps, lists etc as seen
// in declarations, but unpacks the payloads of sinks and streams.
whisker::array::ptr build_user_type_footprint(
    const t_service& service,
    const whisker::prototype_database& prototype_database) {
  std::vector<const t_type*> types;
  std::unordered_set<const t_type*> seen;

  // Helper to extract the necesary types from a single type identified in
  // some component of a method declaration.  Deals with maps, lists, streams
  // that surround actual types.
  auto extract_type = [&](const t_type* type) -> void {
    // Maintain insertion order for stable output.
    // Insert into types in order of detection (parsing), use "seen"
    // to avoid duplicates.
    if (seen.count(type) == 0) {
      types.emplace_back(type);
      seen.insert(type);
    }
  };

  // Go through each method declaration and identfiy the places that could
  // contain user defined types.
  std::deque<const t_function*> pending;
  for (const auto& function : service.functions()) {
    pending.emplace_back(&function);
  }
  while (!pending.empty()) {
    const auto& function = *pending.front();
    pending.erase(pending.begin());
    for (const auto& param : function.params().fields()) {
      extract_type(param.type().get_type());
    }
    if (const auto& excs = function.exceptions();
        !t_throws::is_null_or_empty(excs)) {
      for (auto& ex : excs->fields()) {
        extract_type(ex.type().get_type());
      }
    }

    extract_type(&function.return_type().deref());
    if (auto& type = function.interaction()) {
      if (auto* srv_type =
              dynamic_cast<const t_service*>(type->get_true_type())) {
        for (const auto& intfunc : srv_type->functions()) {
          pending.emplace_back(&intfunc);
        }
        continue;
      }
    }
    if (function.sink()) {
      extract_type(&function.sink()->elem_type().deref());
      if (!function.sink()->final_response_type().empty()) {
        extract_type(&function.sink()->final_response_type().deref());
      }
    }
    if (function.stream()) {
      extract_type(&function.stream()->elem_type().deref());
    }
  }
  whisker::array::raw ret;
  for (const t_type* typeptr : types) {
    // This line below should be this:
    // auto obj = resolve_derived_t_type(prototype_database, *typeptr);
    //
    // resolve_derived_t_type() does not produce the correct result for
    // right now - the right result being a match of the type names
    // used in service stub definitions, due to problems with inconsistent
    // behavior of the cpp_type property across different implementations/types.
    auto obj = prototype_database.create<t_type>(*typeptr);
    ret.emplace_back(std::move(obj));
  }
  return whisker::array::of(std::move(ret));
}

// Program's transitive_schema_includes depends on consistent order.
struct program_less {
  bool operator()(const t_program* a, const t_program* b) const {
    return a->path() < b->path();
  }
};

/**
 * Collect all transitive includes of a program into a sorted set, excluding
 * programs with the `DisableSchemaConst` annotation.
 */
void collect_transitive_includes(
    const t_program& program,
    std::set<const t_program*, program_less>& result) {
  for (const t_program* include : program.get_includes_for_codegen()) {
    if (include->has_structured_annotation(kDisableSchemaConstUri)) {
      continue;
    }
    if (result.insert(include).second) {
      collect_transitive_includes(*include, result);
    }
  }
}

/**
 * To reduce build time, the generated constants code only includes the
 * headers for direct thrift includes in the .cpp file and not in the .h file,
 * which means constants from transitive includes are not visible. To allow
 * constructing a flattened array of schemas for transitive dependencies
 * without undoing this optimization we indirect through the flattened array
 * of one of the direct includes to reach the schema of the transitive
 * include.
 *
 * This builds the information for how we will access the schema for all of
 * the transitive dependencies of a program. Each entry has:
 * - program: the included program
 * - schema_provider_program: the  program whose _includes array provides access
 *   (equals the root program for direct includes)
 * - schema_index: 0-based position within the provider's sorted transitive
 *   include set (template accounts for each program's own schema being
 *   inserted at 0)
 */
whisker::object program_transitive_schema_includes(
    const t_program& program, const whisker::prototype_database& proto) {
  // Accumulate the full set of transitive schema includes in a stable order
  std::map<const t_program*, whisker::object, program_less> items;
  for (const t_program* include : program.get_includes_for_codegen()) {
    if (include->has_structured_annotation(kDisableSchemaConstUri)) {
      continue;
    }
    // Direct include: schema_provider_program is the root program itself.
    items.emplace(
        include,
        whisker::map::of({
            {"program",
             whisker::make::native_handle(proto.create<t_program>(*include))},
            {"schema_provider_program",
             whisker::make::native_handle(proto.create<t_program>(program))},
            {"schema_index", whisker::make::null},
        }));
    // Get all transitive includes of this direct include (sorted by path).
    // The index within this sorted set determines the position in the
    // corresponding _includes array
    std::set<const t_program*, program_less> transitive_includes;
    collect_transitive_includes(*include, transitive_includes);
    int64_t i = 0;
    for (const t_program* transitive : transitive_includes) {
      if (!items.contains(transitive)) {
        items.emplace(
            transitive,
            whisker::map::of({
                {"program",
                 whisker::make::native_handle(
                     proto.create<t_program>(*transitive))},
                {"schema_provider_program",
                 whisker::make::native_handle(
                     proto.create<t_program>(*include))},
                {"schema_index", whisker::make::i64(i)},
            }));
      }
      ++i;
    }
  }
  whisker::array::raw result;
  result.reserve(items.size());
  for (auto& [_, item] : items) {
    result.emplace_back(std::move(item));
  }
  return whisker::make::array(std::move(result));
}

struct cpp2_field_generator_context {
  const t_field* serialization_prev = nullptr;
  const t_field* serialization_next = nullptr;
  int isset_index = -1;
};

std::vector<const t_field*> get_structured_fields_in_layout_order(
    const t_structured& strct);

/**
 * Check fields for the meeting any of the following criteria:
 * All enums
 * All primitives except empty strings
 * All non-empty structs and containers
 * All non-optional references with basetypes, enums, non-empty structs, and
 * containers
 */
bool is_field_explicitly_constructed(
    const t_field& field, const t_structured& parent_struct) {
  const t_type* type = field.type()->get_true_type();
  if (cpp2::is_explicit_ref(&field) &&
      field.qualifier() == t_field_qualifier::optional) {
    return false;
  }
  if (type->is<t_enum>()) {
    return true;
  }
  if (type->is<t_primitive_type>()) {
    return !type->is_string_or_binary() || field.default_value() != nullptr ||
        cpp2::is_explicit_ref(&field);
  }
  if (type->is<t_struct>() || type->is<t_union>()) {
    return type != &parent_struct &&
        (cpp2::is_explicit_ref(&field) ||
         (field.default_value() != nullptr &&
          !field.default_value()->is_empty()));
  }
  if (type->is<t_container>()) {
    return cpp2::is_explicit_ref(&field) ||
        (field.default_value() != nullptr &&
         !field.default_value()->is_empty());
  }
  return false;
}

class cpp2_generator_context {
 public:
  explicit cpp2_generator_context(
      source_manager& sm, const t_program* root, int program_split_count)
      : root_program_{root} {
    root_program_has_schema_const_ =
        root_program_->find(
            {schematizer::name_schema(sm, *root_program_), source_range{}}) !=
        nullptr;
    if (program_split_count > 0) {
      program_structured_definition_splits_ = cpp2::lpt_split(
          root->structured_definitions(), program_split_count, [](auto t) {
            return t->fields().size();
          });
    }

    // Compute topologically sorted structured definitions and typedefs for the
    // root program. We combine these because the adapter trait used in typedefs
    // requires the typedefed struct to be complete, and the typedefs themselves
    // cannot be forward declared. Topo sort the combined set to fulfill these
    // requirements.
    {
      std::vector<const t_type*> nodes;
      nodes.reserve(
          root->structured_definitions().size() + root->typedefs().size());
      nodes.insert(
          nodes.end(), root->typedefs().begin(), root->typedefs().end());
      nodes.insert(
          nodes.end(),
          root->structured_definitions().begin(),
          root->structured_definitions().end());
      type_definitions_topological_order_ =
          cpp2::topological_sort<const t_type*>(
              nodes.begin(),
              nodes.end(),
              /*edges=*/cpp2::gen_dependency_graph(root, nodes),
              /*throwOnCycle=*/true);
    }
  }

  bool is_orderable(const t_structured& structured_type) {
    return cpp2::OrderableTypeUtils::is_orderable(
        is_orderable_memo_, structured_type);
  }

  cpp_name_resolver& resolver() { return resolver_; }

  const cpp2_field_generator_context* get_field_context(
      const t_field* field) const {
    auto it = field_context_map_.find(field);
    return it == field_context_map_.end() ? nullptr : &it->second;
  }

  /**
   * The set of included programs whose constants are referenced for field
   * default values in the root program. These programs' `module_constants.h`
   * needs to be included in the root program's `module_types.h` for const
   * referencing.
   */
  const std::unordered_set<const t_program*>& field_default_const_ref_programs()
      const {
    return field_default_const_ref_programs_;
  }

  bool has_schema_const(const t_program& program) const {
    check_root_program(program);
    return root_program_has_schema_const_;
  }

  const std::vector<const t_field*>& fields_in_layout_order(
      const t_structured& strct) const {
    check_root_program(*strct.program());
    auto it = fields_in_layout_order_.find(&strct);
    assert(it != fields_in_layout_order_.end());
    return it->second;
  }

  /**
   * Structured definitions and typedefs defined by the program, in topological
   * order.
   */
  const std::vector<const t_type*>& type_definitions_topological_order(
      const t_program& program) const {
    check_root_program(program);
    return type_definitions_topological_order_;
  }

  // --- Program split state ---
  void set_program_split(int32_t split_id) {
    assert(!program_structured_definition_splits_.empty());
    program_split_id_ = split_id;
  }
  void clear_program_split() { program_split_id_ = std::nullopt; }
  std::optional<int32_t> program_split_id() const { return program_split_id_; }
  const std::vector<t_structured*>&
  program_current_split_structured_definitions() const {
    assert(program_split_id_.has_value());
    return program_structured_definition_splits_.at(*program_split_id_);
  }
  std::vector<const t_enum*> program_current_split_enums() const {
    assert(program_split_id_.has_value());
    std::vector<const t_enum*> split;
    size_t split_count = program_structured_definition_splits_.size();
    for (size_t i = *program_split_id_; i < root_program_->enums().size();
         i += split_count) {
      split.emplace_back(root_program_->enums()[i]);
    }
    return split;
  }

  // --- Service split state ---
  void set_service_split(int32_t split_id, int32_t split_count) {
    current_service_split_id_ = split_id;
    current_service_split_count_ = split_count;
  }
  void clear_service_split() {
    current_service_split_id_ = 0;
    current_service_split_count_ = 1;
  }
  int32_t current_service_split_id() const { return current_service_split_id_; }
  int32_t current_service_split_count() const {
    return current_service_split_count_;
  }

  void register_visitors(t_whisker_generator::context_visitor& visitor) {
    using context = t_whisker_generator::whisker_generator_visitor_context;
    // Compute field isset indexes and serialization order, which requires a
    // back-reference to the parent structured definition. Not using field
    // visitor here, since it requires forward/backward context.
    visitor.add_structured_definition_visitor(
        [this](const context& ctx, const t_structured& node) {
          if (&ctx.program() == root_program_) {
            fields_in_layout_order_[&node] =
                get_structured_fields_in_layout_order(node);
          }

          cpp2_field_generator_context field_ctx;
          for (const t_field& field : node.fields()) {
            if (cpp2::field_has_isset(&field)) {
              field_ctx.isset_index++;
            }
            field_context_map_[&field] = field_ctx;
          }

          if (node.has_structured_annotation(kSerializeInFieldIdOrderUri)) {
            const t_field* prev = nullptr;
            for (const t_field* curr : node.fields_id_order()) {
              if (prev != nullptr) {
                field_context_map_[prev].serialization_next = curr;
                field_context_map_[curr].serialization_prev = prev;
              }
              prev = curr;
            }
          } else {
            const t_field* prev = nullptr;
            for (const t_field& curr : node.fields()) {
              if (prev != nullptr) {
                field_context_map_[prev].serialization_next = &curr;
                field_context_map_[&curr].serialization_prev = prev;
              }
              prev = &curr;
            }
          }
        });

    visitor.add_field_visitor([this](const context& ctx, const t_field& node) {
      // If this field is in our root program and its default value is a
      // constant from an included program, track it so we can include the
      // corresponding `module_constants.h` in `module_types.h`
      if (node.default_value() == nullptr || &ctx.program() != root_program_) {
        // Field doesn't have a default or originates in an included program
        return;
      }
      // The program the default_value's owning const originates from
      const t_program* const_program =
          node.default_value()->get_owner() == nullptr
          ? nullptr
          : node.default_value()->get_owner()->program();
      if (const_program != nullptr && const_program != root_program_) {
        // Default value is from an included program - track it
        field_default_const_ref_programs_.emplace(const_program);
      }
    });
  }

 private:
  const t_program* root_program_;
  std::unordered_map<const t_type*, bool> is_orderable_memo_;
  cpp_name_resolver resolver_;
  bool root_program_has_schema_const_;

  // Although generator fields can be in a different order than the IDL
  // order, field_generator_context should be always computed in the IDL order,
  // as the context does not change by reordering. Without the map, each
  // different reordering recomputes field_generator_context, and each
  // field takes O(N) to loop through node_list_view<t_field> or
  // std::vector<t_field*> to find the exact t_field to compute
  // field_generator_context.
  std::unordered_map<const t_field*, cpp2_field_generator_context>
      field_context_map_;
  std::unordered_set<const t_program*> field_default_const_ref_programs_;
  std::unordered_map<const t_structured*, std::vector<const t_field*>>
      fields_in_layout_order_;
  std::vector<const t_type*> type_definitions_topological_order_;

  // Program split: LPT-partitioned structured definitions
  std::vector<std::vector<t_structured*>> program_structured_definition_splits_;
  // Current program split ID, set per loop iteration in generate_structs.
  std::optional<int32_t> program_split_id_;

  // Current service split state, set per loop iteration in
  // generate_out_of_line_service
  int32_t current_service_split_id_ = 0;
  int32_t current_service_split_count_ = 1;

  void check_root_program(const t_program& program) const {
    if (&program != root_program_) {
      throw whisker::eval_error(
          "This property is only implemented for the root program");
    }
  }
};

int checked_stoi(const std::string& s, const std::string& msg) {
  std::size_t pos = 0;
  int ret = std::stoi(s, &pos);
  if (pos != s.size()) {
    throw std::runtime_error(msg);
  }
  return ret;
}

int get_split_count(const t_mstch_generator::compiler_options_map& options) {
  auto iter = options.find("types_cpp_splits");
  if (iter == options.end()) {
    return 0;
  }
  return checked_stoi(
      iter->second, "Invalid types_cpp_splits value: `" + iter->second + "`");
}

bool needs_op_encode(const t_type& type);
bool field_needs_op_encode(const t_field& field, const t_structured& strct);

bool is_zero_copy_arg(const t_type& type) {
  const auto& true_type = *type.get_true_type();
  if (true_type.is_binary() || true_type.is<t_structured>()) {
    return true;
  } else if (const t_list* list = true_type.try_as<t_list>()) {
    return is_zero_copy_arg(*list->elem_type());
  } else if (const t_set* set = true_type.try_as<t_set>()) {
    return is_zero_copy_arg(*set->elem_type());
  } else if (const t_map* map = true_type.try_as<t_map>()) {
    return is_zero_copy_arg(map->key_type().deref()) ||
        is_zero_copy_arg(map->val_type().deref());
  }
  return false;
}

bool is_field_private(
    const t_field& field, bool deprecated_public_required_fields) {
  // Lazy and cpp.ref fields are always private.
  if (cpp2::is_lazy(&field) || cpp2::is_ref(&field)) {
    return true;
  }
  if (field.qualifier() == t_field_qualifier::required) {
    return !deprecated_public_required_fields;
  }
  return true;
}

bool is_field_eligible_for_storage_name_mangling(
    const t_structured& strct,
    const t_field& field,
    bool deprecated_public_required_fields) {
  if (strct.is<t_union>()) {
    // For unions, we should set this to false since we don't want to mangle
    // the field name inside MyUnion::Type.
    return false;
  }

  if (!should_mangle_field_storage_name_in_struct(strct)) {
    return false;
  }

  return is_field_private(field, deprecated_public_required_fields);
}

bool type_transitively_refers_to_struct(const t_type& type) {
  const t_type* resolved = type.get_true_type();
  // fast path is unnecessary but may avoid allocations
  if (resolved->is<t_struct>() || resolved->is<t_union>()) {
    return true;
  }
  if (!resolved->is<t_container>()) {
    return false;
  }
  // type is a container: traverse (breadthwise, but could be depthwise)
  std::queue<const t_type*> queue;
  queue.push(resolved);
  while (!queue.empty()) {
    auto next = queue.front();
    queue.pop();
    if (next->is<t_struct>() || next->is<t_union>()) {
      return true;
    }
    if (!next->is<t_container>()) {
      continue;
    }
    if (const t_list* list = next->try_as<t_list>()) {
      queue.push(&list->elem_type().deref());
    } else if (const t_set* set = next->try_as<t_set>()) {
      queue.push(&set->elem_type().deref());
    } else if (const t_map* map = next->try_as<t_map>()) {
      queue.push(&map->key_type().deref());
      queue.push(&map->val_type().deref());
    } else {
      assert(false);
    }
  }
  return false;
}

class t_mstch_cpp2_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "cpp2"; }

  void process_options(
      const std::map<std::string, std::string>& options) override {
    t_mstch_generator::process_options(options);
    client_name_to_split_count_ = get_client_name_to_split_count();
    out_dir_base_ = get_out_dir_base(this->options());
  }

  void generate_program() override;
  void fill_validator_visitors(ast_validator&) const override;
  static std::string include_prefix(
      const t_program* program, const compiler_options_map& options);

 private:
  void set_mstch_factories();
  void render_whisker_file(
      std::string_view template_name, const std::filesystem::path& output) {
    whisker::object context = whisker::make::map({
        {"program",
         whisker::make::native_handle(
             render_state().prototypes->create<t_program>(*program_))},
    });
    t_whisker_generator::render_to_file(output, template_name, context);
  }

  void generate_sinit(const t_program* program);
  void generate_visitation();
  void generate_constants(const t_program* program);
  void generate_metadata(const t_program* program);
  void generate_structs(const t_program* program);
  void generate_out_of_line_service(const t_service* service);
  void generate_out_of_line_services(const std::vector<t_service*>& services);
  void generate_inline_services(const std::vector<t_service*>& services);

  void initialize_context(context_visitor& visitor) override {
    cpp_context_ = std::make_unique<cpp2_generator_context>(
        source_mgr_, program_, get_split_count(options()));
    cpp_context_->register_visitors(visitor);
  }

  whisker::map::raw globals(prototype_database& proto) const override {
    whisker::map::raw globals = t_mstch_generator::globals(proto);
    // Global accessor for `cpp_enable_same_program_const_referencing_`.
    // Only the template for module_types.h overrides this, setting it to FALSE,
    // it is defaulted to TRUE for all other templates.
    // Controls whether references to consts in the current Thrift file can be
    // emitted as references, or must be inlined. By default, we can emit all
    // const usage as references.
    // See comment on render_to_file for module_types.h for more details.
    globals["cpp_enable_same_program_const_referencing?"] =
        whisker::dsl::make_function(
            "cpp_enable_same_program_const_referencing?",
            [this](whisker::dsl::function::context ctx) -> whisker::object {
              ctx.declare_named_arguments({});
              ctx.declare_arity(0);
              return whisker::make::boolean(
                  cpp_enable_same_program_const_referencing_);
            });
    return globals;
  }

  prototype<t_program>::ptr make_prototype_for_program(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_program(proto);
    auto def = whisker::dsl::prototype_builder<h_program>::extends(base);
    def.property(
        "cpp_qualified_namespace", &cpp2::get_gen_unprefixed_namespace);
    def.property(
        "const_referenced_in_field_default?", [this](const t_program& program) {
          return cpp_context_->field_default_const_ref_programs().contains(
              &program);
        });
    def.property("frozen_packed?", [this](const t_program&) {
      return get_compiler_option("frozen") == "packed";
    });
    def.property("cpp_declare_hash?", [](const t_program& self) {
      bool cpp_declare_in_structs = std::any_of(
          self.structs_and_unions().begin(),
          self.structs_and_unions().end(),
          [](const t_structured* strct) {
            return strct->has_unstructured_annotation(
                       {"cpp.declare_hash", "cpp2.declare_hash"}) ||
                strct->has_structured_annotation(kCppDeclareHashSpecialization);
          });
      bool cpp_declare_in_typedefs = std::any_of(
          self.typedefs().begin(),
          self.typedefs().end(),
          [](const auto* typedf) {
            return typedf->type()->has_unstructured_annotation(
                       {"cpp.declare_hash", "cpp2.declare_hash"}) ||
                typedf->type()->has_structured_annotation(
                    kCppDeclareHashSpecialization);
          });
      return cpp_declare_in_structs || cpp_declare_in_typedefs;
    });
    def.property("include_prefix", [this](const t_program& self) {
      return include_prefix(&self, compiler_options());
    });
    def.property("has_schema?", [this](const t_program& self) {
      return cpp_context_->has_schema_const(self);
    });
    def.property("schema_includes_const?", [this](const t_program& self) {
      return cpp_context_->has_schema_const(self) &&
          !self.has_structured_annotation(kDisableSchemaConstUri);
    });
    def.property("schema_name", [this](const t_program& self) {
      return schematizer::name_schema(source_mgr_, self);
    });
    def.property(
        "type_definitions_topological_order", [&](const t_program& self) {
          return to_type_array(
              cpp_context_->type_definitions_topological_order(self), proto);
        });
    def.property(
        "current_split_structured_definitions", [&](const t_program& self) {
          if (cpp_context_->program_split_id().has_value() &&
              program_ == &self) {
            return to_type_array(
                cpp_context_->program_current_split_structured_definitions(),
                proto);
          }
          return to_type_array(self.structured_definitions(), proto);
        });
    def.property("current_split_enums", [&](const t_program& self) {
      if (cpp_context_->program_split_id().has_value() && program_ == &self) {
        return to_type_array(
            cpp_context_->program_current_split_enums(), proto);
      }
      return to_type_array(self.enums(), proto);
    });
    def.property("thrift_includes", [&proto](const t_program& program) {
      // TODO(T256504524): Migrate to `includes_for_codegen` property in
      // `t_whisker_generator` in the future
      return to_array(
          program.get_includes_for_codegen(), proto.of<t_program>());
    });
    def.property("cpp_includes", [](const t_program& program) {
      // C++ includes from IDL file
      whisker::array::raw includes;
      if (program.language_includes().count("cpp")) {
        for (std::string include : program.language_includes().at("cpp")) {
          if (include.at(0) != '<') {
            include = fmt::format("\"{}\"", include);
          }
          includes.emplace_back(std::move(include));
        }
      }
      return whisker::make::array(std::move(includes));
    });
    def.property("extra_cpp_includes", [this](const t_program&) {
      // C++ includes from compiler options
      std::optional<std::string_view> extra_includes_option =
          get_compiler_option("includes");
      if (extra_includes_option.value_or("").empty()) {
        return whisker::make::array();
      }
      std::vector<std::string> extra_includes;
      boost::split(extra_includes, extra_includes_option.value(), [](char c) {
        return c == ':';
      });
      whisker::array::raw result;
      for (std::string& s : extra_includes) {
        result.emplace_back(std::move(s));
      }
      return whisker::make::array(std::move(result));
    });
    def.property(
        "transitive_schema_includes", [&proto](const t_program& program) {
          return program_transitive_schema_includes(program, proto);
        });
    return std::move(def).make();
  }

  prototype<t_named>::ptr make_prototype_for_named(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_named(proto);
    auto def = whisker::dsl::prototype_builder<h_named>::extends(base);
    def.property("cpp_name", [](const t_named& named) {
      return cpp2::get_name(&named);
    });
    // This property is necessary rather than just looping
    // `structured_annotations` with an `is_runtime_annotation?` condition for
    // enumerating with first/last scenarios, where the first/last *runtime*
    // (i.e. condition passing) annotation may not be the same as the first/last
    // annotation (i.e. loop iteration)
    def.property(
        "structured_runtime_annotations", [&proto](const t_named& named) {
          std::vector<const t_const*> runtime_annotations;
          runtime_annotations.reserve(named.structured_annotations().size());
          for (const auto& annotation : named.structured_annotations()) {
            if (is_runtime_annotation(*annotation.type())) {
              runtime_annotations.push_back(&annotation);
            }
          }
          return to_array(runtime_annotations, proto.of<t_const>());
        });
    return std::move(def).make();
  }

  prototype<t_structured>::ptr make_prototype_for_structured(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_structured(proto);
    auto def = whisker::dsl::prototype_builder<h_structured>::extends(base);

    def.property("cpp_fullname", [this](const t_structured& strct) {
      return cpp_context_->resolver().get_underlying_namespaced_name(strct);
    });
    def.property("is_struct_orderable?", [this](const t_structured& strct) {
      return cpp_context_->is_orderable(strct) &&
          !strct.has_unstructured_annotation("no_default_comparators");
    });
    def.property(
        "nondefault_copy_ctor_and_assignment?", [](const t_structured& strct) {
          if (strct.has_unstructured_annotation("cpp.allocator")) {
            return true;
          }
          for (const auto& f : strct.fields()) {
            if (cpp2::field_transitively_refers_to_unique(&f) ||
                cpp2::is_lazy(&f) || cpp_name_resolver::find_first_adapter(f)) {
              return true;
            }
          }
          return false;
        });
    def.property(
        "cpp_underlying_name", &cpp_name_resolver::get_underlying_name);
    def.property("cpp_underlying_type", [this](const t_structured& strct) {
      return cpp_context_->resolver().get_underlying_type_name(strct);
    });
    def.property("is_directly_adapted?", [this](const t_structured& strct) {
      return cpp_context_->resolver().is_directly_adapted(strct);
    });
    def.property(
        "dependent_direct_adapter?", [this](const t_structured& strct) {
          auto adapter =
              cpp_context_->resolver().find_nontransitive_adapter(strct);
          return adapter &&
              !adapter->get_value_from_structured_annotation_or_null(
                  "adaptedType");
        });
    def.property("cpp_methods", [](const t_structured& strct) {
      return strct.get_unstructured_annotation({"cpp.methods"});
    });
    def.property("cpp_declare_hash", [](const t_structured& strct) {
      return strct.has_unstructured_annotation(
                 {"cpp.declare_hash", "cpp2.declare_hash"}) ||
          strct.has_structured_annotation(kCppDeclareHashSpecialization);
    });
    def.property("cpp_declare_equal_to", [](const t_structured& strct) {
      return strct.has_unstructured_annotation(
                 {"cpp.declare_equal_to", "cpp2.declare_equal_to"}) ||
          strct.has_structured_annotation(kCppDeclareEqualToSpecialization);
    });
    def.property("cpp_noncopyable", [](const t_structured& strct) {
      if (strct.has_unstructured_annotation(
              {"cpp.noncopyable", "cpp2.noncopyable"})) {
        return true;
      }
      bool result = false;
      cpp2::for_each_transitive_field(&strct, [&result](const t_field* field) {
        if (!field->type().get_type()->has_unstructured_annotation(
                {"cpp.noncopyable", "cpp2.noncopyable"})) {
          return true;
        }
        switch (gen::cpp::find_ref_type(*field)) {
          case gen::cpp::reference_type::shared_const:
          case gen::cpp::reference_type::shared_mutable: {
            return true;
          }
          case gen::cpp::reference_type::boxed_intern:
          case gen::cpp::reference_type::boxed:
          case gen::cpp::reference_type::none:
          case gen::cpp::reference_type::unique:
            break;
        }
        result = true;
        return false;
      });
      return result;
    });
    def.property("cpp_noncomparable", [](const t_structured& strct) {
      return strct.has_unstructured_annotation(
          {"cpp.noncomparable", "cpp2.noncomparable"});
    });
    def.property("cpp_nonorderable?", [](const t_structured& strct) {
      return strct.has_structured_annotation(kCppNonOrderable);
    });
    def.property(
        "is_eligible_for_constexpr?", [this](const t_structured& strct) {
          return is_eligible_for_constexpr_(&strct) ||
              strct.has_unstructured_annotation("cpp.methods");
        });
    def.property("virtual", [](const t_structured& strct) {
      return strct.has_unstructured_annotation({"cpp.virtual", "cpp2.virtual"});
    });
    def.property("cpp_allocator", [](const t_structured& strct) {
      return strct.get_unstructured_annotation("cpp.allocator");
    });
    def.property("cpp_frozen2_exclude?", [this](const t_structured& strct) {
      // TODO(dokwon): Fix frozen2 compatibility with adapter.
      return strct.has_unstructured_annotation("cpp.frozen2_exclude") ||
          strct.has_structured_annotation(kCppFrozen2ExcludeUri) ||
          cpp_context_->resolver().is_directly_adapted(strct);
    });
    def.property("cpp_allocator_via", [](const t_structured& strct) {
      if (const std::string* name =
              strct.find_unstructured_annotation_or_null("cpp.allocator_via")) {
        for (const auto& field : strct.fields()) {
          if (cpp2::get_name(&field) == *name) {
            return mangle_field_name(*name);
          }
        }
        throw std::runtime_error(
            fmt::format("No cpp.allocator_via field \"{}\"", *name));
      }
      return whisker::string("");
    });
    def.property("lazy_fields?", [](const t_structured& strct) {
      for (const auto& field : strct.fields()) {
        if (cpp2::is_lazy(&field)) {
          return true;
        }
      }
      return false;
    });
    def.property("indexing?", [](const t_structured& strct) {
      for (const auto& field : strct.fields()) {
        if (cpp2::is_lazy(&field)) {
          return true;
        }
      }
      return false;
    });
    def.property("write_lazy_field_checksum?", [](const t_structured& strct) {
      return !strct.has_structured_annotation(kCppDisableLazyChecksumUri);
    });
    def.property("isset_fields?", [](const t_structured& strct) {
      for (const auto& field : strct.fields()) {
        if (cpp2::field_has_isset(&field)) {
          return true;
        }
      }
      return false;
    });
    def.property("isset_fields_size", [](const t_structured& strct) {
      int64_t size = 0;
      for (const auto& field : strct.fields()) {
        if (cpp2::field_has_isset(&field)) {
          size++;
        }
      }
      return size;
    });
    def.property("isset_bitset_option", [](const t_structured& strct) {
      static const std::string kPrefix =
          "apache::thrift::detail::IssetBitsetOption::";
      if (const t_const* anno = cpp2::packed_isset(strct)) {
        for (const auto& [key, val] : anno->value()->get_map()) {
          if (key->get_string() == "atomic" && !val->get_bool()) {
            return fmt::format("{}Packed", kPrefix);
          }
        }
        return fmt::format("{}PackedWithAtomic", kPrefix);
      }
      return fmt::format("{}Unpacked", kPrefix);
    });
    def.property("is_large?", [](const t_structured& strct) {
      // Outline constructors and destructors if the struct has at least one
      // member with a non-trivial destructor (involving at least a branch and a
      // likely deallocation).
      // TODO(ott): Support unions.
      if (strct.is<t_exception>()) {
        return true;
      }
      for (const auto& field : strct.fields()) {
        const auto* resolved_typedef = field.type()->get_true_type();
        if (cpp2::is_ref(&field) || resolved_typedef->is_string_or_binary() ||
            resolved_typedef->is<t_container>()) {
          return true;
        }
      }
      return false;
    });
    def.property("legacy_api?", [](const t_structured&) { return true; });
    def.property(
        "has_non_optional_and_non_terse_field?",
        [this](const t_structured& strct) {
          const auto& fields = strct.fields();
          return std::any_of(
              fields.begin(),
              fields.end(),
              [enabled_terse_write = has_compiler_option(
                   "deprecated_terse_writes")](auto& field) {
                return (!enabled_terse_write ||
                        !cpp2::deprecated_terse_writes(&field)) &&
                    !field.has_structured_annotation(
                        kCppDeprecatedTerseWriteUri) &&
                    field.qualifier() != t_field_qualifier::optional &&
                    field.qualifier() != t_field_qualifier::terse;
              });
        });
    def.property(
        "fields_with_runtime_annotation?", [](const t_structured& strct) {
          const auto& fields = strct.fields();
          return std::any_of(
              fields.begin(), fields.end(), has_runtime_annotation);
        });
    def.property(
        "extra_namespace",
        [this](const t_structured& strct) -> whisker::object {
          auto* extra = cpp_context_->resolver().get_extra_namespace(strct);
          return extra ? whisker::make::string(*extra) : whisker::make::null;
        });
    def.property("any?", [](const t_structured& strct) {
      return strct.uri() != "" &&
          !strct.has_unstructured_annotation("cpp.detail.no_any");
    });
    def.property("is_trivially_destructible?", [](const t_structured& strct) {
      for (const auto& field : strct.fields()) {
        const t_type* type = field.type()->get_true_type();
        if (cpp2::is_ref(&field) || cpp2::is_custom_type(field) ||
            !is_scalar(*type)) {
          return false;
        }
      }
      return true;
    });
    def.property("isset_fields", [&proto](const t_structured& strct) {
      whisker::array::raw fields;
      for (const auto& field : strct.fields()) {
        if (cpp2::field_has_isset(&field)) {
          fields.emplace_back(proto.create<t_field>(field));
        }
      }
      return whisker::make::array(std::move(fields));
    });
    def.property("mixin_fields", [](const t_structured& strct) {
      whisker::array::raw mixins;
      for (cpp2::mixin_member m : cpp2::get_mixins_and_members(strct)) {
        mixins.emplace_back(
            whisker::make::map(
                {{"mixin_name", whisker::make::string(m.mixin->name())},
                 {"field_name", whisker::make::string(m.member->name())}}));
      }
      return whisker::make::array(std::move(mixins));
    });
    def.property(
        "fields_with_runtime_annotation", [&proto](const t_structured& strct) {
          whisker::array::raw fields;
          for (const auto& field : strct.fields()) {
            if (has_runtime_annotation(field)) {
              fields.emplace_back(proto.create<t_field>(field));
            }
          }
          return whisker::make::array(std::move(fields));
        });
    def.property("fields_in_layout_order", [&](const t_structured& strct) {
      return to_array(
          cpp_context_->fields_in_layout_order(strct), proto.of<t_field>());
    });
    def.property(
        "explicitly_constructed_fields", [&](const t_structured& strct) {
          std::vector<const t_field*> filtered;
          for (const t_field* field :
               cpp_context_->fields_in_layout_order(strct)) {
            if (is_field_explicitly_constructed(*field, strct)) {
              filtered.emplace_back(field);
            }
          }
          return to_array(filtered, proto.of<t_field>());
        });

    return std::move(def).make();
  }

  prototype<t_union>::ptr make_prototype_for_union(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_union(proto);
    auto def =
        whisker::dsl::prototype_builder<h_union>::extends(std::move(base));
    def.property("min_union_member", &get_extremal_union_member<std::less<>>);
    def.property(
        "max_union_member", &get_extremal_union_member<std::greater<>>);
    return std::move(def).make();
  }

  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("cpp_qualified_namespace", [](const t_type& type) {
      return cpp2::get_gen_unprefixed_namespace(*type.program());
    });

    def.property("cpp_qualified_underlying_name", [this](const t_type& type) {
      return cpp_context_->resolver().get_underlying_namespaced_name(type);
    });

    def.property("resolves_to_complex_return?", [](const t_type& type) {
      return is_complex_return(type.get_true_type());
    });

    def.property("cpp_fullname", [this](const t_type& type) {
      return cpp_context_->resolver().get_namespaced_name(
          *type.program(), type);
    });

    def.property("cpp_type", [&](const t_type& type) {
      return cpp_context_->resolver().get_native_type(type);
    });

    def.property("cpp_standard_type", [&](const t_type& type) {
      return cpp_context_->resolver().get_standard_type(type);
    });

    def.property("cpp_adapter", [](const t_type& type) {
      const std::string* adapter = cpp_name_resolver::find_first_adapter(type);
      return adapter == nullptr ? whisker::make::null
                                : whisker::make::string(*adapter);
    });

    def.property("resolves_to_fixed_size?", [](const t_type& type) {
      const t_type* resolved = type.get_true_type();
      if (const auto* primitive = resolved->try_as<t_primitive_type>()) {
        switch (primitive->primitive_type()) {
          case t_primitive_type::type::t_bool:
          case t_primitive_type::type::t_byte:
          case t_primitive_type::type::t_i16:
          case t_primitive_type::type::t_i32:
          case t_primitive_type::type::t_i64:
          case t_primitive_type::type::t_float:
          case t_primitive_type::type::t_double:
            return true;
          default:
            return false;
        }
      }
      return resolved->is<t_enum>();
    });

    def.property("non_empty_struct?", [](const t_type& type) {
      const auto* as_struct = type.get_true_type()->try_as<t_structured>();
      return as_struct != nullptr && as_struct->has_fields();
    });

    def.property("type_class", [](const t_type& type) {
      return cpp2::get_gen_type_class(*type.get_true_type());
    });

    def.property("type_tag", [this](const t_type& type) {
      return cpp_context_->resolver().get_type_tag(type);
    });

    def.property("cpp_use_allocator?", [](const t_type& type) {
      return !!t_typedef::get_first_unstructured_annotation_or_null(
          &type, {"cpp.use_allocator"});
    });
    def.property("use_op_encode?", &needs_op_encode);
    def.property(
        "transitively_refers_to_struct?", &type_transitively_refers_to_struct);

    return std::move(def).make();
  }

  prototype<t_typedef>::ptr make_prototype_for_typedef(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_typedef(proto);
    auto def = whisker::dsl::prototype_builder<h_typedef>::extends(base);

    def.property("cpp_underlying_type", [&](const t_typedef& t) {
      return cpp_context_->resolver().get_underlying_type_name(t);
    });

    return std::move(def).make();
  }

  prototype<t_enum>::ptr make_prototype_for_enum(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_enum(proto);
    auto def = whisker::dsl::prototype_builder<h_enum>::extends(base);

    def.property("cpp_min", [](const t_enum& e) -> whisker::object {
      auto values = e.values();
      if (values.empty()) {
        return {};
      }
      auto min = std::min_element(
          values.begin(),
          values.end(),
          [](const t_enum_value& a, const t_enum_value& b) {
            return a.get_value() < b.get_value();
          });
      return whisker::object(cpp2::get_name(&*min));
    });

    def.property("cpp_max", [](const t_enum& e) -> whisker::object {
      auto values = e.values();
      if (values.empty()) {
        return {};
      }
      auto max = std::max_element(
          values.begin(),
          values.end(),
          [](const t_enum_value& a, const t_enum_value& b) {
            return a.get_value() < b.get_value();
          });
      return whisker::object(cpp2::get_name(&*max));
    });

    def.property("cpp_enum_type", [](const t_enum& e) -> std::string {
      if (const auto* annot =
              e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
        const auto& type = annot->get_value_from_structured_annotation("type");
        switch (static_cast<enum_underlying_type>(type.get_integer())) {
          case enum_underlying_type::i8:
            return "::std::int8_t";
          case enum_underlying_type::u8:
            return "::std::uint8_t";
          case enum_underlying_type::i16:
            return "::std::int16_t";
          case enum_underlying_type::u16:
            return "::std::uint16_t";
          case enum_underlying_type::u32:
            return "::std::uint32_t";
          default:
            throw std::runtime_error("unknown enum underlying type");
        }
      }
      return e.has_unstructured_annotation("cpp.deprecated_enum_unscoped")
          ? "int"
          : "";
    });

    def.property("cpp_is_unscoped", [](const t_enum& e) {
      return e.get_unstructured_annotation("cpp.deprecated_enum_unscoped");
    });

    def.property("cpp_declare_bitwise_ops", [](const t_enum& e) {
      return e.has_unstructured_annotation("cpp.declare_bitwise_ops") ||
          e.has_structured_annotation(kBitmaskEnumUri);
    });

    return std::move(def).make();
  }

  prototype<t_field>::ptr make_prototype_for_field(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_field(proto);
    auto def = whisker::dsl::prototype_builder<h_field>::extends(base);

    def.property("cpp_adapter", [](const t_field& self) {
      const std::string* adapter =
          cpp_name_resolver::find_structured_adapter_annotation(self);
      return adapter == nullptr ? whisker::make::null
                                : whisker::make::string(*adapter);
    });
    def.property("cpp_first_adapter", [](const t_field& self) {
      const std::string* adapter = cpp_name_resolver::find_first_adapter(self);
      return adapter == nullptr ? whisker::make::null
                                : whisker::make::string(*adapter);
    });
    def.property("name_hash", [](const t_field& field) {
      return fmt::format("__fbthrift_hash_{}", cpp2::sha256_hex(field.name()));
    });
    def.property("has_isset?", [](const t_field& field) {
      return cpp2::field_has_isset(&field);
    });
    def.property("isset_index", [this](const t_field& field) {
      const cpp2_field_generator_context* field_context =
          cpp_context_->get_field_context(&field);
      assert(field_context);
      return whisker::make::i64(field_context->isset_index);
    });
    def.property("cpp_type", [this](const t_field& field) {
      const t_structured* parent = context().get_field_parent(&field);
      assert(parent != nullptr);
      return cpp_context_->resolver().get_native_type(field, *parent);
    });
    def.property("cpp_storage_name", [this](const t_field& field) {
      const t_structured* strct = context().get_field_parent(&field);
      assert(strct != nullptr);
      if (!is_field_eligible_for_storage_name_mangling(
              *strct,
              field,
              has_compiler_option("deprecated_public_required_fields"))) {
        return cpp2::get_name(&field);
      }
      return mangle_field_name(cpp2::get_name(&field));
    });
    def.property("cpp_storage_type", [this](const t_field& field) {
      const t_structured* parent = context().get_field_parent(&field);
      assert(parent != nullptr);
      return cpp_context_->resolver().get_storage_type(field, *parent);
    });
    def.property("cpp_standard_type", [this](const t_field& field) {
      return cpp_context_->resolver().get_standard_type(field);
    });
    def.property(
        "eligible_for_storage_name_mangling?", [this](const t_field& field) {
          const t_structured* strct = context().get_field_parent(&field);
          assert(strct != nullptr);
          return is_field_eligible_for_storage_name_mangling(
              *strct,
              field,
              has_compiler_option("deprecated_public_required_fields"));
        });
    def.property("has_deprecated_accessors?", [this](const t_field& field) {
      return !cpp2::is_explicit_ref(&field) && !cpp2::is_lazy(&field) &&
          !cpp_name_resolver::find_first_adapter(field) &&
          !cpp_name_resolver::find_field_interceptor(field) &&
          !has_compiler_option("no_getters_setters") &&
          field.qualifier() != t_field_qualifier::terse;
    });
    def.property("cpp_ref?", [](const t_field& field) {
      return cpp2::is_explicit_ref(&field);
    });
    def.property("opt_cpp_ref?", [](const t_field& field) {
      return cpp2::is_explicit_ref(&field) &&
          field.qualifier() == t_field_qualifier::optional;
    });
    def.property("non_opt_cpp_ref?", [](const t_field& field) {
      return cpp2::is_explicit_ref(&field) &&
          field.qualifier() != t_field_qualifier::optional;
    });
    def.property("terse_cpp_ref?", [](const t_field& field) {
      return cpp2::is_explicit_ref(&field) &&
          field.qualifier() == t_field_qualifier::terse;
    });
    def.property("cpp_ref_unique?", [](const t_field& field) {
      return cpp2::is_unique_ref(&field);
    });
    def.property("cpp_ref_shared?", [](const t_field& field) {
      return gen::cpp::find_ref_type(field) ==
          gen::cpp::reference_type::shared_mutable;
    });
    def.property("cpp_ref_not_boxed?", [](const t_field& field) {
      auto ref_type = gen::cpp::find_ref_type(field);
      return ref_type != gen::cpp::reference_type::none &&
          ref_type != gen::cpp::reference_type::boxed &&
          ref_type != gen::cpp::reference_type::boxed_intern;
    });
    def.property("cpp_exactly_one_adapter?", [](const t_field& field) {
      bool hasFieldAdapter =
          cpp_name_resolver::find_structured_adapter_annotation(field);
      bool hasTypeAdapter =
          cpp_name_resolver::find_first_adapter(*field.type());
      return hasFieldAdapter != hasTypeAdapter;
    });
    def.property(
        "cpp_field_interceptor", [](const t_field& field) -> whisker::object {
          if (const std::string* interceptor =
                  cpp_name_resolver::find_field_interceptor(field)) {
            return whisker::make::string(*interceptor);
          }
          return whisker::make::null;
        });
    // The field accessor is inlined and erased by default, unless 'noinline' is
    // specified in FieldInterceptor.
    def.property("cpp_accessor_attribute", [](const t_field& field) {
      if (const t_const* annotation = field.find_structured_annotation_or_null(
              kCppFieldInterceptorUri)) {
        if (annotation->get_value_from_structured_annotation_or_null(
                "noinline")) {
          return whisker::make::string("FOLLY_NOINLINE");
        }
      }
      return whisker::make::string("FOLLY_ERASE");
    });
    def.property("cpp_noncopyable?", [](const t_field& field) {
      return field.type().get_type()->has_unstructured_annotation(
          {"cpp.noncopyable", "cpp2.noncopyable"});
    });
    def.property("zero_copy_arg", [](const t_field& field) -> std::string {
      return is_zero_copy_arg(*field.type()) ? "true" : "false";
    });
    def.property("visibility", [this](const t_field& field) -> std::string {
      return is_field_private(
                 field,
                 has_compiler_option("deprecated_public_required_fields"))
          ? "private"
          : "public";
    });
    def.property("metadata_name", [](const t_field& field) {
      auto key = field.id();
      auto suffix = key >= 0 ? std::to_string(key) : fmt::format("_{}", -key);
      return fmt::format("{}_{}", field.name(), suffix);
    });
    def.property(
        "lazy?", [](const t_field& field) { return cpp2::is_lazy(&field); });
    def.property("lazy_ref?", [](const t_field& field) {
      return cpp2::is_lazy_ref(&field);
    });
    def.property("boxed_ref?", [](const t_field& field) {
      return gen::cpp::find_ref_type(field) == gen::cpp::reference_type::boxed;
    });
    def.property("intern_boxed_ref?", [](const t_field& field) {
      return gen::cpp::find_ref_type(field) ==
          gen::cpp::reference_type::boxed_intern;
    });
    def.property("use_field_ref?", [](const t_field& field) {
      return gen::cpp::is_field_accessor_template(field);
    });
    def.property("field_ref_type", [this](const t_field& field) {
      return cpp_context_->resolver().get_reference_type(field);
    });
    def.property("transitively_refers_to_unique?", [](const t_field& field) {
      return cpp2::field_transitively_refers_to_unique(&field);
    });
    def.property("tablebased_qualifier", [](const t_field& field) {
      static const std::string kPrefix =
          "::apache::thrift::detail::FieldQualifier::";
      switch (field.qualifier()) {
        case t_field_qualifier::none:
        case t_field_qualifier::required:
          return fmt::format("{}Unqualified", kPrefix);
        case t_field_qualifier::optional:
          return fmt::format("{}Optional", kPrefix);
        case t_field_qualifier::terse:
          return fmt::format("{}Terse", kPrefix);
        default:
          throw std::runtime_error("unknown qualifier");
      }
    });
    def.property("type_tag", [this](const t_field& field) {
      const t_structured* parent = context().get_field_parent(&field);
      assert(parent != nullptr);
      return cpp_context_->resolver().get_type_tag(field, *parent);
    });
    def.property("raw_string_or_binary?", [](const t_field& field) {
      return field.type()->get_true_type()->is_string_or_binary() &&
          !cpp_name_resolver::find_first_adapter(field);
    });
    def.property("use_op_encode?", [this](const t_field& field) {
      const t_structured* parent = context().get_field_parent(&field);
      assert(parent != nullptr);
      return field_needs_op_encode(field, *parent);
    });
    def.property("serialization_prev_field", [&](const t_field& field) {
      const cpp2_field_generator_context* field_context =
          cpp_context_->get_field_context(&field);
      assert(field_context);
      return proto.create_nullable<t_field>(field_context->serialization_prev);
    });
    def.property("serialization_next_field", [&](const t_field& field) {
      const cpp2_field_generator_context* field_context =
          cpp_context_->get_field_context(&field);
      assert(field_context != nullptr);
      return proto.create_nullable<t_field>(field_context->serialization_next);
    });
    def.property("deprecated_terse_writes?", [this](const t_field& field) {
      return field.has_structured_annotation(kCppDeprecatedTerseWriteUri) ||
          (has_compiler_option("deprecated_terse_writes") &&
           cpp2::deprecated_terse_writes(&field));
    });
    def.property(
        "deprecated_terse_writes_with_non_redundant_custom_default?",
        [this](const t_field& field) {
          bool is_deprecated_terse =
              field.has_structured_annotation(kCppDeprecatedTerseWriteUri) ||
              (has_compiler_option("deprecated_terse_writes") &&
               cpp2::deprecated_terse_writes(&field));
          return is_deprecated_terse && field.default_value() &&
              !detail::is_initializer_default_value(
                     field.type().deref(), *field.default_value());
        });
    // Not optional, terse, or deprecated terse.
    def.property("fill?", [this](const t_field& field) {
      bool is_deprecated_terse =
          field.has_structured_annotation(kCppDeprecatedTerseWriteUri) ||
          (has_compiler_option("deprecated_terse_writes") &&
           cpp2::deprecated_terse_writes(&field));
      return (field.qualifier() == t_field_qualifier::none ||
              field.qualifier() == t_field_qualifier::required) &&
          !is_deprecated_terse;
    });

    return std::move(def).make();
  }

  prototype<t_function>::ptr make_prototype_for_function(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_function(proto);
    auto def = whisker::dsl::prototype_builder<h_function>::extends(base);

    def.property("cpp_return_type", [&](const t_function& f) -> std::string {
      return cpp_context_->resolver().get_return_type(f);
    });

    // Specifies if the generated recv_* functions have an additional argument
    // representing the return value.
    def.property("cpp_recv_arg?", [&](const t_function& f) {
      return !f.return_type()->is_void() || f.sink_or_stream();
    });

    def.property("stack_arguments?", [this](const t_function& function) {
      return cpp2::is_stack_arguments(compiler_options(), function);
    });

    def.property("event_based?", [this](const t_function& f) {
      const t_interface* parent = context().get_function_parent(&f);
      assert(parent != nullptr);
      return f.get_unstructured_annotation("thread") == "eb" ||
          f.has_structured_annotation(kCppProcessInEbThreadUri) ||
          parent->has_unstructured_annotation("process_in_event_base") ||
          parent->has_structured_annotation(kCppProcessInEbThreadUri);
    });

    def.property("sync_returns_by_outparam?", [](const t_function& f) {
      return is_complex_return(f.return_type()->get_true_type()) &&
          !f.interaction() && !f.sink_or_stream();
    });

    def.property("prefixed_name", [this](const t_function& f) {
      const t_interface* parent = context().get_function_parent(&f);
      assert(parent != nullptr);
      const std::string& name = cpp2::get_name(&f);
      return parent->is<t_interaction>()
          ? fmt::format("{}_{}", parent->name(), name)
          : name;
    });

    def.property(
        "has_deprecated_header_client_methods", [this](const t_function& f) {
          const t_interface* parent = context().get_function_parent(&f);
          assert(parent != nullptr);
          return f.has_structured_annotation(
                     kCppGenerateDeprecatedHeaderClientMethodsUri) ||
              f.has_unstructured_annotation(
                  "cpp.generate_deprecated_header_client_methods") ||
              parent->has_structured_annotation(
                  kCppGenerateDeprecatedHeaderClientMethodsUri) ||
              parent->has_unstructured_annotation(
                  "cpp.generate_deprecated_header_client_methods");
        });

    def.property("virtual_client_methods?", [this](const t_function& f) {
      const t_interface* parent = context().get_function_parent(&f);
      assert(parent != nullptr);
      return !generate_reduced_client(*parent) && !f.interaction() &&
          !f.is_bidirectional_stream();
    });

    def.property("legacy_client_methods?", [this](const t_function& f) {
      const t_interface* parent = context().get_function_parent(&f);
      assert(parent != nullptr);
      return !generate_reduced_client(*parent) && !f.interaction() &&
          !f.is_bidirectional_stream();
    });

    return std::move(def).make();
  }

  prototype<t_interface>::ptr make_prototype_for_interface(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interface(proto);
    auto def =
        whisker::dsl::prototype_builder<h_interface>::extends(std::move(base));
    def.property("reduced_client?", &generate_reduced_client);
    def.property("metadata_name", [](const t_interface& self) {
      return fmt::format("{}_{}", self.program()->name(), self.name());
    });
    return std::move(def).make();
  }

  std::vector<const t_function*> current_split_functions(
      const t_service& service) const {
    int split_count = cpp_context_->current_service_split_count();
    int split_id = cpp_context_->current_service_split_id();
    std::vector<const t_function*> result;
    for (size_t id = split_id; id < service.functions().size();
         id += split_count) {
      result.push_back(&service.functions()[id]);
    }
    return result;
  }

  prototype<t_service>::ptr make_prototype_for_service(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_service(proto);
    auto def = whisker::dsl::prototype_builder<h_service>::extends(base);

    def.property(
        "cpp_requires_method_decorator?", [](const t_service& service) {
          return service.has_structured_annotation(
              apache::thrift::compiler::kCppGenerateServiceMethodDecorator);
        });
    def.property("qualified_name", &cpp2::get_service_qualified_name);
    def.property("user_type_footprint", [&](const t_service& service) {
      return build_user_type_footprint(service, proto);
    });

    // Override `functions` from t_interface to respect service split state
    def.property("functions", [this, &proto](const t_service& service) {
      return to_array(current_split_functions(service), proto.of<t_function>());
    });
    def.property("has_sink_functions?", [this](const t_service& service) {
      std::vector<const t_function*> funcs = current_split_functions(service);
      return std::any_of(funcs.begin(), funcs.end(), [](const t_function* f) {
        return f->sink() != nullptr;
      });
    });
    def.property("has_stream_functions?", [this](const t_service& service) {
      std::vector<const t_function*> funcs = current_split_functions(service);
      return std::any_of(funcs.begin(), funcs.end(), [](const t_function* f) {
        return f->stream() != nullptr;
      });
    });

    return std::move(def).make();
  }

  prototype<t_interaction>::ptr make_prototype_for_interaction(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_interaction(proto);
    auto def = whisker::dsl::prototype_builder<h_interaction>::extends(base);
    def.property("event_base?", [](const t_interaction& self) {
      return self.has_unstructured_annotation("process_in_event_base") ||
          self.has_structured_annotation(kCppProcessInEbThreadUri);
    });
    def.property("serial?", [](const t_interaction& self) {
      return self.has_unstructured_annotation("serial") ||
          self.has_structured_annotation(kSerialUri);
    });
    // Interactions don't get split, so check all functions
    def.property("has_sink_functions?", [](const t_interaction& self) {
      return std::any_of(
          self.functions().begin(),
          self.functions().end(),
          [](const t_function& f) { return f.sink() != nullptr; });
    });
    def.property("has_stream_functions?", [](const t_interaction& self) {
      return std::any_of(
          self.functions().begin(),
          self.functions().end(),
          [](const t_function& f) { return f.stream() != nullptr; });
    });
    return std::move(def).make();
  }

  prototype<t_const>::ptr make_prototype_for_const(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const(proto);
    auto def = whisker::dsl::prototype_builder<h_const>::extends(base);
    def.property("external?", [this](const t_const& self) {
      return self.program() != program_;
    });
    def.property("outline_init?", [this](const t_const& self) {
      return resolves_to_container_or_struct(self.type()->get_true_type()) ||
          cpp_context_->resolver().find_structured_adapter_annotation(self) ||
          cpp_context_->resolver().find_first_adapter(*self.type());
    });
    def.property("cpp_adapter", [this](const t_const& self) {
      const std::string* adapter =
          cpp_context_->resolver().find_structured_adapter_annotation(self);
      return adapter == nullptr ? whisker::make::null
                                : whisker::make::string(*adapter);
    });
    def.property("cpp_type", [this](const t_const& self) {
      return cpp_context_->resolver().get_native_type(self);
    });
    def.property("extra_arg", [&proto](const t_const& self) {
      return proto.create_nullable<t_const>(
          cpp2::get_transitive_annotation_of_adapter_or_null(self));
    });
    return std::move(def).make();
  }

  prototype<t_const_value>::ptr make_prototype_for_const_value(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_const_value(proto);
    auto def = whisker::dsl::prototype_builder<h_const_value>::extends(base);

    // This is an override of referenceable_from? in the base whisker
    // generator's t_const_value prototype, with an additional condition on the
    // value type which is specific to the cpp2 generator
    def.function(
        "referenceable_from?",
        [](const t_const_value& self, whisker::dsl::function::context ctx) {
          ctx.declare_arity(1);
          ctx.declare_named_arguments({});
          const t_const* from_const =
              ctx.raw().positional_arguments()[0].is_null()
              ? nullptr
              : ctx.argument<whisker::native_handle<t_const>>(0).ptr().get();
          const t_const* owner = self.get_owner();
          // value can be referenced if it is not anonymous, and is being
          // referenced from any const that's not the owner and the type is what
          // we expect it to be
          return owner != nullptr && owner != from_const &&
              same_types(
                     self.type().empty() ? nullptr : &self.type().deref(),
                     owner->type());
        });

    return std::move(def).make();
  }

  std::unordered_map<std::string, int> get_client_name_to_split_count() const;

  std::unique_ptr<cpp2_generator_context> cpp_context_;
  std::unordered_map<std::string, int32_t> client_name_to_split_count_;
  bool cpp_enable_same_program_const_referencing_ = true;
  mutable cpp2::is_eligible_for_constexpr is_eligible_for_constexpr_;
};

class cpp_mstch_program : public mstch_program {
 public:
  cpp_mstch_program(
      const t_program* program,
      mstch_context& ctx,
      mstch_element_position pos,
      const cpp2_generator_context* cpp_context)
      : mstch_program(program, ctx, pos), cpp_context_(*cpp_context) {
    register_methods(
        this,
        {{"program:split_structs",
          {with_no_caching, &cpp_mstch_program::split_structs}},
         {"program:split_enums",
          {with_no_caching, &cpp_mstch_program::split_enums}},
         {"program:structs_and_typedefs",
          &cpp_mstch_program::structs_and_typedefs}});
  }
  std::string get_program_namespace(const t_program* program) override {
    return cpp2::get_gen_namespace(*program);
  }
  mstch::node structs_and_typedefs() {
    // Equivalent Whisker property: `type_definitions_topological_order`
    const std::vector<const t_type*>& sorted =
        cpp_context_.type_definitions_topological_order(*program_);

    // Generate the sorted nodes
    mstch::array ret;
    ret.reserve(sorted.size());
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    std::transform(
        sorted.begin(),
        sorted.end(),
        std::back_inserter(ret),
        [&](const t_type* node) -> mstch::node {
          if (auto typedf = node->try_as<t_typedef>()) {
            return context_.typedef_factory->make_mstch_object(
                typedf, context_);
          }
          return make_mstch_element_cached(
              static_cast<const t_structured*>(node),
              *context_.struct_factory,
              context_.struct_cache,
              id,
              0,
              0);
        });
    return ret;
  }

  mstch::node split_structs() {
    // Equivalent Whisker property: `current_split_structured_definitions`
    if (std::optional<int> split_id = cpp_context_.program_split_id()) {
      return make_mstch_array(
          cpp_context_.program_current_split_structured_definitions(),
          *context_.struct_factory);
    }
    return make_mstch_array_cached(
        program_->structured_definitions(),
        *context_.struct_factory,
        context_.struct_cache,
        program_cache_id(program_, get_program_namespace(program_)));
  }

  mstch::node split_enums() {
    // Equivalent Whisker property: `current_split_enums`
    if (std::optional<int> split_id = cpp_context_.program_split_id()) {
      return make_mstch_array(
          cpp_context_.program_current_split_enums(), *context_.enum_factory);
    }
    std::string id =
        program_cache_id(program_, get_program_namespace(program_));
    return make_mstch_array_cached(
        program_->enums(), *context_.enum_factory, context_.enum_cache, id);
  }

 private:
  const cpp2_generator_context& cpp_context_;
};

// Retained for `get_functions` override, which overrides mstch_service's
// `service:functions`, during Whisker migration.
class cpp_mstch_service : public mstch_service {
 public:
  cpp_mstch_service(
      const t_service* service,
      mstch_context& ctx,
      mstch_element_position pos,
      const cpp2_generator_context* cpp_context,
      const t_service* containing_service = nullptr)
      : mstch_service(service, ctx, pos, containing_service),
        cpp_context_(*cpp_context) {}

 private:
  const std::vector<const t_function*>& get_functions() const override {
    int split_count = cpp_context_.current_service_split_count();
    if (split_count <= 1) {
      return mstch_service::get_functions();
    }
    int split_id = cpp_context_.current_service_split_id();
    // TODO(T256504524): This is a very temporary hack as an intermediate step
    // of migrating to Whisker, because `get_functions()` requires the return
    // value to be a reference. Once the cpp2 migration is complete, this and
    // `mstch_service::get_functions` are getting nuked from orbit (this is the
    // only override).
    if (split_functions_.empty() || cached_split_id_ != split_id) {
      split_functions_.clear();
      for (size_t id = split_id; id < service_->functions().size();
           id += split_count) {
        split_functions_.push_back(&service_->functions()[id]);
      }
      cached_split_id_ = split_id;
    }
    return split_functions_;
  }

  const cpp2_generator_context& cpp_context_;
  mutable std::vector<const t_function*> split_functions_;
  mutable int32_t cached_split_id_ = -1;
};

// Retained for cpp_mstch_service's `get_functions` override, which overrides
// mstch_service's `service:functions`, during Whisker migration.
class cpp_mstch_interaction : public cpp_mstch_service {
 public:
  using ast_type = t_interaction;

  cpp_mstch_interaction(
      const t_interaction* interaction,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_service* containing_service,
      const cpp2_generator_context* cpp_context)
      : cpp_mstch_service(
            interaction, ctx, pos, cpp_context, containing_service) {}
};

bool check_container_needs_op_encode(const t_type& type) {
  const auto* true_type = type.get_true_type();
  if (auto list_container = true_type->try_as<t_list>()) {
    return needs_op_encode(*list_container->elem_type());
  } else if (auto set_container = true_type->try_as<t_set>()) {
    return needs_op_encode(*set_container->elem_type());
  } else if (auto map_container = true_type->try_as<t_map>()) {
    return needs_op_encode(*map_container->key_type()) ||
        needs_op_encode(*map_container->val_type());
  }
  return false;
}

bool needs_op_encode(const t_type& type) {
  return (type.program() &&
          type.program()->inherit_annotation_or_null(
              type, kCppUseOpEncodeUri)) ||
      t_typedef::get_first_structured_annotation_or_null(
             &type, kCppUseOpEncodeUri) ||
      cpp_name_resolver::find_first_adapter(type) ||
      check_container_needs_op_encode(type);
}

// Enable `@cpp.UseOpEncode` for following fields:
// - A package is annotated with `@cpp.UseOpEncode`
// - A parent struct is annotated with `@cpp.UseOpEncode`
// - A container has a key or element type marked with `@cpp.UseOpEncode`
// - A container has an adapted key or element type.
// - A field is adapted.
bool field_needs_op_encode(const t_field& field, const t_structured& strct) {
  return (strct.program() &&
          strct.program()->inherit_annotation_or_null(
              strct, kCppUseOpEncodeUri)) ||
      strct.has_structured_annotation(kCppUseOpEncodeUri) ||
      cpp_name_resolver::find_first_adapter(field) ||
      check_container_needs_op_encode(*field.type());
}

class cpp_mstch_struct : public mstch_struct {
 public:
  cpp_mstch_struct(
      const t_structured* s,
      mstch_context& ctx,
      mstch_element_position pos,
      const cpp2_generator_context* cpp_context)
      : mstch_struct(s, ctx, pos), cpp_context_(*cpp_context) {
    register_methods(
        this,
        {
            {"struct:fields_in_layout_order",
             &cpp_mstch_struct::fields_in_layout_order},
        });
  }

 private:
  const cpp2_generator_context& cpp_context_;

  mstch::node fields_in_layout_order() {
    // TODO(T256504524): An equivalent Whisker property has been created, but we
    // first need to migrate templates that rely on mstch behavior like implicit
    // `first?`/`last?` properties
    return make_mstch_fields(cpp_context_.fields_in_layout_order(*struct_));
  }
};

// Computes the alignment of field on the target platform.
// Throws exception if cannot compute the alignment.
size_t compute_alignment(const t_enum& e) {
  if (const auto* annot =
          e.find_structured_annotation_or_null(kCppEnumTypeUri)) {
    const auto& type = annot->get_value_from_structured_annotation("type");
    switch (static_cast<enum_underlying_type>(type.get_integer())) {
      case enum_underlying_type::i8:
      case enum_underlying_type::u8:
        return 1;
      case enum_underlying_type::i16:
      case enum_underlying_type::u16:
        return 2;
      case enum_underlying_type::u32:
        return 4;
      default:
        throw std::runtime_error("unknown enum underlying type");
    }
  }
  return 4;
}

size_t compute_alignment(
    const t_field* field, std::unordered_map<const t_field*, size_t>& memo) {
  const size_t kMaxAlign = alignof(std::max_align_t);
  auto find = memo.emplace(field, 0);
  auto& ret = find.first->second;
  if (!find.second) {
    return ret;
  }
  if (cpp2::is_ref(field)) {
    return ret = 8;
  }
  if (cpp2::is_custom_type(*field)) {
    return ret = kMaxAlign;
  }

  const t_type* type = field->type()->get_true_type();

  size_t result = type->visit(
      [&](const t_primitive_type& primitive) -> size_t {
        switch (primitive.primitive_type()) {
          case t_primitive_type::type::t_bool:
          case t_primitive_type::type::t_byte:
            return 1;
          case t_primitive_type::type::t_i16:
            return 2;
          case t_primitive_type::type::t_i32:
          case t_primitive_type::type::t_float:
            return 4;
          case t_primitive_type::type::t_i64:
          case t_primitive_type::type::t_double:
          case t_primitive_type::type::t_string:
          case t_primitive_type::type::t_binary:
            return 8;
          default:
            throw std::logic_error(
                "Computing alignment of unknown primitive type");
        }
      },
      [&](const t_enum& enm) -> size_t { return compute_alignment(enm); },
      [&](const t_container&) -> size_t { return 8; },
      [&](const t_structured& structured) {
        // The type member of a union is an int
        // The __isset member generated in presence of non-required fields of
        // structs/exns only has bool fields so its alignment is 1
        size_t align = structured.is<t_union>() ? 4 : 1;
        for (const auto& field_2 : structured.fields()) {
          size_t field_align = compute_alignment(&field_2, memo);
          align = std::max(align, field_align);
          if (align == kMaxAlign) {
            // No need to continue because the structured already has the
            // maximum alignment.
            break;
          }
        }
        return align;
      },
      [&](const t_service&) -> size_t {
        throw std::logic_error("Computing alignment of service");
      },
      [&](const t_typedef&) -> size_t {
        throw std::logic_error("Unreachable: typedefs resolved above");
      });

  return ret = result;
}

// Returns the struct members reordered to minimize padding if the
// @cpp.MinimizePadding annotation is specified.
std::vector<const t_field*> get_structured_fields_in_layout_order(
    const t_structured& strct) {
  if (!strct.has_unstructured_annotation("cpp.minimize_padding") &&
      !strct.has_structured_annotation(kCppMinimizePaddingUri)) {
    return strct.fields().copy();
  }

  // Compute field alignments.
  struct FieldAlign {
    const t_field* field = nullptr;
    size_t align = 0;
  };
  std::vector<FieldAlign> field_alignments;
  field_alignments.reserve(strct.fields().size());
  std::unordered_map<const t_field*, size_t> memo;
  for (const auto& field : strct.fields()) {
    size_t align = compute_alignment(&field, memo);
    assert(align);
    field_alignments.push_back(FieldAlign{&field, align});
  }

  // Sort by decreasing alignment using stable sort to avoid unnecessary
  // reordering.
  std::stable_sort(
      field_alignments.begin(),
      field_alignments.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.align > rhs.align; });

  // Construct the reordered field vector.
  std::vector<const t_field*> result;
  result.reserve(strct.fields().size());
  std::transform(
      field_alignments.begin(),
      field_alignments.end(),
      std::back_inserter(result),
      [](const FieldAlign& fa) { return fa.field; });
  return result;
}

void t_mstch_cpp2_generator::generate_program() {
  const auto* program = get_program();
  set_mstch_factories();

  generate_sinit(program);
  generate_structs(program);
  generate_constants(program);
  if (has_option("single_file_service")) {
    generate_inline_services(program->services());
  } else {
    generate_out_of_line_services(program->services());
  }
  generate_metadata(program);
  generate_visitation();
}

void t_mstch_cpp2_generator::set_mstch_factories() {
  mstch_context_.add<cpp_mstch_program>(cpp_context_.get());
  mstch_context_.add<cpp_mstch_service>(cpp_context_.get());
  mstch_context_.add<cpp_mstch_interaction>(cpp_context_.get());
  mstch_context_.add<cpp_mstch_struct>(cpp_context_.get());
}

void t_mstch_cpp2_generator::generate_constants(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_constants.h", name + "_constants.h");
  render_to_file(prog, "module_constants.cpp", name + "_constants.cpp");
}

void t_mstch_cpp2_generator::generate_metadata(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_metadata.h", name + "_metadata.h");
  if (!has_option("no_metadata")) {
    render_to_file(prog, "module_metadata.cpp", name + "_metadata.cpp");
  }
}

void t_mstch_cpp2_generator::generate_sinit(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_sinit.cpp", name + "_sinit.cpp");
}

void t_mstch_cpp2_generator::generate_visitation() {
  const std::string& name = program_->name();
  render_whisker_file("module_visitation.h", name + "_visitation.h");
  render_whisker_file("module_for_each_field.h", name + "_for_each_field.h");
  render_whisker_file("module_visit_union.h", name + "_visit_union.h");
  render_whisker_file(
      "module_visit_by_thrift_field_metadata.h",
      name + "_visit_by_thrift_field_metadata.h");
}

void t_mstch_cpp2_generator::generate_structs(const t_program* program) {
  const auto& name = program->name();
  const auto& prog = cached_program(program);

  render_to_file(prog, "module_data.h", name + "_data.h");
  render_to_file(prog, "module_data.cpp", name + "_data.cpp");

  // module_types.h is an exception to same program const referencing, because
  // module_constants.h (where const accessors are declared) depends on
  // module_types.h, so module_types.h cannot include module_constants.h without
  // a circular dependency. This restriction only applies within the same
  // program - module_types.h CAN include and reference consts from other
  // programs.
  cpp_enable_same_program_const_referencing_ = false;
  render_to_file(prog, "module_types.h", name + "_types.h");
  cpp_enable_same_program_const_referencing_ = true;

  render_to_file(prog, "module_types_fwd.h", name + "_types_fwd.h");
  render_to_file(prog, "module_types.tcc", name + "_types.tcc");

  if (int split_count = get_split_count(options())) {
    auto digit = std::to_string(split_count - 1).size();
    for (int split_id = 0; split_id < split_count; ++split_id) {
      auto s = std::to_string(split_id);
      s = std::string(digit - s.size(), '0') + s;
      cpp_context_->set_program_split(split_id);
      render_to_file(
          prog, "module_types.cpp", name + "_types." + s + ".split.cpp");
      render_to_file(
          prog,
          "module_types_binary.cpp",
          name + "_types_binary." + s + ".split.cpp");
      render_to_file(
          prog,
          "module_types_compact.cpp",
          name + "_types_compact." + s + ".split.cpp");
      render_to_file(
          prog,
          "module_types_serialization.cpp",
          name + "_types_serialization." + s + ".split.cpp");
    }
    cpp_context_->clear_program_split();
  } else {
    render_to_file(prog, "module_types.cpp", name + "_types.cpp");
    render_to_file(prog, "module_types_binary.cpp", name + "_types_binary.cpp");
    render_to_file(
        prog, "module_types_compact.cpp", name + "_types_compact.cpp");
    render_to_file(
        prog,
        "module_types_serialization.cpp",
        name + "_types_serialization.cpp");
  }

  render_to_file(
      prog,
      "module_types_custom_protocol.h",
      name + "_types_custom_protocol.h");
  if (has_option("frozen2")) {
    render_to_file(prog, "module_layouts.h", name + "_layouts.h");
    render_to_file(prog, "module_layouts.cpp", name + "_layouts.cpp");
  }
}

void t_mstch_cpp2_generator::generate_out_of_line_service(
    const t_service* service) {
  const auto& name = service->name();
  auto mstch_service =
      make_mstch_service_cached(get_program(), service, mstch_context_);

  mstch::map context = {
      {"program", cached_program(get_program())},
      {"service", mstch_service},
  };

  render_to_file(mstch_service, "ServiceAsyncClient.h", name + "AsyncClient.h");
  render_to_file(context, "service.cpp", name + ".cpp");
  render_to_file(mstch_service, "service.h", name + ".h");
  render_to_file(mstch_service, "service.tcc", name + ".tcc");
  render_to_file(
      mstch_service, "types_custom_protocol.h", name + "_custom_protocol.h");

  auto iter = client_name_to_split_count_.find(name);
  if (iter != client_name_to_split_count_.end()) {
    auto split_count = iter->second;
    auto digit = std::to_string(split_count - 1).size();
    for (int split_id = 0; split_id < split_count; ++split_id) {
      auto s = std::to_string(split_id);
      s = std::string(digit - s.size(), '0') + s;
      cpp_context_->set_service_split(split_id, split_count);
      // We need to create a fresh `cpp_mstch_service` for every iteration,
      // because properties on the base `mstch_service` get cached but we need
      // volatile behavior for anything that calls `get_functions` because the
      // split ID changes the functions returned.
      // Once the relevant properties/templates are migrated to Whisker, this
      // will no longer be necessary.
      render_to_file(
          std::make_shared<cpp_mstch_service>(
              service,
              mstch_context_,
              mstch_element_position{},
              cpp_context_.get(),
              /*containing_service=*/nullptr),
          "ServiceAsyncClient.cpp",
          name + "." + s + ".async_client_split.cpp");
      cpp_context_->clear_service_split();
    }
  } else {
    render_to_file(
        mstch_service, "ServiceAsyncClient.cpp", name + "AsyncClient.cpp");
  }

  for (const char* protocol : {"binary", "compact"}) {
    render_to_file(
        mstch_service,
        "service_processmap_protocol.cpp",
        name + "_processmap_" + protocol + ".cpp");
  }
}

void t_mstch_cpp2_generator::generate_out_of_line_services(
    const std::vector<t_service*>& services) {
  for (const auto* service : services) {
    generate_out_of_line_service(service);
  }

  mstch::array mstch_services;
  mstch_services.reserve(services.size());
  for (const t_service* service : services) {
    mstch_services.emplace_back(
        make_mstch_service_cached(get_program(), service, mstch_context_));
  }
  mstch::map context{
      {"services", std::move(mstch_services)},
  };
  const auto& module_name = get_program()->name();
  render_to_file(
      context, "module_handlers_out_of_line.h", module_name + "_handlers.h");
  render_to_file(
      context, "module_clients_out_of_line.h", module_name + "_clients.h");
  render_to_file(
      context, "module_clients_fwd.h", module_name + "_clients_fwd.h");
}

void t_mstch_cpp2_generator::generate_inline_services(
    const std::vector<t_service*>& services) {
  mstch::array mstch_services;
  mstch_services.reserve(services.size());
  for (const t_service* service : services) {
    mstch_services.emplace_back(
        make_mstch_service_cached(get_program(), service, mstch_context_));
  }
  auto any_service_has_any_function = [&](auto&& predicate) -> bool {
    return std::any_of(
        services.cbegin(), services.cend(), [&](const t_service* service) {
          auto funcs = service->functions();
          return std::any_of(
              funcs.cbegin(), funcs.cend(), [&](auto const& func) {
                return predicate(func);
              });
        });
  };
  auto has_method_decorator = std::any_of(
      services.cbegin(), services.cend(), [&](const t_service* service) {
        return service->has_structured_annotation(
            apache::thrift::compiler::kCppGenerateServiceMethodDecorator);
      });

  mstch::map context = {
      {"program", cached_program(get_program())},
      {"any_sinks?",
       any_service_has_any_function(std::mem_fn(&t_function::sink))},
      {"any_streams?",
       any_service_has_any_function(std::mem_fn(&t_function::stream))},
      {"any_interactions?",
       any_service_has_any_function([](const t_function& func) {
         return func.is_interaction_constructor() || func.interaction();
       })},
      {"any_method_decorators?", has_method_decorator},
      {"services", std::move(mstch_services)},
  };
  const auto& module_name = get_program()->name();
  render_to_file(context, "module_clients.h", module_name + "_clients.h");
  render_to_file(
      context, "module_clients_fwd.h", module_name + "_clients_fwd.h");
  render_to_file(context, "module_clients.cpp", module_name + "_clients.cpp");
  render_to_file(
      context, "module_handlers-inl.h", module_name + "_handlers-inl.h");
  render_to_file(context, "module_handlers.h", module_name + "_handlers.h");
  render_to_file(context, "module_handlers.cpp", module_name + "_handlers.cpp");
}

std::string t_mstch_cpp2_generator::include_prefix(
    const t_program* program, const compiler_options_map& options) {
  const std::string& prefix = program->include_prefix();
  std::string include_prefix;
  if (const auto& it = options.find("include_prefix"); it != options.end()) {
    include_prefix = it->second;
  }
  auto out_dir_base = get_out_dir_base(options);
  if (prefix.empty()) {
    if (include_prefix.empty()) {
      return prefix;
    } else {
      return include_prefix + "/" + out_dir_base + "/";
    }
  }
  if (std::filesystem::path(prefix).has_root_directory()) {
    return include_prefix + "/" + out_dir_base + "/";
  }
  return prefix + out_dir_base + "/";
}

static auto split(const std::string& s, char delimiter) {
  std::vector<std::string> ret;
  boost::algorithm::split(ret, s, [&](char c) { return c == delimiter; });
  return ret;
}

std::unordered_map<std::string, int>
t_mstch_cpp2_generator::get_client_name_to_split_count() const {
  auto client_cpp_splits = get_option("client_cpp_splits");
  if (!client_cpp_splits) {
    return {};
  }

  auto map = *client_cpp_splits;
  if (map.size() < 2 || map[0] != '{' || *map.rbegin() != '}') {
    throw std::runtime_error("Invalid client_cpp_splits value: `" + map + "`");
  }
  map = map.substr(1, map.size() - 2);
  if (map.empty()) {
    return {};
  }
  std::unordered_map<std::string, int> ret;
  for (const auto& kv : split(map, ',')) {
    auto a = split(kv, ':');
    if (a.size() != 2) {
      throw std::runtime_error(
          "Invalid pair `" + kv + "` in client_cpp_splits value: `" + map +
          "`");
    }
    ret[a[0]] = checked_stoi(
        a[1],
        "Invalid pair `" + kv + "` in client_cpp_splits value: `" + map + "`");
  }
  return ret;
}

// Make sure there is no incompatible annotation.
void validate_struct_annotations(
    sema_context& ctx,
    const t_structured& s,
    const t_mstch_generator::compiler_options_map& options) {
  if (cpp2::packed_isset(s)) {
    if (options.count("tablebased") != 0) {
      ctx.report(
          s,
          "tablebased-isset-bitpacking-rule",
          diagnostic_level::error,
          "Tablebased serialization is incompatible with isset bitpacking for struct `{}`",
          s.name());
    }
  }

  for (const auto& field : s.fields()) {
    if (cpp2::is_mixin(field)) {
      // Mixins cannot be refs
      if (cpp2::is_explicit_ref(&field)) {
        ctx.report(
            field,
            "mixin-ref-rule",
            diagnostic_level::error,
            "Mixin field `{}` can not be a ref in cpp.",
            field.name());
      }
    }
  }
}

class validate_splits {
 public:
  explicit validate_splits(
      int split_count,
      const std::unordered_map<std::string, int>& client_name_to_split_count)
      : split_count_(split_count),
        client_name_to_split_count_(client_name_to_split_count) {}

  void operator()(sema_context& ctx, const t_program& program) {
    validate_type_cpp_splits(
        program.structured_definitions().size() + program.enums().size(),
        ctx,
        program);
    validate_client_cpp_splits(program.services(), ctx);
  }

 private:
  int split_count_ = 0;
  std::unordered_map<std::string, int> client_name_to_split_count_;

  void validate_type_cpp_splits(
      const int32_t object_count, sema_context& ctx, const t_program& program) {
    if (split_count_ > object_count) {
      ctx.report(
          program,
          "more-splits-than-objects-rule",
          diagnostic_level::error,
          "`types_cpp_splits={}` is misconfigured: it can not be greater "
          "than the number of objects, which is {}.",
          split_count_,
          object_count);
    }
  }

  void validate_client_cpp_splits(
      const std::vector<t_service*>& services, sema_context& ctx) {
    if (client_name_to_split_count_.empty()) {
      return;
    }
    for (const t_service* s : services) {
      auto iter = client_name_to_split_count_.find(s->name());
      if (iter != client_name_to_split_count_.end() &&
          iter->second > static_cast<int32_t>(s->functions().size())) {
        ctx.report(
            *s,
            "more-splits-than-functions-rule",
            diagnostic_level::error,
            "`client_cpp_splits={}` (For service {}) is misconfigured: it "
            "can not be greater than the number of functions, which is {}.",
            iter->second,
            s->name(),
            s->functions().size());
      }
    }
  }
};

void forbid_deprecated_terse_writes_ref(
    sema_context& ctx,
    const t_structured& strct,
    const t_mstch_generator::compiler_options_map& options) {
  for (auto& field : strct.fields()) {
    const bool isUniqueRef =
        gen::cpp::find_ref_type(field) == gen::cpp::reference_type::unique;
    const bool isDeprecatedTerseWrites =
        field.qualifier() == t_field_qualifier::none &&
        (options.count("deprecated_terse_writes") ||
         field.has_structured_annotation(kCppDeprecatedTerseWriteUri));

    if (field.has_structured_annotation(
            kCppAllowLegacyDeprecatedTerseWritesRefUri)) {
      if (!isUniqueRef) {
        ctx.report(
            field,
            diagnostic_level::error,
            "@cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `{}`"
            " since it's not cpp.Ref{{Unique}} field.",
            field.name());
      }
      if (!isDeprecatedTerseWrites) {
        ctx.report(
            field,
            diagnostic_level::error,
            "@cpp.AllowLegacyDeprecatedTerseWritesRef can not be applied to `{}`"
            " since it's not cpp.DeprecatedTerseWrite field.",
            field.name());
      }
      continue;
    }

    if (!isUniqueRef || !isDeprecatedTerseWrites) {
      continue;
    }

    ctx.report(
        field,
        diagnostic_level::error,
        "@cpp.Ref{{Unique}} can not be applied to `{}`"
        " since it's cpp.DeprecatedTerseWrite field.",
        field.name());
  }
}

void validate_lazy_fields(sema_context& ctx, const t_field& field) {
  if (cpp2::is_lazy(&field)) {
    auto t = field.type()->get_true_type();
    const char* field_type = nullptr;
    if (t->is_any_int() || t->is_bool() || t->is_byte()) {
      field_type = "Integral field";
    }
    if (t->is_floating_point()) {
      field_type = "Floating point field";
    }
    if (field_type) {
      ctx.report(
          field,
          "no-lazy-int-float-field-rule",
          diagnostic_level::error,
          "{} `{}` can not be marked as lazy, since doing so won't bring "
          "any benefit.",
          field_type,
          field.name());
    }
  }
}

// TODO(dokwon): Remove this validation once `deprecated_terse_writes` cpp2
// options are completely removed.
void validate_deprecated_terse_writes(
    sema_context& ctx,
    const t_field& field,
    const t_mstch_generator::compiler_options_map& options) {
  if (options.count("deprecated_terse_writes") != 0 &&
      field.has_structured_annotation(kCppDeprecatedTerseWriteUri)) {
    ctx.error(
        "Cannot use thrift_cpp2_options `deprecated_terse_writes` with @cpp.DeprecatedTerseWrite.");
  }
}

void t_mstch_cpp2_generator::fill_validator_visitors(
    ast_validator& validator) const {
  validator.add_structured_definition_visitor(
      std::bind(
          validate_struct_annotations,
          std::placeholders::_1,
          std::placeholders::_2,
          options()));
  validator.add_struct_visitor(
      std::bind(
          forbid_deprecated_terse_writes_ref,
          std::placeholders::_1,
          std::placeholders::_2,
          options()));
  validator.add_program_visitor(
      validate_splits(get_split_count(options()), client_name_to_split_count_));
  validator.add_field_visitor(validate_lazy_fields);
  validator.add_field_visitor(
      std::bind(
          validate_deprecated_terse_writes,
          std::placeholders::_1,
          std::placeholders::_2,
          options()));
}

THRIFT_REGISTER_GENERATOR(
    mstch_cpp2, "cpp2", R"(    (NOTE: the list below may not be exhaustive)
    any
      Register types with the AnyRegistry.
    client_cpp_splits={[<service name:str>:<split count:int>[,...]*]}
      Enable splitting of client method .cpp files (into N
      *.async_client_split.cpp" files). The given split count cannot be greater
      than the number of methods in the corresponding service. See also
      types_cpp_splits below.
    deprecated_clear
      Use the deprecated semantics for "clearing" Thrift structs, which assigns
      the *standard* default value instead of the *intrinsic* defaults (see
      https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#default-values).
    deprecated_enforce_required
      Enforce required fields (deprecated since 2019).
    deprecated_public_required_fields
      Make member variables corresponding to required fields public instead of
      private. In addition to exposing directly the field (which is unsafe to
      begin with), this prevents the generation of the reference accessors
      that do not have the _ref() suffix.
    deprecated_terse_writes
      Enable deprecated terse writes, which are discouraged in favor of
      @thrift.TerseWrite. See:
      https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/field-qualifiers.md#terse-writes-compiler-option
    disable_custom_type_ordering_if_structure_has_uri (IGNORED - ALWAYS SET)
      Without this option, custom set/map are considered orderable if parent structure has uri.
    frozen[=packed]
      Enable frozen structs. If the packed parameter is given, structure members
      will be packed with an alignment of 1 (i.e., #pragma pack(push, 1)).
      NOTE: this capability is not actively maintained. Use at your own risks.
    frozen2
      Enable frozen2 (see https://fburl.com/thrift_frozen2).
      NOTE: this capability is not actively maintained. Use at your own risks.
    includes=<extra_include:str>:...
      Add cpp_include for each of the given values.
    include_prefix
      Override the "include prefix" for all generated files, i.e. the directory
      from which application code should include headers, typically:
      <include_prefix>/gen-cpp2/...
    json
      Enable SimpleJson serialization.
    no_getters_setters
      Do not generate (deprecated) field getter and setter methods, even when
      it would be possible to do so. This is enouraged, and eventually will be
      enabled by default as getters and setters are deprecated in favor of field
      references (i.e., field() or field_ref() methods). Other conditions that
      would prevent getters/setters from being generated (even if this option is
      not enabled) include if the corresponding field: is a reference field
      (@cpp.Ref, cpp[2].ref_[type]), is adapted (@cpp.Adapter), is lazy
      (@cpp.Lazy), has a @cpp.FieldIntercaptor or is terse.
    no_metadata
      Generate empty metadata, do not generate _metadata.cpp.
    py3cpp
      if specified, output folder is "gen-py3cpp" instead of "gen-cpp2".
    single_file_service
      Generate all RPC services and client code in a single file, respectively.
    sync_methods_return_try
      Generate (deprecated) sync code for RPC methods that returns a folly::Try.
    tablebased
      Enable the table-based serialization.
    types_cpp_splits=<split_count:int>
      Enable splitting of type .cpp files (into the given number of files).
      Cannot be greater than the number of objects. See also client_cpp_splits
      above.
)"

);

} // namespace
} // namespace apache::thrift::compiler
