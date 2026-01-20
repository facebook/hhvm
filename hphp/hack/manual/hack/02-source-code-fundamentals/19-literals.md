# Literals

## Boolean Literals

The literals `true` and `false` represent the Boolean values True and False, respectively. The type of a Boolean
literal is `bool`. For example:

```hack
$val = true;
if ($val === false) {
  // ...
}
```

## Integer Literals

Integer literals can be written as decimal; hexadecimal (with prefix `0x` or `0X`, and including letters A-F or a-f);
octal (with prefix `0`); or binary (with prefix `0b` or `0B`).  The type of an integer literal is `int`.  For example:

```hack
$count = 10;      // decimal 10
0b101010 >> 4;    // binary 101010 and decimal 4
0XAf << 012;      // hexadecimal Af and octal 12
```

Integer literals can also contain underscores as digit separators. They function only as visual aids, they have no
actual meaning. For example:

```hack
$amount = 394_493_392; // completely equivalent to 394493392
$address = 0x49AD_DF30;
$permission = 012_212_001;
```

## Floating-Point Literals

Floating-point literals typically have an integer part, a decimal point, and a fractional part. They may also have an
exponent part. They are written using decimal digits.  The type of a floating-point literal is `float`.  For example:

```hack
123.456 + 0.6E27 + 2.34e-3;
```

The predefined constants `INF` and `NAN` provide access to the floating- point values for infinity and Not-a-Number, respectively.

Floating-point literals may also contain underscores as digit separators in the integer part, the fractional part, and the exponent part. For example:

```hack
123_456.49_30e-30_30;
```

## String Literals

A string literal can have one of the following forms:
  - [Single-Quoted String Literals](#single-quoted-string-literals)
  - [Double-Quoted String Literals](#double-quoted-string-literals)
  - [Heredoc String Literals](#heredoc-string-literals)
  - [Nowdoc String Literals](#nowdoc-string-literals)

A string literal is a sequence of zero or more characters delimited in some fashion. The delimiters are not part of
the literal's content. The type of a string literal is `string`.

### Single-Quoted String Literals

A single-quoted string literal is a string literal delimited by single-quotes ('). The literal can contain any source
character except single-quote (') and backslash (\\), which can only be represented by their corresponding escape sequence, \\' and \\\\.  For example:

```hack
'Welcome to Hack!';
'Can embed a single quote (\') and a backslash (\\) like this';
```

### Double-Quoted String Literals

A double-quoted string literal is a string literal delimited by double-quotes ("). The literal can contain any source
character except double-quote (") and backslash (\\), which can only be represented by their corresponding escape sequence, \\" and \\\\.  For example:

```hack
"Welcome to Hack!";
"Can embed a double quote (\") and a backslash (\\) like this";
```

Certain other (and sometimes non-printable) characters can also be expressed as escape sequences.  An *escape sequence*
represents a single-character encoding.  For example:

```hack
"First line 1\nSecond line 2\n\nFourth line\n";
"Can embed a double quote (\") and a backslash (\\) like this";
```

Here are the supported escape sequences:

Escape sequence | Character name | Unicode character
--------------- | --------------| ------
`\$`  | Dollar sign | U+0024
`\"`  | Double quote | U+0022
`\\`  | Backslash | U+005C
`\e`  | Escape | U+001B
`\f`  | Form feed | U+000C
`\n`  | New line | U+000A
`\r`  | Carriage Return | U+000D
`\t`  | Horizontal Tab | U+0009
`\v`  | Vertical Tab | U+000B
`\ooo` |  1-3-digit octal digit value ooo |
`\xhh` or `\Xhh`  | 1-2-digit hexadecimal digit value hh | U+00hh
`\u{xxxxxx}` | UTF-8 encoding of Unicode codepoint U+xxxxxx | U+xxxxxx

Within a double-quoted string literal a dollar ($) character *not* escaped by a backslash (\\) is handled using *[variable
substitution rules](#variable-substitution)*.

### Heredoc String Literals

A heredoc string literal is a string literal delimited by `<<< id` and `id`. The literal can contain any source character.
Certain other (and sometimes non-printable) characters can also be expressed as [escape sequences](#double-quoted-string-literals).

For example:

```
$var = 42;
$s = <<<   ID
Wow, look at this text!
We can even have a semicolon here! ; or '' or ""!
Variable substitution: $var
ID;
echo ">$s<\n";
```

Heredoc literals also support [variable substitution](#variable-substitution).

When working with heredoc literals, keep the following rules in mind:
* The start and end `id` must be the same.
* Only horizontal white space is permitted between `<<<` and the start `id`.
* No white space is permitted between the start `id` and the new-line that follows.
* No white space is permitted between the new-line and the end `id` that follows.
* Except for an optional semicolon (`;`), no characters&mdash;not even comments or white space&mdash;are permitted between the end `id` and the new-line that terminates that source line.

### Nowdoc String Literals

A nowdoc string literal looks like a [heredoc string literal](#heredoc-string-literals) except that in the former the start
`id` is enclosed in single quotes (`'`).

For example:

```
$var = 42;
$s = <<<   'ID'
Wow, look at this text!
We can even have a semicolon here! ; or '' or ""!
Variable substitution: $var
ID;
echo ">$s<\n";
```
The two forms of string literal (heredoc, nowdoc) have the same semantics and constraints except that **nowdoc literals do not support variable substitution.**

Remember: White space is not permitted between the start `id` and its enclosing single quotes (`'`).

## Variable Substitution
When a variable name is seen inside a double-quoted string, after that variable is evaluated, its value is converted to `string`
and is substituted into the string in place of the variable-substitution expression. Subscript or property accesses are resolved
according to the rules of the [subscript operator](/hack/expressions-and-operators/subscript) and
[member selection operator](/hack/expressions-and-operators/member-selection), respectively. If the character sequence following
the `$` does not parse as a recognized name, then the `$` character is instead interpreted verbatim and no variable substitution
is performed.

Consider the following example:

```hack
class C {
  public int $p1 = 2;
}

<<__EntryPoint>>
function main(): void {
  $x = 123;
  echo ">\$x.$x"."<\n";

  $myC = new C();
  echo "\$myC->p1 = >$myC->p1<\n";
}
```

## The Null Literal

There is one null-literal value, `null`, which has type `null`.  For example:

```hack
function has_default_arg(num $arg, ?num $base = null): void {}
```

Here, `null` is used as a default argument value in the function `has_default_arg`.

In the following example:

```hack
type IdSet = shape('id' => ?string, 'url' => ?string, 'count' => int);

function get_IdSet(): IdSet {
  return shape('id' => null, 'url' => null, 'count' => 0);
}
```

`null` is used to initialize two data fields in a shape.
