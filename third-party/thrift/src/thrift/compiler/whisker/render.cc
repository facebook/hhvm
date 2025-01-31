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
#include <iterator>
#include <ostream>
#include <set>
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

/**
 * A class defining a partial block as created by `{{#let partial}}` blocks.
 *
 * When the renderer encounters a `{{#let partial}}` block, it produces a
 * `native_handle<partial_definition>` object and binds it to the name provided
 * in the partial block definition.
 *
 * When a partial is applied via `{{#partial ...}}` statement, the renderer will
 * assert that the provided object is actually a
 * `native_handle<partial_definition>`.
 */
class partial_definition final {
 public:
  using ptr = managed_ptr<partial_definition>;

  source_range loc;
  std::string name;
  std::set<std::string> arguments;
  // The AST's lifetime is managed by the renderer.
  std::reference_wrapper<const ast::bodies> bodies;
};

/**
 * A class that keeps track of the stack of partial applications, including the
 * locations in the source file where partials have been applied.
 *
 * This is necessary for partial applications, and aids with pragmas and
 * debugging.
 */
class source_stack {
 public:
  /**
   * State that is kept for each "source" file. This includes the root node and
   * each partial application's source location.
   */
  struct frame {
    /**
     * The evaluation context of the source frame. When a new frame is pushed:
     *   - macros — retain the context from the previous frame.
     *   - partials — derive a new context from the previous frame.
     */
    eval_context context;
    /**
     * For all elements except the top of the stack, this is the location of the
     * partial application within that source that led to the current stack.
     *
     * For the top of the source, this is an empty source_location.
     *
     * When a partial application occurs, the top of the stack has the location
     * saved before the new source is pushed to the stack. After the partial
     * application completes, the saved location is dropped.
     */
    source_location apply_location;
    bool ignore_newlines = false;

    explicit frame(eval_context ctx) : context(std::move(ctx)) {}
  };

  /**
   * Returns the current source stack frame, or nullptr if the stack is empty.
   */
  frame* top() {
    if (frames_.empty()) {
      return nullptr;
    }
    return &frames_.back();
  }

  /**
   * An RAII guard that pushes and pops sources from the stack of partial
   * applications.
   */
  auto make_frame_guard(eval_context eval_ctx, source_location apply_location) {
    class frame_guard {
     public:
      explicit frame_guard(
          source_stack& stack,
          eval_context eval_ctx,
          source_location apply_location)
          : stack_(stack) {
        if (auto* frame = stack_.top()) {
          frame->apply_location = std::move(apply_location);
        }
        stack.frames_.emplace_back(std::move(eval_ctx));
      }
      ~frame_guard() noexcept {
        assert(!stack_.frames_.empty());
        stack_.frames_.pop_back();
        if (auto* source = stack_.top()) {
          source->apply_location = source_location();
        }
      }
      frame_guard(frame_guard&& other) = delete;
      frame_guard& operator=(frame_guard&& other) = delete;
      frame_guard(const frame_guard& other) = delete;
      frame_guard& operator=(const frame_guard& other) = delete;

     private:
      source_stack& stack_;
    };
    return frame_guard{*this, std::move(eval_ctx), std::move(apply_location)};
  }

  using backtrace = std::vector<resolved_location>;
  /**
   * Creates a back trace for debugging that contains the chain of partial
   * applications within the source.
   *
   * The source_location from the current stack frame must be provided by the
   * caller.
   */
  backtrace make_backtrace_at(const source_location& current) const {
    assert(!frames_.empty());
    assert(current != source_location());

    std::vector<resolved_location> result;
    result.emplace_back(resolved_location(current, diags_.source_mgr()));
    for (auto frame = std::next(frames_.rbegin()); frame != frames_.rend();
         ++frame) {
      assert(frame->apply_location != source_location());
      result.emplace_back(
          resolved_location(frame->apply_location, diags_.source_mgr()));
    }
    return result;
  }

  explicit source_stack(diagnostics_engine& diags) : diags_(diags) {}

