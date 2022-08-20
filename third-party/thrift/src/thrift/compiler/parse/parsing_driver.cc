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
#include <cstdarg>
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
    driver_.doctext = doc{driver_.strip_doctext(text), loc};
  }
};

parsing_driver::parsing_driver(
    source_manager& sm,
    diagnostic_context& ctx,
    std::string path,
    parsing_params parse_params)
    : source_mgr_(&sm),
      lex_handler_(std::make_unique<lex_handler_impl>(*this)),
      lexer_(std::make_unique<lexer>(*lex_handler_, ctx, source())),
      params(std::move(parse_params)),
      ctx_(ctx) {
  program_bundle =
      std::make_unique<t_program_bundle>(std::make_unique<t_program>(path));
  program = program_bundle->root_program();
  scope_cache = program->scope();
}

/**
 * The default destructor needs to be explicitly defined in the .cc file since
 * it invokes the destructor of parse_ (of type unique_ptr<yy::parser>). It
 * cannot go in the header file since yy::parser is only forward-declared there.
 */
parsing_driver::~parsing_driver() = default;

int parsing_driver::get_lineno(source_location loc) {
  return loc != source_location() ? resolved_location(loc, *source_mgr_).line()
                                  : 0;
}

source_location parsing_driver::location() const {
  return lexer_->location();
}

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
    src = source_mgr_->add_file(path);
  } catch (const std::runtime_error& ex) {
    end_parsing(ex.what());
  }
  lexer_ = std::make_unique<lexer>(*lex_handler_, ctx_, src);
  program->set_src_range({src.start, src.start});

  // Create a new scope and scan for includes.
  assert(!ctx_.visiting());
  ctx_.begin_visit(*program);
  ctx_.report(
      src.start, diagnostic_level::info, "Scanning {} for includes\n", path);
  mode = parsing_mode::INCLUDES;
  try {
    auto actions = parser_actions{*this};
    if (!compiler::parse(*lexer_, actions, ctx_)) {
      end_parsing("Parser error during include pass.");
    }
  } catch (const std::string& x) {
    end_parsing(x);
  }

  // Recursively parse all the include programs
  const auto& includes = program->get_included_programs();
  // Always enable allow_neg_field_keys when parsing included files.
  // This way if a thrift file has negative keys, --allow-neg-keys doesn't have
  // to be used by everyone that includes it.
  auto old_params = params;
  auto old_program = program;
  for (auto included_program : includes) {
    circular_deps_.insert(path);

    // Fail on circular dependencies.
    if (circular_deps_.count(included_program->path()) != 0) {
      end_parsing(fmt::format(
          "Circular dependency found: file `{}` is already parsed.",
          included_program->path()));
    }

    // This must be after the previous circular include check, since the emitted
    // error message above is supposed to reference the parent file name.
    params.allow_neg_field_keys = true;
    ctx_.end_visit(*old_program);
    program = included_program;
    parse_file();
    ctx_.begin_visit(*old_program);

    size_t num_removed = circular_deps_.erase(path);
    (void)num_removed;
    assert(num_removed == 1);
  }
  params = old_params;
  program = old_program;

  // Parse the program file
  try {
    src = source_mgr_->add_file(path);
    lexer_ = std::make_unique<lexer>(*lex_handler_, ctx_, src);
  } catch (const std::runtime_error& ex) {
    end_parsing(ex.what());
  }

  mode = parsing_mode::PROGRAM;
  ctx_.report(
      src.start, diagnostic_level::info, "Parsing {} for types\n", path);
  try {
    auto actions = parser_actions{*this};
    if (!compiler::parse(*lexer_, actions, ctx_)) {
      end_parsing("Parser error during types pass.");
    }
  } catch (const std::string& x) {
    end_parsing(x);
  }
  ctx_.end_visit(*program);
}

