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

#include <thrift/compiler/parse/parse_ast.h>

#include <stdlib.h>
#include <cmath>
#include <cstddef>
#include <limits>
#include <set>

#include <boost/optional.hpp>
#include <fmt/format.h>

#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_scope.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parser.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

// Cleans up text commonly found in doxygen-like comments.
//
// Warning: mixing tabs and spaces may mess up formatting.
std::string clean_up_doctext(std::string docstring) {
  // Convert to C++ string, and remove Windows's carriage returns.
  docstring.erase(
      remove(docstring.begin(), docstring.end(), '\r'), docstring.end());

  // Separate into lines.
  std::vector<std::string> lines;
  std::string::size_type pos = std::string::npos;
  std::string::size_type last;
  while (true) {
    last = (pos == std::string::npos) ? 0 : pos + 1;
    pos = docstring.find('\n', last);
    if (pos == std::string::npos) {
      // First bit of cleaning. If the last line is only whitespace, drop it.
      std::string::size_type nonwhite =
          docstring.find_first_not_of(" \t", last);
      if (nonwhite != std::string::npos) {
        lines.push_back(docstring.substr(last));
      }
      break;
    }
    lines.push_back(docstring.substr(last, pos - last));
  }

  // A very profound docstring.
  if (lines.empty()) {
    return {};
  }

  // Clear leading whitespace from the first line.
  pos = lines.front().find_first_not_of(" \t");
  lines.front().erase(0, pos);

  // If every nonblank line after the first has the same number of spaces/tabs,
  // then a comment prefix, remove them.
  enum class prefix {
    none = 0,
    star = 1, // length of '*'
    slashes = 3, // length of '///'
    inline_slash = 4, // length of '///<'
  };
  prefix found_prefix = prefix::none;
  std::string::size_type prefix_len = 0;
  std::vector<std::string>::iterator l_iter;

  // Start searching for prefixes from second line, since lexer already removed
  // initial prefix/suffix.
  for (l_iter = lines.begin() + 1; l_iter != lines.end(); ++l_iter) {
    if (l_iter->empty()) {
      continue;
    }

    pos = l_iter->find_first_not_of(" \t");
    if (found_prefix == prefix::none) {
      if (pos != std::string::npos) {
        if (l_iter->at(pos) == '*') {
          found_prefix = prefix::star;
          prefix_len = pos;
        } else if (l_iter->compare(pos, 4, "///<") == 0) {
          found_prefix = prefix::inline_slash;
          prefix_len = pos;
        } else if (l_iter->compare(pos, 3, "///") == 0) {
          found_prefix = prefix::slashes;
          prefix_len = pos;
        } else {
          found_prefix = prefix::none;
          break;
        }
      } else {
        // Whitespace-only line.  Truncate it.
        l_iter->clear();
      }
    } else if (
        l_iter->size() > pos && pos == prefix_len &&
        ((found_prefix == prefix::star && l_iter->at(pos) == '*') ||
         (found_prefix == prefix::inline_slash &&
          l_iter->compare(pos, 4, "///<") == 0) ||
         (found_prefix == prefix::slashes &&
          l_iter->compare(pos, 3, "///") == 0))) {
      // Business as usual
    } else if (pos == std::string::npos) {
      // Whitespace-only line.  Let's truncate it for them.
      l_iter->clear();
    } else {
      // The pattern has been broken.
      found_prefix = prefix::none;
      break;
    }
  }

  // If our prefix survived, delete it from every line.
  if (found_prefix != prefix::none) {
    // Get the prefix too.
    prefix_len += static_cast<int>(found_prefix);
    for (l_iter = lines.begin() + 1; l_iter != lines.end(); ++l_iter) {
      l_iter->erase(0, prefix_len);
    }
  }

  // Now delete the minimum amount of leading whitespace from each line.
  prefix_len = std::string::npos;
  for (l_iter = lines.begin() + 1; l_iter != lines.end(); ++l_iter) {
    if (l_iter->empty()) {
      continue;
    }
    pos = l_iter->find_first_not_of(" \t");
    if (pos != std::string::npos &&
        (prefix_len == std::string::npos || pos < prefix_len)) {
      prefix_len = pos;
    }
  }

  // If our whitespace prefix survived, delete it from every line.
  if (prefix_len != std::string::npos) {
    for (l_iter = lines.begin() + 1; l_iter != lines.end(); ++l_iter) {
      l_iter->erase(0, prefix_len);
    }
  }

  // Remove trailing whitespace from every line.
  for (l_iter = lines.begin(); l_iter != lines.end(); ++l_iter) {
    pos = l_iter->find_last_not_of(" \t");
    if (pos != std::string::npos && pos != l_iter->length() - 1) {
      l_iter->erase(pos + 1);
    }
  }

  // If the first line is empty, remove it.
  // Don't do this earlier because a lot of steps skip the first line.
  if (lines.front().empty()) {
    lines.erase(lines.begin());
  }

  // Now rejoin the lines and copy them back into doctext.
  docstring.clear();
  for (l_iter = lines.begin(); l_iter != lines.end(); ++l_iter) {
    docstring += *l_iter;
    docstring += '\n';
  }

  return docstring;
}

