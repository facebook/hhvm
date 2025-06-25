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
#include <list>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
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
    for (const ast::text::content& c : text.parts) {
      detail::variant_match(
          c, [&](const auto& s) { current_line().buffer += s.value; });
    }
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
  auto make_indent_guard(const std::optional<ast::text::whitespace>& indent) {
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
      explicit indent_guard(outputter& out, const ast::text::whitespace& indent)
          : out_(out) {
        out_.next_indent_.emplace_back(&indent);
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
      for (const auto& part : next_indent_) {
        indent += part->value;
      }
      current_line_ = {{}, std::move(indent)};
    }
    return *current_line_;
  }

  std::ostream& sink_;
  // The pointers are never null. The AST nodes outlive the renderer so a
  // non-owning reference is safe here.
  std::vector<const ast::text::whitespace*> next_indent_;
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
  std::map<ast::identifier, object, ast::identifier::compare_by_name> captures;
  // The AST's lifetime is managed by the renderer.
  std::reference_wrapper<const ast::bodies> bodies;
};

/**
 * A class that keeps track of the stack of imported modules and partial
 * applications. Each item in the stack is one of:
 *   - root source (where rendering began)
 *   - target of an import statement
 *   - partial application
 *   - macro
 *
 * Each previous item in the stack stores the source location from which a jump
 * occurred to the current item. This allows backtraces to be generated at any
 * point during rendering.
 *
 * This is also aids with pragmas and debugging.
 */
class source_stack {
 public:
  /**
   * State that is kept for each "source" file. This includes the root node and
   * each partial application's source location.
   */
  struct frame {
    /**
     * The frame from which the current frame was jumped to. This is
     * nullptr for the root frame.
     */
    frame* prev;
    /**
     * The evaluation context of the source frame. When a new frame is pushed:
     *   - macros — retain the context from the previous frame.
     *   - partials — derive a new context from the previous frame.
     */
    eval_context context;

    struct for_root {};
    struct for_import {};
    struct for_macro {};
    using for_partial = partial_definition::ptr;
    using reason = std::variant<for_root, for_import, for_macro, for_partial>;
    /**
     * The reason why this frame came into existence from the previous one. This
     * is used to annotate backtraces with useful information.
     */
    reason cause;

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
    source_location jumped_from;

    /**
     * For `{{#pragma ignore-newlines}}`.
     */
    bool ignore_newlines = false;

    explicit frame(frame* prev, eval_context ctx, reason cause)
        : prev(prev), context(std::move(ctx)), cause(std::move(cause)) {}
  };

  /**
   * Returns the current source stack frame, or nullptr if the stack is empty.
   */
  frame* top() { return frames_.empty() ? nullptr : &frames_.back(); }
  const frame* top() const {
    return frames_.empty() ? nullptr : &frames_.back();
  }

  /**
   * An RAII guard that pushes and pops sources from the stack of partial
   * applications.
   */
  auto make_frame_guard(
      eval_context eval_ctx,
      frame::reason reason,
      source_location jumped_from) {
    class frame_guard {
     public:
      explicit frame_guard(
          source_stack& stack,
          eval_context eval_ctx,
          frame::reason&& reason,
          source_location jumped_from)
          : stack_(stack) {
        if (auto* frame = stack_.top()) {
          // Save the jump location since we're jumping to a new frame. This
          // allows collecting a backtrace.
          frame->jumped_from = jumped_from;
        }
        stack.frames_.emplace_back(
            stack_.top(), std::move(eval_ctx), std::move(reason));
      }
      ~frame_guard() noexcept {
        assert(!stack_.frames_.empty());
        stack_.frames_.pop_back();
        if (auto* source = stack_.top()) {
          // Reset the jump location since we've return to its origin.
          source->jumped_from = source_location();
        }
      }
      frame_guard(frame_guard&& other) = delete;
      frame_guard& operator=(frame_guard&& other) = delete;
      frame_guard(const frame_guard& other) = delete;
      frame_guard& operator=(const frame_guard& other) = delete;

     private:
      source_stack& stack_;
    };
    return frame_guard{
        *this, std::move(eval_ctx), std::move(reason), jumped_from};
  }

