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

#include <thrift/compiler/parse/parsing_driver.h>

#include <stdlib.h>
#include <cmath>
#include <limits>
#include <memory>

#include <boost/filesystem.hpp>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parser.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

class parsing_terminator : public std::runtime_error {
 public:
  parsing_terminator()
      : std::runtime_error(
            "Internal exception used to terminate the parsing process.") {}
};

} // namespace

class parsing_driver::lex_handler_impl : public lex_handler {
 private:
  parsing_driver& driver_;

 public:
  explicit lex_handler_impl(parsing_driver& d) : driver_(d) {}

  // Consume doctext and store it in `driver_.doctext`.
  //
  // It is non-trivial for a yacc-style LR(1) parser to accept doctext
  // as an optional prefix before either a definition or standalone at
  // the header. Hence this method of "pushing" it into the driver and
  // "pop-ing" it on the node as needed.
  void on_doc_comment(fmt::string_view text, source_location loc) override {
    driver_.clear_doctext();
    driver_.doctext = comment{driver_.strip_doctext(text), loc};
  }
};

std::unique_ptr<t_program_bundle> parsing_driver::parse() {
  std::unique_ptr<t_program_bundle> result;
  try {
    parse_file();
    result = std::move(program_bundle);
  } catch (const parsing_terminator&) {
    // No need to do anything here. The purpose of the exception is simply to
    // end the parsing process by unwinding to here.
  }
  return result;
}

void parsing_driver::parse_file() {
  // Skip already parsed files.
  const std::string& path = program->path();
  if (!already_parsed_paths_.insert(path).second) {
    return;
  }

  auto src = source();
  try {
    src = source_mgr_.get_file(path);
  } catch (const std::runtime_error& e) {
    diags_.error(source_location(), "{}", e.what());
    end_parsing();
  }
  auto lex_handler = lex_handler_impl(*this);
  auto lexer = compiler::lexer(src, lex_handler, diags_);
  program->set_src_range({src.start, src.start});

  // Create a new scope and scan for includes.
  diags_.report(
      src.start, diagnostic_level::info, "Scanning {} for includes\n", path);
  mode = parsing_mode::INCLUDES;
  try {
    if (!compiler::parse(lexer, *this, diags_)) {
      diags_.error(*program, "Parser error during include pass.");
      end_parsing();
    }
  } catch (const std::string& x) {
    diags_.error(source_location(), "{}", x);
    assert(false);
    end_parsing();
  }

  // Recursively parse all the included programs.
  const std::vector<t_include*>& includes = program->includes();
  // Always enable allow_neg_field_keys when parsing included files.
  // This way if a thrift file has negative keys, --allow-neg-keys doesn't have
  // to be used by everyone that includes it.
  auto old_params = params_;
  auto old_program = program;
  for (auto include : includes) {
    t_program* included_program = include->get_program();
    circular_deps_.insert(path);

    // Fail on circular dependencies.
    if (circular_deps_.count(included_program->path()) != 0) {
      diags_.error(
          *include,
          "Circular dependency found: file `{}` is already parsed.",
          included_program->path());
      end_parsing();
    }

    // This must be after the previous circular include check, since the emitted
    // error message above is supposed to reference the parent file name.
    params_.allow_neg_field_keys = true;
    program = included_program;
    try {
      parse_file();
    } catch (...) {
      if (!params_.allow_missing_includes) {
        throw;
      }
    }

    size_t num_removed = circular_deps_.erase(path);
    (void)num_removed;
    assert(num_removed == 1);
  }
  params_ = old_params;
  program = old_program;

  // Parse the program file
  try {
    src = source_mgr_.get_file(path);
  } catch (const std::runtime_error& e) {
    diags_.error(source_location(), "{}", e.what());
    end_parsing();
  }
  lexer = compiler::lexer(src, lex_handler, diags_);

  mode = parsing_mode::PROGRAM;
  diags_.report(
      src.start, diagnostic_level::info, "Parsing {} for types\n", path);
  try {
    if (!compiler::parse(lexer, *this, diags_)) {
      diags_.error(*program, "Parser error during types pass.");
      end_parsing();
    }
  } catch (const std::string& x) {
    diags_.error(source_location(), "{}", x);
    assert(false);
    end_parsing();
  }
}