[[noreturn]] void parsing_driver::end_parsing() {
  throw parsing_terminator{};
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
  boost::filesystem::path path{filename};
  if (path.has_root_directory()) {
    try {
      return boost::filesystem::canonical(path).string();
    } catch (const boost::filesystem::filesystem_error& e) {
      end_parsing(fmt::format(
          "Could not find file: {}. Error: {}", filename, e.what()));
    }
  }

  // relative path, start searching
  // new search path with current dir global
  std::vector<std::string> sp = params.incl_searchpath;
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
    ctx_.report(loc, diagnostic_level::debug, "Could not find: {}.", filename);
  }
  // File was not found.
  error(loc, "Could not find include file {}", filename);
  end_parsing();
}

void parsing_driver::validate_not_ambiguous_enum(
    source_location loc, const std::string& name) {
  if (scope_cache->is_ambiguous_enum_value(name)) {
    std::string possible_enums =
        scope_cache->get_fully_qualified_enum_value_names(name).c_str();
    warning(
        loc,
        "The ambiguous enum `{}` is defined in more than one place. "
        "Please refer to this enum using ENUM_NAME.ENUM_VALUE.{}",
        name,
        possible_enums.empty() ? "" : " Possible options: " + possible_enums);
  }
}

void parsing_driver::clear_doctext() {
  if (doctext && mode == parsing_mode::PROGRAM) {
    ctx_.warning_legacy_strict(doctext->loc, "uncaptured doctext");
  }
  doctext = boost::none;
}

boost::optional<doc> parsing_driver::pop_doctext() {
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
    t_node* node, std::unique_ptr<t_annotations> annotations) {
  if (annotations != nullptr) {
    node->reset_annotations(annotations->strings);
  }
}

void parsing_driver::set_attributes(
    t_named& node,
    std::unique_ptr<t_def_attrs> attrs,
    std::unique_ptr<t_annotations> annots,
    const source_range& range) const {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  node.set_src_range(range);
  if (attrs != nullptr) {
    if (attrs->doc) {
      node.set_doc(std::move(attrs->doc->text));
    }
    if (attrs->struct_annotations != nullptr) {
      for (auto& an : *attrs->struct_annotations) {
        node.add_structured_annotation(std::move(an));
      }
    }
  }
  set_annotations(&node, std::move(annots));
}

void parsing_driver::set_doctext(t_node& node, boost::optional<doc> doc) const {
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
  result->set_lineno(get_lineno());
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

void parsing_driver::set_fields(t_structured& tstruct, t_field_list&& fields) {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  assert(tstruct.fields().empty());
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  t_field_id next_id = -1;
  for (auto& field : fields) {
    maybe_allocate_field_id(next_id, *field);
    if (!tstruct.try_append_field(std::move(field))) {
      // Since we process root_program twice, we need to check parsing mode to
      // avoid double reporting
      if (mode == parsing_mode::PROGRAM ||
          program != program_bundle->root_program()) {
        ctx_.error(
            *field,
            "Field identifier {} for \"{}\" has already been used.",
            field->get_key(),
            field->get_name());
      }
    }
  }
}

t_type_ref parsing_driver::new_type_ref(
    const t_type& type, std::unique_ptr<t_annotations> annotations) {
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
      std::make_unique<t_typedef>(
          const_cast<t_program*>(type.program()), type.get_name(), type),
      std::move(annotations));
}

t_type_ref parsing_driver::new_type_ref(
    std::unique_ptr<t_templated_type> node,
    std::unique_ptr<t_annotations> annotations) {
  if (mode != parsing_mode::PROGRAM) {
    return {};
  }

  assert(node != nullptr);
  const t_type* type = node.get();
  set_annotations(node.get(), std::move(annotations));
  program->add_type_instantiation(std::move(node));
  return *type;
}