class parsing_terminator : public std::runtime_error {
 public:
  parsing_terminator()
      : std::runtime_error(
            "internal exception for terminating the parsing process") {}
};

[[noreturn]] void end_parsing() {
  throw parsing_terminator();
}

using include_handler = std::function<t_program*(
    source_range range, const std::string& include_name, const t_program& p)>;

// A semantic analyzer and AST builder for a single Thrift program.
class ast_builder : public parser_actions {
 private:
  diagnostics_engine& diags_;
  t_program& program_; // The program being built.
  t_scope* scope_; // The program scope.
  std::unordered_map<std::string, t_named*> definitions_;
  const parsing_params& params_;
  include_handler on_include_;
  boost::optional<comment> doctext_; // The last parsed doctext comment.

  // Checks that the constant name does not refer to an ambiguous enum.
  // An ambiguous enum is one that is redefined but not referred to by
  // ENUM_NAME.ENUM_VALUE.
  void validate_not_ambiguous_enum(
      source_location loc, const std::string& name) {
    if (scope_->is_ambiguous_enum_value(name)) {
      std::string possible_enums =
          scope_->get_fully_qualified_enum_value_names(name).c_str();
      diags_.warning(
          loc,
          "The ambiguous enum `{}` is defined in more than one place. "
          "Please refer to this enum using ENUM_NAME.ENUM_VALUE.{}",
          name,
          possible_enums.empty() ? "" : " Possible options: " + possible_enums);
    }
  }

  // Clears any previously stored doctext string and prints a warning if
  // information is discarded.
  void clear_doctext() {
    if (doctext_) {
      diags_.warning_legacy_strict(doctext_->range.end, "uncaptured doctext");
    }
    doctext_ = boost::none;
  }

  // Returns a previously pushed doctext.
  boost::optional<comment> pop_doctext() {
    return std::exchange(doctext_, boost::none);
  }

  // Strips comment text and aligns leading whitespace on multiline doctext.
  std::string strip_doctext(std::string_view text) {
    std::string str(text.data(), text.size());
    if (str.compare(0, 3, "/**") == 0) {
      str = str.substr(3, str.length() - 3 - 2);
    } else if (str.compare(0, 3, "///") == 0) {
      str = str.substr(3, str.length() - 3);
    }
    if ((str.size() >= 1) && str[0] == '<') {
      str = str.substr(1, str.length() - 1);
    }

    return clean_up_doctext(str);
  }

  // Updates doctext of the given node.
  void set_doctext(t_named& node, boost::optional<comment> doc) const {
    if (!doc) {
      return;
    }
    // Concatenate prefix doctext with inline doctext via a newline
    // (discouraged).
    if (node.has_doc()) {
      diags_.warning(
          doc->range.begin,
          "Combining prefix and inline doctext. Tracked source range will be inaccurate.");
    }
    node.set_doc(
        node.has_doc() ? fmt::format("{}\n{}", node.doc(), doc->text)
                       : std::move(doc->text),
        doc->range);
  }

  // Sets the annotations on the given node.
  static void set_annotations(
      t_node* node, std::unique_ptr<deprecated_annotations> annotations) {
    if (annotations) {
      node->reset_annotations(annotations->strings);
    }
  }

  // Sets the attributes on the given node.
  void set_attributes(
      t_named& node,
      std::unique_ptr<attributes> attrs,
      const source_range& range) const {
    node.set_src_range(range);
    if (!attrs) {
      return;
    }
    if (attrs->doc) {
      node.set_doc(std::move(attrs->doc->text), attrs->doc->range);
    }
    for (auto& annotation : attrs->annotations) {
      node.add_structured_annotation(std::move(annotation));
    }
    set_annotations(&node, std::move(attrs->deprecated_annotations));
  }

