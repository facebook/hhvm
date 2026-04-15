# Whisker

export const Collapse = (props) => {
  const {children, open = false, title = "Collapse"} = props;
  return (
    <details open={open}>
      <summary>{title}</summary>
      {children}
    </details>
  );
}

export const Grammar = (props) => {
  return <Collapse {...props} title="Grammar" open={false} />
}

export const Example = (props) => {
  const {title = "Example", ...rest} = props;
  return <Collapse {...rest} title={title} open={true} />
}

Whisker is a **templating language** with a canonical implementation in C++. It draws inspiration from [Mustache](https://mustache.github.io/mustache.5.html), [Handlebars](https://handlebarsjs.com/), and [EmberJS templates](https://emberjs.com/), and builds upon the strengths of each.

Within the Thrift ecosystem, Whisker plays a crucial role in the compiler's backend infrastructure. It generates code in C++, Python, Rust, Go, and other languages.

Whisker was developed to replace the open-source library, [`mstch`](https://github.com/no1msd/mstch), which did not scale to meet the needs of Thrift.

## Basics

The core capability of Whisker can be expressed as one function:
```cpp
std::string render(
  std::string template_text,
  context variables
);
```
This function takes in a Whisker template source as a string, a *context* (variables) object, and returns the rendered output.

A typical Whisker template:
```whisker
Hello {{name}}
You have just won {{value}} dollars!
{{#if in_ca}}
Well, {{taxed_value}} dollars, after taxes.
{{/if in_ca}}
```

With the following input context:
```json title=Context
{
  "name": "Chris",
  "value": 10000,
  "taxed_value": 6000,
  "in_ca": true
}
```

Produces the output:
```text
Hello Chris,
You have just won 10000 dollars!
Well, 6000 dollars, after taxes.
```

Whisker's templating syntax combines raw text and *templates* (denoted by `{{` and `}}`). The formal grammar is detailed below.

## Grammar Syntax

The grammar of the templating language is described with [PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar)-like production rules, similar to [Pest](https://pest.rs/).

Production rules are of the form:
```
rule → { <term> <op> <term> ... }
```

`<term>` represents other rules. `<op>` represents a combinator of rules.

The supported set of combinators are:
* `a ~ b` — exactly one *a* followed by exactly one *b*.
* `a*` — zero or more repetitions of *a*.
* `a+` — one or more repetitions of *a*.
* `a?` — exactly zero or one repetition of *a*.
* `!a` — assert that there is no match to *a* (without consuming input).
* `a | b` — exactly one *a* or one *b* (*a* matches first)
* `(a <op> b)` — parentheses to disambiguate groups of rules and combinators.

## File Structure

A Whisker source file is composed of two sections: a leading `header` followed by the main `body`.

The `body` is a repeated pattern of textual content, variable interpolations, and control flow constructs (condition, iteration).
The rest of this document describes its contents.

The `header` is an *unrendered* section of the source. It may contain:
  * [Comments](#comments)
  * [Pragma statements](#pragma-statements)
  * [Import statements](#import-statements)
  * Whitespace-only [text](#text) (including `newline`) between header elements.

Whisker ignores whitespace in the `header` and does not render it.

<Grammar>

```
root → { header* ~ body* }

header     → { comment | pragma-statement | import-statement }

body       → { text | newline | template | comment }
text       → { <see below> }
newline    → { <see below> }
template   → { <see below> }
comment    → { <see below> }
```

</Grammar>

## Text

`text` is a sequence of unicode characters except `newline`s and `{{`, which denote the opening of a `template`, or a `comment`.

`newline` is either `"\r"`, `"\n"`, or `"\r\n"`.

:::note
To interpret `{{` as literal text, use the escape sequence `\{{`.
:::

## Comments

Rendering ignores comments, which do not appear in the output. Whisker supports two types of comments:

```whisker
{{! This is a comment }}
{{!-- This is a comment that contains "}}" --}}
```

<Grammar>

```
comment → { basic-comment | escaped-comment }

basic-comment   → { "{{!" ~ <unicode text until we see "}}"> ~ "}}" }
escaped-comment → { "{{!--" ~ <raw text until we see "--}}"> ~ "--}}" }
```

</Grammar>

## Templates

A `template` is the fundamental building block for the variability expressed in the output produced by Whisker. It is of the form `{{ ... }}`.

There are four categories of `template`s:
* ***Interpolations*** — `{{foo}}` — *interpolate* an *expression* into the rendered output.
* ***Blocks*** — `{{#foo}} ... {{/foo}}` — *control flow* capabilities like conditionals ([`{{#if}}`](#if-blocks)), or loops ([`{{#each}}`](#each-blocks)) etc.
  * All blocks open with an `{{# ...}}` tag.
  * All blocks close with a corresponding `{{/ ...}}` tag.
  * All blocks contain zero or more body elements inside them.
* ***Statements*** — `{{#foo}}` — non-rendering operations like name bindings ([`{{#let}}`](#let-statements)), `{{#else}}` etc.
  * All statements are of the form `{{# ...}}`. Unlike blocks, there is no closing tag.
* [***Macros***](#macros) — `{{> foo}}` — for reusable templates.

:::note
Whisker also supports Mustache-compatible *section blocks*: `{{#variable}}...{{/variable}}` and inverted sections `{{^variable}}...{{/variable}}`. These are retained for backwards compatibility with [`mstch`](https://github.com/no1msd/mstch). New templates should prefer `{{#if}}` and `{{#each}}` for clarity.
:::

<Grammar>

```
template → { interpolation | block | statement | macro }

interpolation → { <see below> }
block         → { <see below> }
statement     → { <see below> }
macro         → { <see below> }
```

</Grammar>

### Interpolations & Expressions

An `interpolation` causes the result of an `expression` to be rendered into the output at the position of the enclosing `{{ ... }}`.

There are three types of `expression`s in Whisker:
* ***Literal*** — "hard-coded" values in the template, like the string literal, `{{ "hello" }}`.
  * `string-literal` — a "C-style" string literal, enclosed in double quotes (`"..."`). Whisker supports a subset of [escape sequences from C](https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences): `\n`, `\r`, `\t`, `\\`, `\'`, and `\"`. Examples:
    * `"hello\nworld"`.
    * `"\"quoted\""`
  * `i64-literal` —  a *decimal* integer literal with an optional leading `-` for negative values. The value must be representable by a 64-bit two's complement signed integer: <code>-2<sup>63</sup> ≤ value < 2<sup>63</sup></code>. Examples:
    * `-742`
    * `0`
    * `124`
  * `boolean-literal` — either `true` or `false`.
  * `null-literal` — *exactly* `null`.
* ***Variable*** — values interpolated from the *context*, like `{{ person.name }}`. Variables allow scoped access into the context using a series of *components* separated by dots (`.`).
  * A *component* consists of an optional *qualifier* (an *identifier*), followed by a required *identifier* (*property*). When the qualifier is present, the syntax is `qualifier:property` (e.g., `{{ person.Base:name }}`).
  * Qualifiers are used to disambiguate property access on [`native_handle`](#data-model) objects with prototype chains. When a `native_handle`'s prototype inherits from a parent prototype, a child property may *shadow* a parent property of the same name. The qualifier allows explicitly accessing the parent's property by matching the parent prototype's registered name.
  * For [`map`](#data-model) objects, the qualifier and property are treated as a single key (e.g., `Base:name` looks up the key `"Base:name"`).
  * Additionally, the special *implicit context* variable, `{{ . }}` or `{{ this }}`, refers to the *context* object itself.
* ***Function call*** — [lisp-like](https://en.wikipedia.org/wiki/Lisp_(programming_language)#Syntax_and_semantics) syntax for invoking functions, often referred to as [S-expressions](https://en.wikipedia.org/wiki/S-expression). Functions are defined in the [*data model*](#data-model). Examples:
  * `{{ (uppercase "hello") }}`
  * `{{ (concat (uppercase person.firstName) " " (uppercase person.lastName)) }}`

:::note
The `and` and `or` builtin functions short-circuit their evaluation, meaning additional arguments will not be evaluated or checked for errors once the result is determined.
:::

:::note
Whisker includes a set of *keywords* that are reserved. These cannot be used as identifiers. See the grammar below.
:::

Every `expression` produces an `object`. However, not all `object`s are *printable*. The top-level `expression` in a `template` **must** be *printable*. `expression`s passed as arguments in a function call do **not** need be *printable*. See the [*data model*](#data-model) for details.

<Example>

```whisker title=example.whisker
{{foo.bar}}
{{! scoped property access }}

{{ (not true) }}
{{! `false` }}

{{ (uppercase "Hello") }}
{{! "HELLO" }}

{{ (int-to-string 16 format="hex")}}
{{! "0x10" }}

{{!--
  Named arguments (like `format="hex"`) must follow after all
  positional arguments (like `16`).
}}
```

</Example>

<Grammar>

```
interpolation → { "{{" ~ expression ~ "}}" }
expression    → { literal | variable | function-call }

literal             → { string-literal | i64-literal | boolean-literal | null-literal }
variable-component  → { [identifier ~ ":"] ~ identifier }
variable            → { "." | "this" | (variable-component ~ ("." ~ variable-component)*) }
function-call       → { "(" ~ (builtin-call | user-defined-call) ~ ")" }

string-literal  → { <see above> }
i64-literal     → { <see above> }
boolean-literal → { "true" | "false" }
null-literal    → { "null" }

identifier → { !keyword ~ (id_prefix ~ id_suffix*) }
keyword → {
  "true"     |
  "false"    |
  "null"     |
  "if"       |
  "unless"   |
  "else"     |
  "each"     |
  "as"       |
  "partial"  |
  "captures" |
  "let"      |
  "and"      |
  "or"       |
  "not"      |
  "with"     |
  "this"     |
  "define"   |
  "for"      |
  "do"       |
  "import"   |
  "export"   |
  "from"     |
  "pragma"   |
}
id_prefix → { alpha |        | '_' | '$' }
id_suffix → { alpha | digits | '_' | '$' | '-' | '+' | '?' }

builtin-call → {
  ("not" ~ expression) |
  (("and" | "or") ~ expression ~ expression+) |
  ("if" ~ expression ~ expression ~ expression) }

user-defined-call → {
  user-defined-call-lookup ~
  positional-argument* ~
  named-argument* }

user-defined-call-lookup → { variable }
positional-argument      → { expression }
named-argument           → { identifier ~ "=" ~ expression }
```

</Grammar>

### If Blocks

Whisker supports a conditionally rendering block type: `{{#if}}`. A typical conditional block might look like:

```whisker
{{#if person.hasName}}
Greetings, {{person.name}}!
{{#else if person.hasId}}
Beep boop, {{person.id}}!
{{#else}}
I don't know who you are.
{{/if person.hasName}}
```

`{{#if}}` blocks **may** include any number of `{{# else if ...}}` statements followed by one optional `{{#else}}` statement. The body of the first condition to obtain will be rendered, otherwise the body following the `{{#else}}` statement if provided, otherwise an empty block.

In this example, `person.hasName` is the *condition*. The condition **must** be an `expression` that evaluates to a `boolean`.

The closing tag must exactly replicate the `expression` of the matching opening tag. This serves to improve readability of complex nested conditions.
The closing `expression` may be omitted if and only if both `{{#if}}` and `{{/if}}` are on the same line.

<Example title="Example (positive)">

```json title=Context
{
  "person": {
    "hasName": true,
    "name": "Chris"
  }
}
```

```text title=Output
Greetings, Chris!
```

</Example>

<Example title="Example (negative)">

```json title=Context
{
  "person": {
    "hasName": false,
  }
}
```

```text title=Output
I don't know who you are.
```

</Example>

<Grammar>

```
if-block       → { if-block-open ~ body* ~ else-if-block* ~ else-block? ~ if-block-close }
if-block-open  → { "{{" ~ "#" ~ "if" ~ expression ~ "}}" }
else-if-block  → { "{{" ~ "#" ~ "else" ~ "if" ~ expression ~ "}}" ~ body* }
else-block     → { "{{" ~ "#" ~ "else" ~ "}}" ~ body* }
if-block-close → { "{{" ~ "/" ~ "if" ~ expression? ~ "}}" }
```

The `expression`s in `if-block-open` and `if-block-close` **must** be the same.

</Grammar>

Whisker `{{#if}}` blocks are based on [EmberJS `{{#if}}`](https://guides.emberjs.com/release/components/conditional-content/) and [Handlebars `{{#if}}`](https://handlebarsjs.com/guide/builtin-helpers.html#if).

### Each Blocks

Whisker supports a block type for repeated rendering: `{{#each}}`. A typical repetition block might look like:

```whisker
Rankings are:
{{#each winners as |winner|}}
{{winner}}
{{/each}}
```

The `expression` being iterated (`winners`) **must** evaluate to an `array`. The body will be rendered once for each element of the `array`.

The first capture name (`winner`) will be [locally bound](#scopes) to the current element in the `array`. The second capture name (`index`) will be [locally bound](#scopes) to the zero-based index (as an `i64`).

<Example>

```json title=Context
{
  "winners": [
    "Alice",
    "Bob",
    "Carol"
  ]
}
```

```text title=Output
Rankings are:
1. Alice
2. Bob
3. Carol
```

</Example>

`{{#each}}` blocks may have multiple captures.
If there is more than one capture, then each element of the expression being iterated **must** itself be an `array` with size equal to the number of captures.
Each element will be *destructured* into the corresponding capture names.

<Example title="Example with destructuring">

```whisker title=example.whisker
Rankings are:
{{#each winners as |position winner|}}
{{position}} - {{winner}}
{{/each}}
```

```json title=Context
{
  "winners": [
    ["1st", "Alice"],
    ["2nd", "Bob"],
    ["3rd", "Carol"]
  ]
}
```

```text title=Output
Rankings are:
1st - Alice
2nd - Bob
3rd - Carol
```

:::note
Whisker's standard library includes a function, `array.enumerate`, which can be used to associate array elements with their indices (similar to [Python's `enumerate`](https://docs.python.org/3/library/functions.html#enumerate)).
:::

</Example>

Captures are **optional**, in which case, the *implicit context* (`{{ . }}`) can be used.

<Example title="Example with implicit context">

```whisker title=example.whisker
Rankings are:
{{#each winners}}
{{.}}
{{/each}}
```

```json title=Context
{
  "winners": [
    "Alice",
    "Bob",
    "Carol"
  ]
}
```

```text title=Output
Rankings are:
Alice
Bob
Carol
```

</Example>

`{{#each}}` blocks may **optionally** have an `{{#else}}` statement. Its body is rendered only if the `array` is empty.

<Example title={<>Example with <code>else</code></>}>

```whisker title=example.whisker
{{#each people as |person|}}
Hello, {{person.name}}!
{{#else}}
There are no people.
{{/each}}
```

```json title=Context
{
  "people": []
}
```

```text title=Output
There are no people.
```

</Example>

#### Separator

The `{{#each}}` block supports an optional `separator` parameter that inserts a string
between iterations:

```
{{#each <iterable> [as |<captures>|] [separator=<expression>]}}
  <body>
{{/each}}
```

The separator expression is evaluated **once** — lazily on the first iteration — and
must produce a string value. If the array is empty (and the `{{#else}}` clause is used),
the separator expression is not evaluated. The separator is written to the output before
each non-first iteration.
The expression can be a string literal (`separator=", "`), a variable lookup
(`separator=my_sep`), or a function call (`separator=(string.concat "," " ")`).

<Example title="Comma-separated list">

```whisker
{{#each items as |item| separator=", " ~}}
    {{~ item ~}}
{{~ /each}}
```

```json title=Context
{"items": ["alpha", "beta", "gamma"]}
```

```text title=Output
alpha, beta, gamma
```

</Example>

<Example title="Single item produces no separator">

```whisker
{{#each items as |item| separator=", " ~}}
    {{~ item ~}}
{{~ /each}}
```

```json title=Context
{"items": ["only"]}
```

```text title=Output
only
```

</Example>

<Example title="Empty separator concatenates iterations">

```whisker
{{#each chars as |c| separator="" ~}}
    {{~ c ~}}
{{~ /each}}
```

```json title=Context
{"chars": ["a", "b", "c"]}
```

```text title=Output
abc
```

</Example>

<Example title="Separator from variable or function call">

The separator can be any expression that evaluates to a string — a literal, a variable
lookup, or a function call:

```whisker
{{#each items as |item| separator=my_separator ~}}
    {{~ item ~}}
{{~ /each}}
```

```json title=Context
{"items": ["x", "y", "z"], "my_separator": " | "}
```

```text title=Output
x | y | z
```

</Example>

<Example title="Building a function call with separator">

A common pattern in code generation — building a comma-separated argument list:

```whisker
fn {{func_name}}(
    {{~ #each args as |arg| separator=", " ~}}
    {{~ arg.type}} {{arg.name ~}}
    {{~ /each ~}}
)
```

```json title=Context
{
  "func_name": "process",
  "args": [
    {"type": "int", "name": "count"},
    {"type": "string", "name": "label"},
    {"type": "bool", "name": "verbose"}
  ]
}
```

```text title=Output
fn process(int count, string label, bool verbose)
```

Note the use of tildes (`~`) to strip template indentation and newlines. The separator
inserts `, ` between arguments. Without tildes, the template whitespace would appear in
the output — the separator parameter does not implicitly trim iteration output.

</Example>

<Example title="Separator with #else for empty arrays">

The `{{#else}}` clause is supported and renders when the iterable is empty:

```whisker
[{{#each items as |item| separator=", " ~}}
    {{~ item ~}}
{{~ #else ~}}
    empty
{{~ /each}}]
```

```json title=Context
{"items": []}
```

```text title=Output
[empty]
```

</Example>

##### Whitespace Control

The separator parameter does **not** implicitly trim whitespace from each iteration's
output. It behaves identically to `{{#each}}` without `separator`, except that the
separator string is written between iterations.

To produce single-line output from multi-line template source, use
[tilde trimming](#tilde-whitespace-trimming) on the `{{#each}}`, body tags, and
`{{/each}}` tags:

```whisker
{{! Without tildes — template whitespace leaks into output: }}
{{#each items as |item| separator=", "}}
    {{item}}
{{/each}}
{{! Output: "    alpha\n,     beta\n,     gamma\n" }}

{{! With tildes — clean single-line output: }}
{{#each items as |item| separator=", " ~}}
    {{~ item ~}}
{{~ /each}}
{{! Output: "alpha, beta, gamma" }}
```

##### Interaction with Conditional Content

If a conditional within the loop body produces empty output for some iterations, the
separator is still inserted. Use array filtering upstream to exclude items:

<Example title="Separator is inserted even for empty iterations">

```whisker
{{#each items as |item| separator=", " ~}}
    {{~ #if item.visible ~}}{{item.name}}{{~ /if ~}}
{{~ /each}}
```

```json title=Context
{
  "items": [
    {"name": "a", "visible": true},
    {"name": "b", "visible": false},
    {"name": "c", "visible": true}
  ]
}
```

```text title=Output
a, , c
```

Note the empty segment between separators — the separator is inserted between every
pair of iterations regardless of whether the body produces output.

</Example>

<Grammar>

```
each-block          → { each-block-open ~ body* ~ else-block? ~ each-block-close }
each-block-open     → { "{{" ~ "#" ~ "each" ~ expression ~ each-block-capture? ~ separator-clause? ~ "}}" }
each-block-capture  → { "as" ~ "|" ~ identifier+ ~ "|" }
separator-clause    → { "separator" ~ "=" ~ expression }
else-block          → { "{{" ~ "#" ~ "else" ~ "}}" ~ body* }
each-block-close    → { "{{" ~ "/" ~ "each" ~ "}}"  }
```

</Grammar>

Whisker `{{#each}}` blocks are based on [EmberJS `{{#each}}`](https://guides.emberjs.com/release/components/looping-through-lists/) and [Handlebars `{{#each}}`](https://handlebarsjs.com/guide/builtin-helpers.html#each).


### With Blocks

Whisker supports a block type for de-structuring: `{{#with}}`. A typical de-structuring block might look like:

```whisker
{{#with person}}
The name's {{lastName}}... {{firstName}} {{lastName}}.
{{/with}}
```

The `expression` being de-structured (`person`) **must** evaluate to a `map`, or a `native_object` that supports `map`-like property access.

The body will be rendered exactly once. The result of the expression (`person`) will become the [*implicit context* `object`](#scopes) within the block, meaning it can be accessed using `{{ . }}`. All its properties can be accessed without the `person.` prefix.

<Example>

```json title=Context
{
  "person": {
    "firstName": "James",
    "lastName": "Bond"
  }
}
```

```text title=Output
The name's Bond... James Bond.
```

</Example>

<Grammar>

```
with-block       → { with-block-open ~ body* ~ with-block-close }
with-block-open  → { "{{" ~ "#" ~ "with" ~ expression ~ "}}" }
with-block-close → { "{{" ~ "/" ~ "with" ~ "}}" }
```

</Grammar>

### Let Statements

Whisker `{{#let}}` statements allow binding the result of an `expression` to an *identifier* in the [current scope](#scopes). A simple `{{#let}}` statement might look like:

```whisker
{{#let result = (add input 1)}}
{{result}}
```

The `expression` is eagerly evaluated exactly once and the name becomes accessible in the lexical scope of the `{{#let}}` statements.

The primary purpose of `{{#let}}` statements is to simplify complex `expression`s by breaking them down into smaller parts.

<Example>

```json title=Context
{
  "input": 41
}
```

```text title=Output
42
```

</Example>

<Example title={<>Example with <code>each</code></>}>

The `{{#let}}` statement will evaluate repeatedly since every iteration of the `{{#each}}` introduces a new scope.

```whisker title=example.whisker
{{#each numbers as |n|}}
  {{#let result = (add n 1)}}
  {{result}}
{{/each}}
```


```json title=Context
{
  "numbers": [1, 2, 3]
}
```

```text title=Output
2
3
4
```

</Example>

<Example title={<>Example with multiple <code>let</code>s</>}>

`{{#let}}` statements can be composed together.

```whisker title=example.whisker
{{#let x = (add 5 4)}}
{{#let y = (add x 1)}}
{{y}}
```

```text title=Output
10
```

</Example>

`{{#let}}` statements may be [exported](#exports) by adding `export` to their definitions.

<Example title={<>Example with <code>export</code></>}>

```whisker
{{#let export answer = 42}}
```

</Example>

<Grammar>

```
let-statement → { "{{" ~ "#" ~ "let" ~ "export"? ~ identifier ~ "=" ~ expression ~ "}}" }
```

</Grammar>

### Pragma Statements

Whisker `{{#pragma}}` statements allow modifying rendering behavior for the current template or partial.

Currently the only pragma supported is `ignore-newlines`, which suppresses all newlines in the current source file.

:::note
Pragma statements may appear in both the [header](#file-structure) and the body of a template.
:::

<Example>

```whisker
{{#pragma ignore-newlines}}
This
 is
 all
 one
 line
.
```

```text title=Output
This is all one line.
```

</Example>

<Grammar>

```
pragma-statement → { "{{" ~ "#" ~ "pragma" ~ ( "ignore-newlines" ) ~ "}}" }
```

</Grammar>

### Partial Blocks & Statements

Partial blocks allow defining reusable templates within a Whisker template. They are not rendered unless *applied* (by name).
The following example of a `{{#let partial}}` block defines a partial named `greeting` that accepts a single argument named `person`:

```whisker
{{#let partial greeting |person|}}
Greetings, {{person.firstName}} {{person.lastName}}!
{{/let partial}}
```

Partial blocks must be rendered using `{{#partial ...}}` statements. A simple example of a `{{#partial}}` statement for the above block might be:

```whisker
{{#partial greeting person=person}}
```

The `{{#partial}}` statement must include all named arguments from the partial block being applied.
Each named argument is an `expression`, which is [bound](#scopes) to the corresponding argument (matching `|...|`) from the partial block.

The contained body of the `{{#let partial}}` block is rendered with a [derived evaluation context](#derived-evaluation-context). Names accessible from the site of the application are **not** *implicitly* available within the block.

<Example>

```whisker
{{#let partial greeting |person|}}
Greetings, {{person.firstName}} {{person.lastName}}!
{{/let partial}}

{{#partial greeting person=dave}}
```

```json title=Context
{
  "dave": {
    "firstName": "Dave",
    "lastName": "Grohl"
  }
}
```

```text title=Output
Greetings, Dave Grohl!
```

</Example>

To implement recursive partials, a partial can access itself by name.

<Example title="Example with recursion">

```whisker
{{! https://en.wikipedia.org/wiki/Collatz_conjecture }}
{{#let partial collatz |n|}}
{{n}}
  {{#if (ne? n 1)}}
    {{#if (even? n)}}
{{#partial collatz n=(div n 2)}}
    {{#else}}
{{#partial collatz n=(add (mul 3 n) 1)}}
    {{/if (even? n)}}
  {{/if (ne? n 1)}}
{{/let partial}}
{{#partial collatz n=6}}
```

```javascript title=Globals
{
  "even?": (n) => n % 2 == 0,
  "mul": (a, b) => a * b,
  "div": (a, b) => a / b,
  "ne?": (a, b) => a != b,
  "add": (a, b) => a + b,
}
```

```text title=Output
6
3
10
5
16
8
4
2
1
```

</Example>

Partial blocks and statements do not require arguments.

<Example title="Example without arguments">

```whisker
{{#let partial copyright}}
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
{{/let partial}}
{{#partial copyright}}
```

```text title=Output
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
```

</Example>

Partial blocks may have multiple arguments. The arguments may be provided in any order.

<Example title="Example with multiple arguments">

```whisker
{{#let partial greeting |firstName lastName|}}
Greetings, {{firstName}} {{lastName}}!
{{/let partial}}

{{#partial greeting lastName=dave.lastName firstName=dave.firstName}}
```

```json title=Context
{
  "dave": {
    "firstName": "Dave",
    "lastName": "Grohl"
  }
}
```

```text title=Output
Greetings, Dave Grohl!
```

</Example>

The name of a partial block is [bound](#scopes) to an [object](#data-model) in the current evaluation context, meaning it follows normal [name resolution rules](#evaluation-context). As a result, a partial block can be passed around like any other object.

<Example title="Example with partial as object">

```whisker
{{#let partial greeting |firstName lastName|}}
Greetings, {{firstName}} {{lastName}}!
{{/let partial}}

{{#let partial with-firstName-lastName |person action|}}
{{#partial action firstName=person.firstName lastName=person.lastName}}
{{/let partial}}

{{#partial with-firstName-lastName person=dave action=greeting}}
```

```json title=Context
{
  "dave": {
    "firstName": "Dave",
    "lastName": "Grohl"
  }
}
```

```text title=Output
Greetings, Dave Grohl!
```

</Example>

Partial blocks may have captures. Captures allow partial blocks to refer to objects in scope at *definition*. This is different from named arguments, which refer to objects in scope at *application*. (i.e. the caller).

A common use of captures is to compose partial blocks together.

<Example title="Example with captures">

```whisker
{{#let partial greeting |firstName lastName|}}
Greetings, {{firstName}} {{lastName}}!
{{/let partial}}

{{#let partial greet-person |person| captures |greeting|}}
{{#partial greeting firstName=person.firstName lastName=person.lastName}}
{{/let partial}}

{{#partial greet-person person=dave}}
```

```json title=Context
{
  "dave": {
    "firstName": "Dave",
    "lastName": "Grohl"
  }
}
```

```text title=Output
Greetings, Dave Grohl!
```

</Example>

Partial statements retain the *preceding indentation* at the site of the application. Every line of the partial being applied is indented by this same amount.

<Example title="Example with indentation">

```whisker title=example.whisker
{{#let partial president |person|}}
{{person.lastName}}
  {{person.firstName}}
{{/let partial}}
Some historic presidents are:
{{#each presidents as |person|}}
  {{#partial president person=person}}
{{/each}}
```

```json title=Context
{
  "presidents": [
    {
      "firstName": "Abraham",
      "lastName": "Lincoln"
    },
    {
      "firstName": "Franklin",
      "lastName": "Roosevelt"
    }
  ]
}
```

Notice that `Abraham` and `Franklin` are doubly indented.
```text title=Output
Some historic presidents are:
  Lincoln
    Abraham
  Roosevelt
    Franklin
```

</Example>

Partial blocks may be [exported](#exports) by adding `export` to their definitions.

<Example title={<>Example with <code>export</code></>}>

```whisker
{{#let export partial foo}}
Hello world!
{{/let partial}}
```

</Example>

<Grammar>

```
partial-block           → { partial-block-open ~ body* ~ partial-block-close }
partial-block-open      → { "{{#" ~ "let" ~
                              partial-name-spec ~
                              partial-block-arguments? ~
                              partial-block-captures? ~
                            "}}" }
partial-name-spec       → { "export"? ~ "partial" ~ identifier }
partial-block-arguments → { "|" ~ identifier+ ~ "|" }
partial-block-captures  → { "captures" ~ "|" ~ identifier+ ~ "|" }
partial-block-close     → { "{{/" ~ "let" ~ "partial" ~ "}}" }

partial-statement → { "{{" ~ "#" ~ "partial" ~ expression ~ partial-argument* ~ "}}" }
partial-argument  → { identifier ~ "=" ~ expression }
```

</Grammar>

Whisker `{{#let partial}}` blocks are based on [Handlebars partial parameters](https://handlebarsjs.com/guide/partials.html#partial-parameters).

### Macros

Macros are reusable templates that are not rendered unless *applied* (by a path). A simple example of macro application might be:

```whisker
{{> path/to/my-partial}}
```

:::note
Macros cannot be defined in Whisker — they must be provided by the runtime environment (e.g. C++).
:::

Macros assume the [scope](#scopes) at the site of application. This behavior is analogous to [C preprocessor macro expansion](https://en.wikipedia.org/wiki/C_preprocessor#Macro_definition_and_expansion).
Names accessible from the site of the application are also available within the block.

<Example>

```whisker title=example.whisker
{{> greeting}}
```

```whisker title=greeting.whisker
Greetings, {{person.firstName}} {{person.lastName}}!
```

```json title=Context
{
  "person": {
    "firstName": "Dave",
    "lastName": "Grohl"
  }
}
```

```text title=Output
Greetings, Dave Grohl!
```

</Example>

<Grammar>

```
macro          → { "{{" ~ ">" ~ macro-lookup ~ "}}" }
macro-lookup   → { path-component ~ ("/" ~ path-component)* }

path-component → { <see below> }
```

A `path-component` must satisfy [POSIX's portable file name character set](https://www.ibm.com/docs/en/zvm/7.3?topic=files-naming). A valid path, therefore, will pass the GNU coreutils [`pathchk`](https://github.com/coreutils/coreutils/blob/v9.5/src/pathchk.c#L157-L202) [command](https://man7.org/linux/man-pages/man1/pathchk.1.html) (minus
length restrictions).

</Grammar>

### Modules

Whisker templates may be partitioned into multiple source files to improve code re-use. `{{#import}}` statements and `export`-ed constructs allow files to share objects.

<Example>

```whisker title=lib.whisker
{{#let export partial copyright}}
// This code belongs to me!
{{/let partial}}
```

```whisker title=example.whisker
{{#import "lib.whisker" as lib}}
{{#partial lib.copyright}}
```

```text title=Output
// This code belongs to me!
```

</Example>

Every Whisker source file falls into one of two categories: **root** or **module**.

Root files are the entry points for rendering. They cannot have [exports](#exports).

Module files are all non-root files. They cannot have renderable `body` elements at the outer-most scope (e.g. `text`).
[Partial blocks](#partial-blocks--statements) containing `body` elements are allowed in modules.
Due to these constraints, a module file is useful only as the target of an `{{#import}}` statement.

:::note
Whisker classifies [macros](#macros) as root files.
:::

#### Exports

The following constructs may be exported from a module:
* [Partial blocks](#partial-blocks--statements)
* [Let statements](#let-statements)

The [`map`](#data-model) formed by a module's exports is called its *export map*.

<Example>

```whisker
{{#let export partial foo}}
Hello
{{/let partial}}

{{#let export answer = 42}}
```

```javascript title=export map
{
  "foo": native_handle(<partial foo>),
  "answer": 42,
}
```

</Example>

Every export must be unique.

<Example title="Example with duplicate export">

It is an error to export `answer` twice.

```whisker
{{#let export answer = 42}}
{{#let export answer = 24}}
```

</Example>

Exports must be at the outer-most scope of the a module.

<Example title="Example with conditional export">

It is an error to export `good-mood?` in a nested scope, such as a conditional block.

```whisker
{{#if sunny?}}
  {{#let export good-mood? = true}}
{{#else}}
  {{#let export good-mood? = false}}
{{/if sunny?}}
```

</Example>

#### Import Statements

Import statements appear in the `header` of any source file. A source file may have multiple imports.
The target of an import must be a module (non-root) file.

<Example>

The `identifier` in an import statement [binds a name](#scopes) to the target module's *export map*.

```whisker title=a.whisker
{{#let export partial copyright}}
// This code belongs to me!
{{/let partial}}
```

```whisker title=b.whisker
{{#let export answer = 42}}
```

```whisker title=example.whisker
{{#import "a.whisker" as module_a}}
{{#import "b.whisker" as module_b}}
{{#partial module_a.copyright}}
{{module_b.answer}}
```

```text title=Output
// This code belongs to me!
42
```

</Example>

<Grammar>

```
import-statement → { "{{" ~ "#" ~ "import" ~ import-path ~ "as" ~ identifier ~ "}}" }
import-path      → { string-literal }
```

</Grammar>

:::note
Whisker resolves an import path to a Whisker source file in an implementation-defined manner.
:::

## Data Model

The *context* provided when Whisker renders a template follows Whisker's *data model*. JSON heavily influences Whisker's type system.

A Whisker `object` is one of the following types:
* `i64` — 64-bit two's complement signed integer (<code>-2<sup>63</sup> ≤ value < 2<sup>63</sup></code>).
* `f64` — [IEEE 754 `binary64` floating point number](https://en.wikipedia.org/wiki/Double-precision_floating-point_format#IEEE_754_double-precision_binary_floating-point_format:_binary64).
* `string` — sequence of Unicode characters.
* `boolean` — 1-bit `true` or `false`.
* `null` — marker indicating ["no value"](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/null).
* `array` — An ordered list of `object` (note the recursion). An `array` can be traversed using [`{{#each}}`](#each-blocks).
* `map` — An unordered set of key-value pairs. Keys are valid *identifiers* represented as `string`s. Values are any `object` (note the recursion). A `map` can be *unpacked* using [`{{#with}}`](#with-blocks) or accessed through [variable interpolation](#interpolations--expressions).
* `native_function` — An implementation-defined type that can be invoked in [Whisker templates](#interpolations--expressions). A value of this type can only be created by the native runtime (e.g. C++).
* `native_handle` — An opaque reference to an implementation-defined type that Whisker can only interact with through a *prototype*. A value of this type without a *prototype* is meaningful only to the native runtime (e.g. C++) in conjunction with `native_function`.

The following types are *printable*: `i64`, `string`.

:::note
There is no literal syntax for `f64` values in Whisker templates. Values of type `f64` can only originate from the native runtime (e.g., as context variables or return values from `native_function`s).
:::

#### Strict Rendering Modes

Whisker's rendering behavior can be configured with the following strictness options, which are set by the native runtime:

* **Strict boolean conditionals** (default: *enabled*) — When enabled, the condition in `{{#if}}` blocks **must** evaluate to a `boolean`. When disabled, non-boolean values are coerced to boolean using [truthiness rules](#truthiness-coercion).
* **Strict printable types** (default: *enabled*) — When enabled, only `i64` and `string` values can be interpolated. When disabled, `f64` is rendered as its decimal representation, `boolean` is rendered as `"true"` or `"false"`, and `null` is rendered as an empty string. `array`, `map`, `native_function`, and `native_handle` are *never* printable.
* **Strict undefined variables** (default: *enabled*) — When enabled, referencing an undefined variable is a fatal error. When disabled, undefined variables silently resolve to `null`.

#### Truthiness Coercion

When strict boolean conditionals are disabled, or when using [section blocks](#templates) (which always use coercion), Whisker coerces values to booleans as follows:

| Type | Falsy | Truthy |
| --- | --- | --- |
| `boolean` | `false` | `true` |
| `null` | always | never |
| `i64` | `0` | non-zero |
| `f64` | `0.0`, `-0.0`, `NaN` | all other values |
| `string` | `""` (empty) | non-empty |
| `array` | `[]` (empty) | non-empty |
| `map` | never | always |
| `native_function` | never | always |
| `native_handle` | never | always |

### Native Function

A `native_function` represents a native runtime (e.g. C++) function implementation that is callable from Whisker templates via [function call expressions](#interpolations--expressions).

A `native_function` accepts two kinds of arguments:
* **Positional arguments** — an ordered list of `object`s. Example: `{{ (add 1 2) }}`.
* **Named arguments** — an unordered set of key-value pairs, where keys are identifiers and values are `object`s. Named arguments must follow all positional arguments. Example: `{{ (array.enumerate items with_first=true) }}`.

A `native_function` produces exactly one value of type `object`.

When a function is accessed as a property of an object (e.g., `{{ (obj.method arg) }}`), the parent object (`obj`) is implicitly passed to the function as a `self` reference. This enables method-call semantics on [`native_handle`](#data-model) objects.

## Evaluation Context

Whisker's evaluation context is responsible for name resolution within Whisker templates.

The evaluation context is a stack of [*lexical scopes*](https://en.wikipedia.org/wiki/Scope_(computer_science)). Typically, blocks such `{{#if}}` or `{{#each}}` push new scopes onto the stack upon entry, and pop them at exit.

The *current* lexical scope (sometimes also called the *current* evaluation context) is the scope at the top of the stack.

```whisker
{{name}}
{{#each people}}
  {{name}}
{{/each}}
{{name}}
```

<Example>

```json title=Context
{
  "name": "outer",
  "people": [
    {"name": "Alice"},
    {"name": "Bob"}
  ]
}
```

```text title=Output
outer
  Alice
  Bob
outer
```

In this example, every iteration of `{{#each}}` pushes a new lexical scope onto the evaluation context stack.
Since the *current* scope is changing, the output for `{{name}}` differs between interpolations.

</Example>

### Scopes

A *lexical scope* has two key properties:
  * an **implicit context** represented by an [`object`](#data-model).
  * a set of **local bindings** stored in a [`map`](#data-model).

An evaluation context consists of three key elements:
  * a **stack of *lexical scopes***.
  * a **global scope** represented by a [`map`](#data-model).

The *global scope* is always the bottom of the stack and cannot be popped.

Whisker uses scopes to describe how top-level names are resolved. This process answers the question: what does `foo` in `{{foo.bar.baz}}` refer to? By contrast, `bar` and `baz` represent *property* resolutions, which are straightforward lookups into a `map` or `map`-like `native_object`.

When resolving an *identifier* like `name`, the process works as follows:
1. Start with the *current* scope (at the top of the stack).
2. Check the **local bindings** `map` for `name`.
    * If found, return the corresponding `object`.
3. Check the **implicit context** `object`'s properties.
    * If a property is found, return the corresponding `object`.
4. Move to the previous scope in the stack and repeat (2) and (3).
    * If the bottom of the stack is reached without resolution, return an error.

Use [`{{#let}}`](#let-statements), [`{{#import}}](#import-statements), `as` captures in [`{{#each}}`](#each-blocks), and similar constructs to add local bindings to the current scope.

`null` represents scopes that lack an **implicit context** `object`. For instance, [`{{#if}}`](#if-blocks) blocks always have a `null` context. [`{{#each}}`](#each-blocks) blocks have a `null` context in the presence of captures.

### Derived Evaluation Context

Whisker renders `{{#let partial}}` blocks within a new evaluation context *derived* from the the call site. This context starts with an empty stack but retains access to the same global scope.

## Standalone Tags

The Mustache spec defines rules around a concept called [*standalone lines*](https://github.com/mustache/spec/blob/v1.4.2/specs/sections.yml#L279-L305).

If a line has control flow tags, but is otherwise only whitespace, then implementations should strip the entire line from the output.
The following tags are standalone-stripping eligible:
* `{{! ... }}` — [comments](#comments)
* `{{# ... }}` — blocks ([`{{#if}}`](#if-blocks), [`{{#each}}`](#each-blocks), [`{{#with}}`](#with-blocks), [`{{#let partial}}`](#partial-blocks--statements)) and statements ([`{{#let}}`](#let-statements), [`{{#partial}}`](#partial-blocks--statements))
* `{{/ ... }}` — closing tag for blocks listed above
* `{{> ... }}` — [macros](#macros)

<Example>

```whisker title=example.whisker
  {{#if true_value}}
    hello
  {{/if true_value}}
```

```json title=Context
{
  "true_value": true
}
```

```text title=Output
    hello

```

Notice that the whitespace (including newline) around the tags, `{{#if true_value}}` and `{{/if true_value}}`, are stripped from the output.
The newline following "hello" is retained.

</Example>

Lines with tags that perform *interpolation*, or with non-whitespace text content, are standalone-stripping ineligible.

<Example title="Example with interpolation">

```whisker title=example.whisker
| *
  {{#if true_value}}  hello
  {{hello}}{{/if true_value}}
| *
```

```json title=Context
{
  "true_value": true,
  "hello": "world"
}
```

```text title=Output
| *
    hello
  world
| *
```

Notice that both lines are standalone-stripping ineligible and retain their whitespace.

</Example>

Only [`newline` tokens in the grammar](#text) denote the end of lines.
In other words, a line break inside a tag does *not* result in a new line for the purpose of standalone line stripping.

<Example title="Example with multi-line tag">

```whisker title=example.whisker
| This Is
  {{#if boolean
          .condition}}
|
  {{/if boolean.condition}}
| A Line
```

```json title=Context
{
  "boolean": {
    "condition": true
  }
}
```

```text title=Output
| This Is
|
| A Line
```

Notice that `boolean` and `.condition` are on separate lines, yet both lines were stripped from the output.

</Example>

[Partial statements](#partial-blocks--statements) and [macros](#macros) have special behavior in the context of standalone line stripping, even though they perform interpolation.
If the application is standalone, then the whitespace **to the left is preserved**, while the one **to the right is stripped**.

<Example title="Example with macro">

```whisker title=example.whisker
| *
  {{#if true_value}}
  {{> my-partial}}
  {{/if true_value}}
| *
```

```whisker title=my-partial.whisker
hello world
```

```json title=Context
{
  "true_value": true
}
```

```text title=Output
| *
  hello world
| *
```

</Example>

A standalone-stripped line can have multiple tags, as long as none of tags are [partial statements](#partial-blocks--statements) or [macros](#macros).

<Example title="Example with multiple tags">

```whisker title=example.whisker
| *
  {{#if a}}{{#if b}}{{#if c}}
| hello
  {{/if c}}{{/if b}}{{/if a}}
| *
```

```json title=Context
{
  "a": true,
  "b": true,
  "c": true
}
```

```text title=Output
| *
| hello
| *
```

</Example>

<Example title="Example with multiple tags and macro">

```whisker title=example.whisker
| *
  {{#if a}}{{#if b}}{{#if c}}{{> my-partial}}
| hello
  {{/if c}}{{/if b}}{{/if a}}
| *
```

```whisker title=my-partial.whisker
hello world
```

```json title=Context
{
  "a": true,
  "b": true,
  "c": true
}
```

```text title=Output
| *
  hello world
| hello
| *
```

:::note
Handlebars does *not* consider multiple tags on the same line as standalone.
Whisker supports such behavior to retain compatibility with [`mstch`](https://github.com/no1msd/mstch).
:::

</Example>

## Tilde Whitespace Trimming

[Standalone line stripping](#standalone-tags) automatically removes lines that contain only tags. However, there are cases where this is insufficient — inline conditionals, multi-tag lines where some tags produce output, or situations where template structure and output structure intentionally diverge. For these cases, Whisker supports **tilde whitespace trimming**.

Adding a `~` inside the `{{`/`}}` delimiters strips whitespace (including newlines) on the indicated side of the tag:

* `{{~ expr }}` — **left tilde**: strips all adjacent whitespace *before* the tag.
* `{{ expr ~}}` — **right tilde**: strips all adjacent whitespace *after* the tag.
* `{{~ expr ~}}` — **both tildes**: strips whitespace on both sides.
* `{{ expr }}` — **no tildes**: no trimming (status quo).

Tilde spacing follows one rule:

* A tilde must be adjacent to the delimiter it modifies.
* A tilde must be separated from tag content by whitespace.

This is a deliberate readability constraint:

```whisker
{{~ #if condition ~}}     ✓  left tilde touches `{{`, right tilde touches `}}`
{{~#if condition ~}}      ✗  no whitespace between left tilde and tag content
{{#if condition ~}}       ✓  whitespace between tag content and right tilde
{{#if condition~}}        ✗  no whitespace between tag content and right tilde
{{#if condition ~ }}      ✗  whitespace between right tilde and `}}`
```

Escaped comments use `-- ~}}` for a right-tilde close:

```whisker
{{!-- comment -- ~}}      ✓
{{!-- comment --~}}       ✗
```

Tilde trimming applies to interpolations (`{{ }}`), blocks (`{{# }}`), closing tags (`{{/ }}`), statements (`{{# }}`), macros (`{{> }}`), partial statements, and comments (`{{! }}`). Import statements (`{{#import}}`) do not support tilde trimming.

<Grammar>

```
template-open  → { "{{" ~ "~"? }
template-close → { "~"? ~ "}}" }
```

</Grammar>

Left tilde strips **all** adjacent whitespace characters (spaces, tabs, newlines) before the tag in the template source, up to the nearest non-whitespace content or tag boundary. Right tilde strips **all** adjacent whitespace characters after the tag, up to the nearest non-whitespace content or tag boundary.

:::note
Tilde trimming only strips **template whitespace** — whitespace that appears literally in the template source text. Whitespace that originates from **interpolation output** (the result of evaluating an expression like `{{name}}`) is never stripped. Tildes walk through the token stream and stop at tag boundaries (`{{` and `}}`), so they cannot cross into adjacent tags or affect their output.
:::

<Example title="Left tilde">

```whisker title=example.whisker
foo   {{~ bar}}
```

```json title=Context
{ "bar": "BAR" }
```

```text title=Output
fooBAR
```

The three spaces between `foo` and the tag are stripped by the left tilde.

</Example>

<Example title="Right tilde">

```whisker title=example.whisker
{{bar ~}}   baz
```

```json title=Context
{ "bar": "BAR" }
```

```text title=Output
BARbaz
```

The three spaces after the tag are stripped by the right tilde.

</Example>

<Example title="Both tildes">

```whisker title=example.whisker
  {{~ bar ~}}
```

```json title=Context
{ "bar": "BAR" }
```

```text title=Output
BAR
```

</Example>

<Example title="Stripping across newlines">

```whisker title=example.whisker
foo
  {{~ bar}}
```

```json title=Context
{ "bar": "BAR" }
```

```text title=Output
fooBAR
```

The left tilde strips the newline after `foo` and the two spaces of indentation.

</Example>

<Example title="Interpolation output is not stripped">

Tilde trimming only strips template whitespace — whitespace written literally in the template. Whitespace that comes from interpolation output is preserved.

```whisker title=example.whisker
{{a}}  {{~ b}}
```

```json title=Context
{ "a": "hello  ", "b": "world" }
```

```text title=Output
hello  world
```

The left tilde on `b` strips the two template spaces between the tags. However, the trailing spaces in `a`'s interpolation output (`"hello  "`) are **not** stripped — they are part of the rendered value, not template text.

</Example>

<Example title="Right tilde does not strip interpolation output">

```whisker title=example.whisker
{{a ~}}  {{b}}
```

```json title=Context
{ "a": "X", "b": "  world" }
```

```text title=Output
X  world
```

The right tilde on `a` strips the two template spaces after the tag. The leading spaces in `b`'s output (`"  world"`) are preserved because they are interpolation output.

</Example>

<Example title="Whitespace-only interpolation is preserved">

Even if an interpolation produces only whitespace, tildes do not strip it.

```whisker title=example.whisker
before  {{~ spacer ~}}  after
```

```json title=Context
{ "spacer": "   " }
```

```text title=Output
before   after
```

The tildes strip the template spaces around the tag, but `spacer`'s three-space output is preserved intact.

</Example>

<Example title="Inline conditional content">

Tildes strip whitespace, joining content from adjacent lines. Since a tilde prevents standalone stripping, use both tildes to control whitespace on both sides:

```whisker title=example.whisker
{{~ #if has_namespace ~}}
    namespace {{namespace}} {
{{/if has_namespace}}
```

```json title=Context
{
  "has_namespace": true,
  "namespace": "my_ns"
}
```

The left `~` strips any whitespace before the tag, and the right `~` eats the newline and the 4 spaces of the next line's indentation.

```text title=Output
namespace my_ns {
```

</Example>

<Example title="Derive macro">

```whisker title=example.whisker
#[derive(
    {{~ #if copy? }}Copy, {{/if copy? ~}}
    Clone, PartialEq
    {{~ #if ord? }}, Eq, PartialOrd, Ord, Hash{{/if ord? ~}}
    {{~ #if serde? }}, ::serde_derive::Serialize, ::serde_derive::Deserialize{{/if serde? ~}}
)]
```

```json title=Context
{
  "copy?": true,
  "ord?": true,
  "serde?": false
}
```

```text title=Output
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
```

Conditional derive traits are inlined without the `{{!` comment hack that was previously needed.

</Example>

<Example title="Untaken branch">

When an `{{#if}}` condition is false and there is no `{{#else}}`, a right tilde on the opening tag strips whitespace *through* the empty block.

```whisker title=example.whisker
before  {{~ #if cond ~}}  skipped  {{~ /if cond ~}}  after
```

```json title=Context
{ "cond": false }
```

```text title=Output
beforeafter
```

The block body is not rendered. The right tilde on `{{#if ~}}` strips whitespace after the opening tag (into the body), and the left tilde on `{{~ /if}}` strips whitespace before the closing tag (end of the body). The right tilde on `{{/if ~}}` strips whitespace after the closing tag. Together, all whitespace between `before` and `after` is removed.

</Example>

### Tilde on Comments

Comments (`{{! }}` and `{{!-- --}}`) support tilde trimming with the same rules as other tags. This is useful when a comment appears inline and you want to suppress surrounding whitespace:

<Example title="Comment with tildes">

```whisker title=example.whisker
foo   {{~ ! annotation ~}}   bar
```

```text title=Output
foobar
```

The left tilde strips the spaces after `foo`, and the right tilde strips the spaces before `bar`. The comment produces no output, so the result is `foobar`.

</Example>

Escaped comments require whitespace between `--` and `~}}`:

```whisker
{{~ !-- long comment --}}        left tilde
{{!-- long comment -- ~}}        right tilde
{{~ !-- long comment -- ~}}      both tildes
```

### Interaction with Standalone Line Stripping

Tilde trimming and [standalone line stripping](#standalone-tags) are **mutually exclusive** mechanisms:

- **Standalone stripping** is automatic and handles the common case: lines containing only tags are removed entirely.
- **Tilde trimming** is explicit and handles edge cases where standalone stripping is insufficient.

A line containing any tag with a tilde (`~`) is **not standalone-eligible**. When a template author uses a tilde, they opt into explicit whitespace control, and the automatic standalone mechanism steps aside. This ensures unsurprising behavior: tildes and standalone never interact, so there is no "double-stripping." If you want whitespace stripped on both sides of a tag, use both tildes (`{{~ expr ~}}`).

<Example title="Both tildes for full whitespace control">

```whisker title=example.whisker
    {{~ #if has_namespace ~}}
    namespace {{namespace}} {
    {{/if has_namespace}}
```

```json title=Context
{
  "has_namespace": true,
  "namespace": "my_ns"
}
```

```text title=Output
namespace my_ns {
```

The `{{~ #if ~}}` tag has tildes, so its line is **not** standalone-stripped. Instead, the left `~` strips the 4 spaces of leading indentation, and the right `~` strips the newline and the 4 spaces of the next line's indentation — producing `namespace my_ns {` with no leading whitespace.

Note that both tildes are needed. A right-only tilde (`{{#if has_namespace ~}}`) would preserve the leading indent because the line is not standalone.

</Example>

#### Behavior Matrix

The following table summarizes the interaction between standalone stripping and tilde trimming:

| Template Line | Standalone? | Tildes | Behavior |
|---|---|---|---|
| `    {{#if x}}` | Yes | None | Line removed (standalone). |
| `    {{#if x ~}}` | No | Right | Not standalone (has tilde). `~` strips whitespace/newline to the right. Leading indent preserved. |
| `    {{~ #if x}}` | No | Left | Not standalone (has tilde). `~` strips whitespace/newline to the left. Trailing newline preserved. |
| `    {{~ #if x ~}}` | No | Both | Not standalone (has tilde). `~` strips whitespace on both sides. |
| `foo {{#if x}}` | No | None | Tag removed; surrounding text and whitespace preserved. |
| `foo {{~ #if x}}` | No | Left | Tag removed; `~` eats whitespace between `foo` and the tag. |
| `foo {{#if x ~}}` | No | Right | Tag removed; `~` eats whitespace/newline after the tag. |

## Built-in functions

### `newline`

A built-in string constant containing a newline character (`"\n"`).
Since `newline` forms an interpolation, it can be used in conjunction with whitespace control features such as tilde whitespace trimming or standalone lines to get the desired output.

<Example>

```whisker title=example.whisker
first line{{newline}}second line
```

```text title=Output
first line
second line
```

</Example>

<Example title="Example with tilde stripping">

When using tildes to build single-line output, `{{newline}}` can reintroduce line breaks where needed.

```whisker title=example.whisker
{{~ #if has_doc ~}}
  {{~ doc ~}}
  {{~ newline ~}}
{{~ /if ~}}
void {{name}}();
```

```json title=Context
{
  "has_doc": true,
  "doc": "// Does important things.",
  "name": "process"
}
```

```text title=Output
// Does important things.
void process();
```

Without `{{newline}}`, the tildes would collapse everything into `// Does important things.void process();`. Here, `{{newline}}` inserts an explicit line break between the doc comment and the declaration.

</Example>

### Boolean logic

#### `and`
The logical and operation for 2 or more expressions. Returns `true` if all
argument expressions evaluate to `true`, otherwise returns `false`.

:::note
This function is *short-circuiting* - if an argument is `false`, any following
expressions are not evaluated.
:::

**Positional Arguments**:
- `a` (expression) — the first operand
- `b` (expression) — the second operand
- `expression...` — additional operands

<Example>

```whisker title=example.whisker
{{ (and var1 var2 var3) }}
```

```json title=Context
{
  "var1": true,
  "var2": false,
  "var3": true
}
```

```text title=Output
false
```

</Example>

---

#### `if`
The ternary conditional operation. Evaluates the first argument expression. If it
evaluates to `true`, then the second argument expression is evaluated and returned.
If the first argument expression evaluates to `false`, then the third argument
expression is evaluated and returned.

:::note
This function is *short-circuiting* - the branch not taken is not evaluated.
:::

**Positional Arguments**:
- `condition` (expression) — the condition
- `true_value` (expression) — the expression to evaluate and return if `condition` is `true`
- `false_value` (expression) — the expression to evaluate and return if `condition` is `false`

<Example>

```whisker title=example.whisker
{{ (if cond "Cond was true" false_var) }}
```

```json title=Context
{ "cond": true }
```

```text title=Output
Cond was true
```

Notice that it was valid to evaluate this template without `false_var` being
defined, due to the short-circuiting behavior of `if`.

</Example>

---

#### `not`
The logical not operation. Returns `true` if the argument expression evaluates
to `false`, otherwise returns `false`.

**Positional Arguments**:
- `expression` — the operand

<Example>

```whisker title=example.whisker
{{ (not var) }}
```

```json title=Context
{ "var": true }
```

```text title=Output
false
```

</Example>

---

#### `or`
The logical or operation for 2 or more expressions. Returns `true` if any of
the arguments are `true`, otherwise returns `false`.

:::note
This function is *short-circuiting* - if an argument is `true`, any following
expressions are not evaluated.
:::

**Positional Arguments**:
- `expression` — the first operand
- `expression` — the second operand
- `expression...` — additional operands

<Example>

```whisker title=example.whisker
{{ (or var1 var2 var3) }}
```

```json title=Context
{
  "var1": false,
  "var2": true,
  "var3": false
}
```

```text title=Output
true
```

</Example>

### Array

---

#### `array.at`
Gets the object from an array at a given index. If the index is negative, or
larger than the size of the array, then an error is thrown.

**Positional Arguments**:
- `array` — the array
- `index` (i64) — the index

<Example>

```whisker title=example.whisker
{{ (array.at my_array 0) }}
```

```json title=Context
{ "my_array": ["foo", "bar", "baz"] }
```

```text title=Output
foo
```

</Example>

---

#### `array.empty?`
Checks an array for emptiness. Returns `true` if the array is empty, otherwise
returns `false`.

**Positional Arguments**:
- `array` — the array

<Example>

```whisker title=example.whisker
{{ (array.empty? my_array) }}
```

```json title=Context
{ "my_array": [] }
```

```text title=Output
true
```

</Example>

---

#### `array.enumerate`
Returns a view of the provided array's items paired with their index. The output
is an array where each element is a tuple of `(index, item)`.

The order of items produced in the output array matches the input array.

:::note
The enumerated tuple may have the following forms, based on the named arguments
`with_first` and `with_last`:
- `(index, item)` (with arguments `with_first=false`, `with_last=false`)
- `(index, item, first?)` (with arguments `with_first=true`, `with_last=false`)
- `(index, item, last?)` (with arguments `with_first=false`, `with_last=true`)
- `(index, item, first?, last?)` (with arguments `with_first=true`, `with_last=true`)
:::

**Positional Arguments**:
- `array` — the array to enumerate

**Named Arguments**:
- `with_first` (boolean) — if `true`, the 3rd element of each tuple is set to the
  value of `index == 0`
- `with_last` (boolean) — if `true`, the last element of each tuple is set to the
  value of `index == <size of array> - 1`. If `with_first` is `true`, then this is
  the 4th element. Otherwise, it is the 3rd element.

<Example>

```whisker title=example.whisker
{{#each (array.enumerate my_array with_first=true) as |index item first?|}}
  {{index}}: {{item}}{{#if first?}} (first){{/if}}
{{/each}}
```

```json title=Context
{
  "my_array": ["foo", "bar", "baz"]
}
```

```text title=Output
0: foo (first)
1: bar
2: baz
```

</Example>

---

#### `array.len`
Produces the length of an array.

**Positional Arguments**:
- `array` - The array to find length of.

<Example>

```whisker title=example.whisker
{{ (array.len my_array) }}
```

```json title=Context
{
  "my_array": ["foo", "bar", "baz"]
}
```

```text title=Output
3
```

</Example>

---

#### `array.of`
Creates an array with the provided arguments in order. This function can be used
to form an "array literal".

**Positional Arguments**:
- `object...` — the items to include in the array, in the desired order.

<Example>

```whisker title=example.whisker
{{# each (array.of 0 1 "foo" false) as |item| }}
  {{item}}
{{/ each }}
```

```text title=Output
  0
  1
  foo
  false
```

</Example>

### Int

#### `int.add`
Returns the sum of the provided arguments.

**Positional Arguments**:
- `i64...` — the integers to add

<Example>

```whisker title=example.whisker
{{ (int.add 1 2 3) }}
```

```text title=Output
6
```

</Example>

---

#### `int.eq?`
Checks two i64s for equality.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.eq? 1 1) }}
```

```text title=Output
true
```

</Example>

---

#### `int.ge?`
Checks if one i64 is greater or equal to than another.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.ge? my_int 42) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
false
```

</Example>

---

#### `int.gt?`
Checks if one i64 is greater than another.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.gt? my_int 42) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
false
```

</Example>

---

#### `int.le?`
Checks if one i64 is less or equal to than another.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.le? my_int 42) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
true
```

</Example>

---

#### `int.lt?`
Checks if one i64 is less than another.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.lt? my_int 42) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
true
```

</Example>

---

#### `int.ne?`
Checks two i64s for inequality.

**Positional Arguments**:
- `a` (i64) — the left-hand side of the comparison
- `b` (i64) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (int.ne? my_int 42) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
true
```

</Example>

---

#### `int.neg`
Negates the provided i64.

**Positional Arguments**:
- `i64` — the integer to negate

<Example>

```whisker title=example.whisker
{{ (int.neg -1) }}
```

```text title=Output
1
```

</Example>

---

#### `int.sub`
Subtracts one i64 from another.

**Positional Arguments**:
- `a` (i64) — the number to subtract from
- `b` (i64) — the number to subtract

<Example>

```whisker title=example.whisker
{{ (int.sub my_int 1) }}
```

```json title=Context
{ "my_int": 12 }
```

```text title=Output
11
```

</Example>

### Map

#### `map.items`
Returns a view of the provided map's items. The output is an array where each
element is a map with "key" and "value" properties.

This function fails if the provided map is not enumerable.

The order of items in the produced array matches the enumeration above. For
whisker::map, properties are sorted lexicographically by name.

**Positional Arguments**:
- `map` — The map to enumerate

<Example>

```whisker title=example.whisker
{{#each (map.items my_map) as |item|}}
  {{item.key}}: {{item.value}}
{{/each}}
```

```json title=Context
{
  "my_map": {
    "key1": "value1",
    "key2": "value2"
  }
}
```

```text title=Output
  key1: value1
  key2: value2
```

</Example>

---

#### `map.has_key?`
Determines if the provided map contains a given key.

**Positional Arguments**:
- `map` — The map to check for key
- `key` (string) — The key to check in the map

<Example>

```whisker title=example.whisker
{{ (map.has_key? my_map "key") }}
```

```json title=Context
{
  "my_map": {
    "key": "value"
  }
}
```

```text title=Output
true
```

</Example>

### Object

#### `object.eq?`
Checks two objects for equality. Returns `true` if the objects are equal, otherwise
returns `false`.

:::note
Value equality between two (unordered) pair of objects is defined as follows:

| Left Type | Right Type | Equality      |
| --------- | ---------- | ------------- |
| `null`    | `null`     | Equal         |
| `i64`     | `i64`      | If same value |
| `f64`     | `f64`      | If same value |
| `string`  | `string`   | If same value |
| `boolean` | `boolean`  | If same value |
| `array`   | `array`    | If all corresponding elements are equal (recursive) |
| `map`     | `map`      | If all both maps have enumerable property keys and
                           each key-value pairs are equal between the two maps (recursive) |
| `native_function` | `native_function` | If same pointer to function |
| `native_handle` | `native_handle` | If same pointer to handle |

For any pairing of objects not listed above, the objects are **NOT** equal.
:::

**Positional Arguments**:
- `a` (object) — the left-hand side of the comparison
- `b` (object) — the right-hand side of the comparison

<Example>

```whisker title=example.whisker
{{ (object.eq? my_obj my_other_obj) }}
```

```json title=Context
{
  "my_obj": ["foo"],
  "my_other_obj": ["foo"]
}
```

```text title=Output
true
```

</Example>

---

#### `object.is?`
Check if an object is of a certain type. Returns `true` if the object is of the
type provided in the `type` named argument.

**Positional Arguments**:
- `object` — the object to check

**Named Arguments**:
- `type` (string) — the type to check for. The value must be one of the following:
  - `null`
  - `i64`
  - `f64`
  - `string`
  - `boolean`
  - `array`
  - `map`
  - `native_function`
  - `native_handle`

<Example>

```whisker title=example.whisker
{{ (object.is? my_i64 type="string") }}
{{ (object.is? my_i64 type="i64") }}
```

```json title=Context
{ "my_i64": 3 }
```

```text title=Output
false
true
```

</Example>

---

#### `object.notnull?`
Checks if an object is not null. Returns `true` if the object is not null, otherwise
returns `false`.

**Positional Arguments**:
- `object` — the object to check

<Example>

```whisker title=example.whisker
{{ (object.notnull? my_obj) }}
```

```json title=Context
{ "my_obj": null }
```

```text title=Output
false
```

</Example>

### String

#### `string.concat`
Concatenates the provided strings.

**Positional Arguments**:
- `string...` — the strings to concatenate

<Example>

```whisker title=example.whisker
{{ (string.concat "foo" "bar" "baz") }}
```

```text title=Output
foobarbaz
```

</Example>

---

#### `string.empty?`
Checks a string for emptiness. Returns `true` if the string is empty, otherwise
returns `false`.

**Positional Arguments**:
- `string` — the string to check

<Example>

```whisker title=example.whisker
{{ (string.empty? my_string) }}
```

```json title=Context
{ "my_string": "foo" }
```

```text title=Output
false
```

</Example>

---

#### `string.len`
Produces the length of a string in bytes (not Unicode codepoints).

**Positional Arguments**:
- `string` - The string to find length of.

<Example>

```whisker title=example.whisker
{{ (string.len my_string) }}
```

```json title=Context
{ "my_string": "foo" }
```

```text title=Output
3
```

</Example>