  struct backtrace_frame {
    /**
     * The resolved source location where a jump happened, or the origin of
     * the backtrace for the top-most frame.
     */
    resolved_location location;
    /**
     * The reason for the frame's existence (e.g. partial application).
     */
    frame::reason cause;
  };
  using backtrace = std::vector<backtrace_frame>;
  /**
   * Creates a back trace for debugging that contains the chain of partial
   * applications within the source.
   *
   * The source_location from the current stack frame must be provided by the
   * caller.
   */
  backtrace make_backtrace_at(const source_location& current) const {
    assert(current != source_location());
    const frame* frame = top();
    assert(frame != nullptr);

    std::vector<backtrace_frame> result;
    result.emplace_back(backtrace_frame{
        resolved_location(current, diags_.source_mgr()),
        frame->cause,
    });
    frame = frame->prev;
    for (; frame != nullptr; frame = frame->prev) {
      assert(frame->jumped_from != source_location());
      result.emplace_back(backtrace_frame{
          resolved_location(frame->jumped_from, diags_.source_mgr()),
          frame->cause,
      });
    }
    return result;
  }

  explicit source_stack(diagnostics_engine& diags) : diags_(diags) {}

 private:
  // Doubly linked list provides stable iterators / pointers for backtraces.
  std::list<frame> frames_;
  diagnostics_engine& diags_;
};

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

/**
 * A class that provides common diagnostic helpers around source_stack that is
 * shared by different rendering components.
 */
class diagnoser {
 public:
  explicit diagnoser(diagnostics_engine& diags, source_stack& source_stack)
      : diags_(diags), source_stack_(source_stack) {}

  diagnostics_engine& engine() noexcept { return diags_; }

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

 private:
  diagnostics_engine& diags_;
  source_stack& source_stack_;
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
bool coerce_to_boolean(const array::ptr& value) {
  return value->size() > 0;
}
bool coerce_to_boolean(const native_function::ptr&) {
  return true;
}
bool coerce_to_boolean(const native_handle<>&) {
  return true;
}
bool coerce_to_boolean(const map::ptr&) {
  return true;
}

/**
 * A class that manages the "machine state" of a Whisker render.
 *
 * The VM manages:
 *   - The program stack, which includes...
 *       - local name bindings
 *       - partial application context switching
 *       - diagnostics (collecting backtraces)
 *   - Resolution of name lookups
 *   - Evaluation of expressions
 */
class virtual_machine {
 public:
  explicit virtual_machine(render_options& opts, diagnostics_engine& diags)
      : opts_(opts), source_stack_(diags), diags_(diags, source_stack_) {}

  source_stack& stack() { return source_stack_; }
  diagnoser& diags() { return diags_; }

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

  using lookup_result = eval_context::lookup_result;

