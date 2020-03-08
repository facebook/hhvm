# Statements

## General

**Syntax**

<pre>
  <i>statement:</i>
    <i>function-static-declaration</i>
    <i>compound-statement</i>
    <i>labeled-statement</i>
    <i>expression-statement</i>
    <i>selection-statement</i>
    <i>iteration-statement</i>
    <i>jump-statement</i>
    <i>try-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](#compound-statements)
* [*expression-statement*](#expression-statements)
* [*function-static-declaration*](07-variables.md#function-statics)
* [*iteration-statement*](#general-2)
* [*jump-statement*](#general-3)
* [*labeled-statement*](#labeled-statements)
* [*selection-statement*](#general-1)
* [*try-statement*](11-statements.md#the-try-statement)

## Compound Statements

**Syntax**

<pre>
  <i>compound-statement:</i>
    {  <i>statement-list<sub>opt</sub></i>  }

  <i>statement-list:</i>
    <i>statement</i>
    <i>statement-list   statement</i>
</pre>

**Defined elsewhere**

* [*statement*](#general)

**Semantics**

A *compound statement* allows a group of zero or more statements to be
treated syntactically as a single statement. A compound statement is
often referred to as a *block*.

**Examples**

```Hack
if (condition)
{	// braces are needed as the true path has more than one statement
	// statement-1
	// statement-2
}
else
{	// braces are optional as the false path has only one statement
	// statement-3
}
// -----------------------------------------
while (condition)
{	// the empty block is equivalent to a null statement
}
```

## Labeled Statements

**Syntax**

<pre>
  <i>labeled-statement:</i>
    <i>case-label</i>
    <i>default-label</i>

  <i>case-label:</i>
    case   <i>expression</i>  :  <i>statement</i>

  <i>default-label:</i>
    default  :  <i>statement</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*statement*](#general)

**Constraints**

*case-label* and *default-label* must only occur inside a [`switch` statement](#the-switch-statement).

**Semantics**

See the `switch` statement.

## Expression Statements

**Syntax**

<pre>
   <i>expression-statement:</i>
     <i>expression<sub>opt</sub></i>  ;
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Semantics**

If present, *expression* is evaluated for its side effects, if any, and
any resulting value is discarded. If *expression* is omitted, the
statement is a *null statement*, which has no effect on execution.

**Examples**

```Hack
$i = 10;  // $i is assigned the value 10; result (10) is discarded
++$i; // $i is incremented; result (11) is discarded
$i++; // $i is incremented; result (11) is discarded
DoIt(); // function DoIt is called; result (return value) is discarded
// -----------------------------------------
$i;   // no side effects, result is discarded. Vacuous but permitted
123;  // likewise for this one and the two statements following
34.5 * 12.6 + 11.987;
true;
```

## Selection Statements

### General

**Syntax**

<pre>
  <i>selection-statement:</i>
    <i>if-statement</i>
    <i>switch-statement</i>
</pre>

**Defined elsewhere**

* [*if-statement*](#the-if-statement)
* [*switch-statement*](#the-switch-statement)

**Semantics**

Based on the value of a controlling expression, a selection statement
selects among a set of statements.

### The `if` Statement

**Syntax**

<pre>
  <i>if-statement:</i>
    if   (   <i>expression</i>   )   <i>statement   elseif-clauses-opt   else-clause-opt</i>

  <i>elseif-clauses:</i>
    <i>elseif-clause</i>
    <i>elseif-clauses   elseif-clause</i>

  <i>elseif-clause:</i>
    elseif   (   <i>expression</i>   )   <i>statement</i>

  <i>else-clause:</i>
    else   <i>statement</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*statement*](#general)

**Constraints**

The controlling expression *expression* must have type `bool` or be
implicitly convertible to that type.

**Semantics**

The two forms of the `if` statement are equivalent; they simply provide
alternate styles.

If *expression* tests `true`, the *statement* that follows immediately is
executed. Otherwise, if an `elseif` clause is present the *statement*
immediately following the `elseif` is executed. Otherwise, any other
`elseif` *expression*s are evaluated. If none of those tests `true`, if an
`else` clause is present the *statement* immediately following the `else` is
executed.

An `else` clause is associated with the lexically nearest preceding `if` or
`elseif` that is permitted by the syntax.

**Examples**
```Hack
if ($count > 0) {
  …
} else {
  …
}
// -----------------------------------------
if (1)
  …
  if (0)
    …
else  // this else does NOT go with the outer if
  …

if (1) {
  …
  if (0)
    …
} else  // this else does go with the outer if
  …
```

### The `switch` Statement

**Syntax**

<pre>
  <i>switch-statement:</i>
    switch  (  <i>expression</i>  )  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](#compound-statements)
* [*expression*](10-expressions.md#yield-operator)

**Constraints**

The controlling expression *expression* must have scalar type.

There must be at most one default label.

Each label expression's type must be a subtype of the switch *expression* type.

The compound statement may be empty; if it is not, then the first statement in 
it must be a labeled statement.

**Semantics**

Based on the value of its *expression*, a `switch` statement transfers
control to a [case label](#labeled-statements); to a [default label](#labeled-statements), if one
exists; or to the statement immediately following the end of the `switch`
statement. A case or default label is only reachable directly within its
closest enclosing `switch` statement.

On entry to the `switch` statement, the controlling expression is
evaluated and then compared with the value of the case-label-expression
values, in lexical order. If one matches, control transfers to the
statement following the corresponding case label. If there is no match,
then if there is a default label, control transfers to the statement
following that; otherwise, control transfers to the statement
immediately following the end of the `switch` statement. If a `switch`
contains more than one case label whose values compare equal to the
controlling expression, the first in lexical order is considered the
match.

An arbitrary number of statements can be associated with any case or
default label. In the absence of a [`break` statement](11-statements.md#the-break-statement) at the end
of a set of such statements, control drops through into any following
case or default label. Thus, if all cases and the default end in break
and there are no duplicate-valued case labels, the order of case and
default labels is insignificant.

In no break *statement* is seen for a case or default before a subsequent case label, default label, or the switch-terminating `}` is encountered, an implementation might issue a warning. However, such a warning can be suppressed by placing a source line containing the special comment [`// FALLTHROUGH`](09-lexical-structure.md#comments), at the end of that case or default statement group.

Case-label values can be runtime expressions, and the types of sibling
case-label values need not be the same.

Switches may be nested, in which case, each `switch` has its own set of
`switch` clauses.

**Examples**

```Hack
$v = 10;
switch ($v) {
default:
  echo "default case: \$v is $v\n";
  break;    // break ends "group" of default statements
case 20:
  echo "case 20\n";
  break;    // break ends "group" of case 20 statements
case 10:
  echo "case 10\n"; // no break, so control drops into next label's "group"
  // FALLTHROUGH
case 30:
  echo "case 30\n"; // no break, but then none is really needed either
}
// -----------------------------------------
$v = 30;
switch ($v) {
case 30.0:  // <===== this case matches with 30
  echo "case 30.0\n";
  break;
default:
  echo "default case: \$v is $v\n";
  break;
case 30:    // <===== rather than this case matching with 30
  echo "case 30\n";
  break;
}
// -----------------------------------------
enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}
…
switch ($p1) {
case ControlStatus::Stopped:
  echo "Stopped: $p1\n";
  break;
…
case ControlStatus::Started:
  echo "Started: $p1\n";
  break;
}
```

## Iteration Statements

### General

**Syntax**

<pre>
  <i>iteration-statement:</i>
    <i>while-statement</i>
    <i>do-statement</i>
    <i>for-statement</i>
    <i>foreach-statement</i>
</pre>

**Defined elsewhere**

* [*do-statement*](#the-do-statement)
* [*for-statement*](#the-for-statement)
* [*foreach-statement*§](#the-foreach-statement)
* [*while-statement*](#the-while-statement)

### The `while` Statement

**Syntax**

<pre>
  <i>while-statement:</i>
    while  (  <i>expression</i>  )  <i>statement</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*statement*](#general)

**Constraints**

The controlling expression *expression* must have type `bool` or be
implicitly convertible to that type.

**Semantics**

If *expression* tests `true`, the *statement* that follows immediately is
executed, and the process is repeated. If *expression* tests `false`,
control transfers to the point immediately following the end of the
`while` statement. The loop body, *statement*, is executed zero or more
times.

**Examples**

```Hack
$i = 1;
while ($i <= 10) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
// -----------------------------------------
while (true) {
  …
  if ($done)
    break;  // break out of the while loop
  …
}
```

### The `do` Statement

**Syntax**

<pre>
  <i>do-statement:</i>
    do  <i>statement</i>  while  (  <i>expression</i>  )  ;
</pre>

**Defined elsewhere**

* [*statement*](#general)
* [*expresion*](10-expressions.md#yield-operator)

(Note: There is no `:/enddo` alternate syntax.)

**Constraints**

The controlling expression *expression* must have type `bool` or be
implicitly convertible to that type.

**Semantics**

First, *statement* is executed and then *expression* is tested. If its
value is `true`, the process is repeated. If *expression* tests `false`,
control transfers to the point immediately following the end of the `do`
statement. The loop body, *statement*, is executed one or more times.

**Examples**

```Hack
$i = 1;
do {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
while ($i <= 10);
```

### The `for` Statement

**Syntax**

<pre>
  <i>for-statement:</i>
    for   (   <i>for-initializeropt</i>   ;   <i>for-controlopt</i>   ;   <i>for-end-of-loopopt</i>   )   <i>statement</i>

  <i>for-initializer:</i>
    <i>for-expression-group</i>

  <i>for-control:</i>
    <i>for-expression-group</i>

  <i>for-end-of-loop:</i>
    <i>for-expression-group</i>

  <i>for-expression-group:</i>
    <i>expression</i>
    <i>for-expression-group</i>   ,   <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*statement*](#general)

Note: Unlike C/C++, Hack does not support a comma operator, per se.
However, the syntax for the `for` statement has been extended from that of
C/C++ to achieve the same results in this context.

**Constraints**

The controlling expression—the right-most *expression* in
*for-control*—must have type `bool` or be implicitly convertible to that
type.

**Semantics**

The group of expressions in *for-initializer* is evaluated once,
left-to-right, for their side effects. Then the group of expressions in
*for-control* is evaluated left-to-right (with all but the right-most
one for their side effects only), with the right-most expression's value
being tested. If that tests `true`, *statement* is executed, and the group
of expressions in *for-end-of-loop* is evaluated left-to-right, for
their side effects only. Then the process is repeated starting with
*for-control*. If the right-most expression in *for-control* tests
`false`, control transfers to the point immediately following the end of
the `for` statement. The loop body, *statement*, is executed zero or more
times.

If *for-initializer* is omitted, no action is taken at the start of the
loop processing. If *for-control* is omitted, this is treated as if
*for-control* was an expression with the value `true`. If
*for-end-of-loop* is omitted, no action is taken at the end of each
iteration.

**Examples**

```Hack
for ($i = 1; $i <= 10; ++$i) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
}
// -----------------------------------------
// omit 1st and 3rd expressions

$i = 1;
for (; $i <= 10;) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
// -----------------------------------------
// omit all 3 expressions

$i = 1;
for (;;) {
  if ($i > 10)
    break;
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
// -----------------------------------------
//  use groups of expressions

for ($a = 100, $i = 1; ++$i, $i <= 10; ++$i, $a -= 10) {
  echo "$i\t$a\n";
}
```

### The `foreach` Statement

**Syntax**

<pre>
  <i>foreach-statement:</i>
    foreach  (  <i>foreach-collection-name</i>  as  <i>foreach-key<sub>opt</sub>  foreach-value</i>  )   <i>statement</i>
    foreach  (  <i>foreach-collection-name</i>  await as  <i>foreach-key<sub>opt</sub>  foreach-value</i>  )   <i>statement</i>

  <i>foreach-collection-name</i>:
    <i>expression</i>

  <i>foreach-key:</i>
    <i>expression</i>  =>

  <i>foreach-value:<i>
    <i>expression</i>
    <i>list-intrinsic</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*list-intrinsic*](10-expressions.md#list)
* [*statement*](#general)

**Constraints**

The variable designated by *foreach-collection-name* must be a
collection.

Each *expression* must designate a variable name.

For the “await as” form, the type of *foreach-collection-name* must implement interface [`AsyncIterator`](17-interfaces.md#interface-AsyncIterator) or interface [`AsyncKeyedIterator`](17-interfaces.md#interface-AsyncKeyedIterator).

**Semantics**

The *foreach* statement iterates over the set of elements in the
collection designated by *foreach-collection-name*, starting at the
beginning, executing *statement* each iteration. On each iteration, the value of the current element is assigned to the
corresponding variable designated by *foreach-value*, provided *foreach-value*’s *expression* is not ([`$_`](§09-lexical-structure.md#names); otherwise, the value of the current element is ignored. The loop body, *statement*, is executed zero or
more times.

If *foreach-key* is present and its *expression* is `$_`, the current element's key value is ignored. If *foreach-key* is present and *expression* is not `$_`,  the variable designated by its *expression*
is assigned the current element's key value.

In the *list-intrinsic* case, a value that is an array is split into
individual elements.

**Examples**

```Hack
$colors = array("red", "white", "blue");
foreach ($colors as $color) {
   …
};
// -----------------------------------------
foreach ($colors as $key => $color) {
  …
}
// -----------------------------------------
// Modify the local copy of an element's value

foreach ($colors as $color) {
  $color = "black";
}
// -----------------------------------------
  $a = array('a' => 10, 'f' => 30);
  foreach ($a as $key => $_) { // 10 and 30 are ignored
    …
  }
// -----------------------------------------
async function countdown1(int $start): AsyncIterator<int> {
  for ($i = $start; $i >= 0; --$i) {
    await \HH\Asio\usleep(1000000); // Sleep for 1 second
    yield $i;
  }
}

async function use_countdown1(): Awaitable<void> {
  $async_gen = countdown1(3);
  foreach ($async_gen await as $value) {
    // $value is of type int here
    // …
  }
}

async function countdown2(int $start): AsyncKeyedIterator<int, string> {
  for ($i = $start; $i >= 0; --$i) {
    await \HH\Asio\usleep(1000000);
    yield $i => (string)$i;
  }
}

async function use_countdown2(): Awaitable<void> {
  foreach (countdown2(3) await as $num => $str) {
    // $num is of type int, $str is of type string
    // …
  }
}
```

## Jump Statements

### General

**Syntax**

<pre>
  <i>jump-statement:</i>
    <i>continue-statement</i>
    <i>break-statement</i>
    <i>return-statement</i>
    <i>throw-statement</i>
</pre>

**Defined elsewhere**

* [*break-statement*](#the-break-statement)
* [*continue-statement*](#the-continue-statement)
* [*return-statement*](#the-return-statement)
* [*throw-statement*](#the-throw-statement)

### The `continue` Statement

**Syntax**

<pre>
  <i>continue-statement:</i>
    continue  ;
</pre>

**Constraints**

A `continue` statement must not attempt to break out of a [finally-block](#the-try-statement).

**Semantics**

A `continue` statement terminates the execution of the innermost enclosing
[iteration](#iteration-statements) or [`switch`](#the-switch-statement) statement.

A `continue` statement may break out of a construct that is fully
contained within a finally-block.

**Examples**

```Hack
for ($i = 1; $i <= 5; ++$i) {
  if (($i % 2) == 0)
    continue;
  echo "$i is odd\n";
}
```

### The `break` Statement

**Syntax**

<pre>
  <i>break-statement:</i>
    break  ;
</pre>

**Constraints**

A `break` statement must not attempt to break out of a [finally-block](#the-try-statement).

**Semantics**

A `break` statement terminates the execution of one or more enclosing
[iteration](#iteration-statements) or [`switch`](#the-switch-statement) statements.

A `break` statement may break out of a construct that is fully contained
within a finally-block.

**Examples**

```Hack
$i = 1;
for (;;) {
  if ($i > 10)
    break;
  …
  ++$i;
}
```

### The `return` Statement

**Syntax**

<pre>
  <i>return-statement:</i>
    return  <i>expression<sub>opt</sub></i>  ;
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

The *expression* in a *return-statement* in a [generator function](10-expressions.md#yield-operator) must be the literal `null`.

A `return` statement must not occur in a [finally-block](11-statements.md#the-try-statement) or in the *compound-statement* of a *function-definition* for a function with [*return-type* `noreturn`](15-functions.md#function-definitions).

For a non-async function, the type of *expression* (or any implicitly returned `null`) must be assignment-compatible with the return type of the [enclosing function](15-functions.md#function-definitions). For an async function, the type of *expression* must be a subtype of the parameter type of the `Awaitable` *return-type* for the enclosing function. However, if `Awaitable`’s parameter type is `void`, *expression* must be omitted.

**Semantics**

A `return` statement from within a function terminates the execution of that function normally. If *expression* is omitted, for a non-async function, no value is returned. For a sync function, an object of type `Awaitable<void>` is returned. If *expression* is present, for a non-async function, the value of *expression* is returned by value. For a sync function, the value of expression is wrapped in an object of type `Awaitable<T>` (where `T` is the type of *expression*), which is returned.

If execution flows into the closing brace (`}`) of a function, `return;` is implied.

A function may have any number of `return` statements, whose returned
values may have different types.

A `return` statement is permitted in a [try-block](#the-try-statement) and a [catch-block](#the-try-statement).

Returning from a constructor or destructor behaves just like returning
from a function having a return type of `void`.

A `return` statement inside a generator function causes the generator to
terminate.

Return statements can be used in the body of anonymous functions.

**Examples**

```Hack
function f(): int { return 100; } // f explicitly returns a value
function h(): void { }    // h implicitly returns null
// -----------------------------------------
// j returns one of three dissimilarly-typed values
function j(int $x): mixed {
  if ($x > 0) {
    return "Positive";
  } else if ($x < 0) {
    return -1;
  }
  // for zero, implied return null
}
// -----------------------------------------
class Point {
  private static int $pointCount = 0;
  public static function getPointCount(): int {
    return self::$pointCount;
  }
  …
}
```

**Implementation Notes**

Although *expression* is a [full expression](10-expressions.md#general), and there is a
[sequence point](10-expressions.md#general) at the end of that expression, as stated in
[§§](10-expressions.md#general), a side effect need not be executed if it can be decided that
no other program code relies on its having happened. (For example, in
the cases of `return $a++;` and `return ++$a;`, it is obvious what value
must be returned in each case, but if `$a` is a variable local to the
enclosing function, `$a` need not actually be incremented.

### The `throw` Statement

**Syntax**

<pre>
  <i>throw-statement:</i>
    throw  <i>expression</i>  ;
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

The type of *expression* must be [[\Exception](19-exception-handling.md#class-exception) or a subclass of that
class.

*expression* must be such that an alias to it can be created. 

**Semantics**

A `throw` statement throws an exception immediately and unconditionally.
Control never reaches the statement immediately following the throw. See
[§§](19-exception-handling.md#general) and [§§](#the-try-statement) for more details of throwing and catching exceptions,
and how uncaught exceptions are dealt with.

Rather than handle an exception, a catch-block may (re-)throw the same
exception that it caught, or it can throw an exception of a different
type.

**Examples**

```Hack
throw new Exception;
throw new Exception("Some message", 123);
class MyException extends Exception { ... }
throw new MyException;
```

## The `try` Statement

**Syntax**

<pre>
  <i>try-statement:</i>
    try  <i>compound-statement   catch-clauses</i>
    try  <i>compound-statement   finally-clause</i>
    try  <i>compound-statement   catch-clauses   finally-clause</i>

  <i>catch-clauses:</i>
    <i>catch-clause</i>
    <i>catch-clauses   catch-clause</i>

  <i>catch-clause:</i>
    catch  (  <i>type-specifier</i>  <i>variable-name</i>  )  <i>compound-statement</i>

  <i>finally-clause:</i>
    finally   <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*compound-statement*](#compound-statements)
* [*type-specifier*](05-types.md#general).
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

In a *catch-clause* the type referred to by the *type-specifier* 
must be [`\Exception`](19-exception-handling.md#class-exception) or a type derived from
that class.

**Semantics**

In a *catch-clause*, the *variable-name* designates an *exception variable*
passed in by value. This variable corresponds to a local variable with a
scope that extends over the catch-block. During execution of the
catch-block, the exception variable represents the exception currently
being handled.

Once an exception is thrown, the Engine searches for the nearest
catch-block that can handle the exception. The process begins at the
current function level with a search for a try-block that lexically
encloses the throw point. All catch-blocks associated with that
try-block are considered in lexical order. If no catch-block is found
that can handle the run-time type of the exception, the function that
called the current function is searched for a lexically enclosing
try-block that encloses the call to the current function. This process
continues until a catch-block is found that can handle the current
exception. 

If a matching catch-block is located, the Engine prepares to transfer
control to the first statement of that catch-block. However, before
execution of that catch-block can start, the Engine first executes, in
order, any finally-blocks associated with try-blocks nested more deeply
than the one that caught the exception. 

If no matching catch-block is found, the behavior is
implementation-defined.

**Examples**

```Hack
function getTextLines(string $filename): Continuation<string> {
  $infile = fopen($filename, 'r');
  if ($infile == false) { /* deal with an file-open failure */ }
  try {
    while ($textLine = fgets($infile)) {  // while not EOF
      yield $textLine;  // leave line terminator attached
    }
  } finally {
    fclose($infile);
  }
}
// -----------------------------------------
class DeviceException extends Exception { … }
class DiskException extends DeviceException { … }
class RemovableDiskException extends DiskException { … }
class FloppyDiskException extends RemovableDiskException { … }

try {
  process(); // call a function that might generate a disk-related exception
}
catch (FloppyDiskException $fde) { … }
catch (RemovableDiskException $rde) { … }
catch (DiskException $de) { … }
catch (DeviceException $dve) { … }
finally { … }
```
