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

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/string.h>
#include <thrift/compiler/whisker/eval_context.h>
#include <thrift/compiler/whisker/render.h>

#include <cmath>
#include <exception>
#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

namespace whisker {

namespace {

/**
 * An abstraction around std::ostream& that buffers lines of output and
 * abstracts away indentation from the main renderer implementation.
 *
 * Line buffering allows the outputter to transparently add indentation to lines
 * as they are rendered. This is primarily needed for indentation of standalone
 * partial applications which have multiple lines. According to the Mustache
 * spec, such applications, should have all their lines indented.
 *   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L13-L15
 */
class outputter {
 public:
  explicit outputter(std::ostream& sink) : sink_(sink) {}
  ~outputter() noexcept { assert(!current_line_.has_value()); }

  void write(const ast::text& text) {
    // ast::text is guaranteed to have no newlines
    current_line().buffer += text.content;
  }

  void write(const ast::newline& newline) {
    current_line().buffer += newline.text;
    writeln_to_sink();
  }

  void write(std::string_view value) {
    for (char c : value) {
      current_line().buffer += c;
      if (detail::is_newline(c)) {
        writeln_to_sink();
      }
    }
  }

  void flush() {
    if (current_line_.has_value()) {
      writeln_to_sink();
    }
  }

  /**
   * An RAII guard that ensures that the outputter is flushed. Destroying the
   * outputter while there is a buffered line of output is not allowed. This
   * guard makes it easy to ensure this invariant.
   */
  auto make_flush_guard() {
    class flush_guard {
     public:
      explicit flush_guard(outputter& out) : out_(out) {}
      ~flush_guard() { out_.flush(); }

      flush_guard(flush_guard&& other) = delete;
      flush_guard& operator=(flush_guard&& other) = delete;

     private:
      outputter& out_;
    };
    return flush_guard(*this);
  }

  /**
   * An RAII guard that ensures that the adds indentation to the *next* line of
   * buffered output. This is needed for indentation of standalone partial
   * applications.
   */
  auto make_indent_guard(const std::optional<std::string>& indent) {
    // This implementation assumes that indent_guard lifetimes are nested.
    //
    // That is, if guard A was created before guard B, then guard B must be
    // destroyed before guard A.
    //
    // Otherwise, the guard will pop off the wrong item in the indentation
    // stack. This assumption allows using a std::vector instead of a std::list
    // for the stack.
    class indent_guard {
     public:
      explicit indent_guard(outputter& out, const std::string& indent)
          : out_(out) {
        out_.next_indent_.emplace_back(indent);
      }
      ~indent_guard() { out_.next_indent_.pop_back(); }

      indent_guard(indent_guard&& other) = delete;
      indent_guard& operator=(indent_guard&& other) = delete;

     private:
      outputter& out_;
    };
    using result = std::optional<indent_guard>;
    if (!indent.has_value()) {
      return result();
    }
    return result(std::in_place, *this, *indent);
  }

 private:
  void writeln_to_sink() {
    assert(current_line_.has_value());
    assert(!current_line_->buffer.empty());
    sink_ << current_line_->indent << std::move(current_line_->buffer);
    current_line_ = std::nullopt;
  }

  struct current_line_info {
    std::string buffer;
    std::string indent;
  };
  // Initialization is deferred so that we create the object with the correct
  // indentation at the time of the first print. This allows the renderer to
  // call make_indent_guard() before we "commit" to the indentation of a line of
  // output.
  std::optional<current_line_info> current_line_;

  current_line_info& current_line() {
    if (!current_line_.has_value()) {
      std::string indent;
      for (const auto& stack : next_indent_) {
        indent += stack;
      }
      current_line_ = {{}, std::move(indent)};
    }
    return *current_line_;
  }