  // Performs a lookup of a variable in the current scope or reports diagnostics
  // on failure. Failing to lookup a variable is a fatal error.
  lookup_result lookup_variable(const ast::variable_lookup& variable_lookup) {
    auto undefined_diag_level = opts_.strict_undefined_variables;
    return whisker::visit(
        current_frame().context.look_up_object(variable_lookup),
        [](const lookup_result& value) -> lookup_result { return value; },
        [&](const eval_scope_lookup_error& err) -> lookup_result {
          diags_.maybe_report(variable_lookup.loc, undefined_diag_level, [&] {
            std::string message;
            auto out = std::back_inserter(message);
            fmt::format_to(
                out,
                "Name '{}' was not found in the current scope.\n",
                err.property_name());

            if (!err.cause().empty()) {
              fmt::format_to(out, "Cause: {}\n", err.cause());
            }

            message += "Tried to search through the following scopes:\n";
            for (std::size_t i = 0; i < err.searched_scopes().size(); ++i) {
              const std::string_view maybe_newline = i == 0 ? "" : "\n";
              object_print_options print_opts;
              print_opts.max_depth = 3;
              fmt::format_to(
                  out,
                  "{}#{} {}",
                  maybe_newline,
                  i,
                  to_string(err.searched_scopes()[i], print_opts));
            }
            return message;
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            diags_.abort_rendering(variable_lookup.loc.begin);
          }
          return lookup_result::without_parent(whisker::make::null);
        },
        [&](const eval_property_lookup_error& err) -> lookup_result {
          auto src_range = detail::variant_match(
              variable_lookup.chain,
              [&](ast::variable_lookup::this_ref) -> source_range {
                return variable_lookup.loc;
              },
              [&](const std::vector<ast::identifier>& chain) -> source_range {
                // Move to the start of the identifier that failed to resolve
                return chain[err.success_path().size()].loc;
              });
          diags_.maybe_report(src_range, undefined_diag_level, [&] {
            object_print_options print_opts;
            print_opts.max_depth = 1;
            return fmt::format(
                "Object '{}' has no property named '{}'.\n"
                "{}"
                "The object with the missing property is:\n{}",
                fmt::join(err.success_path(), "."),
                err.property_name(),
                err.cause().empty() ? std::string()
                                    : fmt::format("Cause: {}\n", err.cause()),
                to_string(err.missing_from(), print_opts));
          });
          if (undefined_diag_level == diagnostic_level::error) {
            // Fail rendering in strict mode
            diags_.abort_rendering(variable_lookup.loc.begin);
          }
          return lookup_result::without_parent(whisker::make::null);
        });
  }

  /**
   * Reports a diagnostic and fails rendering depending on the type of the
   * provided value and render_options::strict_boolean_conditional.
   */
  void maybe_report_boolean_coercion(
      const ast::expression& expr, const object& value) {
    auto diag_level = opts_.strict_boolean_conditional;
    diags_.maybe_report(expr.loc, diag_level, [&] {
      return fmt::format(
          "Condition '{}' is not a boolean. The encountered value is:\n{}",
          expr.to_string(),
          to_string(value));
    });
    if (diag_level == diagnostic_level::error) {
      // Fail rendering in strict mode
      diags_.abort_rendering(expr.loc.begin);
    }
  }
  bool evaluate_as_bool(const ast::expression& expr) {
    object result = evaluate(expr);
    return result.visit(
        [&](boolean value) { return value; },
        [&](const auto& value) {
          maybe_report_boolean_coercion(expr, result);
          return coerce_to_boolean(value);
        });
  }

  object evaluate(const ast::expression& expr) {
    using expression = ast::expression;
    using function_call = expression::function_call;
    return detail::variant_match(
        expr.which,
        [](const expression::string_literal& s) -> object {
          return whisker::make::string(s.text);
        },
        [](const expression::i64_literal& i) -> object {
          return whisker::make::i64(i.value);
        },
        [](const expression::null_literal&) -> object {
          return whisker::make::null;
        },
        [](const expression::true_literal&) -> object {
          return whisker::make::true_;
        },
        [](const expression::false_literal&) -> object {
          return whisker::make::false_;
        },
        [&](const ast::variable_lookup& variable_lookup) -> object {
          return lookup_variable(variable_lookup).found;
        },
        [&](const function_call& func) -> object {
          return detail::variant_match(
              func.which,
              [&](function_call::builtin_not) -> object {
                // enforced by the parser
                assert(func.positional_arguments.size() == 1);
                assert(func.named_arguments.empty());
                return evaluate_as_bool(func.positional_arguments[0])
                    ? whisker::make::false_
                    : whisker::make::true_;
              },
              [&](function_call::builtin_and) -> object {
                // enforced by the parser
                assert(func.named_arguments.empty());
                for (const expression& arg : func.positional_arguments) {
                  if (!evaluate_as_bool(arg)) {
                    return whisker::make::false_;
                  }
                }
                return whisker::make::true_;
              },
              [&](function_call::builtin_or) -> object {
                // enforced by the parser
                assert(func.named_arguments.empty());
                for (const expression& arg : func.positional_arguments) {
                  if (evaluate_as_bool(arg)) {
                    return whisker::make::true_;
                  }
                }
                return whisker::make::false_;
              },
              [&](const function_call::user_defined& user_defined) -> object {
                const ast::variable_lookup& name = user_defined.name;
                lookup_result lookup = lookup_variable(name);
                if (!lookup->is_native_function()) {
                  diags_.report_fatal_error(
                      name.loc.begin,
                      "Object '{}' is not a function. The encountered value is:\n{}",
                      name.chain_string(),
                      to_string(*lookup));
                }
                const native_function::ptr& f = lookup->as_native_function();

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
                    diags_.engine(),
                    lookup.parent,
                    std::move(positional_args),
                    std::move(named_args)};
                try {
                  return f->invoke(std::move(ctx));
                } catch (const eval_error& err) {
                  diags_.report_fatal_error(
                      name.loc.begin,
                      "Function '{}' threw an error:\n{}",
                      name.chain_string(),
                      err.what());
                }
              });
        });
  }

  /**
   * Creates a name binding in the local scope of the provided context with the
   * provided value.
   */
  void bind_local_in(
      eval_context& ctx, source_location loc, std::string name, object value) {
    whisker::visit(
        ctx.bind_local(std::move(name), std::move(value)),
        [](std::monostate) {
          // The binding was successful
        },
        [&](const eval_name_already_bound_error& err) {
          diags_.report_fatal_error(
              loc,
              "Name '{}' is already bound in the current scope.",
              err.name());
        });
  }
  void bind_local(source_location loc, std::string name, object value) {
    bind_local_in(
        current_frame().context, loc, std::move(name), std::move(value));
  }

