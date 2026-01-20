# Comments

:::note
Hash comments (`#`) are no longer supported, and are a parse error since [HHVM 4.133](https://hhvm.com/blog/2021/10/26/hhvm-4.133.html).
:::

Hack has three comment syntaxes.

```hack
// A single line comment.

/* A multi line comment.
 *
 */

/**
 * A doc comment starts with two asterisks.
 *
 * It summarizes the purpose of a definition, such as a
 * function, class or method.
 */
function foo(): void {}
```

Multi-line comments start with `/*` and end with `*/`. Comments starting `/**`
are also used for documentation.

Single-line comments start with `//` and end with a newline.

`#` is not a valid comment character, as it is used to represent an
[Enum Class Label](/hack/built-in-types/enum-class-label).

A number of special comments are recognized; they are:

- `// FALLTHROUGH` in [switch statements](/hack/statements/switch)
- `// strict` and `// partial` in
  [headers](/hack/source-code-fundamentals/program-structure)
- `/* HH_FIXME[1234] */` or `/* HH_IGNORE_ERROR[1234] */`, which suppresses
  typechecker error reporting for error 1234.
- `/* HH_FIXME[12001] */`, which suppresses warning for code 12001