  void add_definition(std::unique_ptr<t_named> definition) {
    const std::string& name = definition->name();
    if (!definitions_.insert(std::make_pair(name, definition.get())).second) {
      diags_.error(*definition, "redefinition of '{}'", name);
    }
    program_.add_definition(std::move(definition));
  }

  // DEPRECATED! Allocates a new field id using automatic numbering.
  //
  // Field ids are assigned starting from -1 and working their way down.
  void allocate_field_id(t_field_id& next_id, t_field& field) {
    if (params_.strict >= 192) {
      diags_.error(
          field,
          "Implicit field ids are deprecated and not allowed with -strict");
    }
    if (next_id < t_field::min_id) {
      diags_.error(
          field,
          "Cannot allocate an id for `{}`. Automatic field ids are exhausted.",
          field.name());
    }
    field.set_implicit_id(next_id--);
  }

  void maybe_allocate_field_id(t_field_id& next_id, t_field& field) {
    if (!field.explicit_id()) {
      // Auto assign an id.
      allocate_field_id(next_id, field);
      return;
    }

    // Check the explicitly provided id.
    if (field.id() > 0) {
      return;
    }
    if (params_.allow_neg_field_keys) {
      // allow_neg_field_keys exists to allow users to add explicitly specified
      // id values to old .thrift files without breaking protocol compatibility.
      if (field.id() != next_id) {
        diags_.warning(
            field,
            "Nonpositive field id ({}) differs from what would be "
            "auto-assigned by thrift ({}).",
            field.id(),
            next_id);
      }
    } else if (field.id() == next_id) {
      diags_.warning(
          field,
          "Nonpositive value ({}) not allowed as a field id.",
          field.id());
    } else {
      // TODO: Make ignoring the user provided value a failure.
      diags_.warning(
          field,
          "Nonpositive field id ({}) differs from what is auto-assigned by "
          "thrift. The id must be positive or {}.",
          field.id(),
          next_id);
      // Ignore user provided value and auto assign an id.
      allocate_field_id(next_id, field);
    }
    // Skip past any negative, manually assigned ids.
    if (field.id() < 0) {
      // Update the next field id to be one less than the value.
      // The field_list parsing will catch any duplicates.
      next_id = field.id() - 1;
    }
  }

  // Creates a reference to a known type, potentally with additional
  // annotations.
  t_type_ref new_type_ref(
      const t_type& type,
      std::unique_ptr<deprecated_annotations> annotations,
      const source_range& range = {}) {
    if (!annotations) {
      return {type, range};
    }

    // Make a copy of the node to hold the annotations.
    if (const auto* tbase_type = dynamic_cast<const t_base_type*>(&type)) {
      // Base types can be copy constructed.
      auto node = std::make_unique<t_base_type>(*tbase_type);
      set_annotations(node.get(), std::move(annotations));
      t_type_ref result(*node, range);
      program_.add_unnamed_type(std::move(node));
      return result;
    }

    // Containers always use a new type, so should never show up here.
    assert(!type.is_container());
    // For all other types, we can just create a dummy typedef node with
    // the same name. Note that this is not a safe assumption as it breaks all
    // dynamic casts and t_type::is_* calls.
    auto unnamed = t_typedef::make_unnamed(
        const_cast<t_program*>(type.program()), type.get_name(), {type, range});
    const t_type* result = unnamed.get();
    unnamed->set_src_range(range);
    set_annotations(unnamed.get(), std::move(annotations));
    program_.add_unnamed_typedef(std::move(unnamed));
    return {*result, range};
  }

  // Creates a reference to a newly instantiated container type.
  t_type_ref new_type_ref(
      source_range range,
      std::unique_ptr<t_container> node,
      std::unique_ptr<deprecated_annotations> annotations) {
    assert(node != nullptr);
    const t_type* type = node.get();
    set_annotations(node.get(), std::move(annotations));
    node->set_src_range(range);
    program_.add_type_instantiation(std::move(node));
    return {*type, range};
  }