  std::ostream& sink_;
  std::vector<std::string> next_indent_;
};

// The following coercion functions follow the rules described in
// render_options::strict_boolean_conditional.

bool coerce_to_boolean(null) {
  return false;
}
bool coerce_to_boolean(i64 value) {
  return value != 0;
}
bool coerce_to_boolean(f64 value) {
  return value != 0.0 && !std::isnan(value);
}
bool coerce_to_boolean(const string& value) {
  return !value.empty();
}
bool coerce_to_boolean(const array& value) {
  return !value.empty();
}
bool coerce_to_boolean(const native_object::ptr& value) {
  if (auto array_like = value->as_array_like(); array_like != nullptr) {
    return array_like->size() != 0;
  }
  if (auto map_like = value->as_map_like(); map_like != nullptr) {
    return true;
  }
  return false;
}
bool coerce_to_boolean(const map&) {
  return true;
}

/**
 * A fatal error that aborts rendering but contains no messaging. Diagnostics
 * should be attached to the diagnostics_engine.
 *
 * This is only used within the render_engine implementation to abruptly
 * terminate rendering.
 */
struct abort_rendering : std::exception {};

class render_engine {
 public:
  explicit render_engine(
      std::ostream& out,
      const object& root_context,
      diagnostics_engine& diags,
      render_options opts)
      : out_(out),
        eval_context_(eval_context::with_root_scope(
            root_context, std::exchange(opts.globals, {}))),
        diags_(diags),
        opts_(std::move(opts)) {}

  bool visit(const ast::root& root) {
    try {
      auto flush_guard = out_.make_flush_guard();
      visit(root.body_elements);
      return true;
    } catch (const abort_rendering&) {
      // errors should have been reported through diagnostics_engine
      return false;
    }
  }

 private:
  // Reports a diagnostic but avoids generating the diagnostic message unless
  // the diagnostic is actually reported. This can avoid expensive computation
  // which is then thrown away without being used.
  template <typename ReportFunc>
  void maybe_report(
      source_range loc, diagnostic_level level, ReportFunc&& report) {
    if (!diags_.params().should_report(level)) {
      return;
    }
    diags_.report(
        loc.begin, level, "{}", std::invoke(std::forward<ReportFunc>(report)));
  }

  void visit(const ast::bodies& bodies) {
    for (const auto& body : bodies) {
      visit(body);
    }
  }

  // Prevent implicit conversion to ast::body. Otherwise, we can silently
  // compile an infinitely recursive visit() chain if there is a missing
  // overload for one of the alternatives in the variant.
  template <
      typename T = ast::body,
      typename = std::enable_if_t<std::is_same_v<T, ast::body>>>
  void visit(const T& body) {
    detail::variant_match(body, [&](const auto& node) { visit(node); });
  }

  void visit(const ast::text& text) { out_.write(text); }
  void visit(const ast::newline& newline) { out_.write(newline); }
  void visit(const ast::comment&) {
    // comments are not rendered in the output
  }

