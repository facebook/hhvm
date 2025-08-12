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

#include <thrift/compiler/whisker/source_location.h>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace whisker::ast {

struct text;
struct newline;
struct comment;
struct section_block;
struct conditional_block;
struct with_block;
struct each_block;
struct macro;
struct partial_block;
struct partial_statement;
struct interpolation;
struct let_statement;
struct pragma_statement;
struct import_statement;

/**
 * The top-level types of constructs allowed in a Whisker source file.
 */
using body = std::variant<
    text,
    newline,
    comment,
    interpolation,
    section_block,
    conditional_block,
    with_block,
    each_block,
    partial_block,
    partial_statement,
    let_statement,
    pragma_statement,
    macro>;
using bodies = std::vector<body>;

/**
 * Elements that can appear at the top of a Whisker source file.
 * Whisker does not render these kinds of elements.
 */
using header = std::variant<comment, pragma_statement, import_statement>;
using headers = std::vector<header>;

// Defines operator!= in terms of operator==
// Remove in C++20 which introduces comparison operator synthesis
#define WHISKER_DEFINE_OPERATOR_INEQUALITY(type)             \
  friend bool operator!=(const type& lhs, const type& rhs) { \
    return !(lhs == rhs);                                    \
  }

/**
 * The root node of a Whisker AST representing a source file.
 */
struct root {
  source_location loc;
  headers header_elements;
  bodies body_elements;
};

/**
 * Raw text content that should be emitted unchanged in the rendered output.
 * This is guaranteed to be on one line (i.e. not contain a new line).
 */
struct text {
  source_range loc;

  /**
   * Whitespace characters except newlines. One or more repetitions of:
   *   - " "
   *   - "\t"
   *   - "\v"
   */
  struct whitespace {
    source_range loc;
    std::string value;
  };
  /**
   * A sequence of non-whitespace characters.
   */
  struct non_whitespace {
    source_range loc;
    std::string value;
  };
  using content = std::variant<whitespace, non_whitespace>;
  std::vector<content> parts;

  /**
   * A concatenation of all composed whitespace and non-whitespace nodes.
   */
  std::string joined() const;
};

/**
 * Raw newline that should be emitted unchanged in the rendered output. One of:
 *   - "\r\n"
 *   - "\n"
 *   - "\r"
 */
struct newline {
  source_range loc;
  std::string text;
};

/**
 * A comment that should be omitted in the rendered output.
 *   https://mustache.github.io/mustache.5.html#Comments
 * Whisker also supports Handlebars' escaped comments:
 *   https://handlebarsjs.com/guide/#template-comments
 */
struct comment {
  source_range loc;
  std::string text;
};

/**
 * A valid Whisker identifier. See whisker::lexer for its definition.
 */
struct identifier {
  source_range loc;
  std::string name;

  /**
   * Determines if two identifiers are syntactically equivalent, excluding
   * their location in source code.
   */
  friend bool operator==(const identifier& lhs, const identifier& rhs) {
    return lhs.name == rhs.name;
  }
  // Remove in C++20 which introduces comparison operator synthesis
  WHISKER_DEFINE_OPERATOR_INEQUALITY(identifier)

  // For std::map and std::set
  struct compare_by_name {
    using is_transparent = void;
    bool operator()(const identifier& lhs, std::string_view rhs) const {
      return lhs.name < rhs;
    }
    bool operator()(const identifier& lhs, const identifier& rhs) const {
      return operator()(lhs, rhs.name);
    }
  };
};

/**
 * A single component in a variable lookup path. A single component represents
 * either a raw identifier lookup ("foo"), or an explicitly qualified lookup
 * ("my_obj:foo").
 */
struct variable_component {
  source_range loc;
  std::optional<identifier> qualifier;
  identifier property;

  variable_component() = delete;

  // Explicit constructor to ensure string representation is initialized
  variable_component(
      source_range loc,
      std::optional<identifier> qualifier,
      identifier property);