  // Creates a reference to a named type.
  t_type_ref new_type_ref(
      std::string name,
      std::unique_ptr<deprecated_annotations> annotations,
      const source_range& range,
      bool is_const = false) {
    t_type_ref result = scope_->ref_type(program_, name, range);

    // TODO: Consider removing this special case for const, which requires a
    // specific declaration order.
    if (!result.resolved() && is_const) {
      diags_.error(
          range.begin,
          "The type '{}' is not defined yet. Types must be "
          "defined before the usage in constant values.",
          name);
    }

    if (auto* node = result.get_unresolved_type()) { // A newly created ph.
      set_annotations(node, std::move(annotations));
    } else if (annotations) {
      // TODO: Remove support for annotations on type references.
      return new_type_ref(result.deref(), std::move(annotations), range);
    }

    return result;
  }

  void check_external_type_resolved(t_type_ref type) {
    if (type.resolved()) {
      return;
    }
    t_placeholder_typedef* unresolved_type = type.get_unresolved_type();
    const std::string& type_name = unresolved_type->name();
    size_t sep_pos = type_name.find(".");
    if (sep_pos == std::string::npos ||
        std::string_view(type_name.data(), sep_pos) == program_.name()) {
      // Local types are handled separately because they can be used before
      // definition.
      return;
    }
    diags_.error(*unresolved_type, "Type `{}` not defined.", type_name);
  }

  // Tries to set the given fields, reporting an error on a collision.
  void set_fields(t_structured& s, t_field_list&& fields) {
    assert(s.fields().empty());
    t_field_id next_id = -1;
    for (auto& field : fields) {
      maybe_allocate_field_id(next_id, *field);
      if (!s.try_append_field(field)) {
        diags_.error(
            *field,
            "Field id {} for `{}` has already been used.",
            field->id(),
            field->name());
      }
    }
  }

  template <typename T>
  T narrow_int(source_location loc, int64_t value, const char* name) {
    using limits = std::numeric_limits<T>;
    if (value < limits::min() || value > limits::max()) {
      diags_.error(
          loc,
          "Integer constant {} outside the range of {} ([{}, {}]).",
          value,
          name,
          limits::min(),
          limits::max());
    }
    return value;
  }

 public:
  ast_builder(
      diagnostics_engine& diags,
      t_program& program,
      const parsing_params& params,
      include_handler on_include)
      : diags_(diags),
        program_(program),
        scope_(program.scope()),
        params_(params),
        on_include_(on_include) {}

  void on_program() override { clear_doctext(); }

  void on_package(
      source_range range,
      std::unique_ptr<attributes> attrs,
      std::string_view name) override {
    set_attributes(program_, std::move(attrs), range);
    if (!program_.package().empty()) {
      diags_.error(range.begin, "Package already specified.");
    }
    try {
      auto package = t_package(fmt::to_string(name));
      package.set_src_range(range);
      program_.set_package(std::move(package));
    } catch (const std::exception& e) {
      diags_.error(range.begin, "{}", e.what());
    }
  }

  void on_include(
      source_range range,
      std::string_view str,
      source_range str_range) override {
    std::string include_name = fmt::to_string(str);
    auto included_program = on_include_(range, include_name, program_);
    auto last_slash = include_name.find_last_of("/\\");
    if (last_slash != std::string::npos) {
      included_program->set_include_prefix(include_name.substr(0, last_slash));
    }
    auto include = std::make_unique<t_include>(included_program, include_name);
    include->set_src_range(range);
    include->set_str_range(str_range);
    program_.add_include(std::move(include));
  }

  void on_cpp_include(source_range, std::string_view str) override {
    program_.add_language_include("cpp", fmt::to_string(str));
  }

  void on_hs_include(source_range, std::string_view str) override {
    program_.add_language_include("hs", fmt::to_string(str));
  }

  void on_namespace(const identifier& language, std::string_view ns) override {
    program_.set_namespace(fmt::to_string(language.str), fmt::to_string(ns));
  }

  boost::optional<comment> on_doctext() override { return pop_doctext(); }

  void on_program_doctext() override {
    // When there is any doctext, assign it to the top-level program.
    set_doctext(program_, pop_doctext());
  }

