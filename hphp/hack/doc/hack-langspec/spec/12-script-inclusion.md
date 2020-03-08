# Script Inclusion Operators

## General

**Syntax**

<pre>
  <i>inclusion-directive:</i>
    <i>require-multiple-directive</i>
    <i>require-once-directive</i>
</pre>

**Defined elsewhere**

* [*require-multiple-directive*](12-script-inclusion.md#the-require-directive)
* [*require-once-directive*](12-script-inclusion.md#the-require_once-directive)

**Semantics:**

When creating large applications or building component libraries, it is
useful to be able to break up the source code into small, manageable
pieces each of which performs some specific task, and which can be
shared somehow, and tested, maintained, and deployed individually. For
example, a programmer might define a series of useful constants and use
them in numerous and possibly unrelated applications. Likewise, a set of
class definitions can be shared among numerous applications needing to
create objects of those types.

An *include file* is a script that is suitable for *inclusion* by
another script. The script doing the including is the *including file*,
while the one being included is the *included file*. A script can be an
including file and an included file, either, or neither.

It is important to understand that unlike the C/C++ (or similar)
preprocessor, script inclusion in PHP is not a text substitution
process. That is, the contents of an included file are not treated as if
they directly replaced the inclusion operation source in the including
file.

The name used to specify an include file may contain an absolute or
relative path. In the latter case, an implementation may use the
configuration directive
[`include_path`](http://docs.hhvm.com/manual/en/ini.core.php#ini.include-path)
to resolve the include file's location.

## The `require` Directive

**Syntax**

<pre>
<i>require-multiple-directive:</i>
  require  <i>include-filename</i>  ;

<i>include-filename:</i>
  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

*expression* must be a string that designates a file that exists, is accessible, and whose format is suitable for inclusion (that is, starts with a [Hack start-tag](04-basic-concepts.md#program-structure)).

**Semantics:**

If the designated file is not accessible, execution is terminated.

Variables defined in an included file take on scope of the source line on which the inclusion occurs in the including file. However, functions and classes defined in the included file are given global scope.

The library function [`get_included_files`](http://www.php.net/get_included_files) provides the names of
all files included or required.

**Examples**

```Hack
require 'Point.php';
require ('Circle.php');
```

## The `require_once` Directive

**Syntax**

<pre>
  <i>require-once-directive:</i>
    require_once  <i>include-filename</i>  ;
</pre>

**Defined elsewhere**

* [*include-filename*](12-script-inclusion.md#the-require-directive)

**Semantics:**

This operator is identical to operator [`require`](12-script-inclusion.md#the-require-directive) except that in
the case of `require_once`, the include file is included once only during
program execution.

**Examples**

```Hack
// Point.php
<?hh …
class Point { … }

// Circle.php
<?hh …
require_once 'Point.php';
class Circle { /* uses Point somehow */ }

// MyApp.php
require_once 'Point.php';    // Point.php included directly
require_once 'Circle.php';   // Point.php now not included indirectly
$p1 = new Point(10, 20);
$c1 = new Circle(9, 7, 2.4);
```