[[noreturn]] void parsing_driver::end_parsing() {
  throw parsing_terminator{};
}

void parsing_driver::validate_header_location(source_location loc) {
  if (programs_that_parsed_definition_.find(program->path()) !=
      programs_that_parsed_definition_.end()) {
    diags_.error(loc, "Headers must be specified before definitions.");
  }
}

// TODO: This doesn't really need to be a member function. Move it somewhere
// else (e.g. `util.{h|cc}`) once everything gets consolidated into `parse/`.
std::string parsing_driver::directory_name(const std::string& filename) {
  boost::filesystem::path fullpath = filename;
  auto parent_path = fullpath.parent_path();
  auto result = parent_path.string();
  // No parent dir, just use the current directory
  if (result.empty()) {
    return ".";
  }
  return result;
}

std::string parsing_driver::find_include_file(
    source_location loc, const std::string& filename) {
  // Absolute path? Just try that
  boost::filesystem::path path(filename);
  if (path.has_root_directory()) {
    try {
      return boost::filesystem::canonical(path).string();
    } catch (const boost::filesystem::filesystem_error& e) {
      diags_.error(
          loc, "Could not find file: {}. Error: {}", filename, e.what());
      end_parsing();
    }
  }

  // relative path, start searching
  // new search path with current dir global
  std::vector<std::string> sp = params_.incl_searchpath;
  sp.insert(sp.begin(), directory_name(program->path()));
  // iterate through paths
  std::vector<std::string>::iterator it;
  for (it = sp.begin(); it != sp.end(); it++) {
    boost::filesystem::path sfilename = filename;
    if ((*it) != "." && (*it) != "") {
      sfilename = boost::filesystem::path(*(it)) / filename;
    }
    if (boost::filesystem::exists(sfilename)) {
      return sfilename.string();
    }
#ifdef _WIN32
    // On Windows, handle files found at potentially long paths.
    sfilename = R"(\\?\)" +
        boost::filesystem::absolute(sfilename)
            .make_preferred()
            .lexically_normal()
            .string();
    if (boost::filesystem::exists(sfilename)) {
      return sfilename.string();
    }
#endif
    diags_.report(
        loc, diagnostic_level::debug, "Could not find: {}.", filename);
  }
  // File was not found.
  diags_.error(loc, "Could not find include file {}", filename);
  end_parsing();
}

void parsing_driver::validate_not_ambiguous_enum(
    source_location loc, const std::string& name) {
  if (scope_cache->is_ambiguous_enum_value(name)) {
    std::string possible_enums =
        scope_cache->get_fully_qualified_enum_value_names(name).c_str();
    diags_.warning(
        loc,
        "The ambiguous enum `{}` is defined in more than one place. "
        "Please refer to this enum using ENUM_NAME.ENUM_VALUE.{}",
        name,
        possible_enums.empty() ? "" : " Possible options: " + possible_enums);
  }
}

void parsing_driver::clear_doctext() {
  if (doctext && mode == parsing_mode::PROGRAM) {
    diags_.warning_legacy_strict(doctext->loc, "uncaptured doctext");
  }
  doctext = boost::none;
}

boost::optional<comment> parsing_driver::pop_doctext() {
  return mode == parsing_mode::PROGRAM ? std::exchange(doctext, boost::none)
                                       : boost::none;
}