  /**
   * Creates an opaque native_handle representing a partial-block definition.
   */
  native_handle<> create_partial_definition(
      const ast::partial_block& partial_block) {
    const std::string& name = partial_block.name.name;

    std::set<std::string> arguments;
    for (const ast::identifier& id : partial_block.arguments) {
      auto [_, inserted] = arguments.insert(id.name);
      if (!inserted) {
        diags_.report_fatal_error(
            id.loc.begin,
            "Duplicate capture name in partial block definition '{}'",
            name);
      }
    }

    std::map<ast::identifier, object, ast::identifier::compare_by_name>
        captures;
    for (const ast::identifier& capture : partial_block.captures) {
      object captured_object =
          lookup_variable(
              ast::variable_lookup{capture.loc, std::vector{capture}})
              .found;
      captures.emplace(std::pair{capture, std::move(captured_object)});
    }

    partial_definition::ptr definition =
        manage_owned<partial_definition>(partial_definition{
            partial_block.loc,
            name,
            std::move(arguments),
            std::move(captures),
            std::cref(partial_block.body_elements),
        });
    return native_handle<>(std::move(definition));
  }

 private:
  render_options& opts_;
  source_stack source_stack_;
  diagnoser diags_;
};

/**
 * A class that manages import and export statements. Its primarily
 * responsibility is to extract the export map from a given Whisker module.
 *
 * Whisker modules do not produce output so importing has no side-effect. As an
 * optimization, this class caches export maps for repeatedly imported modules.
 */
class module_importer {
 public:
  explicit module_importer(render_options& opts, virtual_machine& vm)
      : opts_(opts), vm_(vm) {}

  /**
   * Given an import statement, resolves its export map.
   *
   * The resulting export map is cached for future imports of the same module.
   */
  object resolve_exports_of(const ast::import_statement& import_statement) {
    source_resolver* resolver = opts_.src_resolver.get();
    if (resolver == nullptr) {
      vm_.diags().report_fatal_error(
          import_statement.loc.begin,
          "No source resolver was provided. Cannot resolve import with path '{}'",
          import_statement.path.text);
    }

    const source_resolver::resolve_import_result imported_module =
        resolver->resolve_import(
            import_statement.path.text,
            import_statement.loc.begin,
            vm_.diags().engine());

    const ast::root& module = whisker::visit(
        imported_module,
        [&](const ast::root* resolved) -> const ast::root& {
          if (resolved == nullptr) {
            vm_.diags().report_fatal_error(
                import_statement.loc.begin,
                "Module with path '{}' was not found",
                import_statement.path.text);
          }
          return *resolved;
        },
        [&](const source_resolver::parsing_error&) -> const ast::root& {
          vm_.diags().report_fatal_error(
              import_statement.loc.begin,
              "Module with path '{}' failed to parse",
              import_statement.path.text);
        });

    if (auto cached = cached_export_maps_.find(&module);
        cached != cached_export_maps_.end()) {
      return cached->second;
    }

    eval_context eval_ctx = vm_.stack().top()->context.make_derived();
    auto frame_guard = vm_.stack().make_frame_guard(
        std::move(eval_ctx),
        source_stack::frame::for_import{},
        import_statement.loc.begin);

    map::raw exports;
    for (const ast::header& header : module.header_elements) {
      detail::variant_match(
          header, [&](const auto& node) { visit(exports, node); });
    }
    for (const ast::body& body : module.body_elements) {
      detail::variant_match(
          body, [&](const auto& node) { visit(exports, node); });
    }
    auto [cached, inserted] = cached_export_maps_.emplace(
        &module, whisker::make::map(std::move(exports)));
    assert(inserted);
    return cached->second;
  }

 private:
  eval_context& eval_ctx() { return vm_.stack().top()->context; }

  void do_export(
      map::raw& exports, std::string name, object value, source_location loc) {
    auto [_, inserted] = exports.emplace(name, std::move(value));
    if (!inserted) {
      vm_.diags().report_fatal_error(
          loc, "Export named '{}' already exists in this module", name);
    }
  }

  void visit(map::raw&, const ast::import_statement& import_statement) {
    auto export_map = resolve_exports_of(import_statement);
    vm_.bind_local(
        import_statement.loc.begin,
        import_statement.name.name,
        std::move(export_map));
  }

