# String

A `string` is a sequence of *bytes* - they are not required to be valid characters in any particular encoding,
for example, they may contain null bytes, or invalid UTF-8 sequences.

## Basic operations

Concatenation and byte indexing are built-in operations; for example:

- `"foo"."bar"` results in `"foobar"`
- `"abc"[1]` is `"b"`
- if the source code is UTF-8, `"a😀c"[1]` is byte `0xf0`, the first of the 4 bytes compromising the "😀" emoji in UTF-8

Other operations are supported by the `Str\` namespace in the [Hack Standard Library](/hsl/overview), such as:

- `Str\length("foo")` is 3
- `Str\length("foo\0")` is 4
- `Str\length("a😀c")` is 6
- `Str\join(vec['foo', 'bar', 'baz'], '!')` is `"foo!bar!baz"`

## Converting to numbers

Use `Str\to_int()` to convert strings to integers; this will raise errors if the input contains additional data, such as `.0` or other trailing characters.

`(int)` and `(float)` are more permissive, but have undefined behavior for inputs containing similar trailing data.

## Bytes vs characters

Functions in the `Str\` namespace with an `_l` suffix such as `Str\length_l()` take a `Locale\Locale` object
as the first parameter, which represents the language, region/country, and encoding (e.g. `en_US.UTF-8`); if a
multibyte encoding such as UTF-8 is specified, the `_l()` functions operate on characters instead of bytes. For
example:

- `Str\length("a😀c")` is 6
- `Str\length_l(Locale\bytes(), "a😀c")` is 6
- `Str\length_l(Locale\create("en_US.UTF-8"), "a😀c")` is 3
- `Str\slice("a😀c", 2)` is `"a\xf0"` (2 bytes)
- `Str\slice_l(Locale\create("en_US.UTF-8"), "a😀c", 2)` is `"a😀"` (5 bytes)

In some encodings, the same character can be represented in multiple different ways, with multiple byte sequences;
the `_l` functions will treat them as equivalent. For example, the letter `é` can be represented by either:

- `"\u{00e9}"`, or "\xc3\xa9" ("LATIN SMALL LETTER E ACUTE")
- `"\u{0065}\u{0301}"`, or `\x65\xcc\x81" ("LATIN SMALL LETTER E", followed by "COMBINING ACUTE ACCENT")

This means that various comparison functions may report strings as equivalent, despite containing different
byte sequences - so if the result of a character-based operation is used for another function, that function
should also be character-based:

```hack
use namespace HH\Lib\{Locale,Str};

<<__EntryPoint>>
function main(): void {
  $haystack = "abc\u{00e9}"; // ends with UTF-8 "LATIN SMALL LETTER E ACUTE"
  $needle = "\u{0065}\u{0301}"; // "LATIN SMALL LETTER E", "COMBINING ACUTE ACCENT"
  $locale = Locale\create("en_US.UTF-8");
  \var_dump(dict[
    'Byte test' => Str\ends_with($haystack, $needle), // false
    'Character test' => Str\ends_with_l($locale, $haystack, $needle), // true
    'Strip byte suffix' => Str\strip_suffix($haystack, $needle), // no change
    'Strip character suffix' => Str\strip_suffix_l($locale, $haystack, $needle), // removed
  ]);
}
```

## Default encoding

If a locale is specified without an encoding (e.g. `en_US`, compared to `en_US.UTF-8` or `en_US.ISO8859-1`), the system behavior will be followed. For example:
- on most Linux systems, `en_US` uses a single-byte-per-character-encoding
- on MacOS, `en_US` uses UTF-8

## The `Locale\bytes()` locale

This locale is similar to the constant `"C"` or `"en_US_POSIX"` locale in other libraries and environments; we refer to it as the `bytes` locale to more clearly distinguish between this constant locale and the variable "active libc locale", and because the behavior is slightly different than libc both for performance, and to accommodate arbitrary byte sequences; for example, `Str\length("a\0b")` is 3, however, the libc `strlen` function returns 1 for the same input.

While operations such as string formatting, uppercase, and lowercase are defined for this locale, they should
only be used when localization is not a concern, for example when the output is intended to be machine-readable
instead of human-readable, such as when generating code.

## Functions without a locale parameter

In HHVM 4.130 and above, the bytes locale is the default locale used by the `Str\` functions that do not take an explicit locale - that is, `Str\foo($bar)` is equivalent to `Str\foo_l(Locale\bytes(), $bar)`. In prior versions, the active native/libc locale would be used instead; `Str\foo_l(Locale\get_native(), $bar)` can be used to    late the prior behavior.

This behavior was changed as:
- functions such as `Str\format()` were frequently used to to generate machine-readable strings, leading to subtle
  bugs when locale was changed. For example, `Str\format('%.2f', 1.23)` could return either '1.23' or '1,23'.
- functions still operated on bytes rather than characters, even with `LC_CTYPE` was set to `UTF-8`.
- users expect functions such as `Str\format()` and `Str\uppercase()` to be pure, however they can not be when
  they depend on the current locale, which is effectively a global variable.

## Recommendations

- use the non-`_l` variants of functions when generating strings intended for machines to read.
- use the non-`_l` variants when looking for searching for or operating on specific byte sequences.
- avoid using `setlocale()` or `Locale\set_native()`; instead, store/pass a `Locale\Locale` object for the
  viewer in a similar way to how you store/pass other information about the viewer, such as their ID.
- prefer `Locale\set_native()` over `setlocale()`.
- if setting a locale, always explicitly specify an encoding, unless you are matching the behavior of other local executables or native libraries
- if either is necessary, restore the default locale with `Locale\set_native(Locale\bytes())`; while this can be
  functionally equivalent to various other locales (e.g. `"en_US"` or `"C"`), HHVM contains optimizations
  specifically for the `Locale\bytes()` locale.

`setlocale()` and `Locale\set_native()` affect many C libraries and extension; in web requests, this can lead
to error messages in logs being translated to the viewer rather than the log reader, though for CLI programs, this
behavior can be desirable.

## Supported encodings

HHVM currently supports UTF-8, and single-byte encodings that are supported by the platform libc.

Other encodings may be supported in the future, however `Locale\set_native()` is likely to be restricted to
the current locales; for example, UTF-16 can not be supported by `Locale\set_native()`, as UTF-16 strings can
contain null bytes.

## Working with `Regex\`
Functions in the `Regex\` namespace operate on bytes. If the string being inspected is UTF-8, use a pattern with the `u` flag. Failing to do so may result in one multi-byte character being interpreted as multiple characters. For example:

- `Regex\replace("\u{1f600}", re"/./", 'Char')` is `CharCharCharChar`
- `Regex\replace("\u{1f600}", re"/./u", 'Char')` is `Char`