std::string parsing_driver::clean_up_doctext(std::string docstring) {
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
      // First bit of cleaning.  If the last line is only whitespace, drop it.
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
  enum Prefix {
    None = 0,
    Star = 1, // length of '*'
    Slashes = 3, // length of '///'
    InlineSlash = 4, // length of '///<'
  };
  Prefix found_prefix = None;
  std::string::size_type prefix_len = 0;
  std::vector<std::string>::iterator l_iter;

  // Start searching for prefixes from second line, since lexer already removed
  // initial prefix/suffix.
  for (l_iter = lines.begin() + 1; l_iter != lines.end(); ++l_iter) {
    if (l_iter->empty()) {
      continue;
    }

    pos = l_iter->find_first_not_of(" \t");
    if (found_prefix == None) {
      if (pos != std::string::npos) {
        if (l_iter->at(pos) == '*') {
          found_prefix = Star;
          prefix_len = pos;
        } else if (l_iter->compare(pos, 4, "///<") == 0) {
          found_prefix = InlineSlash;
          prefix_len = pos;
        } else if (l_iter->compare(pos, 3, "///") == 0) {
          found_prefix = Slashes;
          prefix_len = pos;
        } else {
          found_prefix = None;
          break;
        }
      } else {
        // Whitespace-only line.  Truncate it.
        l_iter->clear();
      }
    } else if (
        l_iter->size() > pos && pos == prefix_len &&
        ((found_prefix == Star && l_iter->at(pos) == '*') ||
         (found_prefix == InlineSlash &&
          l_iter->compare(pos, 4, "///<") == 0) ||
         (found_prefix == Slashes && l_iter->compare(pos, 3, "///") == 0))) {
      // Business as usual
    } else if (pos == std::string::npos) {
      // Whitespace-only line.  Let's truncate it for them.
      l_iter->clear();
    } else {
      // The pattern has been broken.
      found_prefix = None;
      break;
    }
  }

  // If our prefix survived, delete it from every line.
  if (found_prefix != None) {
    // Get the prefix too.
    prefix_len += found_prefix;
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

void parsing_driver::set_annotations(
    t_node* node, std::unique_ptr<deprecated_annotations> annotations) {
  if (annotations != nullptr) {
    node->reset_annotations(annotations->strings);
  }
}

void parsing_driver::set_attributes(
    t_named& node,
    std::unique_ptr<attributes> attrs,
    std::unique_ptr<deprecated_annotations> annots,
    const source_range& range) const {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  node.set_src_range(range);
  if (attrs != nullptr) {
    if (attrs->doc) {
      node.set_doc(std::move(attrs->doc->text));
    }
    for (auto& annotation : attrs->annotations) {
      node.add_structured_annotation(std::move(annotation));
    }
  }
  set_annotations(&node, std::move(annots));
}

void parsing_driver::set_doctext(
    t_node& node, boost::optional<comment> doc) const {
  if (!doc) {
    return;
  }
  // Concatenate prefix doctext with inline doctext via a newline (discouraged).
  node.set_doc(
      node.has_doc() ? node.doc() + "\n" + std::move(doc->text)
                     : std::move(doc->text));
}

std::unique_ptr<t_const> parsing_driver::new_struct_annotation(
    std::unique_ptr<t_const_value> const_struct, const source_range& range) {
  auto ttype = const_struct->ttype(); // Copy the t_type_ref.
  auto result = std::make_unique<t_const>(
      program, std::move(ttype), "", std::move(const_struct));
  result->set_src_range(range);
  return result;
}

std::unique_ptr<t_throws> parsing_driver::new_throws(
    std::unique_ptr<t_field_list> exceptions) {
  assert(exceptions != nullptr);
  auto result = std::make_unique<t_throws>();
  set_fields(*result, std::move(*exceptions));
  return result;
}

t_type_ref parsing_driver::new_type_ref(
    const t_type& type,
    std::unique_ptr<deprecated_annotations> annotations,
    const source_range& range) {
  if (annotations == nullptr || mode != parsing_mode::PROGRAM) {
    return type;
  }

  // Make a copy of the node to hold the annotations.
  // TODO(afuller): Remove the need for copying the underlying type by making
  // t_type_ref annotatable directly.
  if (const auto* tbase_type = dynamic_cast<const t_base_type*>(&type)) {
    // base types can be copy constructed.
    auto node = std::make_unique<t_base_type>(*tbase_type);
    set_annotations(node.get(), std::move(annotations));
    t_type_ref result(*node);
    program->add_unnamed_type(std::move(node));
    return result;
  }

  // Containers always use a new type, so should never show up here.
  assert(!type.is_container());
  // For all other types, we can just create a dummy typedef node with
  // the same name.
  // NOTE(afuller): This is not a safe assumption as it breaks all
  // dynamic casts and t_type::is_* calls.
  return *add_unnamed_typedef(
      t_typedef::make_unnamed(
          const_cast<t_program*>(type.program()), type.get_name(), type),
      std::move(annotations),
      range);
}

t_type_ref parsing_driver::new_type_ref(
    source_range range,
    std::unique_ptr<t_templated_type> node,
    std::unique_ptr<deprecated_annotations> annotations) {
  if (mode != parsing_mode::PROGRAM) {
    return {};
  }

  assert(node != nullptr);
  const t_type* type = node.get();
  set_annotations(node.get(), std::move(annotations));
  node->set_src_range(range);
  program->add_type_instantiation(std::move(node));
  return *type;
}

t_type_ref parsing_driver::new_type_ref(
    std::string name,
    std::unique_ptr<deprecated_annotations> annotations,
    const source_range& range,
    bool is_const) {
  if (mode != parsing_mode::PROGRAM) {
    return {};
  }

  t_type_ref result = scope_cache->ref_type(*program, name, range);

  // TODO(afuller): Remove this special case for const, which requires a
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
  } else if (annotations != nullptr) { // Oh no!
    // TODO(afuller): Remove support for annotations on type references.
    return new_type_ref(result.deref(), std::move(annotations), range);
  }

  return result;
}

void parsing_driver::set_functions(
    t_interface& node, std::unique_ptr<t_function_list> functions) {
  if (functions != nullptr) {
    node.set_functions(std::move(*functions));
  }
}

void parsing_driver::add_include(std::string name, const source_range& range) {
  if (mode != parsing_mode::INCLUDES) {
    return;
  }

  std::string path;
  try {
    path = find_include_file(range.begin, name);
  } catch (...) {
    if (!params_.allow_missing_includes) {
      throw;
    }
    path = name;
  }
  assert(!path.empty()); // Should have throw an exception if not found.

  if (program_cache.find(path) == program_cache.end()) {
    auto included_program = program->add_include(path, name, range);
    program_cache[path] = included_program.get();
    program_bundle->add_program(std::move(included_program));
  } else {
    auto include =
        std::make_unique<t_include>(program_cache[path], std::move(name));
    include->set_src_range(range);
    program->add_include(std::move(include));
  }
}

const t_type* parsing_driver::add_unnamed_typedef(
    std::unique_ptr<t_typedef> node,
    std::unique_ptr<deprecated_annotations> annotations,
    const source_range& range) {
  const t_type* result(node.get());
  node->set_src_range(range);
  set_annotations(node.get(), std::move(annotations));
  program->add_unnamed_typedef(std::move(node));
  return result;
}

void parsing_driver::allocate_field_id(t_field_id& next_id, t_field& field) {
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

void parsing_driver::maybe_allocate_field_id(
    t_field_id& next_id, t_field& field) {
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
        field, "Nonpositive value ({}) not allowed as a field id.", field.id());
  } else {
    // TODO(afuller): Make ignoring the user provided value a failure.
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

void parsing_driver::set_fields(t_structured& s, t_field_list&& fields) {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
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
T parsing_driver::narrow_int(
    source_location loc, int64_t value, const char* name) {
  using limits = std::numeric_limits<T>;
  if (mode == parsing_mode::PROGRAM &&
      (value < limits::min() || value > limits::max())) {
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

std::string parsing_driver::strip_doctext(fmt::string_view text) {
  if (mode != apache::thrift::compiler::parsing_mode::PROGRAM) {
    return {};
  }

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

parsing_driver::parsing_driver(
    source_manager& sm,
    diagnostics_engine& diags,
    std::string path,
    parsing_params parse_params)
    : source_mgr_(sm), diags_(diags), params_(std::move(parse_params)) {
  program_bundle =
      std::make_unique<t_program_bundle>(std::make_unique<t_program>(path));
  program = program_bundle->root_program();
  scope_cache = program->scope();
}

void parsing_driver::on_standard_header(
    source_location loc, std::unique_ptr<attributes> attrs) {
  validate_header_location(loc);
  if (attrs && !attrs->annotations.empty()) {
    diags_.error(
        *attrs->annotations.front(),
        "Structured annotations are not supported for a given entity.");
  }
}

void parsing_driver::on_program_header(
    source_range range,
    std::unique_ptr<attributes> attrs,
    std::unique_ptr<deprecated_annotations> annotations) {
  validate_header_location(range.begin);
  set_attributes(*program, std::move(attrs), std::move(annotations), range);
}

void parsing_driver::on_package(source_range range, fmt::string_view name) {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  if (!program->package().empty()) {
    diags_.error(range.begin, "Package already specified.");
  }
  try {
    program->set_package(t_package(fmt::to_string(name)));
  } catch (const std::exception& e) {
    diags_.error(range.begin, "{}", e.what());
  }
}

void parsing_driver::on_definition(
    source_range range,
    std::unique_ptr<t_named> defn,
    std::unique_ptr<attributes> attrs,
    std::unique_ptr<deprecated_annotations> annotations) {
  if (mode == parsing_mode::PROGRAM) {
    programs_that_parsed_definition_.insert(program->path());
  }
  set_attributes(*defn, std::move(attrs), std::move(annotations), range);

  if (mode != parsing_mode::PROGRAM) {
    return;
  }

  // Add to scope.
  // TODO: Consider moving program-level scope management to t_program.
  if (auto* tnode = dynamic_cast<t_interaction*>(defn.get())) {
    scope_cache->add_interaction(program->scope_name(*defn), tnode);
  } else if (auto* tnode = dynamic_cast<t_service*>(defn.get())) {
    scope_cache->add_service(program->scope_name(*defn), tnode);
  } else if (auto* tnode = dynamic_cast<t_const*>(defn.get())) {
    scope_cache->add_constant(program->scope_name(*defn), tnode);
  } else if (auto* tnode = dynamic_cast<t_enum*>(defn.get())) {
    scope_cache->add_type(program->scope_name(*defn), tnode);
    // Register enum value names in scope.
    for (const auto& value : tnode->consts()) {
      // TODO: Remove the ability to access unscoped enum values.
      scope_cache->add_constant(program->scope_name(value), &value);
      scope_cache->add_constant(program->scope_name(*defn, value), &value);
    }
  } else if (auto* tnode = dynamic_cast<t_type*>(defn.get())) {
    auto* tnode_true_type = tnode->get_true_type();
    if (tnode_true_type && tnode_true_type->is_enum()) {
      for (const auto& value :
           static_cast<const t_enum*>(tnode_true_type)->consts()) {
        scope_cache->add_constant(program->scope_name(*defn, value), &value);
      }
    }
    scope_cache->add_type(program->scope_name(*defn), tnode);
  } else {
    throw std::logic_error("Unsupported declaration.");
  }
  // Add to program.
  program->add_definition(std::move(defn));
}

std::unique_ptr<t_service> parsing_driver::on_service(
    source_range range,
    const identifier& name,
    const identifier& base,
    std::unique_ptr<t_function_list> functions) {
  auto find_base_service = [&]() -> const t_service* {
    if (mode == parsing_mode::PROGRAM && base.str.size() != 0) {
      auto base_name = fmt::to_string(base.str);
      if (auto* result = scope_cache->find_service(base_name)) {
        return result;
      }
      if (auto* result =
              scope_cache->find_service(program->scope_name(base_name))) {
        return result;
      }
      diags_.error(
          range.begin, "Service \"{}\" has not been defined.", base_name);
    }
    return nullptr;
  };
  auto service = std::make_unique<t_service>(
      program, fmt::to_string(name.str), find_base_service());
  service->set_src_range(range);
  set_functions(*service, std::move(functions));
  return service;
}

std::unique_ptr<t_function> parsing_driver::on_function(
    source_range range,
    std::unique_ptr<attributes> attrs,
    t_function_qualifier qual,
    std::vector<t_type_ref> return_type,
    const identifier& name,
    t_field_list params,
    std::unique_ptr<t_throws> throws,
    std::unique_ptr<deprecated_annotations> annotations) {
  auto function = std::make_unique<t_function>(
      program, std::move(return_type), fmt::to_string(name.str));
  function->set_qualifier(qual);
  set_fields(function->params(), std::move(params));
  function->set_exceptions(std::move(throws));
  function->set_src_range(range);
  // TODO: Leave the param list unnamed.
  function->params().set_name(function->name() + "_args");
  set_attributes(*function, std::move(attrs), std::move(annotations), range);
  return function;
}

t_type_ref parsing_driver::on_stream_return_type(
    source_range range, type_throws_spec spec) {
  auto stream_response =
      std::make_unique<t_stream_response>(std::move(spec.type));
  stream_response->set_exceptions(std::move(spec.throws));
  return new_type_ref(range, std::move(stream_response), {});
}

t_type_ref parsing_driver::on_sink_return_type(
    source_range range,
    type_throws_spec sink_spec,
    type_throws_spec final_response_spec) {
  auto sink = std::make_unique<t_sink>(
      std::move(sink_spec.type), std::move(final_response_spec.type));
  sink->set_sink_exceptions(std::move(sink_spec.throws));
  sink->set_final_response_exceptions(std::move(final_response_spec.throws));
  return new_type_ref(range, std::move(sink), {});
}

t_type_ref parsing_driver::on_list_type(
    source_range range,
    t_type_ref element_type,
    std::unique_ptr<deprecated_annotations> annotations) {
  return new_type_ref(
      range,
      std::make_unique<t_list>(std::move(element_type)),
      std::move(annotations));
}

t_type_ref parsing_driver::on_set_type(
    source_range range,
    t_type_ref key_type,
    std::unique_ptr<deprecated_annotations> annotations) {
  return new_type_ref(
      range,
      std::make_unique<t_set>(std::move(key_type)),
      std::move(annotations));
}

t_type_ref parsing_driver::on_map_type(
    source_range range,
    t_type_ref key_type,
    t_type_ref value_type,
    std::unique_ptr<deprecated_annotations> annotations) {
  return new_type_ref(
      range,
      std::make_unique<t_map>(std::move(key_type), std::move(value_type)),
      std::move(annotations));
}

std::unique_ptr<t_function> parsing_driver::on_performs(
    source_range range, t_type_ref type) {
  std::string name = type.get_type() ? "create" + type.get_type()->get_name()
                                     : "<interaction placeholder>";
  auto function =
      std::make_unique<t_function>(program, std::move(type), std::move(name));
  function->set_src_range(range);
  function->set_is_interaction_constructor();
  return function;
}

std::unique_ptr<t_throws> parsing_driver::on_throws(t_field_list exceptions) {
  return new_throws(std::make_unique<t_field_list>(std::move(exceptions)));
}

std::unique_ptr<t_typedef> parsing_driver::on_typedef(
    source_range range, t_type_ref type, const identifier& name) {
  auto typedef_node = std::make_unique<t_typedef>(
      program, fmt::to_string(name.str), std::move(type));
  typedef_node->set_src_range(range);
  return typedef_node;
}

std::unique_ptr<t_struct> parsing_driver::on_struct(
    source_range range, const identifier& name, t_field_list fields) {
  auto struct_node =
      std::make_unique<t_struct>(program, fmt::to_string(name.str));
  struct_node->set_src_range(range);
  set_fields(*struct_node, std::move(fields));
  return struct_node;
}

std::unique_ptr<t_union> parsing_driver::on_union(
    source_range range, const identifier& name, t_field_list fields) {
  auto union_node =
      std::make_unique<t_union>(program, fmt::to_string(name.str));
  union_node->set_src_range(range);
  set_fields(*union_node, std::move(fields));
  return union_node;
}

std::unique_ptr<t_exception> parsing_driver::on_exception(
    source_range range,
    t_error_safety safety,
    t_error_kind kind,
    t_error_blame blame,
    const identifier& name,
    t_field_list fields) {
  auto exception =
      std::make_unique<t_exception>(program, fmt::to_string(name.str));
  exception->set_src_range(range);
  exception->set_safety(safety);
  exception->set_kind(kind);
  exception->set_blame(blame);
  set_fields(*exception, std::move(fields));
  return exception;
}

std::unique_ptr<t_field> parsing_driver::on_field(
    source_range range,
    std::unique_ptr<attributes> attrs,
    boost::optional<int64_t> id,
    t_field_qualifier qual,
    t_type_ref type,
    const identifier& name,
    std::unique_ptr<t_const_value> value,
    std::unique_ptr<deprecated_annotations> annotations,
    boost::optional<comment> doc) {
  auto valid_id = id ? narrow_int<t_field_id>(range.begin, *id, "field ids")
                     : boost::optional<t_field_id>();
  auto field = std::make_unique<t_field>(
      std::move(type), fmt::to_string(name.str), valid_id);
  field->set_qualifier(qual);
  if (mode == parsing_mode::PROGRAM) {
    field->set_default_value(std::move(value));
  }
  field->set_src_range(range);
  set_attributes(*field, std::move(attrs), std::move(annotations), range);
  if (doc) {
    set_doctext(*field, doc);
  }
  return field;
}

t_type_ref parsing_driver::on_type(
    const t_base_type& type,
    std::unique_ptr<deprecated_annotations> annotations) {
  return new_type_ref(type, std::move(annotations));
}

t_type_ref parsing_driver::on_type(
    source_range range,
    fmt::string_view name,
    std::unique_ptr<deprecated_annotations> annotations) {
  return new_type_ref(fmt::to_string(name), std::move(annotations), range);
}

std::unique_ptr<t_enum> parsing_driver::on_enum(
    source_range range, const identifier& name, t_enum_value_list values) {
  auto enum_node = std::make_unique<t_enum>(program, fmt::to_string(name.str));
  enum_node->set_src_range(range);
  enum_node->set_values(std::move(values));
  return enum_node;
}

std::unique_ptr<t_enum_value> parsing_driver::on_enum_value(
    source_range range,
    std::unique_ptr<attributes> attrs,
    const identifier& name,
    boost::optional<int64_t> value,
    std::unique_ptr<deprecated_annotations> annotations,
    boost::optional<comment> doc) {
  auto enum_value = std::make_unique<t_enum_value>(fmt::to_string(name.str));
  enum_value->set_src_range(range);
  set_attributes(*enum_value, std::move(attrs), std::move(annotations), range);
  if (value) {
    enum_value->set_value(
        narrow_int<int32_t>(range.begin, *value, "enum values"));
  }
  if (doc) {
    set_doctext(*enum_value, std::move(doc));
  }
  return enum_value;
}

std::unique_ptr<t_const> parsing_driver::on_const(
    source_range range,
    t_type_ref type,
    const identifier& name,
    std::unique_ptr<t_const_value> value) {
  auto constant = std::make_unique<t_const>(
      program, std::move(type), fmt::to_string(name.str), std::move(value));
  constant->set_src_range(range);
  return constant;
}

std::unique_ptr<t_const_value> parsing_driver::on_const_ref(
    const identifier& name) {
  auto find_const =
      [this](source_location loc, const std::string& name) -> const t_const* {
    validate_not_ambiguous_enum(loc, name);
    if (const t_const* constant = scope_cache->find_constant(name)) {
      return constant;
    }
    if (const t_const* constant =
            scope_cache->find_constant(program->scope_name(name))) {
      validate_not_ambiguous_enum(loc, program->scope_name(name));
      return constant;
    }
    return nullptr;
  };

  auto name_str = fmt::to_string(name.str);
  if (const t_const* constant = find_const(name.loc, name_str)) {
    // Copy const_value to perform isolated mutations.
    auto result = constant->get_value()->clone();
    // We only want to clone the value, while discarding all real type
    // information.
    result->set_ttype({});
    result->set_is_enum(false);
    result->set_enum(nullptr);
    result->set_enum_value(nullptr);
    return result;
  }

  // TODO(afuller): Make this an error.
  if (mode == parsing_mode::PROGRAM) {
    diags_.warning(
        name.loc,
        "The identifier '{}' is not defined yet. Constants and enums should "
        "be defined before using them as default values.",
        name.str);
  }
  return std::make_unique<t_const_value>(std::move(name_str));
}

std::unique_ptr<t_const_value> parsing_driver::on_integer(
    source_location loc, int64_t value) {
  if (mode == parsing_mode::PROGRAM && !params_.allow_64bit_consts &&
      (value < INT32_MIN || value > INT32_MAX)) {
    diags_.warning(
        loc, "64-bit constant {} may not work in all languages", value);
  }
  auto node = std::make_unique<t_const_value>();
  node->set_integer(value);
  return node;
}

std::unique_ptr<t_const_value> parsing_driver::on_float(double value) {
  auto const_value = std::make_unique<t_const_value>();
  const_value->set_double(value);
  return const_value;
}

std::unique_ptr<t_const_value> parsing_driver::on_string_literal(
    std::string value) {
  return std::make_unique<t_const_value>(std::move(value));
}

std::unique_ptr<t_const_value> parsing_driver::on_bool_literal(bool value) {
  auto const_value = std::make_unique<t_const_value>();
  const_value->set_bool(value);
  return const_value;
}

std::unique_ptr<t_const_value> parsing_driver::on_list_literal() {
  auto const_value = std::make_unique<t_const_value>();
  const_value->set_list();
  return const_value;
}

std::unique_ptr<t_const_value> parsing_driver::on_map_literal() {
  auto const_value = std::make_unique<t_const_value>();
  const_value->set_map();
  return const_value;
}

std::unique_ptr<t_const_value> parsing_driver::on_struct_literal(
    source_range range, fmt::string_view name) {
  auto const_value = std::make_unique<t_const_value>();
  const_value->set_map();
  const_value->set_ttype(
      new_type_ref(fmt::to_string(name), nullptr, range, /*is_const=*/true));
  return const_value;
}

int64_t parsing_driver::on_integer(source_range range, sign s, uint64_t value) {
  constexpr uint64_t max = std::numeric_limits<int64_t>::max();
  if (s == sign::minus) {
    if (mode == parsing_mode::PROGRAM && value > max + 1) {
      diags_.error(range.begin, "integer constant -{} is too small", value);
    }
    return -value;
  }
  if (mode == parsing_mode::PROGRAM && value > max) {
    diags_.error(range.begin, "integer constant {} is too large", value);
  }
  return value;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