  /**
   * Determines if two identifiers are syntactically equivalent, excluding
   * their location in source code.
   */
  friend bool operator==(
      const variable_component& lhs, const variable_component& rhs) {
    return lhs.qualifier == rhs.qualifier && lhs.property == rhs.property;
  }
  // Remove in C++20 which introduces comparison operator synthesis
  WHISKER_DEFINE_OPERATOR_INEQUALITY(variable_component)

  /**
   * Returns a source-identical string representation of this variable
   * component.
   * For a qualified lookup, it takes the form "qualifier:property".
   * For a raw identifier lookup, it is just "property".
   */
  const std::string& as_string() const;

 private:
  // Store string representation to avoid regenerating repeatedly
  std::string as_string_;
};

/**
 * A "path" of components that represent a lookup of a variable where each
 * chain component is separated by a dot. This is a subset of Mustache's
 * variables:
 *   https://mustache.github.io/mustache.5.html#Variables
 */
struct variable_lookup {
  source_range loc;
  // this_ref is a special case: {{.}} referring to the current object.
  struct this_ref {
    // Remove in C++20 which introduces comparison operator synthesis
    friend bool operator==(const this_ref&, const this_ref&) { return true; }
    WHISKER_DEFINE_OPERATOR_INEQUALITY(this_ref)
  };
  std::variant<this_ref, std::vector<variable_component>> chain;

  /**
   * Determines if two variable lookups are syntactically equivalent, excluding
   * their location in source code.
   */
  friend bool operator==(
      const variable_lookup& lhs, const variable_lookup& rhs) {
    return lhs.chain == rhs.chain;
  }
  // Remove in C++20 which introduces comparison operator synthesis
  WHISKER_DEFINE_OPERATOR_INEQUALITY(variable_lookup)

  std::string chain_string() const;
};

/**
 * The base value type in Whisker, which can be used in interpolation or as
 * arguments to blocks such as conditionals.
 */
struct expression {
  source_range loc;

  /**
   * A function call expression like `(foo.bar "hello" "world")`.
   */
  struct function_call {
    /**
     * Base class for all built-in functions.
     */
    struct builtin {};
    /**
     * Binary functions which are also associative and can be "chained", such as
     * `(and arg1 ... argN)`, `(or arg1 ... argN)` etc.
     */
    struct builtin_binary_associative {};

    /**
     * The `(not arg1)` function.
     */
    struct builtin_not : builtin {
      // Remove in C++20 which introduces comparison operator synthesis
      friend bool operator==(const builtin_not&, const builtin_not&) {
        return true;
      }
      WHISKER_DEFINE_OPERATOR_INEQUALITY(builtin_not)
    };

    /**
     * The `(and arg1 ... argN)` function.
     */
    struct builtin_and : builtin, builtin_binary_associative {
      // Remove in C++20 which introduces comparison operator synthesis
      friend bool operator==(const builtin_and&, const builtin_and&) {
        return true;
      }
      WHISKER_DEFINE_OPERATOR_INEQUALITY(builtin_and)
    };

    /**
     * The `(or arg1 ... argN)` function.
     */
    struct builtin_or : builtin, builtin_binary_associative {
      // Remove in C++20 which introduces comparison operator synthesis
      friend bool operator==(const builtin_or&, const builtin_or&) {
        return true;
      }
      WHISKER_DEFINE_OPERATOR_INEQUALITY(builtin_or)
    };

    /**
     * A user-defined function call whose name is variable (chain of
     * identifiers).
     *
     * Example:
     *   `(my_lib.snake_case "FooBar")` // "foo_bar"
     */
    struct user_defined {
      variable_lookup name;

      friend bool operator==(const user_defined& lhs, const user_defined& rhs) {
        return lhs.name == rhs.name;
      }
      // Remove in C++20 which introduces comparison operator synthesis
      WHISKER_DEFINE_OPERATOR_INEQUALITY(user_defined)
    };

    std::variant<builtin_not, builtin_and, builtin_or, user_defined> which;