t_type_ref parsing_driver::new_type_ref(
    std::string name,
    std::unique_ptr<t_annotations> annotations,
    const source_range& range,
    bool is_const) {
  if (mode != parsing_mode::PROGRAM) {
    return {};
  }

  t_type_ref result = scope_cache->ref_type(*program, name, range);

  // TODO(afuller): Remove this special case for const, which requires a
  // specific declaration order.
  if (!result.resolved() && is_const) {
    error(
        range.begin,
        "The type '{}' is not defined yet. Types must be "
        "defined before the usage in constant values.",
        name);
  }

  if (auto* node = result.get_unresolved_type()) { // A newly created ph.
    node->set_lineno(get_lineno());
    set_annotations(node, std::move(annotations));
  } else if (annotations != nullptr) { // Oh no!
    // TODO(afuller): Remove support for annotations on type references.
    return new_type_ref(result.deref(), std::move(annotations));
  }

  return result;
}

void parsing_driver::set_functions(
    t_interface& node, std::unique_ptr<t_function_list> functions) {
  if (functions != nullptr) {
    node.set_functions(std::move(*functions));
  }
}

void parsing_driver::add_def(std::unique_ptr<t_named> node) {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }

  // Add to scope.
  // TODO(afuller): Move program level scope management to t_program.
  if (auto* tnode = dynamic_cast<t_interaction*>(node.get())) {
    scope_cache->add_interaction(program->scope_name(*node), tnode);
  } else if (auto* tnode = dynamic_cast<t_service*>(node.get())) {
    scope_cache->add_service(program->scope_name(*node), tnode);
  } else if (auto* tnode = dynamic_cast<t_const*>(node.get())) {
    scope_cache->add_constant(program->scope_name(*node), tnode);
  } else if (auto* tnode = dynamic_cast<t_enum*>(node.get())) {
    scope_cache->add_type(program->scope_name(*node), tnode);
    // Register enum value names in scope.
    for (const auto& value : tnode->consts()) {
      // TODO(afuller): Remove ability to access unscoped enum values.
      scope_cache->add_constant(program->scope_name(value), &value);
      scope_cache->add_constant(program->scope_name(*node, value), &value);
    }
  } else if (auto* tnode = dynamic_cast<t_type*>(node.get())) {
    scope_cache->add_type(program->scope_name(*node), tnode);
  } else {
    throw std::logic_error("Unsupported declaration.");
  }
  // Add to program.
  program->add_definition(std::move(node));
}

void parsing_driver::add_include(std::string name, const source_range& range) {
  if (mode != parsing_mode::INCLUDES) {
    return;
  }

  std::string path = find_include_file(range.begin, name);
  assert(!path.empty()); // Should have throw an exception if not found.

  if (program_cache.find(path) == program_cache.end()) {
    auto included_program =
        program->add_include(path, name, get_lineno(), range);
    program_cache[path] = included_program.get();
    program_bundle->add_program(std::move(included_program));
  } else {
    auto include = std::make_unique<t_include>(program_cache[path]);
    include->set_lineno(get_lineno());
    include->set_src_range(range);
    program->add_include(std::move(include));
  }
}

void parsing_driver::set_package(std::string name, const source_range& range) {
  if (mode != parsing_mode::PROGRAM) {
    return;
  }
  if (!program->package().empty()) {
    error(range.begin, "Package already specified.");
  }
  try {
    program->set_package(t_package(std::move(name)));
  } catch (const std::exception& e) {
    error(location(), "{}", e.what());
  }
}

const t_type* parsing_driver::add_unnamed_typedef(
    std::unique_ptr<t_typedef> node,
    std::unique_ptr<t_annotations> annotations) {
  const t_type* result(node.get());
  set_annotations(node.get(), std::move(annotations));
  program->add_unnamed_typedef(std::move(node));
  return result;
}