  void visit(map::raw& exports, const ast::partial_block& partial_block) {
    native_handle<> definition = vm_.create_partial_definition(partial_block);
    vm_.bind_local(
        partial_block.name.loc.begin,
        partial_block.name.name,
        object(native_handle<>(definition)));

    if (partial_block.exported) {
      do_export(
          exports,
          partial_block.name.name,
          object(std::move(definition)),
          partial_block.loc.begin);
    }
  }

  void visit(map::raw& exports, const ast::let_statement& let_statement) {
    object value = vm_.evaluate(let_statement.value);
    vm_.bind_local(let_statement.loc.begin, let_statement.id.name, value);

    if (let_statement.exported) {
      do_export(
          exports,
          let_statement.id.name,
          std::move(value),
          let_statement.loc.begin);
    }
  }

  void visit(map::raw&, const ast::text& text) {
    for (const ast::text::content& part : text.parts) {
      detail::variant_match(
          part,
          [&](const ast::text::whitespace&) {
            // whitespace can be safely ignored
          },
          [&](const ast::text::non_whitespace& t) {
            vm_.diags().report_fatal_error(
                t.loc.begin,
                "Modules cannot have non-whitespace text at the top-level");
          });
    }
  }

  void visit(map::raw&, const ast::newline&) {
    // newline can be safely ignored
  }
  void visit(map::raw&, const ast::comment&) {
    // comments can be safely ignored
  }
  void visit(map::raw&, const ast::pragma_statement&) {
    // The only supported pragma (ignore-newlines) is safe to ignore.
  }

  [[noreturn]] void visit(map::raw&, const ast::interpolation& variable) {
    vm_.diags().report_fatal_error(
        variable.loc.begin,
        "Modules cannot have interpolations at the top-level");
  }
  [[noreturn]] void visit(map::raw&, const ast::section_block& section_block) {
    vm_.diags().report_fatal_error(
        section_block.loc.begin,
        "Modules cannot have section blocks at the top-level");
  }
  [[noreturn]] void visit(
      map::raw&, const ast::conditional_block& conditional_block) {
    vm_.diags().report_fatal_error(
        conditional_block.loc.begin,
        "Modules cannot have conditional blocks at the top-level");
  }
  [[noreturn]] void visit(map::raw&, const ast::with_block& with_block) {
    vm_.diags().report_fatal_error(
        with_block.loc.begin,
        "Modules cannot have with blocks at the top-level");
  }
  [[noreturn]] void visit(map::raw&, const ast::each_block& each_block) {
    vm_.diags().report_fatal_error(
        each_block.loc.begin,
        "Modules cannot have each blocks at the top-level");
  }
  [[noreturn]] void visit(
      map::raw&, const ast::partial_statement& partial_statement) {
    vm_.diags().report_fatal_error(
        partial_statement.loc.begin,
        "Modules cannot have partial statements at the top-level");
  }
  [[noreturn]] void visit(map::raw&, const ast::macro& macro) {
    vm_.diags().report_fatal_error(
        macro.loc.begin, "Modules cannot have macros at the top-level");
  }

  // Owned and kept alive by the render_engine.
  render_options& opts_;
  virtual_machine& vm_;

  // The Whisker module's AST object identity (pointer value) is used to cache
  // repeated imports. This is a guarantee afforded by the source_resolver.
  std::unordered_map<const ast::root*, object> cached_export_maps_;
};

/**
 * A class that walks Whisker ASTs to drive the Whisker VM. During the walk, the
 * render engine interpolates the contemporary state of the VM to write to an
 * output buffer.
 */
class render_engine {
 public:
  explicit render_engine(
      std::ostream& out, diagnostics_engine& diags, render_options opts)
      : out_(out),
        opts_(std::move(opts)),
        vm_(opts_, diags),
        importer_(opts_, vm_) {}

