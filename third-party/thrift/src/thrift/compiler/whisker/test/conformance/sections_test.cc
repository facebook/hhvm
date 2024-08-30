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

#include <folly/portability/GTest.h>

#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

class SectionsTest : public RenderTest {};

// Truthy sections should have their contents rendered.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L41-L45
TEST_F(SectionsTest, Truthy) {
  EXPECT_EQ(
      "\"This should be rendered.\"",
      *render(
          "\"{{#boolean}}This should be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::boolean(true)}})));
}

// Falsey sections should have their contents omitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L47-L51
TEST_F(SectionsTest, Falsey) {
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{#boolean}}This should not be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::boolean(false)}})));
}

// Null is falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L53-L57
TEST_F(SectionsTest, NullIsFalsey) {
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{#boolean}}This should not be rendered.{{/boolean}}\"",
          w::map({{"boolean", w::null}})));
}

// Objects and hashes should be pushed onto the context stack.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L59-L63
TEST_F(SectionsTest, Context) {
  EXPECT_EQ(
      "\"Hi Joe.\"",
      *render(
          "\"{{#context}}Hi {{name}}.{{/context}}\"",
          w::map({{"context", w::map({{"name", w::string("Joe")}})}})));
}

// Names missing in the current context are looked up in the stack.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L65-L69
TEST_F(SectionsTest, ParentContexts) {
  EXPECT_EQ(
      "\"foo, bar, baz\"",
      *render(
          "\"{{#sec}}{{a}}, {{b}}, {{c.d}}{{/sec}}\"",
          w::map(
              {{"a", w::string("foo")},
               {"b", w::string("wrong")},
               {"sec",
                w::map(
                    {{"b", w::string("bar")},
                     {"c", w::map({{"d", w::string("baz")}})}})}})));
}

// Non-false sections have their value at the top of context, accessible as
// {{.}} or through the parent context. This gives a simple way to display
// content conditionally if a variable exists.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L71-L78
TEST_F(SectionsTest, VariableTest) {
  EXPECT_EQ(
      "\"bar is bar\"",
      *render(
          "\"{{#foo}}{{.}} is {{foo}}{{/foo}}\"",
          w::map({{"foo", w::string("bar")}})));
}

// All elements on the context stack should be accessible within lists.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L80-L93
TEST_F(SectionsTest, ListContexts) {
  const object tname =
      w::map({{"upper", w::string("A")}, {"lower", w::string("a")}});
  const object bottoms = w::array(
      {w::map({{"bname", w::string("x")}}),
       w::map({{"bname", w::string("y")}})});
  const object middles = w::map({
      {"mname", w::i64(1)},
      {"bottoms", bottoms},
  });
  const object tops = w::map(
      {{"tops", w::array({w::map({{"tname", tname}, {"middles", middles}})})}});

  EXPECT_EQ(
      "a1.A1x.A1y.",
      *render(
          "{{#tops}}{{#middles}}{{tname.lower}}{{mname}}.{{#bottoms}}{{tname.upper}}{{mname}}{{bname}}.{{/bottoms}}{{/middles}}{{/tops}}",
          tops));
}

// All elements on the context stack should be accessible.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L95-L134
TEST_F(SectionsTest, DeeplyNestedContexts) {
  strict_boolean_conditional = diagnostic_level::debug;
  const object context = w::map(
      {{"a", w::map({{"one", w::i64(1)}})},
       {"b", w::map({{"two", w::i64(2)}})},
       {"c",
        w::map(
            {{"three", w::i64(3)},
             {"d", w::map({{"four", w::i64(4)}, {"five", w::i64(5)}})}})}});
  EXPECT_EQ(
      "1\n"
      "121\n"
      "12321\n"
      "1234321\n"
      "123454321\n"
      "12345654321\n"
      "123454321\n"
      "1234321\n"
      "12321\n"
      "121\n"
      "1\n",
      render(
          "{{#a}}\n"
          "{{one}}\n"
          "{{#b}}\n"
          "{{one}}{{two}}{{one}}\n"
          "{{#c}}\n"
          "{{one}}{{two}}{{three}}{{two}}{{one}}\n"
          "{{#d}}\n"
          "{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n"
          "{{#five}}\n"
          "{{one}}{{two}}{{three}}{{four}}{{five}}{{four}}{{three}}{{two}}{{one}}\n"
          "{{one}}{{two}}{{three}}{{four}}{{.}}6{{.}}{{four}}{{three}}{{two}}{{one}}\n"
          "{{one}}{{two}}{{three}}{{four}}{{five}}{{four}}{{three}}{{two}}{{one}}\n"
          "{{/five}}\n"
          "{{one}}{{two}}{{three}}{{four}}{{three}}{{two}}{{one}}\n"
          "{{/d}}\n"
          "{{one}}{{two}}{{three}}{{two}}{{one}}\n"
          "{{/c}}\n"
          "{{one}}{{two}}{{one}}\n"
          "{{/b}}\n"
          "{{one}}\n"
          "{{/a}}\n",
          context));
}

// Lists should be iterated; list items should visit the context stack.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L136-L140
TEST_F(SectionsTest, List) {
  EXPECT_EQ(
      "\"123\"",
      *render(
          "\"{{#list}}{{item}}{{/list}}\"",
          w::map(
              {{"list",
                w::array(
                    {w::map({{"item", w::i64(1)}}),
                     w::map({{"item", w::i64(2)}}),
                     w::map({{"item", w::i64(3)}})})}})));
}

// Empty lists should behave like falsey values.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L142-L146
TEST_F(SectionsTest, EmptyList) {
  EXPECT_EQ(
      "\"\"",
      *render(
          "\"{{#list}}Yay lists!{{/list}}\"",
          w::map({{"list", w::array({})}})));
}

// Multiple sections per template should be permitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L148-L162
TEST_F(SectionsTest, Doubled) {
  EXPECT_EQ(
      "* first\n"
      "* second\n"
      "* third\n",
      *render(
          "{{#bool}}\n"
          "* first\n"
          "{{/bool}}\n"
          "* {{two}}\n"
          "{{#bool}}\n"
          "* third\n"
          "{{/bool}}\n",
          w::map({{"bool", w::boolean(true)}, {"two", w::string("second")}})));
}

// Nested truthy sections should have their contents rendered.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L164-L168
TEST_F(SectionsTest, NestedTruthy) {
  EXPECT_EQ(
      "| A B C D E |",
      *render(
          "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |",
          w::map({{"bool", w::boolean(true)}})));
}

// Nested falsey sections should be omitted.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L170-L174
TEST_F(SectionsTest, NestedFalsey) {
  EXPECT_EQ(
      "| A  E |",
      *render(
          "| A {{#bool}}B {{#bool}}C{{/bool}} D{{/bool}} E |",
          w::map({{"bool", w::boolean(false)}})));
}

// Failed context lookups should be considered falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L176-L180
TEST_F(SectionsTest, ContextMisses) {
  strict_boolean_conditional = diagnostic_level::debug;
  strict_undefined_variables = diagnostic_level::debug;
  EXPECT_EQ(
      "[]",
      *render("[{{#missing}}Found key 'missing'!{{/missing}}]", w::map({})));
}

// Implicit iterators should directly interpolate strings.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L184-L189
TEST_F(SectionsTest, ImplicitIteratorString) {
  EXPECT_EQ(
      "\"(a)(b)(c)(d)(e)\"",
      *render(
          "\"{{#list}}({{.}}){{/list}}\"",
          w::map(
              {{"list",
                w::array(
                    {w::string("a"),
                     w::string("b"),
                     w::string("c"),
                     w::string("d"),
                     w::string("e")})}})));
}

// Implicit iterators should cast integers to strings and interpolate.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L191-L196
TEST_F(SectionsTest, ImplicitIteratorInteger) {
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "\"(1)(2)(3)(4)(5)\"",
      *render(
          "\"{{#list}}({{.}}){{/list}}\"",
          w::map(
              {{"list",
                w::array(
                    {w::i64(1),
                     w::i64(2),
                     w::i64(3),
                     w::i64(4),
                     w::i64(5)})}})));
}

// Implicit iterators should cast decimals to strings and interpolate.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L198-L203
TEST_F(SectionsTest, ImplicitIteratorDecimal) {
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "\"(1.1)(2.2)(3.3)(4.4)(5.5)\"",
      *render(
          "\"{{#list}}({{.}}){{/list}}\"",
          w::map(
              {{"list",
                w::array(
                    {w::f64(1.1),
                     w::f64(2.2),
                     w::f64(3.3),
                     w::f64(4.4),
                     w::f64(5.5)})}})));
}