    /**
     * Unnamed arguments that are identified by their ordering in the function
     * invocation.
     */
    std::vector<expression> positional_arguments;

    struct named_argument {
      identifier name;
      // Using the heap to avoid mutually recursion with `expression`.
      // Using std::shared_ptr so that this struct is copyable.
      std::shared_ptr<expression> value;

      friend bool operator==(
          const named_argument& lhs, const named_argument& rhs) {
        static const auto as_tuple = [](const named_argument& arg) {
          return std::tie(arg.name, *arg.value);
        };
        return as_tuple(lhs) == as_tuple(rhs);
      }
      // Remove in C++20 which introduces comparison operator synthesis
      WHISKER_DEFINE_OPERATOR_INEQUALITY(named_argument)
    };
    /**
     * Named arguments that are identified by their name, with no restrictions
     * on their ordering. Every argument must have a unique identifier. All
     * named arguments must appear after all positional arguments.
     *
     * Using std::map for stable ordering when printing the AST.
     */
    std::map<std::string, named_argument> named_arguments;

    /**
     * The name of the function call lookup as seen in the source code.
     */
    std::string name() const;

    friend bool operator==(const function_call& lhs, const function_call& rhs) {
      static const auto as_tuple = [](const function_call& f) {
        // Ignore the source range of the function call.
        return std::tie(f.which, f.positional_arguments, f.named_arguments);
      };
      return as_tuple(lhs) == as_tuple(rhs);
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(function_call)
  };

  struct string_literal {
    std::string text;

    friend bool operator==(
        const string_literal& lhs, const string_literal& rhs) {
      return lhs.text == rhs.text;
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(string_literal)
  };

  struct i64_literal {
    std::int64_t value;

    friend bool operator==(const i64_literal& lhs, const i64_literal& rhs) {
      return lhs.value == rhs.value;
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(i64_literal)
  };

  struct null_literal {
    friend bool operator==(const null_literal&, const null_literal&) {
      return true;
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(null_literal)
  };

  struct true_literal {
    friend bool operator==(const true_literal&, const true_literal&) {
      return true;
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(true_literal)
  };

  struct false_literal {
    friend bool operator==(const false_literal&, const false_literal&) {
      return true;
    }
    // Remove in C++20 which introduces comparison operator synthesis
    WHISKER_DEFINE_OPERATOR_INEQUALITY(false_literal)
  };

  std::variant<
      string_literal,
      i64_literal,
      null_literal,
      true_literal,
      false_literal,
      variable_lookup,
      function_call>
      which;

  /**
   * Determines if two expressions are syntactically equivalent, excluding their
   * location in source code.
   */
  friend bool operator==(const expression& lhs, const expression& rhs) {
    return lhs.which == rhs.which;
  }
  // Remove in C++20 which introduces comparison operator synthesis
  WHISKER_DEFINE_OPERATOR_INEQUALITY(expression)

  /**
   * Returns a human-readable text representation of the expression.
   */
  std::string to_string() const;
};

/**
 * A top-level use of an expression within a template body. It is similar to
 * expression except its source_range includes the surrounding "{{ }}".
 */
struct interpolation {
  source_range loc;
  expression content;

  std::string to_string() const { return content.to_string(); }
};

/**
 * A Whisker construct for binding an expression to a name. The expression is
 * evaluated exactly once.
 */
struct let_statement {
  source_range loc;

  bool exported;
  identifier id;
  expression value;
};

/**
 * A Whisker construct for changing rendering behavior.
 */
struct pragma_statement {
  enum class pragmas {
    ignore_newlines,
  };
  source_range loc;

  pragmas pragma;

  std::string_view to_string() const;
};

/**
 * A Whisker constructor for importing objects exported from other templates.
 *
 * {{#import "<path>" as <name>}}
 */
struct import_statement {
  source_range loc;