void parsing_driver::allocate_field_id(t_field_id& next_id, t_field& field) {
  if (params.strict >= 192) {
    ctx_.error(
        field,
        "Implicit field keys are deprecated and not allowed with -strict");
  }
  if (next_id < t_field::min_id) {
    ctx_.error(
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
  if (field.id() <= 0) {
    // TODO(afuller): Move this validation to ast_validator.
    if (params.allow_neg_field_keys) {
      /*
       * allow_neg_field_keys exists to allow users to add explicitly
       * specified id values to old .thrift files without breaking
       * protocol compatibility.
       */
      if (field.id() != next_id) {
        ctx_.warning(
            field,
            "Nonpositive field id ({}) differs from what would be "
            "auto-assigned by thrift ({}).",
            field.id(),
            next_id);
      }
    } else if (field.id() == next_id) {
      ctx_.warning(
          field,
          "Nonpositive value ({}) not allowed as a field id.",
          field.id());
    } else {
      // TODO(afuller): Make ignoring the user provided value a failure.
      ctx_.warning(
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
      /*
       * Update next field id to be one less than the value.
       * The FieldList parsing will catch any duplicate id values.
       */
      next_id = field.id() - 1;
    }
  }
}

std::unique_ptr<t_const_value> parsing_driver::to_const_value(
    source_location loc, int64_t value) {
  if (mode == parsing_mode::PROGRAM && !params.allow_64bit_consts &&
      (value < INT32_MIN || value > INT32_MAX)) {
    warning(loc, "64-bit constant {} may not work in all languages", value);
  }
  auto node = std::make_unique<t_const_value>();
  node->set_integer(value);
  return node;
}

int64_t parsing_driver::to_int(uint64_t val, bool negative) {
  constexpr uint64_t i64max = std::numeric_limits<int64_t>::max();
  if (negative) {
    if (mode == parsing_mode::PROGRAM && val > i64max + 1) {
      error(location(), "integer constant -{} is too small", val);
    }
    return -val;
  }

  if (mode == parsing_mode::PROGRAM && val > i64max) {
    error(location(), "integer constant {} is too large", val);
  }
  return val;
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

const t_service* parsing_driver::find_service(const std::string& name) {
  if (mode == parsing_mode::PROGRAM) {
    if (auto* result = scope_cache->find_service(name)) {
      return result;
    }
    if (auto* result = scope_cache->find_service(program->scope_name(name))) {
      return result;
    }
    error(location(), "Service \"{}\" has not been defined.", name);
  }
  return nullptr;
}

const t_const* parsing_driver::find_const(
    source_location loc, const std::string& name) {
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
}

std::unique_ptr<t_const_value> parsing_driver::copy_const_value(
    source_location loc, const std::string& name) {
  if (const t_const* constant = find_const(loc, name)) {
    // Copy const_value to perform isolated mutations
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
    warning(
        loc,
        "The identifier '{}' is not defined yet. Constants and enums should "
        "be defined before using them as default values.",
        name);
  }
  return std::make_unique<t_const_value>(name);
}

void parsing_driver::set_parsed_definition() {
  if (mode == parsing_mode::PROGRAM) {
    programs_that_parsed_definition_.insert(program->path());
  }
}

void parsing_driver::validate_header_location() {
  if (programs_that_parsed_definition_.find(program->path()) !=
      programs_that_parsed_definition_.end()) {
    error(location(), "Headers must be specified before definitions.");
  }
}

void parsing_driver::validate_header_annotations(
    std::unique_ptr<t_def_attrs> statement_attrs,
    std::unique_ptr<t_annotations> annotations) {
  // Ideally the errors below have to be handled by a grammar, but it's not
  // expressive enough to avoid conflicts when doing so.
  if (statement_attrs && statement_attrs->struct_annotations.get()) {
    error(
        location(),
        "Structured annotations are not supported for a given entity.");
  }
  if (annotations) {
    error(location(), "Annotations are not supported for a given entity.");
  }
}

void parsing_driver::set_program_annotations(
    std::unique_ptr<t_def_attrs> statement_attrs,
    std::unique_ptr<t_annotations> annotations,
    const source_range& loc) {
  set_attributes(
      *program, std::move(statement_attrs), std::move(annotations), loc);
}

} // namespace compiler
} // namespace thrift
} // namespace apache