  bool visit(const ast::root& root, object root_context) && {
    try {
      auto flush_guard = out_.make_flush_guard();
      auto eval_ctx = eval_context::with_root_scope(
          vm_.diags().engine(),
          std::move(root_context),
          std::exchange(opts_.globals, {}));
      auto source_frame_guard = vm_.stack().make_frame_guard(
          std::move(eval_ctx), frame::for_root{}, source_location());
      visit(root.header_elements);
      visit(root.body_elements);
      return true;
    } catch (const render_error& err) {
      if (!vm_.diags().engine().params().should_report(
              opts_.show_source_backtrace_on_failure)) {
        return false;
      }
      const source_stack::backtrace& backtrace = err.backtrace();
      assert(!backtrace.empty());

      auto source_trace = [&]() -> std::string {
        std::string result;
        for (std::size_t i = 0; i < backtrace.size(); ++i) {
          const source_stack::backtrace_frame& frame = backtrace[i];
          const auto name = detail::variant_match(
              frame.cause,
              [](frame::for_root) -> std::string_view { return ""; },
              [](frame::for_import) -> std::string_view { return "<module>"; },
              [](frame::for_macro) -> std::string_view { return "<macro>"; },
              [](const frame::for_partial& partial) -> std::string_view {
                return partial->name;
              });
          const auto location = name.empty()
              ? fmt::format("{}", frame.location.file_name())
              : fmt::format("{} @ {}", name, frame.location.file_name());
          fmt::format_to(
              std::back_inserter(result),
              "#{} {} <line:{}, col:{}>\n",
              i,
              location,
              frame.location.line(),
              frame.location.column());
        }
        return result;
      }();

      vm_.diags().engine().error(
          source_location(),
          "The source backtrace is:\n{}",
          std::move(source_trace));
      return false;
    }
  }

 private:
  using frame = source_stack::frame;
  eval_context& eval_ctx() { return vm_.current_frame().context; }

  void visit(const ast::headers& headers) {
    for (const auto& header : headers) {
      visit(header);
    }
  }
  void visit(const ast::bodies& bodies) {
    for (const auto& body : bodies) {
      visit(body);
    }
  }
  // Prevent implicit conversion to ast::header or ast::body. Otherwise, we can
  // silently compile an infinitely recursive visit() chain if there is a
  // missing overload for one of the alternatives in the variant.
  template <
      typename T,
      std::enable_if_t<
          std::is_same_v<T, ast::header> || std::is_same_v<T, ast::body>>* =
          nullptr>
  void visit(const T& elements) {
    detail::variant_match(elements, [&](const auto& node) { visit(node); });
  }

  void visit(const ast::text& text) { out_.write(text); }
  void visit(const ast::newline& newline) {
    if (!vm_.stack().top()->ignore_newlines) {
      out_.write(newline);
    }
  }
  void visit(const ast::comment&) {
    // comments are not rendered in the output
  }

  void visit(const ast::import_statement& import_statement) {
    object export_map = importer_.resolve_exports_of(import_statement);
    vm_.bind_local(
        import_statement.loc.begin,
        import_statement.name.name,
        std::move(export_map));
  }

  void visit(const ast::let_statement& let_statement) {
    if (let_statement.exported) {
      vm_.diags().report_fatal_error(
          let_statement.loc.begin,
          "Exports are not allowed in root source files or partial blocks");
    }
    vm_.bind_local(
        let_statement.loc.begin,
        let_statement.id.name,
        vm_.evaluate(let_statement.value));
  }

  void visit(const ast::pragma_statement& pragma_statement) {
    using pragma = ast::pragma_statement::pragmas;
    switch (pragma_statement.pragma) {
      case pragma::ignore_newlines:
        vm_.stack().top()->ignore_newlines = true;
        break;
    }
  }

  void visit(const ast::interpolation& interpolation) {
    object result = vm_.evaluate(interpolation.content);

    const auto report_unprintable_message_only = [&](diagnostic_level level) {
      vm_.diags().maybe_report(interpolation.loc, level, [&] {
        return fmt::format(
            "Object named '{}' is not printable. The encountered value is:\n{}",
            interpolation.to_string(),
            to_string(result));
      });
    };

    const auto report_unprintable = [&]() {
      auto level = opts_.strict_printable_types;
      report_unprintable_message_only(level);
      if (level == diagnostic_level::error) {
        // Fail rendering in strict mode
        vm_.diags().abort_rendering(interpolation.loc.begin);
      }
    };

    // See render_options::strict_printable_types for printing rules
    auto output = result.visit(
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
          vm_.diags().abort_rendering(interpolation.loc.begin);
        });
    out_.write(std::move(output));
  }

