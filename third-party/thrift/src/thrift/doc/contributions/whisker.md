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

Whisker is a **templating language** with a canonical implementation in C++. It is heavily inspired by [Mustache](https://mustache.github.io/mustache.5.html), [Handlebars](https://handlebarsjs.com/), and [EmberJS templates](https://emberjs.com/) (each of which is an extension of the previous).

Within Thrift, Whisker is heavily used within the compiler's backend infrastructure for generating code in many target languages, including C++, Python, Rust, and Go.

Whisker was built to replace the open-source library, [`mstch`](https://github.com/no1msd/mstch), which did not scale to meet the needs of Thrift.

## Basics

The core capability of Whisker can be expressed as one function:
```cpp
std::string render(
  std::string template_text,
  context variables
);
```

A typical Whisker template might be:
```handlebars
Hello {{name}}
You have just won {{value}} dollars!
{{#if in_ca}}
Well, {{taxed_value}} dollars, after taxes.
{{/if}}
```

Given the following input:
```json title=Context
{
  "name": "Chris",
  "value": 10000,
  "taxed_value": 6000,
  "in_ca": true
}
```

Will produce the following text:
```
Hello Chris,
You have just won 10000 dollars!
Well, 6000 dollars, after taxes.
```

Whisker's templating syntax is a mix of raw text and *templates* (denoted by `{{` and `}}`). The formal grammar is covered below.

## Grammar Syntax

The grammar of the templating language is described with [PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar)-like production rules, closely resembling the syntax of [Pest](https://pest.rs/).

Production rules are of the form:
```
rule → { <term> <op> <term> ... }
```

`<term>` represents other rules. `<op>` represents a combinator of rules.
The supported set of combinators are:

* `a ~ b` — exactly one a followed by exactly one b.
* `a*` — zero or more repetitions of a.
* `a+` — one or more repetitions of a.
* `a?` — exactly zero or one repetition of a.
* `!a` — assert that there is no match to a (without consuming input).
* `a | b` — exactly one a or one b (a matches first)
* `(a <op> b)` — parentheses to disambiguate groups of rules and combinators.

## Text & Template

A Whisker source file is a repeated pattern of *textual content* and *template* placeholders.

`text` is a sequence of unicode characters except `newline`s and `{{`, which denotes the opening of a `template` (or `comment`).

`newline` is either `"\n"`, `"\r"` or `"\r\n"`.

:::note
To interpret `{{` as `text`, the escape sequence, `\{{`, can be used.
:::

<Grammar>

```
root → { body* }
body → { text | newline | template | comment }

template → { <see below> }
comment  → { <see below> }
```

</Grammar>

## Comments

Comments in Whisker are omitted from the produced output.

There are two types of comments:
```handlebars
{{! This is a comment }}
{{!-- This is a comment that is allowed to contain "}}" --}}
```

<Grammar>

```
comment → { basic-comment | escaped-comment }

basic-comment   → { "{{!" ~ <unicode text until we see "}}"> ~ "}}" }
escaped-comment → { "{{!--" ~ <raw text until we see "--}}"> ~ "--}}" }
```

</Grammar>

## Templates

A `template` is the basic building block for expressing variability in the produced output of a Whisker template.

There are four categories of `template`s:
* ***Expressions*** — `{{foo}}` — *interpolate* a value from the *context* into the rendered output.
* ***Blocks*** — `{{#foo}} ... {{/foo}}` — *control flow* capabilities like conditionals (`{{#if}}`), or loops (`{{#each}}`) etc.
  * All blocks open with an `{{# ...}}` tag.
  * All blocks close with a corresponding `{{/ ...}}` tag.
  * All blocks contain zero or more `body` elements inside them.
* ***Statements*** — `{{#foo}}` — language features that are not rendered into the output like name bindings (`{{#let}}`), `{{#else}}` etc.
  * All statements are of the form `{{# ...}}`. Unlike blocks, there is no closing tag.
* [***Partial Application***](#partial-applications) — `{{> foo}}` — subtitute a different Whisker source body into the output.

<Grammar>

```
template → { ("{{" ~ expression ~ "}}") | block | statement | partial-apply }

expression    → { <see below> }
block         → { <see below> }
statement     → { <see below> }
partial-apply → { <see below> }
```

</Grammar>

### Expressions

An `expression` represents an *interpolation*. That is, the result of an `expression` is rendered into the output at the position of the enclosing `{{ ... }}`.

There are three types of `expression`s in Whisker:
* ***Literal*** — values "hard-coded" into the template, like the string literal, `{{ "hello" }}`.
  * `string-literal` — a "C-style" string literal, which is wrapped by `"..."`. A subset of [escape sequences from C](https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences) are supported: `\n`, `\r`, `\t`, `\\`, `\'`, and `\"`. Examples:
    * `"hello\nworld"`.
    * `"\"quoted\""`
  * `i64-literal` —  a "C-style" *decimal* integer literal, possibly with a leading `-` for non-positive numbers. The value must be representable by a 64-bit two's complement signed integer: <code>-2<sup>63</sup> ≤ value < 2<sup>63</sup></code>. Examples:
    * `-742`
    * `0`
    * `124`
  * `boolean-literal` — *exactly* `true` or `false`.
  * `null-literal` — *exactly* `null`.
* ***Variable*** — values to be interpolated from the provided *context*, like `{{ person.name }}`. Such an interpolation is a series of *identifiers* delimited with `.`, representing scoped access into the *context*.
  * In addition, the special *implicit context* variable, `{{ . }}`, can be used to refer to the *context* object itself.
* ***Function call*** — A [lisp-like](https://en.wikipedia.org/wiki/Lisp_(programming_language)#Syntax_and_semantics) function invocation syntax, often also called [S-expressions](https://en.wikipedia.org/wiki/S-expression). The section on the [*data model*](#data-model) describes the semantics of a function call. Examples:
  * `{{ (uppercase "hello") }}`
  * `{{ (concat (uppercase person.firstName) " " (uppercase person.lastName)) }}`

:::warning
Function call expressions have not been implemented yet.
:::

:::note
Whisker includes a set of *keywords* that are reserved. These cannot be used as identifiers. See the grammar below.
:::

Every `expression` produces an `object`. However, not all `object`s are *printable*. The top-level `expression` in a `template` **must** be *printable*. `expression`s passed as arguments in a function call do **not** need be *printable*. The section on the [*data model*](#data-model) elaborates further.

<Grammar>

```
expression → { literal | variable | function-call }

literal             → { string-literal | i64-literal | boolean-literal | null-literal }
variable            → { "." | (identifier ~ ("." ~ identifier)*) }
function-call       → { "(" ~ function-lookup ~ expression* ~ ")" }

string-literal  → { <see above> }
i64-literal     → { <see above> }
boolean-literal → { "true" | "false" }
null-literal    → { "null" }

identifier → { !keyword ~ (id_prefix ~ id_suffix*) }
keyword → {
  "true"    |
  "false"   |
  "null"    |
  "if"      |
  "unless"  |
  "else"    |
  "each"    |
  "as"      |
  "partial" |
  "let"     |
  "and"     |
  "or"      |
  "not"     |
  "with"    |
  "this"    |
  "define"  |
  "for"     |
  "do"      |
  "import"  |
  "export"  |
  "from"    |
}
id_prefix → { alpha |        | '_' | '$' }
id_suffix → { alpha | digits | '_' | '$' | '-' | '+' | ':' | '?' | '/' }

function-lookup → { variable | "and" | "or" | "not" }
```

</Grammar>

### If Blocks

Whisker supports a conditional block type: `{{#if}}`. A typical conditional block might be:

```handlebars
{{#if person.hasName}}
Greetings, {{person.name}}!
{{#else}}
I don't know who you are.
{{/if}}
```

In this example, `person.hasName` is the *condition*. The condition **must** be an `expression` that evaluates to a `boolean`. If the value is `true`, then the conditional `body` is rendered.

`{{#if}}` blocks may **optionally** have one `{{#else}}` statement.
If the `expression` value is `true`, then the `body` preceding the `{{#else}}` is rendered.
If the `expression` value is `false`, then the `body` following the `{{#else}}` is rendered.

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

:::note
`{{#unless}}` blocks have the same behavior as `{{#if}}` except that its *condition* is negated. This block is **deprecated** since the behavior can be replicated with `{{#if}}` in conjunction with the `not` function.
:::

<Example title={<>Example of <code>unless</code> being redundant</>}>

```handlebars title=example.whisker
{{! The following blocks have the same behavior }}

{{#unless failed?}}
Nice!
{{/unless}}

{{#if (not failed?)}}
Nice!
{{/if}}
```

</Example>

<Grammar>

```
if-block       → { if-block-open ~ body* ~ else-block? ~ if-block-close }
if-block-open  → { "{{" ~ "#" ~ "if" ~ expression ~ "}}" }
else-block     → { "{{" ~ "else" ~ "}}" ~ body* }
if-block-close → { "{{" ~ "/" ~ "if" ~ "}}" }
```

</Grammar>

Whisker `{{#if}}` blocks are based on [EmberJS `{{#if}}`](https://guides.emberjs.com/release/components/conditional-content/) and [Handlebars `{{#if}}`](https://handlebarsjs.com/guide/builtin-helpers.html#if).

### Each Blocks

:::warning
`{{#each}}` blocks have not been implemented yet.
:::

Whisker supports a block type for repetition: `{{#each}}`. A typical repetition block might be:

```handlebars
Rankings are:
{{#each winners as |winner index|}}
{{(add index 1)}}. {{winner}}
{{/each}}
```

The `expression` being repeated upon (`winners`) **must** evaluate to an `array`, or a `native_object` that supports `array`-like iteration.

The `body` will be rendered once for each element of the `array`. The first capture name (`winner`) will be [locally bound](#scopes) to the current `object`. The second capture name (`index`) will be [locally bound](#scopes) to the index of the current `object` (as an `i64`).

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

The `index` argument in the `as` *captures* is **optional**.

<Example title="Example without index">

```handlebars
Rankings are:
{{#each winners as |winner|}}
{{winner}}
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

The entire `as` *captures* are **optional**, in which case, the *implicit context* (`{{ . }}`) can be used instead.

<Example title="Example with implicit context">

```handlebars title=example.whisker
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

`{{#each}}` blocks may **optionally** have an `{{#else}}` statement. The body following the `{{#else}}` is rendered only if there were zero repetitions performed.

<Example title={<>Example with <code>else</code></>}>

```handlebars title=example.whisker
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
each-block-capture → { "as" ~ "|" ~ identifier ~ identifier? ~ "|" }
else-block         → { "{{" ~ "else" ~ "}}" ~ body* }
each-block-close   → { "{{" ~ "/" ~ "each" ~ "}}"  }
```

</Grammar>

Whisker `{{#each}}` blocks are based on [EmberJS `{{#each}}`](https://guides.emberjs.com/release/components/looping-through-lists/) and [Handlebars `{{#each}}`](https://handlebarsjs.com/guide/builtin-helpers.html#each).


### With Blocks

:::warning
`{{#with}}` blocks have not been implemented yet.
:::

Whisker supports a block type for de-structuring: `{{#with}}`. A typical de-structuring block might be:

```handlebars
{{#with person}}
The name's {{lastName}}... {{firstName}} {{lastName}}.
{{/with}}
```

The `expression` being de-structured (in this case, `person`) **must** evaluate to a `map`, or a `native_object` that supports `map`-like property access.

The `body` will be rendered exactly once. The result of the expression (`person`) will become the [*current* evaluation scope](#evaluation-context) within the block. This means that all its properties can be accessed without the `person.` prefix.

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

:::warning
`{{#let}}` statements have not been implemented yet.
:::

Whisker `{{#let}}` statements allow binding the result of an `expression` to an *identifier* in the [*current* lexical scope](#scopes) of the [evaluation context](#evaluation-context). A simple `{{#let}}` statement might be:

```handlebars
{{#let result = (add input 1)}}
{{result}}
```

The `expression` is eagerly evaluated exactly once and the name becomes accessible in the lexical scope of the `{{#let}}` statements.

The primary purpose of `{{#let}}` statements is to simply complex `expression`s by breaking them down into smaller parts.

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

```handlebars title=example.whisker
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

```handlebars title=example.whisker
{{#let x = (add 5 4)}}
{{#let y = (add x 1)}}
{{y}}
```

```text title=Output
10
```

</Example>

<Grammar>

```
let → { "{{" ~ "#" ~ "let" ~ identifier ~ "=" ~ expression ~ "}}" }
```

</Grammar>

### Partial Blocks

:::warning
`{{#partial}}` blocks have not been implemented yet.
:::

Partial blocks allow defining reusable templates within a Whisker template itself. A simple example of a `{{#partial}}` block might be:

```handlebars
{{#partial greeting as |person|}}
Greetings, {{person.firstName}} {{person.lastName}}!
{{/partial}}

{{! example partial application }}
{{> greeting person=person}}
```

Partial blocks must be applied with [partial applications](#partial-applications). See below.

<Grammar>

```
partial-block         → { partial-block-open ~ body* ~ partial-block-close }
partial-block-open    → { "{{#" ~ "partial" ~ path-component ~ routine-capture? ~ "}}" }
partial-block-capture → { "as" ~ "|" ~ identifier+ ~ "|" }
partial-block-close   → { "{{/" ~ "partial" ~ "}}" }

path-component → { <see below> }
```

</Grammar>

Whisker `{{#partial}}` blocks are based on [Handlebars partial parameters](https://handlebarsjs.com/guide/partials.html#partial-parameters).

### Partial Applications

Partials are `template`s that are not immediately applied. They have to be *applied* by name within Whisker template source. Partials allow reusing templates. A simple example of partial application might be:

```handlebars
{{> path/to/my-partial}}
```

:::note
Currently, partials cannot be defined in Whisker — they must be provided by the runtime environment (e.g. C++).
This will change once [`{{#partial}}` blocks](#partial-blocks) are implemented.
:::

Partial applications (without captures) assume the [evaluation context](#scopes) at the site of application.

<Example title="Example with implied context (macro)">

```handlebars
{{#partial greeting as |person|}}
Greetings, {{person.firstName}} {{person.lastName}}!
{{/partial}}

{{> greeting}}
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

[Partial applications (with captures)](#partial-blocks) have names (provided via `as`) [bound](#scopes) to `expression`s provided during application.

The contained `body` is rendered with a [derived evaluation context](#derived-evaluation-context).
That is, names accessible from the site of the application are **not** *implicitly* available within the block.

<Example>

```handlebars
{{#partial greeting as |person|}}
Greetings, {{person.firstName}} {{person.lastName}}!
{{/partial}}

{{> greeting person=dave}}
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

Partial applications retain the *preceding indentation* at the site of the application. Every line of the partial being applied is indented by this same amount.

<Example title="Example with indentation">

```handlebars title=example.whisker
Some historic presidents are:
{{#each presidents as person}}
  {{> common/president}}
{{/each}}
```

```handlebars title=common/president.whisker
{{person.lastName}}
  {{person.firstName}}
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

<Grammar>

```
partial-apply    → { "{{" ~ ">" ~ partial-lookup ~ partial-argument* ~ "}}" }
partial-lookup   → { path-component ~ ("/" ~ path-component)* }
partial-argument → { identifier ~ "=" ~ expression }

path-component → { <see below> }
```

A `path-component` must satisfy [POSIX's portable file name character set](https://www.ibm.com/docs/en/zvm/7.3?topic=files-naming). A valid path, therefore, will pass the GNU coreutils [`pathchk`](https://github.com/coreutils/coreutils/blob/v9.5/src/pathchk.c#L157-L202) [command](https://man7.org/linux/man-pages/man1/pathchk.1.html) (minus
length restrictions).

</Grammar>

## Data Model

The *context* provided during when rendering a template follows Whisker's *data model*. Whisker's type system is heavily influenced by JSON. The root of the type system is `object` — all types are subtypes of `object`.

`object` is a union of the following types:
* `i64` — 64-bit two's complement signed integer (<code>-2<sup>63</sup> ≤ value < 2<sup>63</sup></code>).
* `f64` — [IEEE 754 `binary64` floating point number](https://en.wikipedia.org/wiki/Double-precision_floating-point_format#IEEE_754_double-precision_binary_floating-point_format:_binary64).
* `string` — sequence of Unicode characters.
* `boolean` — 1-bit `true` or `false`.
* `null` — marker indicating ["no value"](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/null).
* `array` — An ordered list of `object` (note the recursion). An `array` can be traversed using [`{{#each}}`](#each-blocks).
* `map` — An unordered set of key-value pairs. Keys are valid *identifiers* represented as `string`s. Values are any `object` (note the recursion). A `map` can be *unpacked* using [`{{#with}}`](#with-blocks) or accessed through [variable interpolation](#expressions).
* `native_object` — An implementation-defined type that can behave like an `array` or `map`. A value of this type can only be created by the native runtime (e.g. C++).
* `native_function` — An implementation-defined type that can be invoked in [Whisker templates](#expressions). A value of this type can only be created by the native runtime (e.g. C++).

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

```handlebars
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

A *lexical scope* has two properties:
  * an **implicit context** [`object`](#data-model).
  * a **local bindings** [`map`](#data-model).

An evaluation context has three properties:
  * a **stack of *lexical scopes***.
  * a **global scope** [`map`](#data-model).

The *global scope* is always the bottom of the stack. It cannot be popped.

Whisker's uses scopes to describe how top-level names are resolved. It provides an answer to the question: what is `foo` in `{{foo.bar.baz}}`? `bar` and `baz` are *property* resolutions which are simple lookups into a `map` or `map`-like `native_object`.

Given an *identifier*, `name`:
1. Start with the *current* scope (top of stack).
2. Look in the **local bindings** `map` for `name`.
    * If an entry is found, return the corresponding `object`.
3. Look in the **implicit context** `object`.
    * If a property is found, return the corresponding `object`.
4. Repeat (2) with the previous scope (from the stack) if possible.
    * If we are at the bottom of the stack, return an error.

Local bindings are introduced into the current scope with [`{{#let}}`](#let-statements) statements, `as` captures in [`{{#each}}`](#each-blocks) or [`{{#partial}}`](#partial-blocks) blocks etc.

### Derived Evaluation Context

`{{#partial}}` blocks with `as` captures are rendered within an evaluation context *derived* from the the call site. This is a new evaluation context with an empty stack, except that it shares the same global scope `map`.

## Standalone Tags

:::note
This section is incomplete.
:::