  // Performs a lookup of a variable in the current scope or reports diagnostics
  // on failure. Failing to lookup a variable is a fatal error.
  const object& lookup_variable(const ast::variable_lookup& variable_lookup) {
    using path_type = std::vector<std::string>;
    const path_type path = detail::variant_match(
        variable_lookup.chain,
        [](ast::variable_lookup::this_ref) -> path_type {
          // path should be empty for {{.}} lookups
          return {};
        },
        [&](const std::vector<ast::identifier>& chain) -> path_type {
          path_type result;
          result.reserve(chain.size());
          for (const ast::identifier& id : chain) {
            result.push_back(id.name);
          }
          return result;
        });

    auto undefined_diag_level = opts_.strict_undefined_variables;

    return whisker::visit(
        eval_context_.lookup_object(path),
        [](const object& value) -> const object& { return value; },
        [&](const eval_scope_lookup_error& err) -> const object& {
          std::vector<std::string> scope_trace;
          scope_trace.reserve(err.searched_scopes().size());
          for (std::size_t i = 0; i < err.searched_scopes().size(); ++i) {
            object_print_options print_opts;
            print_opts.max_depth = 1;
            scope_trace.push_back(fmt::format(
                "#{} {}",
                i,
                to_string(err.searched_scopes()[i], std::move(print_opts))));
          }

          maybe_report(variable_lookup.loc, undefined_diag_level, [&] {
            return fmt::format(
                "Name '{}' was not found in the current scope. Tried to search through the following scopes:\n{}",
                err.property_name(),
                fmt::join(scope_trace, "\n"));
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            throw abort_rendering();
          }
          return whisker::make::null;
        },
        [&](const eval_property_lookup_error& err) -> const object& {
          auto src_range = detail::variant_match(
              variable_lookup.chain,
              [&](ast::variable_lookup::this_ref) -> source_range {
                return variable_lookup.loc;
              },
              [&](const std::vector<ast::identifier>& chain) -> source_range {
                // Move to the start of the identifier that failed to resolve
                return chain[err.success_path().size()].loc;
              });
          maybe_report(std::move(src_range), undefined_diag_level, [&] {
            object_print_options print_opts;
            print_opts.max_depth = 1;
            return fmt::format(
                "Object '{}' has no property named '{}'. The object with the missing property is:\n{}",
                fmt::join(err.success_path(), "."),
                err.property_name(),
                to_string(err.missing_from(), std::move(print_opts)));
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            throw abort_rendering();
          }
          return whisker::make::null;
        });
  }

  object evaluate(const ast::expression& expr) {
    return detail::variant_match(
        expr.content,
        [&](const ast::variable_lookup& variable_lookup) {
          return lookup_variable(variable_lookup);
        },
        [&](const ast::expression::function_call& func) {
          return detail::variant_match(
              func.which, [&](ast::expression::function_call::not_tag) {
                assert(func.args.size() == 1); // enforced by the parser
                return object{!evaluate_as_bool(func.args[0])};
              });
        });
  }

  void visit(const ast::interpolation& interpolation) {
    const object& value = evaluate(interpolation.content);

    const auto report_unprintable_message_only = [&](diagnostic_level level) {
      maybe_report(interpolation.loc, level, [&] {
        return fmt::format(
            "Object named '{}' is not printable. The encountered value is:\n{}",
            interpolation.to_string(),
            to_string(value));
      });
    };

    const auto report_unprintable = [&]() {
      auto level = opts_.strict_printable_types;
      report_unprintable_message_only(level);
      if (level == diagnostic_level::error) {
        // Fail rendering in strict mode
        throw abort_rendering();
      }
    };

    // See render_options::strict_printable_types for printing rules
    auto output = value.visit(
        [](const string& value) -> std::string { return value; },
        [](i64 value) -> std::string { return std::to_string(value); },
        [&](f64 value) -> std::string {
          report_unprintable();
          return fmt::format("{}", value);
        },
        [&](boolean value) -> std::string {
          report_unprintable();
          return value ? "true" : "false";
        },
        [&](null) -> std::string {
          report_unprintable();
          return "";
        },
        [&](auto&&) -> std::string {
          // Other types are never printable
          report_unprintable_message_only(diagnostic_level::error);
          throw abort_rendering();
        });
    out_.write(std::move(output));
  }

  /**
   * Reports a diagnostic and fails rendering depending on the type of the
   * provided value and render_options::strict_boolean_conditional.
   */
  void maybe_report_boolean_coercion(
      const ast::expression& expr, const object& value) {
    auto diag_level = opts_.strict_boolean_conditional;
    maybe_report(expr.loc, diag_level, [&] {
      return fmt::format(
          "Condition '{}' is not a boolean. The encountered value is:\n{}",
          expr.to_string(),
          to_string(value));
    });
    if (diag_level == diagnostic_level::error) {
      // Fail rendering in strict mode
      throw abort_rendering();
    }
  }
  bool evaluate_as_bool(const ast::expression& expr) {
    const object result = evaluate(expr);
    return result.visit(
        [&](boolean value) { return value; },
        [&](const auto& value) {
          maybe_report_boolean_coercion(expr, result);
          return coerce_to_boolean(value);
        });
  }

  void visit(const ast::section_block& section) {
    const object& section_variable = lookup_variable(section.variable);

    const auto maybe_report_coercion = [&] {
      maybe_report_boolean_coercion(
          ast::expression{section.variable.loc, section.variable},
          section_variable);
    };

    const auto do_visit = [&](const object& scope) {
      eval_context_.push_scope(scope);
      visit(section.body_elements);
      eval_context_.pop_scope();
    };

    const auto do_conditional_visit = [&](bool condition) {
      if (condition ^ section.inverted) {
        do_visit(section_variable);
      }
    };

    // See render_options::strict_boolean_conditional for the coercion
    // rules
    section_variable.visit(
        [&](const array& value) {
          if (section.inverted) {
            // This array is being used as a conditional
            maybe_report_coercion();
            if (!coerce_to_boolean(value)) {
              // Empty arrays are falsy
              do_visit(whisker::make::null);
            }
            return;
          }
          for (const auto& element : value) {
            do_visit(element);
          }
        },
        [&](const native_object::ptr& value) {
          if (section.inverted) {
            // This native_object is being used as a conditional
            maybe_report_coercion();
            if (!coerce_to_boolean(value)) {
              // Empty array-like objects are falsy
              do_visit(whisker::make::null);
            }
            return;
          }
          // When used as a section_block, a native_object which is both
          // "map"-like and "array"-like is ambiguous. We arbitrarily choose
          // "array"-like as the winner. In practice, a native_object is most
          // likely to be one or the other.
          //
          // This is one of the reasons that section blocks are deprecated in
          // favor of `{{#each}}` and `{{#with}}`.
          if (auto array_like = value->as_array_like()) {
            const std::size_t size = array_like->size();
            for (std::size_t i = 0; i < size; ++i) {
              do_visit(array_like->at(i));
            }
            return;
          }
          if (auto map_like = value->as_map_like()) {
            do_visit(section_variable);
            return;
          }

          // Since this native_object is neither array-like nor map-like, it is
          // being used as a conditional
          maybe_report_coercion();
          if (coerce_to_boolean(value)) {
            do_visit(whisker::make::null);
          }
        },
        [&](const map&) {
          if (section.inverted) {
            // This map is being used as a conditional
            maybe_report_coercion();
            return;
          }
          // When maps are used in sections, they are "unpacked" into the block.
          // In other words, their properties become available in the current
          // scope.
          do_visit(section_variable);
        },
        [&](boolean value) { do_conditional_visit(value); },
        [&](const auto& value) {
          maybe_report_coercion();
          do_conditional_visit(coerce_to_boolean(value));
        });
  }

  void visit(const ast::conditional_block& conditional_block) {
    const auto do_visit = [&](const ast::bodies& body_elements) {
      eval_context_.push_scope(whisker::make::null);
      visit(body_elements);
      eval_context_.pop_scope();
    };

    const bool condition = evaluate_as_bool(conditional_block.condition);
    if (condition ^ conditional_block.unless) {
      do_visit(conditional_block.body_elements);
    } else if (conditional_block.else_clause.has_value()) {
      do_visit(conditional_block.else_clause->body_elements);
    }
  }

  void visit(const ast::partial_apply& partial_apply) {
    std::vector<std::string> path;
    path.reserve(partial_apply.path.parts.size());
    for (const ast::path_component& component : partial_apply.path.parts) {
      path.push_back(component.value);
    }

    auto* partial_resolver = opts_.partial_resolver.get();
    if (partial_resolver == nullptr) {
      diags_.error(
          partial_apply.loc.begin,
          "No partial resolver was provided. Cannot resolve partial with path '{}'",
          partial_apply.path_string());
      throw abort_rendering();
    }

    auto resolved_partial =
        partial_resolver->resolve(path, partial_apply.loc.begin, diags_);
    if (!resolved_partial.has_value()) {
      diags_.error(
          partial_apply.loc.begin,
          "Partial with path '{}' was not found",
          partial_apply.path_string());
      throw abort_rendering();
    }

    // Partials are "inlined" into their invocation site. In other words, they
    // execute within the scope where they are invoked.
    auto indent_guard =
        out_.make_indent_guard(partial_apply.standalone_offset_within_line);
    visit(resolved_partial->body_elements);
  }

  outputter out_;
  eval_context eval_context_;
  diagnostics_engine& diags_;
  render_options opts_;
};

} // namespace

bool render(
    std::ostream& out,
    const ast::root& root,
    const object& root_context,
    diagnostics_engine& diags,
    render_options opts) {
  render_engine engine{out, root_context, diags, std::move(opts)};
  return engine.visit(root);
}

} // namespace whisker
