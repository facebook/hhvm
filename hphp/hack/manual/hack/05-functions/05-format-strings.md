The Hack typechecker checks that format strings are being used correctly.

## Quickstart

```Hack error
// Correct.
Str\format("First: %d, second: %s", 1, "foo");

// Typechecker error: Too few arguments for format string.
Str\format("First: %d, second: %s", 1);
```

This requires that the format string argument is a string literal, not a variable.

```Hack error
$string = "Number is: %d";

// Typechecker error: Only string literals are allowed here.
Str\format($string, 1);
```

## Defining Functions with Format Strings
You can define your own functions with format string arguments too.

```Hack
function takes_format_string(
  \HH\FormatString<\PlainSprintf> $format,
  mixed ...$args
): void {}

function use_it(): void {
  takes_format_string("First: %d, second: %s", 1, "foo");
}
```

`HH\FormatString<PlainSprintf>` will check that you've used the right
number of arguments. `HH\FormatString<Str\SprintfFormat>` will also
check that arguments match the type in the format string.

## Format Specifiers
See `PlainSprintf` for in-depth information on all format specifiers.

| Specifier | Expected Input | Expected Output                                                                                                     |
|-----------|----------------|---------------------------------------------------------------------------------------------------------------------|
| b         | int            | Binary (`63` => `111111`)                                                                                           |
| c         | int            | ASCII character (`63` => `?`)                                                                                       |
| d         | int            | Signed int (`-1` => `-1`)                                                                                           |
| u         | int            | Unsigned int (`-1` => `18446744073709551615`)                                                                       |
| e         | float          | Scientific Notation, as lowercase (`0.34` => `3.4e`)                                                                |
| E         | float          | Scientific Notation, as uppercase (`0.34` => `3.4E`)                                                                |
| f         | float          | Locale Floating Point Number                                                                                        |
| F         | float          | Non-Locale Floating Point Number                                                                                    |
| g         | float          | Locale Floating Point Number or Scientific Notation (`3.141592653` => `3.14159`, `3141592653` => `3.1415e+9`).      |
| o         | int            | Octal (`63` => `77`)                                                                                                |
| s         | string         | string                                                                                                              |
| x         | int            | Hexadecimal, as lowercase (`63` => `3f`)                                                                            |
| X         | int            | Hexadecimal, as uppercase (`63` => `3F`)                                                                            |
