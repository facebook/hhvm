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
  * [Whitespace-only `text` (including `newline`)](#text).
  * [Import statements](#import-statements)

Whisker ignores whitespaces in the `header` and does not render them.

<Grammar>

```
root → { (whitespace* ~ header)* ~ body* }

header     → { comment | pragma-statement | import-statement }

body     → { text | template }
text     → { <see below> }
template → { <see below> }
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
* ***Variable*** — values interpolated from the *context*, like `{{ person.name }}`. Variables allow scoped access into the context using a series of *identifiers* separated by dots (`.`).
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
variable            → { "." | "this" | (identifier ~ ("." ~ identifier)*) }
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
id_suffix → { alpha | digits | '_' | '$' | '-' | '+' | ':' | '?' | '/' }

builtin-call → {
  ("not" ~ expression) |
  (("and" | "or") ~ expression ~ expression+) }

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

<Grammar>

```
each-block         → { each-block-open ~ body* ~ else-block ~ each-block-close }
each-block-open    → { "{{" ~ "#" ~ "each" ~ expression ~ each-block-capture? ~ "}}" }
each-block-capture → { "as" ~ "|" ~ identifier+ ~ "|" }
else-block         → { "{{" ~ "#" ~ "else" ~ "}}" ~ body* }
each-block-close   → { "{{" ~ "/" ~ "each" ~ "}}"  }
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
pragma-statement → { "{{" ~ "#" ~ "pragma" ~ ( "single-line" ) ~ "}}" }
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
{{#each presidents as person}}
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
partial-block-captures  → { "as" ~ "|" ~ identifier+ ~ "|" }
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
{{else}}
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
* `map` — An unordered set of key-value pairs. Keys are valid *identifiers* represented as `string`s. Values are any `object` (note the recursion). A `map` can be *unpacked* using [`{{#with}}`](#with-blocks) or accessed through [variable interpolation](#expressions).
* `native_object` — An implementation-defined type that can behave like an `array` or `map`. A value of this type can only be created by the native runtime (e.g. C++).
* `native_function` — An implementation-defined type that can be invoked in [Whisker templates](#expressions). A value of this type can only be created by the native runtime (e.g. C++).
* `native_handle` — An opaque reference to an implementation-defined type that Whisker cannot directly interact with. A value of this type is meaningful only to the native runtime (e.g. C++) in conjunction with `native_function`.

The following types are *printable*: `i64`, `string`.

### Native Object

:::note
This section is incomplete.
:::

A `native_object` is transparent to Whisker templates. A `native_object` *may* allow property access via variable interpolation, or [`{{#with}}`](#with-blocks). A `native_object` *may* allow iteration via [`{{#each}}`](#each-blocks).

`native_object` allows the native runtime (e.g. C++) to bridge types and data to Whisker templates.

### Native Function

:::note
This section is incomplete.
:::

A `native_function` represents a native runtime (e.g. C++) function implementation that is callable from Whisker templates via [function call expressions](#expressions).

A `native_function` accepts arguments of type `object` with arbitrary arity, and produces exactly one value of type `object`.

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