  void visit(const ast::section_block& section) {
    object section_variable = vm_.lookup_variable(section.variable).found;

    const auto maybe_report_coercion = [&] {
      vm_.maybe_report_boolean_coercion(
          ast::expression{section.variable.loc, section.variable},
          section_variable);
    };

    const auto do_visit = [&](object scope) {
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
    section_variable.visit(
        [&](const array::ptr& value) {
          if (section.inverted) {
            // This array is being used as a conditional
            maybe_report_coercion();
            if (!coerce_to_boolean(value)) {
              // Empty arrays are falsy
              do_visit(whisker::make::null);
            }
            return;
          }
          const std::size_t size = value->size();
          for (std::size_t i = 0; i < size; ++i) {
            do_visit(value->at(i));
          }
        },
        [&](const map::ptr&) {
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
      eval_ctx().push_scope(whisker::make::null);
      visit(body_elements);
      eval_ctx().pop_scope();
    };

    // Returns whether the else clause should be evaluated.
    auto visit_else_if = [&](const ast::conditional_block& b) {
      for (const auto& clause : b.else_if_clauses) {
        if (vm_.evaluate_as_bool(clause.condition)) {
          do_visit(clause.body_elements);
          return true;
        }
      }
      return false;
    };

    const bool condition = vm_.evaluate_as_bool(conditional_block.condition);
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
    object result = vm_.evaluate(expr);
    result.visit(
        [&](const map::ptr&) {
          // maps can be de-structured.
        },
        [&](auto&&) {
          vm_.diags().report_fatal_error(
              expr.loc.begin,
              "Expression '{}' does not evaluate to a map. The encountered value is:\n{}",
              expr.to_string(),
              to_string(result));
        });
    eval_ctx().push_scope(std::move(result));
    visit(with_block.body_elements);
    eval_ctx().pop_scope();
  }

  void visit(const ast::each_block& each_block) {
    const ast::expression& expr = each_block.iterable;
    object result = vm_.evaluate(expr);

    const auto do_visit = [this, &each_block, &expr](i64 index, object scope) {
      const std::vector<ast::identifier>& captured = each_block.captured;
      if (captured.empty()) {
        // When captures are not present, each element becomes the implicit
        // context object (`{{.}}`).
        eval_ctx().push_scope(std::move(scope));
      } else if (captured.size() == 1) {
        // When there is only one capture, then we assign the name directly to
        // the element. There is no need to destructure the element.
        eval_ctx().push_scope(whisker::make::null);
        const auto& capture = captured.front();
        vm_.bind_local(capture.loc.begin, capture.name, std::move(scope));
      } else {
        // If there are multiple captures, then we need to destructure the
        // element. The element must be an array (representing a tuple).
        if (!scope.is_array()) {
          vm_.diags().report_fatal_error(
              expr.loc.begin,
              "Item at index {} does not evaluate to an array. "
              "Each blocks with multiple captures require each item to be an array. "
              "The encountered value is:\n{}",
              index,
              to_string(scope));
        }
        eval_ctx().push_scope(whisker::make::null);
        const array::ptr& tuple = scope.as_array();
        if (tuple->size() != captured.size()) {
          vm_.diags().report_fatal_error(
              expr.loc.begin,
              "Item at index {} does not evaluate to an array with {} elements (to capture). "
              "The encountered value is:\n{}",
              index,
              captured.size(),
              to_string(scope));
        }
        for (std::size_t i = 0; i < captured.size(); ++i) {
          vm_.bind_local(captured[i].loc.begin, captured[i].name, tuple->at(i));
        }
      }
      visit(each_block.body_elements);
      eval_ctx().pop_scope();
    };

    const auto do_visit_else = [this, &each_block]() {
      if (!each_block.else_clause.has_value()) {
        return;
      }
      eval_ctx().push_scope(whisker::make::null);
      visit(each_block.else_clause->body_elements);
      eval_ctx().pop_scope();
    };

    result.visit(
        [&](const array::ptr& arr) {
          const std::size_t size = arr->size();
          if (size == 0) {
            do_visit_else();
            return;
          }
          for (std::size_t i = 0; i < size; ++i) {
            do_visit(i64(i), arr->at(i));
          }
        },
        [&](auto&&) {
          vm_.diags().report_fatal_error(
              expr.loc.begin,
              "Expression '{}' does not evaluate to an array. The encountered value is:\n{}",
              expr.to_string(),
              to_string(result));
        });
  }

  void visit(const ast::partial_block& partial_block) {
    if (partial_block.exported) {
      vm_.diags().report_fatal_error(
          partial_block.loc.begin,
          "Exports are not allowed in root source files or partial blocks");
    }

    native_handle<> definition = vm_.create_partial_definition(partial_block);
    vm_.bind_local(
        partial_block.name.loc.begin,
        partial_block.name.name,
        object(std::move(definition)));
  }

  void visit(const ast::partial_statement& partial_statement) {
    object lookup = vm_.evaluate(partial_statement.partial);

    partial_definition::ptr partial = std::invoke([&] {
      if (lookup.is_native_handle()) {
        if (std::optional<native_handle<partial_definition>> handle =
                lookup.as_native_handle().try_as<partial_definition>()) {
          return handle->ptr();
        }
      }
      vm_.diags().report_fatal_error(
          partial_statement.partial.loc.begin,
          "Expression '{}' does not evaluate to a partial. The encountered value is:\n{}",
          partial_statement.partial.to_string(),
          to_string(lookup));
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
      vm_.diags().report_fatal_error(
          partial_statement.loc.begin,
          "Partial '{}' is missing named arguments: {}",
          partial->name,
          fmt::join(missing, ", "));
    }
    if (!extras.empty()) {
      vm_.diags().report_fatal_error(
          partial_statement.loc.begin,
          "Partial '{}' received unexpected named arguments: {}",
          partial->name,
          fmt::join(extras, ", "));
    }

    // Partials get a new evaluation context derived from the current one.
    auto derived_ctx = eval_ctx().make_derived();
    // Make the partial itself available in the derived context
    vm_.bind_local_in(
        derived_ctx,
        partial_statement.partial.loc.begin,
        partial->name,
        std::move(lookup));

    for (const std::string& argument : partial->arguments) {
      auto arg = named_arguments.find(argument);
      // We checked against argument mismatches above
      assert(arg != named_arguments.end());
      const ast::identifier& id = arg->second.name;
      const ast::expression& expr = arg->second.value;
      vm_.bind_local_in(
          derived_ctx, id.loc.begin, argument, vm_.evaluate(expr));
    }

    for (const auto& [capture, value] : partial->captures) {
      vm_.bind_local_in(derived_ctx, capture.loc.begin, capture.name, value);
    }

    auto source_frame_guard = vm_.stack().make_frame_guard(
        std::move(derived_ctx), partial, partial_statement.loc.begin);
    auto indent_guard = out_.make_indent_guard(
        partial_statement.standalone_indentation_within_line);
    visit(partial->bodies);
  }

  void visit(const ast::macro& macro) {
    std::vector<std::string> path;
    path.reserve(macro.path.parts.size());
    for (const ast::path_component& component : macro.path.parts) {
      path.push_back(component.value);
    }

    source_resolver* resolver = opts_.src_resolver.get();
    if (resolver == nullptr) {
      vm_.diags().report_fatal_error(
          macro.loc.begin,
          "No source resolver was provided. Cannot resolve macro with path '{}'",
          macro.path_string());
    }

    // Kept alive by the source_resolver implementation
    source_resolver::resolve_macro_result resolved_macro_module =
        resolver->resolve_macro(path, macro.loc.begin, vm_.diags().engine());

    const ast::root& resolved_macro = whisker::visit(
        resolved_macro_module,
        [&](const ast::root* resolved) -> const ast::root& {
          if (resolved == nullptr) {
            vm_.diags().report_fatal_error(
                macro.loc.begin,
                "Macro with path '{}' was not found",
                macro.path_string());
          }
          return *resolved;
        },
        [&](const source_resolver::parsing_error&) -> const ast::root& {
          vm_.diags().report_fatal_error(
              macro.loc.begin,
              "Macro with path '{}' failed to parse",
              macro.path_string());
        });

    // Macros are "inlined" into their invocation site. In other words, they
    // execute within the scope where they are invoked.
    auto source_frame_guard = vm_.stack().make_frame_guard(
        eval_ctx(), frame::for_macro{}, macro.loc.begin);
    auto indent_guard =
        out_.make_indent_guard(macro.standalone_indentation_within_line);
    visit(resolved_macro.header_elements);
    visit(resolved_macro.body_elements);
  }

  outputter out_;
  render_options opts_;
  virtual_machine vm_;
  module_importer importer_;
};

} // namespace

source_resolver::resolve_macro_result source_resolver::resolve_macro(
    const std::vector<std::string>& path,
    source_location include_from,
    diagnostics_engine& diags) {
  return resolve_import(
      fmt::format("{}", fmt::join(path, "/")), include_from, diags);
}

bool render(
    std::ostream& out,
    const ast::root& root,
    const object& root_context,
    diagnostics_engine& diags,
    render_options opts) {
  render_engine engine{out, diags, std::move(opts)};
  return std::move(engine).visit(root, root_context);
}

} // namespace whisker