  comment on_inline_doc(source_range range, std::string_view text) override {
    return {strip_doctext(text), range};
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::string_view name) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    t_type_ref type = new_type_ref(fmt::to_string(name), nullptr, range);
    // Once Thrift Patch is decoupled from the compiler we will be able to
    // always resolve external types. Until then just resolve annotation types.
    check_external_type_resolved(type);
    const_value->set_ttype(type);
    return on_structured_annotation(range, std::move(const_value));
  }

  std::unique_ptr<t_const> on_structured_annotation(
      source_range range, std::unique_ptr<t_const_value> value) override {
    auto ttype = value->ttype(); // Copy the t_type_ref.
    auto result = std::make_unique<t_const>(
        &program_, std::move(ttype), "", std::move(value));
    result->set_src_range(range);
    return result;
  }

  void on_service(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      const identifier& base,
      node_list<t_function> functions) override {
    auto find_base_service = [&]() -> const t_service* {
      if (base.str.size() != 0) {
        auto base_name = base.str;
        if (const t_service* result = scope_->find_service(base_name)) {
          return result;
        }
        if (const t_service* result =
                scope_->find_service(program_.scope_name(base_name))) {
          return result;
        }
        diags_.error(
            range.begin, "Service \"{}\" has not been defined.", base_name);
      }
      return nullptr;
    };
    auto service = std::make_unique<t_service>(
        &program_, fmt::to_string(name.str), find_base_service());
    set_attributes(*service, std::move(attrs), range);
    service->set_extends_range({base.loc, base.loc + base.str.size()});
    service->set_functions(std::move(functions));
    add_definition(std::move(service));
  }

  void on_interaction(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      node_list<t_function> functions) override {
    auto interaction =
        std::make_unique<t_interaction>(&program_, fmt::to_string(name.str));
    set_attributes(*interaction, std::move(attrs), range);
    interaction->set_functions(std::move(functions));
    add_definition(std::move(interaction));
  }

  std::unique_ptr<t_function> on_function(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_function_qualifier qual,
      return_clause ret,
      const identifier& name,
      t_field_list params,
      std::unique_ptr<t_throws> throws) override {
    auto return_name = ret.name.str;
    t_type_ref interaction;
    t_type_ref return_type = t_type_ref::from_ptr(
        ret.type, {ret.name.loc, ret.name.loc + ret.name.str.size()});
    if (size_t size = return_name.size()) {
      // Handle an interaction or return type name.
      std::string qualified_name;
      if (return_name.find('.') == std::string::npos) {
        qualified_name = program_.scope_name(return_name);
        return_name = qualified_name;
      }
      if (auto interaction_ptr = scope_->find_interaction(return_name)) {
        interaction = t_type_ref::from_ptr(
            interaction_ptr, {ret.name.loc, ret.name.loc + size});
      } else if (ret.type) {
        diags_.error(
            ret.name.loc, "'{}' does not name an interaction", return_name);
      } else {
        return_type =
            on_type({ret.name.loc, ret.name.loc + size}, return_name, {});
      }
    }

    if (qual == t_function_qualifier::oneway) {
      if ((ret.type && !ret.type->is_void()) || interaction) {
        diags_.error(range.begin, "oneway function must return 'void'");
      }
      if (throws && throws->has_fields()) {
        diags_.error(range.begin, "oneway function can't throw exceptions");
      }
    }

    auto function = std::make_unique<t_function>(
        &program_,
        return_type,
        fmt::to_string(name.str),
        nullptr,
        std::move(ret.sink_or_stream),
        interaction);
    function->set_qualifier(qual);
    set_fields(function->params(), std::move(params));
    function->set_exceptions(std::move(throws));
    function->set_src_range(range);
    // TODO: Leave the param list unnamed.
    function->params().set_name(function->name() + "_args");
    set_attributes(*function, std::move(attrs), range);
    return function;
  }

  std::unique_ptr<t_sink> on_sink(
      source_range range,
      type_throws_spec sink_spec,
      type_throws_spec final_response_spec) override {
    auto sink = std::make_unique<t_sink>(
        std::move(sink_spec.type), std::move(final_response_spec.type));
    sink->set_src_range(range);
    sink->set_sink_exceptions(std::move(sink_spec.throws));
    sink->set_final_response_exceptions(std::move(final_response_spec.throws));
    return sink;
  }

  std::unique_ptr<t_stream_response> on_stream(
      source_range range, type_throws_spec spec) override {
    auto stream = std::make_unique<t_stream_response>(std::move(spec.type));
    stream->set_src_range(range);
    stream->set_exceptions(std::move(spec.throws));
    return stream;
  }

  t_type_ref on_list_type(
      source_range range,
      t_type_ref element_type,
      std::unique_ptr<deprecated_annotations> annotations) override {
    return new_type_ref(
        range,
        std::make_unique<t_list>(std::move(element_type)),
        std::move(annotations));
  }

  t_type_ref on_set_type(
      source_range range,
      t_type_ref key_type,
      std::unique_ptr<deprecated_annotations> annotations) override {
    return new_type_ref(
        range,
        std::make_unique<t_set>(std::move(key_type)),
        std::move(annotations));
  }

  t_type_ref on_map_type(
      source_range range,
      t_type_ref key_type,
      t_type_ref value_type,
      std::unique_ptr<deprecated_annotations> annotations) override {
    return new_type_ref(
        range,
        std::make_unique<t_map>(std::move(key_type), std::move(value_type)),
        std::move(annotations));
  }

  std::unique_ptr<t_function> on_performs(
      source_range range, const identifier& interaction_name) override {
    auto ret = return_clause();
    ret.name = interaction_name;
    auto name = fmt::format("create{}", interaction_name.str);
    auto fun =
        on_function(range, {}, {}, std::move(ret), {name, range.begin}, {}, {});
    if (!fun->interaction()) {
      diags_.error(*fun, "expected interaction name");
    }
    fun->set_is_interaction_constructor();
    return fun;
  }

  std::unique_ptr<t_throws> on_throws(t_field_list exceptions) override {
    auto result = std::make_unique<t_throws>();
    set_fields(*result, std::move(exceptions));
    return result;
  }

  void on_typedef(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_type_ref type,
      const identifier& name) override {
    auto typedef_node = std::make_unique<t_typedef>(
        &program_, fmt::to_string(name.str), std::move(type));
    set_attributes(*typedef_node, std::move(attrs), range);
    auto* true_type = typedef_node->get_true_type();
    if (true_type && true_type->is_enum()) {
      for (const auto& value :
           static_cast<const t_enum*>(true_type)->consts()) {
        scope_->add_enum_value(
            program_.scope_name(*typedef_node, value), &value);
      }
    }
    add_definition(std::move(typedef_node));
  }

  void on_struct(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_field_list fields) override {
    auto struct_node =
        std::make_unique<t_struct>(&program_, fmt::to_string(name.str));
    set_attributes(*struct_node, std::move(attrs), range);
    set_fields(*struct_node, std::move(fields));
    add_definition(std::move(struct_node));
  }

  void on_union(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_field_list fields) override {
    auto union_node =
        std::make_unique<t_union>(&program_, fmt::to_string(name.str));
    set_attributes(*union_node, std::move(attrs), range);
    set_fields(*union_node, std::move(fields));
    add_definition(std::move(union_node));
  }

  void on_exception(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_error_safety safety,
      t_error_kind kind,
      t_error_blame blame,
      const identifier& name,
      t_field_list fields) override {
    auto exception =
        std::make_unique<t_exception>(&program_, fmt::to_string(name.str));
    set_attributes(*exception, std::move(attrs), range);
    exception->set_safety(safety);
    exception->set_kind(kind);
    exception->set_blame(blame);
    set_fields(*exception, std::move(fields));
    add_definition(std::move(exception));
  }

  std::unique_ptr<t_field> on_field(
      source_range range,
      std::unique_ptr<attributes> attrs,
      boost::optional<int64_t> id,
      t_field_qualifier qual,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value,
      boost::optional<comment> doc) override {
    auto valid_id = id ? narrow_int<t_field_id>(range.begin, *id, "field ids")
                       : boost::optional<t_field_id>();
    auto field = std::make_unique<t_field>(
        std::move(type), fmt::to_string(name.str), valid_id);
    field->set_qualifier(qual);
    field->set_default_value(std::move(value));
    field->set_src_range(range);
    set_attributes(*field, std::move(attrs), range);
    set_doctext(*field, std::move(doc));
    return field;
  }

  t_type_ref on_type(
      source_range range,
      const t_base_type& type,
      std::unique_ptr<deprecated_annotations> annotations) override {
    return new_type_ref(type, std::move(annotations), range);
  }

  t_type_ref on_type(
      source_range range,
      std::string_view name,
      std::unique_ptr<deprecated_annotations> annotations) override {
    return new_type_ref(fmt::to_string(name), std::move(annotations), range);
  }

  void on_enum(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      t_enum_value_list values) override {
    auto enum_node =
        std::make_unique<t_enum>(&program_, fmt::to_string(name.str));
    set_attributes(*enum_node, std::move(attrs), range);
    enum_node->set_values(std::move(values));

    // Register enum value names in scope.
    for (const auto& value : enum_node->consts()) {
      // TODO: Remove the ability to access unscoped enum values.
      scope_->add_enum_value(program_.scope_name(value), &value);
      scope_->add_enum_value(program_.scope_name(*enum_node, value), &value);
    }

    add_definition(std::move(enum_node));
  }

  std::unique_ptr<t_enum_value> on_enum_value(
      source_range range,
      std::unique_ptr<attributes> attrs,
      const identifier& name,
      boost::optional<int64_t> value,
      boost::optional<comment> doc) override {
    auto enum_value = std::make_unique<t_enum_value>(fmt::to_string(name.str));
    enum_value->set_src_range(range);
    set_attributes(*enum_value, std::move(attrs), range);
    if (value) {
      enum_value->set_value(
          narrow_int<int32_t>(range.begin, *value, "enum values"));
    }
    set_doctext(*enum_value, std::move(doc));
    return enum_value;
  }

  void on_const(
      source_range range,
      std::unique_ptr<attributes> attrs,
      t_type_ref type,
      const identifier& name,
      std::unique_ptr<t_const_value> value) override {
    auto constant = std::make_unique<t_const>(
        &program_, std::move(type), fmt::to_string(name.str), std::move(value));
    set_attributes(*constant, std::move(attrs), range);
    add_definition(std::move(constant));
  }

  std::unique_ptr<t_const_value> on_const_ref(const identifier& name) override {
    auto find_const =
        [this](source_location loc, const std::string& name) -> const t_const* {
      validate_not_ambiguous_enum(loc, name);
      if (const t_const* constant = scope_->find_constant(name)) {
        return constant;
      }
      if (const t_const* constant =
              scope_->find_constant(program_.scope_name(name))) {
        validate_not_ambiguous_enum(loc, program_.scope_name(name));
        return constant;
      }
      return nullptr;
    };

    auto name_str = fmt::to_string(name.str);
    if (const t_const* constant = find_const(name.loc, name_str)) {
      // Copy const_value to perform isolated mutations.
      auto result = constant->value()->clone();
      // We only want to clone the value, while discarding all real type
      // information.
      result->set_ttype({});
      result->set_is_enum(false);
      result->set_enum(nullptr);
      result->set_enum_value(nullptr);
      result->set_ref_range({name.loc, name.loc + name.str.size()});
      return result;
    }

    // TODO: Make this an error.
    diags_.warning(
        name.loc,
        "The identifier '{}' is not defined yet. Constants and enums should "
        "be defined before using them as default values.",
        name.str);
    auto ret = std::make_unique<t_const_value>(std::move(name_str));
    ret->set_ref_range({name.loc, name.loc + name.str.size()});
    return ret;
  }

  std::unique_ptr<t_const_value> on_integer(
      source_location loc, int64_t value) override {
    if (!params_.allow_64bit_consts &&
        (value < INT32_MIN || value > INT32_MAX)) {
      diags_.warning(
          loc, "64-bit constant {} may not work in all languages", value);
    }
    auto node = std::make_unique<t_const_value>();
    node->set_integer(value);
    return node;
  }

  std::unique_ptr<t_const_value> on_float(double value) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_double(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_string_literal(std::string value) override {
    return std::make_unique<t_const_value>(std::move(value));
  }

  std::unique_ptr<t_const_value> on_bool_literal(bool value) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_bool(value);
    return const_value;
  }

  std::unique_ptr<t_const_value> on_list_literal() override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_list();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_map_literal() override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    return const_value;
  }

  std::unique_ptr<t_const_value> on_struct_literal(
      source_range range, std::string_view name) override {
    auto const_value = std::make_unique<t_const_value>();
    const_value->set_map();
    const_value->set_ttype(
        new_type_ref(fmt::to_string(name), nullptr, range, /*is_const=*/true));
    return const_value;
  }

  int64_t on_integer(source_range range, sign s, uint64_t value) override {
    constexpr uint64_t max = std::numeric_limits<int64_t>::max();
    if (s == sign::minus) {
      if (value > max + 1) {
        diags_.error(range.begin, "integer constant -{} is too small", value);
      }
      return -value;
    }
    if (value > max) {
      diags_.error(range.begin, "integer constant {} is too large", value);
    }
    return value;
  }

  [[noreturn]] void on_error() override { end_parsing(); }

  // Parses a single .thrift file and populates program_ with the parsed AST.
  void parse_file(source_manager& sm, source_location loc) {
    const std::string& path = program_.path();
    auto src = sm.get_file(path);
    if (!src) {
      diags_.error(loc, "failed to open file: {}", path);
      end_parsing();
    }
    program_.set_src_range({src->start, src->start});

    // Consume doctext and store it in `doctext_`. Documentation comments are
    // handled separately to avoid complicating the main parser logic and the
    // grammar.
    auto on_doc_comment = [this](std::string_view text, source_range range) {
      clear_doctext();
      doctext_ = comment{strip_doctext(text), range};
    };
    auto lexer = compiler::lexer(*src, diags_, on_doc_comment);

    diags_.report(src->start, diagnostic_level::info, "Parsing {}\n", path);
    if (!compiler::parse(lexer, *this, diags_)) {
      end_parsing();
    }
  }
};

} // namespace