  expression::string_literal path;
  identifier name;
};

/**
 * A Whisker construct for conditionals and/or iteration. This matches Mustache:
 *   https://mustache.github.io/mustache.5.html#Sections
 */
struct section_block {
  source_range loc;
  /**
   * {{# ⇒ inverted == false
   * {{^ ⇒ inverted == true
   */
  bool inverted;
  variable_lookup variable;
  bodies body_elements;
};

/**
 * The {{#else}} clause of {{#if}} or {{#each}} blocks.
 */
struct else_block {
  source_range loc;
  bodies body_elements;
};

/**
 * A Whisker construct for conditionals, i.e. the if-block.
 * This matches Handlebars:
 *   https://handlebarsjs.com/guide/builtin-helpers.html#if
 */
struct conditional_block {
  source_range loc;

  expression condition;
  bodies body_elements;

  // Any {{#else if}} clauses, if present.
  struct else_if_block {
    source_range loc;
    expression condition;
    bodies body_elements;
  };
  std::vector<else_if_block> else_if_clauses;

  std::optional<else_block> else_clause;
};

/**
 * A Whisker construct for "de-structuring" a map object.
 * This matches Handlebars:
 *   https://handlebarsjs.com/guide/builtin-helpers.html#with
 */
struct with_block {
  source_range loc;

  expression value;
  bodies body_elements;
};

/**
 * A Whisker construct for iteration.
 * This matches Handlebars:
 *   https://handlebarsjs.com/guide/builtin-helpers.html#each
 */
struct each_block {
  source_range loc;

  expression iterable;
  /**
   * The name of the variable to assign to each element of the iterable.
   * If there are multiple captures, then each element of the iterable must be
   * an array with the same number of elements as the number of captures.
   */
  std::vector<identifier> captured;

  bodies body_elements;
  std::optional<else_block> else_clause;
};

/**
 * A Whisker construct for partially applied (reusable) templates.
 *
 * To form text output, a partial block must be applied with a set of arguments
 * via partial application.
 */
struct partial_block {
  source_range loc;

  bool exported;
  identifier name;
  std::set<identifier, identifier::compare_by_name> arguments;
  std::set<identifier, identifier::compare_by_name> captures;
  bodies body_elements;
};

/**
 * A Whisker construct for partial application of a partial block (defined
 * above).
 *
 * The partial block is applied with a set of arguments and the result is
 * printed to the output.
 */
struct partial_statement {
  source_range loc;
  /**
   * An expression that evaluates to an (implemented-defintion) partial block
   * instance.
   */
  expression partial;
  struct named_argument {
    identifier name;
    expression value;
  };
  std::map<std::string, named_argument> named_arguments;
  /**
   * Standalone partials exhibit different indentation behavior:
   *   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L13-L15
   *
   * If this is a standalone partial, the value is the preceding whitespace
   * necessary before the partial interpolation. Otherwise, this is
   * std::nullopt.
   */
  std::optional<ast::text::whitespace> standalone_indentation_within_line;
};

/*
 * A valid Whisker path component for macro application. See whisker::lexer
 * for its definition.
 */
struct path_component {
  source_range loc;
  std::string value;
};

/**
 * A '/' delimited series of path components representing a POSIX portable file
 * path. This is used for macros.
 */
struct macro_lookup {
  source_range loc;
  std::vector<path_component> parts;

  std::string as_string() const;
};

/**
 * A Whisker construct for partially applied templates. This matches Mustache:
 *   https://mustache.github.io/mustache.5.html#Partials
 */
struct macro {
  source_range loc;
  macro_lookup path;
  /**
   * Standalone macros exhibit different indentation behavior:
   *   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L13-L15
   *
   * If this is a standalone macro, the value is the preceding whitespace
   * necessary before the macro interpolation. Otherwise, this is std::nullopt.
   */
  std::optional<ast::text::whitespace> standalone_indentation_within_line;

  std::string path_string() const;
};

#undef WHISKER_DEFINE_OPERATOR_INEQUALITY

} // namespace whisker::ast