 private:
  // Using std::vector as a stack so we can iterate over it
  std::vector<frame> frames_;
  diagnostics_engine& diags_;
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
bool coerce_to_boolean(const native_function::ptr&) {
  return true;
}
bool coerce_to_boolean(const native_handle<>&) {
  return true;
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
struct render_error : std::exception {
  explicit render_error(source_stack::backtrace backtrace)
      : backtrace_(std::move(backtrace)) {
    assert(!backtrace_.empty());
  }
  const source_stack::backtrace& backtrace() const { return backtrace_; }

 private:
  source_stack::backtrace backtrace_;
};

class render_engine {
 public:
  explicit render_engine(
      std::ostream& out,
      object::ptr root_context,
      diagnostics_engine& diags,
      render_options opts)
      : out_(out),
        root_context_(std::move(root_context)),
        diags_(diags),
        source_stack_(diags_),
        opts_(std::move(opts)) {}

  bool visit(const ast::root& root) {
    try {
      auto flush_guard = out_.make_flush_guard();
      auto eval_ctx = eval_context::with_root_scope(
          std::move(root_context_), std::exchange(opts_.globals, {}));
      auto source_frame_guard = source_stack_.make_frame_guard(
          std::move(eval_ctx), source_location());
      visit(root.body_elements);
      return true;
    } catch (const render_error& err) {
      if (!diags_.params().should_report(
              opts_.show_source_backtrace_on_failure)) {
        return false;
      }
      const source_stack::backtrace& backtrace = err.backtrace();
      assert(!backtrace.empty());

      auto source_trace = [&]() -> std::string {
        std::string result;
        for (std::size_t i = 0; i < backtrace.size(); ++i) {
          const auto& frame = backtrace[i];
          fmt::format_to(
              std::back_inserter(result),
              "#{} {} <line:{}, col:{}>\n",
              i,
              frame.file_name(),
              frame.line(),
              frame.column());
        }
        return result;
      }();

      diags_.error(
          source_location(),
          "The source backtrace is:\n{}",
          std::move(source_trace));
      return false;
    }
  }

 private:
  using frame = source_stack::frame;
  /**
   * Returns the current frame of the source stack. The frame holds necessary
   * information such as the current evaluation context.
   *
   * The first frame is pushed when rendering begins (ast::root) so the source
   * stack is (almost) never empty.
   */
  frame& current_frame() {
    assert(source_stack_.top() != nullptr);
    return *source_stack_.top();
  }
  eval_context& eval_ctx() { return current_frame().context; }

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

  [[noreturn]] void abort_rendering(const source_location& loc) {
    throw render_error(source_stack_.make_backtrace_at(loc));
  }

  template <typename... T>
  [[noreturn]] void report_fatal_error(
      source_location loc, fmt::format_string<T...> msg, T&&... args) {
    diags_.error(loc, msg, std::forward<T>(args)...);
    abort_rendering(loc);
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
  void visit(const ast::newline& newline) {
    if (!source_stack_.top()->ignore_newlines) {
      out_.write(newline);
    }
  }
  void visit(const ast::comment&) {
    // comments are not rendered in the output
  }

  // Performs a lookup of a variable in the current scope or reports diagnostics
  // on failure. Failing to lookup a variable is a fatal error.
  object::ptr lookup_variable(const ast::variable_lookup& variable_lookup) {
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
        eval_ctx().lookup_object(path),
        [](const object::ptr& value) -> object::ptr { return value; },
        [&](const eval_scope_lookup_error& err) -> object::ptr {
          auto scope_trace = [&]() -> std::string {
            std::string result =
                "Tried to search through the following scopes:\n";
            for (std::size_t i = 0; i < err.searched_scopes().size(); ++i) {
              const std::string_view maybe_newline = i == 0 ? "" : "\n";
              object_print_options print_opts;
              print_opts.max_depth = 1;
              fmt::format_to(
                  std::back_inserter(result),
                  "{}#{} {}",
                  maybe_newline,
                  i,
                  to_string(*err.searched_scopes()[i], std::move(print_opts)));
            }
            return result;
          }();

          maybe_report(variable_lookup.loc, undefined_diag_level, [&] {
            return fmt::format(
                "Name '{}' was not found in the current scope. {}",
                err.property_name(),
                scope_trace);
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            abort_rendering(variable_lookup.loc.begin);
          }
          return manage_as_static(whisker::make::null);
        },
        [&](const eval_property_lookup_error& err) -> object::ptr {
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
                to_string(*err.missing_from(), std::move(print_opts)));
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            abort_rendering(variable_lookup.loc.begin);
          }
          return manage_as_static(whisker::make::null);
        });
  }

  object::ptr evaluate(const ast::expression& expr) {
    using expression = ast::expression;
    using function_call = expression::function_call;
    return detail::variant_match(
        expr.which,
        [](const expression::string_literal& s) -> object::ptr {
          return manage_owned<object>(whisker::make::string(s.text));
        },
        [](const expression::i64_literal& i) -> object::ptr {
          return manage_owned<object>(whisker::make::i64(i.value));
        },
        [](const expression::null_literal&) -> object::ptr {
          return manage_as_static(whisker::make::null);
        },
        [](const expression::true_literal&) -> object::ptr {
          return manage_as_static(whisker::make::true_);
        },
        [](const expression::false_literal&) -> object::ptr {
          return manage_as_static(whisker::make::false_);
        },
        [&](const ast::variable_lookup& variable_lookup) -> object::ptr {
          return lookup_variable(variable_lookup);
        },
        [&](const function_call& func) -> object::ptr {
          return detail::variant_match(
              func.which,
              [&](function_call::builtin_not) -> object::ptr {
                // enforced by the parser
                assert(func.positional_arguments.size() == 1);
                assert(func.named_arguments.empty());
                return evaluate_as_bool(func.positional_arguments[0])
                    ? manage_as_static(whisker::make::false_)
                    : manage_as_static(whisker::make::true_);
              },
              [&](function_call::builtin_and) -> object::ptr {
                // enforced by the parser
                assert(func.named_arguments.empty());
                for (const expression& arg : func.positional_arguments) {
                  if (!evaluate_as_bool(arg)) {
                    return manage_as_static(whisker::make::false_);
                  }
                }
                return manage_as_static(whisker::make::true_);
              },
              [&](function_call::builtin_or) -> object::ptr {
                // enforced by the parser
                assert(func.named_arguments.empty());
                for (const expression& arg : func.positional_arguments) {
                  if (evaluate_as_bool(arg)) {
                    return manage_as_static(whisker::make::true_);
                  }
                }
                return manage_as_static(whisker::make::false_);
              },
              [&](const function_call::user_defined& user_defined)
                  -> object::ptr {
                const ast::variable_lookup& name = user_defined.name;
                object::ptr lookup_result = lookup_variable(name);
                if (!lookup_result->is_native_function()) {
                  report_fatal_error(
                      name.loc.begin,
                      "Object '{}' is not a function. The encountered value is:\n{}",
                      name.chain_string(),
                      to_string(*lookup_result));
                }
                const native_function::ptr& f =
                    lookup_result->as_native_function();

                native_function::positional_arguments_t positional_args;
                positional_args.reserve(func.positional_arguments.size());
                for (const expression& arg : func.positional_arguments) {
                  positional_args.push_back(evaluate(arg));
                }

                native_function::named_arguments_t named_args;
                for (const auto& [arg_name, entry] : func.named_arguments) {
                  [[maybe_unused]] const auto& [_, inserted] =
                      named_args.emplace(arg_name, evaluate(*entry.value));
                  assert(inserted);
                }

                native_function::context ctx{
                    expr.loc,
                    diags_,
                    std::move(positional_args),
                    std::move(named_args)};
                try {
                  return f->invoke(std::move(ctx));
                } catch (const native_function::fatal_error& err) {
                  report_fatal_error(
                      name.loc.begin,
                      "Function '{}' threw an error:\n{}",
                      name.chain_string(),
                      err.what());
                }
              });
        });
  }

  void bind_local(
      eval_context& ctx,
      source_location loc,
      std::string name,
      object::ptr value) {
    whisker::visit(
        ctx.bind_local(std::move(name), value),
        [](std::monostate) {
          // The binding was successful
        },
        [&](const eval_name_already_bound_error& err) {
          report_fatal_error(
              loc,
              "Name '{}' is already bound in the current scope.",
              err.name());
        });
  }

  void visit(const ast::let_statement& let_statement) {
    bind_local(
        eval_ctx(),
        let_statement.loc.begin,
        let_statement.id.name,
        evaluate(let_statement.value));
  }

  void visit(const ast::pragma_statement& pragma_statement) {
    using pragma = ast::pragma_statement::pragmas;
    switch (pragma_statement.pragma) {
      case pragma::ignore_newlines:
        source_stack_.top()->ignore_newlines = true;
        break;
    }
  }

  void visit(const ast::interpolation& interpolation) {
    object::ptr result = evaluate(interpolation.content);

    const auto report_unprintable_message_only = [&](diagnostic_level level) {
      maybe_report(interpolation.loc, level, [&] {
        return fmt::format(
            "Object named '{}' is not printable. The encountered value is:\n{}",
            interpolation.to_string(),
            to_string(*result));
      });
    };

    const auto report_unprintable = [&]() {
      auto level = opts_.strict_printable_types;
      report_unprintable_message_only(level);
      if (level == diagnostic_level::error) {
        // Fail rendering in strict mode
        abort_rendering(interpolation.loc.begin);
      }
    };

    // See render_options::strict_printable_types for printing rules
    auto output = result->visit(
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
          abort_rendering(interpolation.loc.begin);
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
      abort_rendering(expr.loc.begin);
    }
  }
  bool evaluate_as_bool(const ast::expression& expr) {
    object::ptr result = evaluate(expr);
    return result->visit(
        [&](boolean value) { return value; },
        [&](const auto& value) {
          maybe_report_boolean_coercion(expr, *result);
          return coerce_to_boolean(value);
        });
  }

  void visit(const ast::section_block& section) {
    object::ptr section_variable = lookup_variable(section.variable);

    const auto maybe_report_coercion = [&] {
      maybe_report_boolean_coercion(
          ast::expression{section.variable.loc, section.variable},
          *section_variable);
    };

    const auto do_visit = [&](object::ptr scope) {
      eval_ctx().push_scope(std::move(scope));
      visit(section.body_elements);
      eval_ctx().pop_scope();
    };

    const auto do_conditional_visit = [&](bool condition) {
      if (condition ^ section.inverted) {
        do_visit(section_variable);
      }
    };

    // See render_options::strict_boolean_conditional for the coercion
    // rules
    section_variable->visit(
        [&](const array& value) {
          if (section.inverted) {
            // This array is being used as a conditional
            maybe_report_coercion();
            if (!coerce_to_boolean(value)) {
              // Empty arrays are falsy
              do_visit(manage_as_static(whisker::make::null));
            }
            return;
          }
          for (const auto& element : value) {
            do_visit(manage_derived_ref(section_variable, element));
          }
        },
        [&](const native_object::ptr& value) {
          if (section.inverted) {
            // This native_object is being used as a conditional
            maybe_report_coercion();
            if (!coerce_to_boolean(value)) {
              // Empty array-like objects are falsy
              do_visit(manage_as_static(whisker::make::null));
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
            do_visit(manage_as_static(whisker::make::null));
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
      eval_ctx().push_scope(manage_as_static(whisker::make::null));
      visit(body_elements);
      eval_ctx().pop_scope();
    };

    // Returns whether the else clause should be evaluated.
    auto visit_else_if = [&](const ast::conditional_block& b) {
      for (const auto& clause : b.else_if_clauses) {
        if (evaluate_as_bool(clause.condition)) {
          do_visit(clause.body_elements);
          return true;
        }
      }
      return false;
    };

    const bool condition = evaluate_as_bool(conditional_block.condition);
    if (condition) {
      do_visit(conditional_block.body_elements);
    } else if (visit_else_if(conditional_block)) {
      // An else if clause was rendered.
    } else if (conditional_block.else_clause.has_value()) {
      do_visit(conditional_block.else_clause->body_elements);
    }
  }

  void visit(const ast::with_block& with_block) {
    const ast::expression& expr = with_block.value;
    object::ptr result = evaluate(expr);
    result->visit(
        [&](const map&) {
          // maps can be de-structured.
        },
        [&](const native_object::ptr& o) {
          // map-like native objects can be de-structured.
          if (o->as_map_like() == nullptr) {
            report_fatal_error(
                expr.loc.begin,
                "Expression '{}' is a native_object which is not map-like. The encountered value is:\n{}",
                expr.to_string(),
                to_string(*result));
          }
        },
        [&](auto&&) {
          report_fatal_error(
              expr.loc.begin,
              "Expression '{}' does not evaluate to a map. The encountered value is:\n{}",
              expr.to_string(),
              to_string(*result));
        });
    eval_ctx().push_scope(std::move(result));
    visit(with_block.body_elements);
    eval_ctx().pop_scope();
  }

  void visit(const ast::each_block& each_block) {
    const ast::expression& expr = each_block.iterable;
    object::ptr result = evaluate(expr);

    const auto do_visit = [this, &each_block](i64 index, object::ptr scope) {
      if (const auto& captured = each_block.captured) {
        eval_ctx().push_scope(manage_as_static(whisker::make::null));
        bind_local(
            eval_ctx(),
            captured->element.loc.begin,
            captured->element.name,
            std::move(scope));
        if (captured->index.has_value()) {
          bind_local(
              eval_ctx(),
              captured->index->loc.begin,
              captured->index->name,
              manage_owned<object>(whisker::make::i64(index)));
        }
      } else {
        // When captures are not present, each element becomes the implicit
        // context object (`{{.}}`).
        eval_ctx().push_scope(std::move(scope));
      }
      visit(each_block.body_elements);
      eval_ctx().pop_scope();
    };

    const auto do_visit_else = [this, &each_block]() {
      if (!each_block.else_clause.has_value()) {
        return;
      }
      eval_ctx().push_scope(manage_as_static(whisker::make::null));
      visit(each_block.else_clause->body_elements);
      eval_ctx().pop_scope();
    };

    result->visit(
        [&](const array& arr) {
          if (arr.empty()) {
            do_visit_else();
            return;
          }
          for (std::size_t i = 0; i < arr.size(); ++i) {
            do_visit(i64(i), manage_derived_ref(result, arr[i]));
          }
        },
        [&](const native_object::ptr& o) {
          // array-like native objects are iterable.
          native_object::array_like::ptr array_like = o->as_array_like();
          if (array_like == nullptr) {
            report_fatal_error(
                expr.loc.begin,
                "Expression '{}' is a native_object which is not array-like. The encountered value is:\n{}",
                expr.to_string(),
                to_string(*result));
          }
          const std::size_t size = array_like->size();
          if (size == 0) {
            do_visit_else();
            return;
          }
          for (std::size_t i = 0; i < size; ++i) {
            do_visit(i64(i), array_like->at(i));
          }
        },
        [&](auto&&) {
          report_fatal_error(
              expr.loc.begin,
              "Expression '{}' does not evaluate to an array. The encountered value is:\n{}",
              expr.to_string(),
              to_string(*result));
        });
  }

  void visit(const ast::partial_block& partial_block) {
    std::string name = partial_block.name.name;

    std::set<std::string> arguments;
    for (const ast::identifier& id : partial_block.arguments) {
      auto [_, inserted] = arguments.insert(id.name);
      if (!inserted) {
        report_fatal_error(
            id.loc.begin,
            "Duplicate capture name in partial block definition '{}'",
            name);
      }
    }

    partial_definition::ptr definition =
        manage_owned<partial_definition>(partial_definition{
            partial_block.loc,
            name,
            std::move(arguments),
            std::cref(partial_block.body_elements),
        });
    bind_local(
        eval_ctx(),
        partial_block.name.loc.begin,
        std::move(name),
        manage_owned<object>(native_handle<>(std::move(definition))));
  }

  void visit(const ast::partial_statement& partial_statement) {
    object::ptr lookup = evaluate(partial_statement.partial);

    partial_definition::ptr partial = std::invoke([&] {
      if (lookup->is_native_handle()) {
        if (std::optional<native_handle<partial_definition>> handle =
                lookup->as_native_handle().try_as<partial_definition>()) {
          return handle->ptr();
        }
      }
      report_fatal_error(
          partial_statement.partial.loc.begin,
          "Expression '{}' does not evaluate to a partial. The encountered value is:\n{}",
          partial_statement.partial.to_string(),
          to_string(*lookup));
    });

    const auto& named_arguments = partial_statement.named_arguments;
    auto [missing, extras] = std::invoke([&] {
      std::set<std::string> not_provided = partial->arguments;
      std::set<std::string> leftovers;
      for (const auto& [name, _] : named_arguments) {
        if (not_provided.erase(name) == 0) {
          leftovers.insert(name);
        }
      }
      return std::pair{std::move(not_provided), std::move(leftovers)};
    });
    if (!missing.empty()) {
      report_fatal_error(
          partial_statement.loc.begin,
          "Partial '{}' is missing named arguments: {}",
          partial->name,
          fmt::join(missing, ", "));
    }
    if (!extras.empty()) {
      report_fatal_error(
          partial_statement.loc.begin,
          "Partial '{}' received unexpected named arguments: {}",
          partial->name,
          fmt::join(extras, ", "));
    }

    // Partials get a new evaluation context derived from the current one.
    auto derived_ctx = eval_ctx().make_derived();
    // Make the partial itself available in the derived context
    bind_local(
        derived_ctx,
        partial_statement.partial.loc.begin,
        partial->name,
        lookup);

    for (const std::string& argument : partial->arguments) {
      auto arg = named_arguments.find(argument);
      // We checked against argument mismatches above
      assert(arg != named_arguments.end());
      const ast::identifier& id = arg->second.name;
      const ast::expression& expr = arg->second.value;
      bind_local(derived_ctx, id.loc.begin, argument, evaluate(expr));
    }

    auto source_frame_guard = source_stack_.make_frame_guard(
        std::move(derived_ctx), partial_statement.loc.begin);
    auto indent_guard =
        out_.make_indent_guard(partial_statement.standalone_offset_within_line);
    visit(partial->bodies);
  }

  void visit(const ast::macro& macro) {
    std::vector<std::string> path;
    path.reserve(macro.path.parts.size());
    for (const ast::path_component& component : macro.path.parts) {
      path.push_back(component.value);
    }

    auto* macro_resolver = opts_.macro_resolver.get();
    if (macro_resolver == nullptr) {
      report_fatal_error(
          macro.loc.begin,
          "No macro resolver was provided. Cannot resolve macro with path '{}'",
          macro.path_string());
    }

    auto resolved_macro =
        macro_resolver->resolve(path, macro.loc.begin, diags_);
    if (!resolved_macro.has_value()) {
      report_fatal_error(
          macro.loc.begin,
          "Macro with path '{}' was not found",
          macro.path_string());
    }

    // Macros are "inlined" into their invocation site. In other words, they
    // execute within the scope where they are invoked.
    auto source_frame_guard = source_stack_.make_frame_guard(
        current_frame().context, macro.loc.begin);
    auto indent_guard =
        out_.make_indent_guard(macro.standalone_offset_within_line);
    visit(resolved_macro->body_elements);
  }

  outputter out_;
  object::ptr root_context_;
  diagnostics_engine& diags_;
  source_stack source_stack_;
  render_options opts_;
};

} // namespace

bool render(
    std::ostream& out,
    const ast::root& root,
    const object& root_context,
    diagnostics_engine& diags,
    render_options opts) {
  render_engine engine{
      out, manage_as_static(root_context), diags, std::move(opts)};
  return engine.visit(root);
}

} // namespace whisker