std::unique_ptr<t_program_bundle> parse_ast(
    source_manager& sm,
    diagnostics_engine& diags,
    const std::string& path,
    const parsing_params& params,
    t_program_bundle* already_parsed) {
  auto programs = std::make_unique<t_program_bundle>(
      std::make_unique<t_program>(
          path, already_parsed ? already_parsed->get_root_program() : nullptr),
      already_parsed);
  assert(!already_parsed || !already_parsed->find_program(path));

  auto circular_deps = std::set<std::string>{path};

  // Always enable allow_neg_field_keys when parsing included files.
  // This way if a Thrift file has negative keys, --allow-neg-keys doesn't
  // have to be used by everyone that includes it.
  auto include_params = params;
  include_params.allow_neg_field_keys = true;

  include_handler on_include = [&](source_range range,
                                   const std::string& include_path,
                                   const t_program& parent) {
    auto path_or_error = sm.find_include_file(
        include_path, parent.path(), params.incl_searchpath);
    if (path_or_error.index() == 1) {
      diags.error(range.begin, "{}", std::get<1>(path_or_error));
      if (!params.allow_missing_includes) {
        end_parsing();
      }
    }

    // Skip already parsed files.
    t_program* program = programs->find_program(include_path);
    const std::string* resolved_path = &include_path;
    if (!program && path_or_error.index() == 0) {
      program = programs->find_program(std::get<0>(path_or_error));
      if (program) {
        // We've already seen this program but know it by another path.
        resolved_path = &std::get<0>(path_or_error);
      }
    }
    if (program) {
      if (program == programs->get_root_program()) {
        // If we're including the root program we must have a dependency cycle.
        assert(circular_deps.count(*resolved_path));
      } else {
        return program;
      }
    }

    // Fail on circular dependencies.
    if (!circular_deps.insert(*resolved_path).second) {
      diags.error(
          range.begin,
          "Circular dependency found: file `{}` is already parsed.",
          *resolved_path);
      end_parsing();
    }

    // Create a new program for a Thrift file in an include statement and
    // set its include_prefix by parsing the directory which it is
    // included from.
    auto included_program =
        std::make_unique<t_program>(*resolved_path, &parent);
    program = included_program.get();
    programs->add_program(std::move(included_program));

    try {
      ast_builder(diags, *program, include_params, on_include)
          .parse_file(sm, range.begin);
    } catch (...) {
      if (!params.allow_missing_includes) {
        throw;
      }
    }

    circular_deps.erase(*resolved_path);
    return program;
  };

  t_program& root_program = *programs->root_program();
  try {
    ast_builder(diags, root_program, params, on_include).parse_file(sm, {});
  } catch (const parsing_terminator&) {
    return {}; // Return a null program bundle if parsing failed.
  }

  // Resolve types in the root program.
  std::string program_prefix = root_program.name() + ".";
  for (t_placeholder_typedef& t :
       root_program.scope()->placeholder_typedefs()) {
    if (!t.resolve() && t.name().find(program_prefix) == 0) {
      diags.error(t, "Type `{}` not defined.", t.name());
    }
  }
  return programs;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