// Implicit iterators should allow iterating over nested arrays.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L205-L210
TEST_F(SectionsTest, ImplicitIteratorArray) {
  EXPECT_EQ(
      "\"(123)(abc)\"",
      *render(
          "\"{{#list}}({{#.}}{{.}}{{/.}}){{/list}}\"",
          w::map({
              {"list",
               w::array(
                   {w::array({w::i64(1), w::i64(2), w::i64(3)}),
                    w::array(
                        {w::string("a"), w::string("b"), w::string("c")})})},
          })));
}

// NOTE:
//   Whisker intentionally does not support HTML escaping.
//   This means that Whisker is not conformant with the
//   ImplicitIteratorHTMLEscaping test.
//
// Implicit iterators with basic interpolation should be HTML escaped.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L212-L217
TEST_F(SectionsTest, ImplicitIteratorHTMLEscaping) {
  // Unsupported
}

// NOTE:
//   Whisker intentionally does not support HTML escaping.
//   This means that Whisker is not conformant with the
//   ImplicitIteratorTripleMustache test.
//
// Implicit iterators in triple mustache should interpolate without HTML
// escaping.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L219-L224
TEST_F(SectionsTest, ImplicitIteratorTripleMustache) {
  // Unsupported
}

// Implicit iterators in an Ampersand tag should interpolate without HTML
// escaping.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L226-L231
TEST_F(SectionsTest, ImplicitIteratorAmpersand) {
  // Unsupported
}

