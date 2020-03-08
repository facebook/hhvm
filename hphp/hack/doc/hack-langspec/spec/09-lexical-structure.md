# Lexical Structure

## Scripts

A [script](04-basic-concepts.md#program-structure) is an ordered sequence of characters. Typically, a
script has a one-to-one correspondence with a file in a file system, but
this correspondence is not required.

Conceptually, a script is translated using the following steps:

1.  Transformation, which converts a script from a particular character
    repertoire and encoding scheme into a sequence of 8-bit characters.

2.  Lexical analysis, which translates a stream of input characters into
    a stream of tokens. 

3.  Syntactic analysis, which translates the stream of tokens into
    executable code.

Conforming implementations must accept scripts encoded with the UTF-8
encoding form (as defined by the Unicode standard), and transform them
into a sequence of characters. Implementations can choose to accept and
transform additional character encoding schemes.

## Grammars

This specification shows the syntax of the Hack programming language
using two grammars. The *lexical grammar* defines how source
characters are combined to form white space, comments, and tokens. The
*syntactic grammar* defines how the resulting tokens are combined to
form Hack programs.

The grammars are presented using *grammar productions*, with each one
defining a non-terminal symbol and the possible expansions of that
non-terminal symbol into sequences of non-terminal or terminal symbols.
In productions, non-terminal symbols are shown in slanted type *like
this*, and terminal symbols are shown in a fixed-width font `like this`. 

The first line of a grammar production is the name of the non-terminal
symbol being defined, followed by one colon for a syntactic grammar
production, and two colons for a lexical grammar production. Each
successive indented line contains a possible expansion of the
non-terminal given as a sequence of non-terminal or terminal symbols.
For example, the production:

<pre>
  <i>single-line-comment::</i>
    // input-characters<sub>opt</sub>
    #  input-characters<sub>opt</sub>
</pre>

defines the lexical grammar production *single-line-comment* as being
the terminals `//` or `#`, followed by an optional *input-characters*. Each
expansion is listed on a separate line.

Although alternatives are usually listed on separate lines, when there
is a large number, the shorthand phrase “one of” may precede a list of
expansions given on a single line. For example,

<pre>
  <i>hexadecimal-digit:: one of</i>
    0   1   2   3   4   5   6   7   8   9
    a   b   c   d   e   f
    A   B   C   D   E   F
</pre>

## Grammar ambiguities

In general, lexical processing results in the creation of the longest possible
lexical element. However, in certain cases involving generic type specifiers
(and array specifiers, which use generic type notation), this is not the case.
For example, in type specifiers like `X<Y<int>>` and `array<array<int>>`, the
source characters `>` and `>` are treated as two separate tokens rather than
as the right-shift operator `>>`.

## Lexical Analysis

### General

The production *input-file* is the root of the lexical structure for a
script. Each script must conform to this production.

**Syntax**

<pre>
  <i>input-file::</i>
    <i>input-element</i>
    <i>input-file   input-element</i>
  <i>input-element::</i>
    <i>comment</i>
    <i>white-space</i>
    <i>token</i>
</pre>

**Defined elsewhere**

* [*comment*](09-lexical-structure.md#comments)
* [*token*](09-lexical-structure.md#general-1)
* [*white-space*](09-lexical-structure.md#white-space)

**Semantics:**

The basic elements of a script are comments, white space, and tokens.

The lexical processing of a script involves the reduction of that script
into a sequence of [tokens](09-lexical-structure.md#tokens) that becomes the input to the
syntactic analysis. Tokens can be separated by [white space](09-lexical-structure.md#white-space) and
[delimited comments](09-lexical-structure.md#comments).

Apart from the exception noted in [§§](09-lexical-structure.md#grammar-ambiguities), lexical processing always results in the creation of the longest
possible lexical element. (For example, `$a+++++$b` must be parsed as
`$a++ ++ +$b`, which syntactically is invalid.)

### Comments

Two forms of comments are supported: *delimited comments* and
*single-line comments*. 

**Syntax**

<pre>
  <i>comment::</i>
    <i>single-line-comment</i>
    <i>delimited-comment</i>

  <i>single-line-comment::</i>
    //   <i>input-characters<sub>opt</sub></i>
    #    <i>input-characters<sub>opt</sub></i>

  <i>input-characters::</i>
    <i>input-character</i>
    <i>input-characters   input-character</i>

  <i>input-character::</i>
    Any source character except new-line

  <i>new-line::</i>
    Carriage-return character (U+000D)
    Line-feed character (U+000A)
    Carriage-return character (U+000D) followed by line-feed character (U+000A)

  <i>delimited-comment::</i>
    /*   No characters or any source character sequence except /*   */
</pre>

**Semantics**

Except within a string literal or a comment, the characters /\* start a
delimited comment, which ends with the characters \*/. Except within a
string literal or a comment, the characters // or \# start a single-line
comment, which ends with a new line. That new line is not part of the
comment. However, if the single-line comment is the last source element
in an embedded script, the trailing new line can be omitted. (Note: this
allows for uses like `<?hh ... // ... ?>`.)

A delimited comment can occur in any place in a script in which [white
space](09-lexical-structure.md#white-space) can occur. (For example;
`/*...*/$c/*...*/=/*...*/567/*...*/;/*...*/` is parsed as `$c=567;`, and
`$k = $i+++/*...*/++$j;` is parsed as `$k = $i+++ ++$j;`.)

A number of single-line, so-called special comments are recognized by a
conforming implementation; they are:

* [`// FALLTHROUGH`](11-statements.md#the-switch-statement)
* [`// strict`](04-basic-concepts.md#program-structure)

where horizontal white space is permitted between the `//` and the start of
the comment text.

**Implementation Notes**

During tokenizing, an implementation can treat a delimited comment as
though it was white space.

### White Space

White space consists of an arbitrary combination of one or more
new-line, space, and horizontal tab characters.

**Syntax**

<pre>
  <i>white-space::</i>
    <i>white-space-character</i>
    <i>white-space   white-space-character</i>

  <i>white-space-character::</i>
    <i>new-line</i>
    Space character (U+0020)
    Horizontal-tab character (U+0009)
</pre>

**Defined elsewhere**

* [*new-line*](09-lexical-structure.md#comments)

**Semantics**

The space and horizontal tab characters are considered *horizontal
white-space characters*.

### Tokens

#### General

There are several kinds of source tokens:

**Syntax**

<pre>
  <i>token::</i>
    <i>variable-name</i>
    <i>name</i>
    <i>keyword</i>
    <i>literal</i>
    <i>operator-or-punctuator</i>
</pre>

**Defined elsewhere**

* [*keyword*](09-lexical-structure.md#keywords)
* [*literal*](09-lexical-structure.md#general-2)
* [*name*](09-lexical-structure.md#names)
* [*operator-or-punctuator*](09-lexical-structure.md#operators-and-punctuators)
* [*variable-name*](09-lexical-structure.md#names)

#### Names

**Syntax**

<pre>
  <i>variable-name::</i>
    $   <i>name</i>

  <i>name::</i>
    <i>name-nondigit</i>
    <i>name   name-nondigit</i>
    <i>name   digit</i>

  <i>name-nondigit::</i>
    <i>nondigit</i>
    one of the characters U+007f–U+00ff

  <i>nondigit::</i> one of
    _
    a   b   c   d   e   f   g   h   i   j   k   l   m
    n   o   p   q   r   s   t   u   v   w   x   y   z
    A   B   C   D   E   F   G   H   I   J   K   L   M
    N   O   P   Q   R   S   T   U   V   W   X   Y   Z
</pre>

**Defined elsewhere**

* [*digit*](09-lexical-structure.md#integer-literals)

**Semantics:**

Names are used to identify the following: [constants](06-constants.md#general), [variables](07-variables.md#general), [labels](11-statements.md#labeled-statements), [enumerated types](13-enums.md#enum-declarations), [functions](15-functions.md#function-definitions), [classes](16-classes.md#class-declarations), [class members](16-classes.md#class-members), [interfaces](17-interfaces.md#interface-declarations), [traits](18-traits.md#general), [type aliases](05-types.md#type-aliases), [namespaces](20-namespaces.md#general),
names in [heredoc comments](09-lexical-structure.md#heredoc-string-literals) and [nowdoc comments](09-lexical-structure.md#nowdoc-string-literals); and [attributes](21-attributes.md#attributes).

A *name* begins with an underscore (_), *name-nondigit*, or extended
name character in the range U+007f–U+00ff. Subsequent characters can
also include *digit*s. A *variable name* is a name with a leading
dollar ($).

The name `$_`, referred to as the *placeholder variable*, is reserved for use in the ([`list` intrinsic](§10-expressions.md#list)) and the ([`foreach` statement](§11-statements.md#The-foreach-Statement)). This name does not actually designate any storage; instead, it’s an indication that the value that would otherwise be stored in a variable in that context, is ignored.

Unless stated otherwise ([§§](14-generic-types-methods-and-functions.md#type-parameters), [§§](15-functions.md#function-calls), [§§](16-classes.md#class-declarations), [§§](18-traits.md#trait-members)),
names are case-sensitive, and every character in a name is significant.

Function and method names beginning with two underscores (__) are
reserved by the Hack language.

**Examples**

```Hack
const int MAX_VALUE = 100;
public function getData(): array<string> { ... }
class Point { ... }
interface ICollection { ... }
```

**Implementation Notes**

An implementation is discouraged from placing arbitrary restrictions on
name length or length of significance.

#### Keywords

A *keyword* is a name-like sequence of characters that is reserved, and
cannot be used as a name.

**Syntax**

<pre>
  <i>keyword::</i> one of
    abstract   arraykey   as   async   await   break   case   catch   class   classname   clone   const   continue   default   do
    echo   else   elseif   enum   extends   final   finally   for   foreach   function   if   implements
    instanceof   insteadof   interface   mixed   namespace   new   newtype   noreturn   num   parent   private
    protected   public   require   require_once   return   self   shape   static   switch   throw   trait   try
    tuple   type   use   while   yield
</pre>

**Semantics**

Keywords are case-sensitive.

Note: Strictly speaking, `false`, `null`, and `true` are not keywords;
however, they do have predefined meanings, and can be thought of as keywords.
Likewise for the names of the [intrinsics](10-expressions.md#general-2).

#### Literals

##### General

The source code representation of a value is called a *literal*.

**Syntax**

<pre>
  <i>literal::
    <i>boolean-literal</i>
    <i>integer-literal</i>
    <i>floating-literal</i>
    <i>string-literal</i>
    <i>null-literal</i>
</pre>

**Defined elsewhere**

* [*boolean-literal*](09-lexical-structure.md#boolean-literals)
* [*floating-literal*](09-lexical-structure.md#floating-point-literals)
* [*integer-literal*](09-lexical-structure.md#integer-literals)
* [*null-literal*](09-lexical-structure.md#the-null-literal)
* [*string-literal*](09-lexical-structure.md#string-literals)

##### Boolean Literals

**Syntax**

<pre>
  <i>boolean-literal::</i>
    true
    false
</pre>

**Semantics**

The type of a *boolean-literal* is bool. The values `true` and `false`
represent the Boolean values True and False, respectively.

**Examples**

```Hack
$done = false;
computeValues($table, true);
```

##### Integer Literals

**Syntax**

<pre>
  <i>integer-literal::</i>
    <i>decimal-literal</i>
    <i>octal-literal</i>
    <i>hexadecimal-literal</i>
    <i>binary-literal</i>

    <i>decimal-literal::</i>
      <i>nonzero-digit</i>
      <i>decimal-literal   digit</i>

    <i>octal-literal::</i>
      0
      <i>octal-literal   octal-digit</i>

    <i>hexadecimal-literal::</i>
      <i>hexadecimal-prefix   hexadecimal-digit</i>
      <i>hexadecimal-literal   hexadecimal-digit</i>

    <i>hexadecimal-prefix:: one of</i>
      0x  0X

    <i>binary-literal::</i>
      <i>binary-prefix   binary-digit</i>
      <i>binary-literal   binary-digit</i>

    <i>binary-prefix:: one of</i>
      0b  0B

    <i>digit:: one of</i>
      0  1  2  3  4  5  6  7  8  9

    <i>nonzero-digit:: one of</i>
      1  2  3  4  5  6  7  8  9

    <i>octal-digit:: one of</i>
      0  1  2  3  4  5  6  7

    <i>hexadecimal-digit:: one of</i>
      0  1  2  3  4  5  6  7  8  9
            a  b  c  d  e  f
            A  B  C  D  E  F

    <i>binary-digit:: one of</i>
        0  1
</pre>

**Constraints**

The value of an integer literal must be representable by its type.

**Semantics**

The value of a decimal integer literal is computed using base 10; that
of an octal integer literal, base 8; that of a hexadecimal integer
literal, base 16; and that of a binary integer literal, base 2.

The type of an *integer-literal* is `int`.

Using a twos-complement system, can the smallest negative value
(-9223372036854775808 for 64 bits) be
represented as a decimal integer literal? No. Consider the
expression -5. This is made up of two tokens: a unary minus followed by
the integer literal 5. As such, **there is no such thing as a
negative-valued decimal integer literal in Hack**. Instead, there is the
non-negative value, which is then negated. Literals written using hexadecimal, octal, or binary
notations are considered to have non-negative values.

**Examples**

```Hack
$count = 10      // decimal 10

0b101010 >> 4    // binary 101010 and decimal 4

0XAF << 023      // hexadecimal AF and octal 23
```

##### Floating-Point Literals

**Syntax**

<pre>
  <i>ﬂoating-literal::</i>
    <i>fractional-literal   exponent-part<sub>opt</sub></i>
    <i>digit-sequence   exponent-part</i>

  <i>fractional-literal::</i>
    <i>digit-sequence<sub>opt</sub></i> . <i>digit-sequence</i>
    <i>digit-sequence</i> .

  <i>exponent-part::</i>
    e  <i>sign<sub>opt</sub>   digit-sequence</i>
    E  <i>sign<sub>opt</sub>   digit-sequence</i>

  <i>sign:: one of</i>
    +  -

  <i>digit-sequence::</i>
    <i>digit</i>
    <i>digit-sequence   digit</i>
</pre>

**Defined elsewhere**

* [*digit*](09-lexical-structure.md#integer-literals)

**Constraints**

The value of a floating-point literal must be representable by its type.

**Semantics**

The type of a *floating-literal* is `float`.

The constants [`INF`](06-constants.md#core-predefined-constants) and [`NAN`](06-constants.md#core-predefined-constants) provide access to the floating-
point values for infinity and Not-a-Number, respectively.

**Examples**

```Hack
$values = array(1.23, 3e12, 543.678E-23);
```

##### String Literals

**Syntax**

<pre>
  <i>string-literal::</i>
    <i>single-quoted-string-literal</i>
    <i>double-quoted-string-literal</i>
    <i>heredoc-string-literal</i>
    <i>nowdoc-string-literal</i>
</pre>

**Defined elsewhere**

* [*double-quoted-string-literal*](09-lexical-structure.md#double-quoted-string-literals)
* [*heredoc-string-literal*](09-lexical-structure.md#heredoc-string-literals)
* [*nowdoc-string-literal*](09-lexical-structure.md#nowdoc-string-literals)
* [*single-quoted-string-literal*](09-lexical-structure.md#single-quoted-string-literals)

Note: By conventional standards, calling [*heredoc-string-literal*s](#heredoc-string-literals)
and [*nowdoc-string-literal*s](#nowdoc-string-literals)) literals is a stretch, as
each is hardly a single token. And given the variable substitution permitted in *double-quote-string-literals*, they are not really literals either.

**Semantics**

A string literal is a sequence of zero or more characters delimited in
some fashion. The delimiters are not part of the literal's content.

The type of a string literal is `string`.

###### Single-Quoted String Literals

**Syntax**

<pre>
  <i>single-quoted-string-literal::</i>
    ' <i>sq-char-sequence<sub>opt</sub></i>  '

  <i>sq-char-sequence::</i>
    <i>sq-char</i>
    <i>sq-char-sequence   sq-char</i>

  <i>sq-char::</i>
    <i>sq-escape-sequence</i>
    \<i><sub>opt</sub></i>   any member of the source character set except single-quote (') or backslash (\)

  <i>sq-escape-sequence:: one of</i>
    \'  \\
</pre>

**Semantics**

A single-quoted string literal is a string literal delimited by
single-quotes ('). The literal can contain any source character except
single-quote (') and backslash (\\), which can only be represented by
their corresponding escape sequence.

A single-quoted string literal is a [c-constant](06-constants.md#general).

**Examples**

```Hack
'This text is taken verbatim'

'Can embed a single quote (\') and a backslash (\\) like this'
```

###### Double-Quoted String Literals

**Syntax**

<pre>
  <i>double-quoted-string-literal::</i>
    " <i>dq-char-sequence<sub>opt</sub></i>  "

  <i>dq-char-sequence::</i>
    <i>dq-char</i>
    <i>dq-char-sequence   dq-char</i>

  <i>dq-char::</i>
    <i>dq-escape-sequence</i>
    any member of the source character set except double-quote (") or backslash (\)
    \  any member of the source character set except "\$efnrtvxX or octal-digit

  <i>dq-escape-sequence::</i>
    <i>dq-simple-escape-sequence</i>
    <i>dq-octal-escape-sequence</i>
    <i>dq-hexadecimal-escape-sequence</i>
    <i>dq-unicode-escape-sequence</i>

  <i>dq-simple-escape-sequence:: one of</i>
    \"   \\   \$   \e   \f   \n   \r   \t   \v

  <i>dq-octal-escape-sequence::</i>
    \   <i>octal-digit</i>
    \   <i>octal-digit   octal-digit</i>
    \   <i>octal-digit   octal-digit   octal-digit</i>

  <i>dq-hexadecimal-escape-sequence::</i>
    \x  <i>hexadecimal-digit   hexadecimal-digit<sub>opt</sub></i>
    \X  <i>hexadecimal-digit   hexadecimal-digit<sub>opt</sub></i>

  <i>dq-unicode-escape-sequence::</i>
    \u{  codepoint-digits  }

  <i>codepoint-digits::</i>
     <i>hexadecimal-digit</i>
     <i>hexadecimal-digit   codepoint-digits</i>
</pre>

**Defined elsewhere**

* [*hexadecimal-digit*](09-lexical-structure.md#integer-literals)
* [*octal-digit*](09-lexical-structure.md#integer-literals)

**Semantics**

A double-quoted string literal is a string literal delimited by
double-quotes ("). The literal can contain any source character except
double-quote (") and backslash (\\), which can only be represented by
their corresponding escape sequence. Certain other (and sometimes
non-printable) characters can also be expressed as escape sequences.

An escape sequence represents a single-character encoding, as described
in the table below:

Escape sequence | Character name | Unicode character
--------------- | --------------| ------
\$  | Dollar sign | U+0024
\"  | Double quote | U+0022
\\  | Backslash | U+005C
\e  | Escape | U+001B
\f  | Form feed | U+000C
\n  | New line | U+000A
\r  | Carriage Return | U+000D
\t  | Horizontal Tab | U+0009
\v  | Vertical Tab | U+000B
\ooo |  1–3-digit octal digit value ooo |
\xhh or \Xhh  | 1–2-digit hexadecimal digit value hh | U+00hh
\u{xxxxxx} | UTF-8 encoding of Unicode codepoint U+xxxxxx | U+xxxxxx

Within a double-quoted string literal, except when recognized as the
start of an escape sequence, a backslash (\\) is retained verbatim.

Within a double-quoted string literal a dollar ($) character not
escaped by a backslash (\\) is handled using a variable substitution rules
described below.

The `\u{xxxxxx}` escape sequence produces the UTF-8 encoding of the Unicode
codepoint with the hexadecimal number specified within the curly braces.
Implementations MUST NOT allow Unicode codepoints beyond U+10FFFF as this is
outside the range UTF-8 can encode (see
[RFC 3629](http://tools.ietf.org/html/rfc3629#section-3)). If a codepoint
larger than U+10FFFF is specified, implementations MUST error.
Implementations MUST pass through `\u` verbatim and not interpret it as an
escape sequence if it is not followed by an opening `{`, but if it is,
implementations MUST produce an error if there is no terminating `}` or the
contents are not a valid codepoint. Implementations MUST support leading zeroes,
but MUST NOT support leading or trailing whitespace for the codepoint between
the opening and terminating braces. Implementations MUST allow Unicode
codepoints that are not Unicode scalar values, such as high and low surrogates.

A Unicode escape sequence cannot be created by variable substitution. For example, given `$v = "41"`,
`"\u{$v}"` results in `"\u41"`, a string of length 4, while `"\u{0$v}"` and `"\u{{$v}}"` contain
ill-formed Unicode escape sequences.

**Variable substitution**

The variable substitution accepts the following syntax:

<pre>
    <i>string-variable::</i>
        <i>variable-name</i>   <i>offset-or-property<sub>opt</sub></i>

    <i>offset-or-property::</i>
        <i>offset-in-string</i>
        <i>property-in-string</i>

    <i>offset-in-string::</i>
        [   <i>name</i>   ]
        [   <i>variable-name</i>   ]
        [   <i>integer-literal</i>   ]

    <i>property-in-string::</i>
        ->   <i>name</i>

</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#general-6)
* [*integer-literal*](#integer-literals)
* [*name*](#names)
* [*variable-name*](#names)

After the variable defined by the syntax above is evaluated, its value is converted
to string according to the rules of [string conversion](08-conversions.md#converting-to-string-type)
and is substituted into the string in place of the variable substitution expression.

Subscript or property access defined by *offset-in-string* and *property-in-string*
is resolved according to the rules of the [subscript operator](10-expressions.md#subscript-operator)
and [member selection operator](10-expressions.md#member-selection-operator) respectively.
The exception is that *name* inside *offset-in-string* is interpreted as a string literal even if it is not
quoted.

If the character sequence following the `$` does not parse as *name* then the `$` character
is instead interpreted verbatim and no variable substitution is performed.

A double-quoted string literal is a [c-constant](06-constants.md#general) if it does not
contain any variable substitution.

**Examples**

```Hack
$x = 123;
echo ">\$x.$x"."<"; // → >$x.123<
// -----------------------------------------
$colors = array("red", "white", "blue");
$index = 2;
echo "\$colors[$index] contains >$colors[$index]<\n";
  // → $colors[2] contains >blue<
// -----------------------------------------
class C {
    public $p1 = 2;
}
$myC = new C();
echo "\$myC->p1 = >$myC->p1<\n";  // → $myC->p1 = >2<
```

###### Heredoc String Literals

**Syntax**

<pre>
  <i>heredoc-string-literal::</i>
    &lt;&lt;&lt;  <i>hd-start-identifier   new-line   hd-char-sequence<sub>opt</sub>  new-line hd-end-identifier</i>  ;<i><sub>opt</sub>   new-line</i>

  <i>hd-start-identifier::</i>
    <i>name</i>

  <i>hd-end-identifier::</i>
    <i>name</i>

  <i>hd-char-sequence::</i>
    <i>hd-char</i>
    <i>hd-char-sequence   hd-char</i>

  <i>hd-char::</i>
    <i>hd-escape-sequence</i>
    any member of the source character set except backslash (\)
    \  any member of the source character set except \$efnrtvxX or
octal-digit

  <i>hd-escape-sequence::</i>
    <i>hd-simple-escape-sequence</i>
    <i>dq-octal-escape-sequence</i>
    <i>dq-hexadecimal-escape-sequence</i>
    <i>dq-unicode-escape-sequence</i>

  <i>hd-simple-escape-sequence:: one of</i>
    \\   \$   \e   \f   \n   \r   \t   \v
</pre>

**Defined elsewhere**

* [*dq-hexadecimal-escape-sequence*](09-lexical-structure.md#double-quoted-string-literals)
* [*dq-octal-escape-sequence*](09-lexical-structure.md#double-quoted-string-literals)
* [*dq-unicode-escape-sequence*](09-lexical-structure.md#double-quoted-string-literals)
* [*name*](09-lexical-structure.md#names)
* [*new-line*](09-lexical-structure.md#comments)

**Constraints**

The start and end identifier must be the same. Only horizontal white
space is permitted between `<<<` and the start identifier. No white
space is permitted between the start identifier and the new-line that
follows. No white space is permitted between the new-line and the end
identifier that follows. Except for an optional semicolon (`;`), no
characters—not even comments or white space—are permitted between the
end identifier and the new-line that terminates that source line.

**Semantics**

A heredoc string literal is a string literal delimited by
"`<<< name`" and "`name`". The literal can contain any source
character. Certain other (and sometimes non-printable) characters can
also be expressed as escape sequences.

A heredoc literal supports variable substitution as defined for
[double-quoted string literals](09-lexical-structure.md#double-quoted-string-literals).

A heredoc string literal is a [c-constant](06-constants.md#general) if it does not contain
any variable substitution.

**Examples**

```Hack
$v = 123;
$s = <<<    ID
S'o'me "\"t e\txt; \$v = $v"
Some more text
ID;
echo ">$s<";
→ >S'o'me "\"t e  xt; $v = 123"
Some more text<
```

###### Nowdoc String Literals

**Syntax**

<pre>
  <i>nowdoc-string-literal::</i>
    &lt;&lt;&lt;  '  <i>hd-start-identifier</i>  '  <i>new-line  hd-char-sequence<sub>opt</sub>   new-line hd-end-identifier</i>  ;<i><sub>opt</sub>   new-line</i>
</pre>

**Defined elsewhere**

* [*hd-char-sequence*](09-lexical-structure.md#heredoc-string-literals)
* [*hd-end-identifier*](09-lexical-structure.md#heredoc-string-literals)
* [*hd-start-identifier*](09-lexical-structure.md#heredoc-string-literals)
* [*new-line*](09-lexical-structure.md#comments)

**Constraints**

No white space is permitted between the start identifier and its
enclosing single quotes ('). See also [§§](09-lexical-structure.md#heredoc-string-literals).

**Semantics**

A nowdoc string literal looks like a [heredoc string literal](09-lexical-structure.md#heredoc-string-literals) except that in the former the start identifier name is
enclosed in single quotes ('). The two forms of string literal have the
same semantics and constraints except that a nowdoc string literal is
not subject to variable substitution.

A nowdoc string literal is a [c-constant](06-constants.md#general).

**Examples**

```Hack
$v = 123;
$s = <<<    'ID'
S'o'me "\"t e\txt; \$v = $v"
Some more text
ID;
echo ">$s<\n\n";
→ >S'o'me "\"t e\txt; \$v = $v"
Some more text<
```

##### The Null Literal

<pre>
  <i>null-literal::</i>
    null
</pre>

**Semantics**

There is one null-literal value, `null`.

A *null-literal* has the null type.

#### Operators and Punctuators

**Syntax**

<pre>
  <i>operator-or-punctuator:: one of</i>
    [   ]    (   )   {    }   .   ->   ++   --   **   *   +   -   ~   !
    $   /   %   &lt;&lt;   >>   &lt;   >   &lt;=   >=   ==   ===   !=   !==   ^   |
    &amp;   &amp;&amp;   ||   ?   ??   :   ; =   **=   *=   /=   %=   +=   -=   .=   &lt;&lt;=
    >>=   &amp;=   ^=   |=   ,   @   ::   =>   ==>   ?->   \   ...    |>   $$
</pre>

**Semantics**

Operators and punctuators are symbols that have independent syntactic
and semantic significance. *Operators* are used in expressions to
describe operations involving one or more *operands*, and that yield a
resulting value, produce a side effect, or some combination thereof.
*Punctuators* are used for grouping and separating.