// Implicit iterators should work on root-level lists.
//  https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L233-L237
TEST_F(SectionsTest, ImplicitIteratorRootLevel) {
  EXPECT_EQ(
      "\"(a)(b)\"",
      *render(
          "\"{{#.}}({{value}}){{/.}}\"",
          w::array(
              {w::map({{"value", w::string("a")}}),
               w::map({{"value", w::string("b")}})})));
}

// Dotted names should be valid for Section tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L241-L245
TEST_F(SectionsTest, DottedNamesTruthy) {
  EXPECT_EQ(
      "\"Here\" == \"Here\"",
      *render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"Here\"",
          w::map({{"a", w::map({{"b", w::map({{"c", w::boolean(true)}})}})}})));
}

// Dotted names should be valid for Section tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L247-L251
TEST_F(SectionsTest, DottedNamesFalsey) {
  EXPECT_EQ(
      "\"\" == \"\"",
      *render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"",
          w::map(
              {{"a", w::map({{"b", w::map({{"c", w::boolean(false)}})}})}})));
}

// Dotted names that cannot be resolved should be considered falsey.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L253-L257
TEST_F(SectionsTest, DottedNamesBrokenChains) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  strict_boolean_conditional = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\" == \"\"",
      render(
          "\"{{#a.b.c}}Here{{/a.b.c}}\" == \"\"", w::map({{"a", w::map({})}})));
}

// Sections should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L261-L265
TEST_F(SectionsTest, SurroundingWhitespace) {
  EXPECT_EQ(
      " | 	|	 | \\n",
      *render(
          " | {{#boolean}}	|	{{/boolean}} | \\n",
          w::map({{"boolean", w::boolean(true)}})));
}

// Sections should not alter internal whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L267-L271
TEST_F(SectionsTest, InternalWhitespace) {
  EXPECT_EQ(
      " |  \n  | \n",
      *render(
          " | {{#boolean}} {{! Important Whitespace }}\n {{/boolean}} | \n",
          w::map({{"boolean", w::boolean(true)}})));
}

// Single-line sections should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L273-L277
TEST_F(SectionsTest, IndentedInlineSections) {
  EXPECT_EQ(
      " YES\n GOOD\n",
      *render(
          " {{#boolean}}YES{{/boolean}}\n {{#boolean}}GOOD{{/boolean}}\n",
          w::map({{"boolean", w::boolean(true)}})));
}

// Standalone lines should be removed from the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L279-L291
TEST_F(SectionsTest, StandaloneLines) {
  EXPECT_EQ(
      "| This Is\n"
      "|\n"
      "| A Line\n",
      *render(
          "| This Is\n"
          "{{#boolean}}\n"
          "|\n"
          "{{/boolean}}\n"
          "| A Line\n",
          w::map({{"boolean", w::boolean(true)}})));
}

// Indented standalone lines should be removed from the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L293-L305
TEST_F(SectionsTest, IndentedStandaloneLines) {
  EXPECT_EQ(
      "| This Is\n"
      "|\n"
      "| A Line\n",
      *render(
          "| This Is\n"
          "  {{#boolean}}\n"
          "|\n"
          "  {{/boolean}}\n"
          "| A Line\n",
          w::map({{"boolean", w::boolean(true)}})));
}

// "\r\n" should be considered a newline for standalone tags.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L307-L311
TEST_F(SectionsTest, StandaloneLineEndings) {
  EXPECT_EQ(
      "|\r\n|",
      *render(
          "|\r\n{{#boolean}}\r\n{{/boolean}}\r\n|",
          w::map({{"boolean", w::boolean(true)}})));
}

// Standalone tags should not require a newline to precede them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L313-L317
TEST_F(SectionsTest, StandaloneWithoutPreviousLine) {
  EXPECT_EQ(
      "#\n/",
      *render(
          "  {{#boolean}}\n#{{/boolean}}\n/",
          w::map({{"boolean", w::boolean(true)}})));
}

// Standalone tags should not require a newline to follow them.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L319-L323
TEST_F(SectionsTest, StandaloneWithoutNewline) {
  EXPECT_EQ(
      "#\n/\n",
      *render(
          "#{{#boolean}}\n/\n  {{/boolean}}",
          w::map({{"boolean", w::boolean(true)}})));
}

// Superfluous in-tag whitespace should be ignored.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L327-L331
TEST_F(SectionsTest, Padding) {
  EXPECT_EQ(
      "|=|",
      *render(
          "|{{# boolean }}={{/ boolean }}|",
          w::map({{"boolean", w::boolean(true)}})));
}

} // namespace whisker
