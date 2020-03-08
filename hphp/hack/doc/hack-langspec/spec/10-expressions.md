# Expressions

## General

An *expression* involves one or more terms and zero or more operators.

A *full expression* is an expression that is not part of another
expression.

A *value side effect* is an action that changes the state of the execution
environment. (Examples of such actions are modifying a variable, writing
to a device or file, or calling a function that performs such
operations.) Throughout this specification, this term is shortened to
*side effect*, which should not be confused with [*type side effect*](05-types.md#type-side-effects).

When an expression is evaluated, it produces a result. It might also
produce a side effect. Only a few operators produce side effects. (For
example, given the [expression statement](11-statements.md#expression-statements) `$v = 10`; the
expression 10 is evaluated to the result 10, and there is no side
effect. Then the assignment operator is executed, which results in the
side effect of `$v` being modified. The result of the whole expression is
the value of `$v` after the assignment has taken place. However, that
result is never used. Similarly, given the expression statement `++$v`;
the expression is evaluated to the result incremented-value-of-`$v`, and
the side effect is that `$v` is actually incremented. Again, the result
is never used.)

The occurrence of value computation and side effects is delimited by
*sequence points*, places in a program's execution at which all the
computations and side effects previously promised are complete, and no
computations or side effects of future operations have yet begun. There
is a sequence point at the end of each full expression. The [logical and](10-expressions.md#logical-and-operator), [logical or](10-expressions.md#logical-inclusive-or-operator), [conditional](10-expressions.md#conditional-operator), and [function-call](#function-call-operator) operators each contain a sequence point. (For example, in the
following series of expression statements, `$a = 10; ++$a; $b = $a;`,
there is sequence point at the end of each full expression, so the
assignment to $a is completed before `$a` is incremented, and the
increment is completed before the assignment to `$b`.)

When an expression contains multiple operators, the *precedence* of
those operators controls the order in which those operators are applied.
(For example, the expression `$a - $b / $c` is evaluated as
`$a - ($b / $c)` because the / operator has higher precedence than the
binary - operator.) The precedence of an operator is defined by the
definition of its associated grammar production.

If an operand occurs between two operators having the same precedence,
the order in which the operations are performed is defined by those
operators' *associativity*. With *left-associative* operators,
operations are performed left-to-right. (For example, `$a + $b - $c` is
evaluated as `($a + $b) - $c.`) With *right-associative* operators,
operations are performed right-to-left. (For example, `$a = $b = $c` is
evaluated as `$a = ($b = $c)`.)

Precedence and associativity can be controlled using *grouping
parentheses*. (For example, in the expression `($a - $b) / $c`, the
subtraction is done before the division. Without the grouping
parentheses, the division would take place first.)

While precedence, associativity, and grouping parentheses control the
order in which operators are applied, they do *not* control the order of
evaluation of the terms themselves. Unless stated explicitly in this
specification, the order in which the operands in an expression are
evaluated relative to each other is unspecified. See the discussion
above about the operators that contain sequence points. (For example, in
the full expression `$list1[$i] = $list2[$i++]`, whether the value
of `$i` on the left-hand side is the old or new `$i`, is unspecified.
Similarly, in the full expression `$j = $i + $i++`, whether the value
of `$i` is the old or new `$i`, is unspecified. Finally, in the full
expression `f() + g() * h()`, the order in which the three functions are
called, is unspecified.)

**Implementation Notes**

An expression that contains no side effects and whose resulting value is
not used need not be evaluated. For example, the expression statements
`6;, $i + 6;`, and `$i/$j`; are well formed, but they contain no side
effects and their results are not used.

A side effect need not be executed if it can be defined that no other
program code relies on its having happened. (For example, in the cases
of return `$a++`; and return `++$a`;, it is obvious what value must be
returned in each case, but if `$a` is a variable local to the enclosing
function, `$a` need not actually be incremented.

## Restrictions on Arithmetic Operations

No arithmetic operation can be performed on the value `null` or on a value of
type `bool`, `string` (not even if the string is numeric), or any nullable type (including nullable arithmetic types).

## Operations on Operands Having One or More Subtypes

None of the subclauses in this Expressions clause discuss the use of operands
of supertypes such as `num`, `arraykey`, or `?int`. Refer to [§§](05-types.md#type-side-effects) for a discussion of type side effects.

## Primary Expressions

### General

**Syntax**

<pre>
  <i>primary-expression:</i>
    <i>variable-name</i>
    <i>qualified-name</i>
    <i>literal</i>
    <i>const-expression</i>
    <i>intrinsic</i>
    <i>collection-literal</i>
    <i>tuple-literal</i>
    <i>shape-literal</i>
    <i>anonymous-function-creation-expression</i>
    <i>awaitable-creation-expression</i>
    (  <i>expression</i>  )
    $this
    $$
</pre>

**Defined elsewhere**

* [*anonymous-function-creation-expression*](10-expressions.md#anonymous-function-creation)
* [*awaitable-creation-expression*](10-expressions.md#async-blocks)
* [*collection-literal*](10-expressions.md#collection-literals)
* [*const-expression*](10-expressions.md#constant-expressions)
* [*expression*](10-expressions.md#yield-operator)
* [*intrinsic*](10-expressions.md#general-2)
* [*literal*](09-lexical-structure.md#general-2)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*shape-literal*](10-expressions.md#shape-literals)
* [*tuple-literal*](10-expressions.md#tuple-literals)
* [*variable-name*](09-lexical-structure.md#names)

**Semantics**

The type and value of a parenthesized expression are identical to those of
the un-parenthesized expression.

The variable `$this` is predefined inside any instance method,
constructor, or destructor when that method is called from within an object
context. `$this` is a [handle](05-types.md#general) that points to the calling
object or to the object being constructed. The type of `$this` is
[`this`](05-types.md#the-this-type). `$this` is a non-modifiable lvalue.

The *pipe variable* `$$` is predefined only within the
*coalesce-expression* of a
[*piped-expression*](10-expressions.md#pipe-operator). The type and value of
`$$` is the type and value of that *coalesce-expression*. `$$` is a
non-modifiable lvalue.

### Intrinsics

#### General

**Syntax**
<pre>
  <i>intrinsic:</i>
    <i>array-intrinsic</i>
    <i>echo-intrinsic</i>
    <i>exit-intrinsic</i>
    <i>invariant-intrinsic</i>
    <i>list-intrinsic</i>
</pre>

**Defined elsewhere**

* [*array-intrinsic*](10-expressions.md#array)
* [*echo-intrinsic*](10-expressions.md#echo)
* [*exit-intrinsic*](10-expressions.md#exit)
* [*invariant-intrinsic*](10-expressions.md#invariant)
* [*list-intrinsic*](10-expressions.md#list)

**Semantics**

The names in this series of subclauses are reserved and are
called *intrinsics*. These names are not keywords; nor are they functions.

Note: The initial Hack execution environment was built on top of that for PHP,
which has an intrinsic called `empty`. And even though an intrinsic by that
name is not supported by Hack strict mode, the case-indistinct name `empty` is
reserved in Hack as well.

#### array

**Syntax**

<pre>
  <i>array-intrinsic:</i>
    array ( <i>array-initializer<sub>opt</sub></i>  )
</pre>

**Defined elsewhere**

* [*array-initializer*](10-expressions.md#array-creation-operator)

**Semantics**

This intrinsic creates and initializes an array. It is equivalent to the
[array-creation operator `[]`](10-expressions.md#array-creation-operator).

#### echo

**Syntax**

<pre>
  <i>echo-intrinsic:</i>
    echo  <i>expression</i>
    echo  (  <i>expression</i>  )
    echo  <i>expression-list-two-or-more</i>

  <i>expression-list-two-or-more:</i>
    <i>expression</i>  ,  <i>expression</i>
    <i>expression-list-two-or-more</i>  ,  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

*expression* must not designate an array nor an instance of a type not
having a [`__toString` method](16-classes.md#method-__tostring).

**Semantics**

After converting each of its *expression*s' values to strings, if
necessary, `echo` concatenates them in lexical order, and writes the
resulting string to [`STDOUT`](05-types.md#resource-types).

For value substitution in string literals, see [§§](09-lexical-structure.md#double-quoted-string-literals) and
[§§](09-lexical-structure.md#heredoc-string-literals). For conversion to string, see [§§](08-conversions.md#converting-to-string-type).

**Examples**

```Hack
$v1 = true;
$v2 = 123;
echo  '>>' . $v1 . '|' . $v2 . "<<\n";    // outputs ">>1|123<<"
echo  '>>' , $v1 , '|' , $v2 , "<<\n";    // outputs ">>1|123<<"
echo ('>>' . $v1 . '|' . $v2 . "<<\n");   // outputs ">>1|123<<"
$v3 = "qqq{$v2}zzz";
echo "$v3\n";
```

#### exit

**Syntax**

<pre>
  <i>exit-intrinsic:</i>
    exit  <i>expression<sub>opt</sub></i>
    exit  (  <i>expression<sub>opt</sub></i>  )
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

When *expression* designates an integer, its value must be in the range
0–254.

**Semantics**

This intrinsic terminates the current script. If *expression* designates
a string, that string is written to [`STDOUT`](05-types.md#resource-types). If *expression*
designates an integer, that represents the script's *exit status code*.
Code 255 is reserved by Hack. Code 0 represents "success". The exit
status code is made available to the execution environment. If
*expression* is omitted or is a string, the exit status code is zero.
`exit` does not have a resulting value.

`exit` performs the following operations, in order:

-   Writes the optional string to `STDOUT`.
-   Calls any functions registered via the library function
    [`register_shutdown_function`](http://www.php.net/register_shutdown_function) in their order of registration.
-   Invokes [destructors](16-classes.md#destructors) for all remaining instances.

**Examples**

```Hack
exit ("Closing down");
exit (1);
exit;
```

#### invariant

**Syntax**

<pre>
  <i>invariant-intrinsic:</i>
    invariant  (  <i>condition</i>  ,  <i>format</i>  )
    invariant  (  <i>condition</i>  ,  <i>format</i>  ,  <i>values</i>  )
</pre>

**Constraints**

*condition* can be any expression allowed as the operand of the [`!` operator](10-expressions.md#unary-arithmetic-operators). *format* is a string that can contain text and/or optional
formatting information as understood by the library function [`sprintf`](http://www.php.net/sprintf).
The optional comma-separated list of values designated by *values* must match
the set of types expected by the optional formatting information inside
*format*.


**Semantics**

If *condition* tests true, the program continues execution; otherwise, the
library function [`invariant_violation`](http://www.php.net/invariant_violation) is called. That function does not
return; instead, it either throws an exception of type
`\HH\InvariantException`, or calls the handler previously registered by the
library function [`invariant_callback_register`](http://www.php.net/invariant_callback_register).

This intrinsic behaves like a function with a `void` return type. It is
intended to indicate a programmer error for a condition that should never
occur.

**Examples**

```Hack
invariant($interf instanceof B, "Object must have type B");
// -----------------------------------------
invariant(!is_null($p), "Value can't be null");
// -----------------------------------------
$max = 100;
invariant(!is_null($p) && $p <= $max, "Value %d must be <= %d", $p, $max);
```

#### list

**Syntax**

<pre>
  <i>list-intrinsic:</i>
    list  (  <i>list-expression-list<sub>opt</sub></i>  )

  <i>list-expression-list:</i>
    <i>expression</i>
    ,
    <i>list-expression-list</i>  ,  <i>expression<sub>opt</sub></i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

*list-intrinsic* must be used as the left-hand operand in a
[*simple-assignment-expression*](10-expressions.md#simple-assignment) of which the right-hand
operand must be an expression that designates a vector-like array or an instance of the class types `Vector`, `ImmVector`, or `Pair` (the
"source").

Each *expression* in *list-expression-list* must designate a
variable (the "target variable").

There must not be fewer element candidates in the source than there are target
variables.

Only the right-most *list-or-variable* can be omitted.


**Semantics**

This intrinsic assigns zero or more elements of the source to the
target variables. On success, it returns a copy of the source.

When the source is a vector-like array, the element having an `int` key of 0 is assigned to the first target
variable, the element having an `int` key of 1 is assigned to the second
target variable, and so on, until all target variables have been
assigned. Any elements having an `int` key outside the range 0–(*n*-1),
where *n* is the number of target variables, are ignored.

If ([`$_`](09-lexical-structure.md#names) is used as a target variable, the value of the corresponding source element is ignored; no assignment takes place. Multiple target variables in the same list-expression-list may be `$_`.

When the source is an instance of the classes `Vector`, `ImmVector`, or `Pair`, the
elements in the source are assigned to the target variables in lexical order,
until all target variables have been assigned.

If the source elements and the target variables overlap in any
way, the behavior is unspecified.

**Examples**

```Hack
// $min, $max, and $avg must be defined at this point
list($min, $max, $avg) = array(0, 100, 67);
  // $min is 0, $max is 100, $avg is 67
$a = array();
$v = list($a[0], $a[2], $a[4]) = array(50, 5100, 567);
  // $a[0] is 50, $a[2] is 5100, $a[4] is 567
list($min, $max, ) = array(10, 1100, 167);
  // $min is 10, $max is 1100
$v = Vector {1, 2, 3};
list($_, $b, $_) = $v; // $b is assigned 2; 1 and 3 are ignored
```

### Collection Literals

Note: The term *literal* as used here is a misnomer; *cl-element-keys* and
*cl-element-values* need not be compile-time constants.

**Syntax**
<pre>
<i>collection-literal:</i>
  <i>non-key-collection-class-type</i>  {  <i>cl-initializer-list-without-keys<sub>opt</sub></i>  }
  <i>key-collection-class-type</i>  {  <i>cl-initializer-list-with-keys<sub>opt</sub></i>  }
  <i>pair-type</i>  {  <i>cl-element-value</i>  ,  <i>cl-element-value</i>  }
<i>non-key-collection-class-type:</i>
  <i>qualified-name</i>
<i>key-collection-class-type:</i>
  <i>qualified-name</i>
<i>pair-type:</i>
  <i>qualified-name</i>
<i>cl-initializer-list-without-keys:</i>
  <i>cl-element-value</i>
  <i>cl-initializer-list-without-keys</i>  ,  <i>cl-element-value</i>
<i>cl-initializer-list-with-keys:</i>
  <i>cl-element-key</i>  =>  <i>cl-element-value</i>
  <i>cl-initializer-list-with-keys</i>  ,  <i>cl-element-key</i>  =>  <i>cl-element-value</i>
<i>cl-element-key:</i>
  <i>expression</i>
<i>cl-element-value:</i>
  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

For *key-collection-class-type*, *qualified-name* must designate the library
type `Map` or `ImmMap`, and in both cases, each *cl-element-key* must have
type `int` or `string`.

For *key-collection-class-type*, *qualified-name* must designate the library
type `Vector`, `ImmVector`, `Set`, or `ImmSet`, and in all such cases, each
*cl-element-value* must have type `int` or `string`.

For *pair-type*, *qualified-name* must designate the library type `Pair`.

**Semantics**

For *non-key-collection-class-types* `Vector` and `ImmVector`, an instance of
the corresponding class is created with elements having values as specified by
*cl-initializer-list-without-keys*, inserted in that order, and assigned
consecutive keys starting at zero. If *cl-initializer-list-without-keys* is
omitted, the resulting vector is empty.

For *non-key-collection-class-types* `Map` and `ImmMap`, an instance of the
corresponding class is created with elements having keys and values as
specified by *cl-initializer-list-with-keys*, inserted in that order. If
*cl-initializer-list-with-keys* is omitted, the resulting map is empty. If two
or more *cl-element-keys* in a *cl-initializer-list-with-keys* contain the
same key, the lexically right-most one is the one whose *cl-element-value* is
used to initialize the element designated by that key.

For *non-key-collection-class-types* `Set` and `ImmSet`, an instance of the
corresponding class is created with elements having values as specified by
*cl-initializer-list-without-keys*, inserted in that order. If
*cl-initializer-list-without-keys* is omitted, the resulting set is empty.
Duplicate *cl-element-values* are ignored.

For type `Pair`, an instance of that class is created with element 0 having
the value of the left-hand *cl-element-value*, and element 1 having the value
of the right-hand *cl-element-value*.

**Examples**

```Hack
Vector {22, 33}                                 // size 2; 22, 33
(Vector {})->addAll(array(3, 6, 9))             // size 0 then size 3; 3, 6, 9
ImmVector {5, $x, 15}                           // size 3; 5, ?, 15
Map {'x' => -1, 'a' => -4, 'x' => 5, 'a' => 12} // size 2; 'x'/5, 'a'/12
ImmSet {1, 1, 1, 5, 10, 1, 'red', 1}            // size 4; 1, 5, 10, 'red'
Pair {55, new C()}
```

### Tuple Literals

Note: The term *literal* as used here is a misnomer; the *expression*s in
*expression-list* need not be compile-time constants.

**Syntax**

<pre>
<i>tuple-literal:</i>
  tuple  (  <i>expression-list-one-or-more</i>  )
<i>expression-list-one-or-more:</i>
  <i>expression</i>
  <i>expression-list-one-or-more</i>  ,  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Semantics**

A *tuple-literal* creates a [tuple](05-types.md#tuple-types) with elements having values as
specified by *expression-list-one-or-more*, inserted in that order.

The type of a *tuple-literal* is "tuple of type
<*element type list in lexical order*>".

Note: Although a tuple of only one element can be created using a tuple
literal, a [*tuple-type-specifier*](05-types.md#tuple-types) must contain at least two elements.

**Examples**

```Hack
return tuple(true, array(99, 88, 77), 10.5);
$t1 = tuple(10, true, 2.3, 'abc', null, $p1, Vector {$p2 + 3, 12}, new C());
$t2 = tuple(100, tuple('abc', false));
```

### Shape Literals

Note: The term *literal* as used here is a misnomer; the *expression*s
in *field-initializer* need not be compile-time constants.

**Syntax**
<pre>
  <i>shape-literal:</i>
    <i>shape  (  <i>field-initializer-list<sub>opt</sub></i>  )</i>
  <i>field-initializer-list:</i>
    <i>field-initializers</i>  ,<sub>opt</sub>
  <i>field-initializers</i>:
    <i>field-initializer</i>
    <i>field-initializers</i>  ,  <i>field-initializer</i>
  <i>field-initializer:</i>
    <i>single-quoted-string-literal</i>  =>  <i>expression</i>
    <i>integer-literal</i>  =>  <i>expression</i>
    <i>qualified-name</i>  =>  <i>expression</i>
    <i>scope-resolution-expression</i>  =>  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*integer-literal*](09-lexical-structure.md#integer-literals)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*scope-resolution-expression*](10-expressions.md#scope-resolution-operator)
* [*single-quoted-string-literal*](09-lexical-structure.md#single-quoted-string-literals)

**Constraints**

Each string in the set of strings designated by all the
*single-quoted-string-literals*, *qualified-names* and *scope-resolution-expressions* in a
*field-initializer-list* must have a distinct value, and each string must
match exactly a field name in the shape type's [*shape-specifier*](05-types.md#shape-types).

Each integer in the set of *integer-literals*, *qualified-names* and *scope-resolution-expressions* in a
*field-initializer-list* must have a distinct value, and each integer must
match exactly a field name in the shape type's *shape-specifier*.

The number of *field-initializers* must match exactly the number of
*field-specifiers* in the shape type's *shape-specifier*.

The type of *expression* in a *field-initializer* must be a subtype of the
corresponding field type in the shape type's *shape-specifier*.

**Semantics**

A *shape-literal* creates a [shape](05-types.md#shape-types) with fields having values as specified by *field-initializer-list*. The order of the
*field-initializers* need not be the same as the order of the
*field-specifiers* in the shape type's *shape-specifier*.

**Examples**

```Hack
shape()
shape('x' => $prevX, 'y' => getY())
shape('id' => null, 'url' => null, 'count' => 0)
```

### Anonymous Function-Creation

**Syntax**

<pre>
<i>anonymous-function-creation-expression:</i>
  async<sub>opt</sub>  function  (  <i>anonymous-function-parameter-list<sub>opt<sub></i>  )  <i>anonymous-function-return<sub>opt</sub></i>  <i>anonymous-function-use-clause<sub>opt</sub></i>  <i>compound-statement</i>

<i>anonymous-function-parameter-list:</i>
  ...
  <i>anonymous-function-parameter-declaration-list</i>
  <i>anonymous-function-parameter-declaration-list</i>  ,
  <i>anonymous-function-parameter-declaration-list</i>  ,  ...

<i>anonymous-function-parameter-declaration-list:</i>
  <i>anonymous-function-parameter-declaration</i>
  <i>anonymous-function-parameter-declaration-list  </i>  ,  <i>anonymous-function-parameter-declaration</i>

<i>anonymous-function-parameter-declaration:</i>
  <i>attribute-specification<sub>opt</sub>  type-specifier<sub>opt</sub> variable-name  default-argument-specifier<sub>opt</sub></i>

<i>anonymous-function-return:</i>
  : <i>return-type</i>

<i>anonymous-function-use-clause:</i>
  use  (  <i>use-variable-name-list</i>  ,<sub>opt</sub> )

<i>use-variable-name-list:</i>
  <i>variable-name</i>
  <i>use-variable-name-list</i>  ,  <i>variable-name</i>
</pre>

**Defined elsewhere**

* [*attribute-specification*](20-namespaces.md#name-lookup)
* [*compound-statement*](11-statements.md#compound-statements)
* [*default-argument-specifier*](15-functions.md#function-definitions)
* [*return-type*](15-functions.md#function-definitions)
* [*type-specifier*](05-types.md#general)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

Each *variable-name* in an *anonymous-function-parameter-declaration-list* must be distinct.

If any *anonymous-function-parameter-declaration* has a
*default-argument-specifier*, then all subsequent
*anonymous-function-parameter-declarations* in the same
*anonymous-function-parameter-declaration-list* must also have a
*default-argument-specifier*.

If the *type-specifier* in *anonymous-function-return* is `void`, the
*compound-statement* must not contain any [`return` statements](11-statements.md#the-return-statement)
having an *expression*. Otherwise, all `return` statements must contain an
*expression* whose type is a subtype of the type indicated by *type-specifier*.

If `async` is present, *return-type* must be a type that implements
[`Awaitable<T>`](17-interfaces.md#interface-awaitable).

**Semantics**

This operator returns an object that encapsulates the [anonymous function](15-functions.md#anonymous-functions) defined within. An anonymous function is defined like, and behaves
like, a [named function](13-functions.md#function-definitions) except
that the former has no name and has an optional
*anonymous-function-use-clause*.

The *use-variable-name-list* is a list of variables from the enclosing
scope, which are to be made available by name to the body of the
anonymous function. The values used for these variables are those at the time the closure object is created, not when it is used to call the function it
encapsulates.

An anonymous function defined inside an instance method has access to
the variable `$this`.

If the *type-specifier* for a parameter is omitted, that type is inferred.

If *anonymous-function-return* is omitted, the return type is inferred.

An anonymous function can be [asynchronous](15-functions.md#asynchronous-functions).

The function-return types `this` and `noreturn` are described in ([§§](15-functions.md#function-definitions)).

**Examples**

```Hack
function doit(int $value, (function (int): int) $process): int {
  return $process($value);
}
$result = doit(5, function (int $p): int { return $p * 2; }); // doubles
$result = doit(5, function (int $p): int { return $p * $p; });  // squares
// -----------------------------------------
function compute(array<int> $values): void {
  $count = 5;
  $callback = function () use ($count)
  {
    …
  };
  $callback();
  …
}
```

### Async Blocks

**Syntax**

<pre>
<i>awaitable-creation-expression:</i>
  async   {   <i>async-statement-list<sub>opt</sub></i>   }

<i>async-statement-list:</i>
  <i>statement</i>
  <i>async-statement-list   statement</i>
</pre>

**Defined elsewhere**

* [*statement*](11-statements.md#general)

**Constraints**

*awaitable-creation-expression* must not be used as the *lambda-body* in a [*lambda-expression*](10-expressions.md#lambda-expressions).

**Semantics**

The (possibly) asynchronous operations designated by *async-statement-list* are executed, in order.

An *awaitable-creation-expression* produces a result of type [`Awaitable<T>`](17-interfaces.md#interface-awaitable), where `T` is the return type of the final statement in *async-statement-list*. If *async-statement-list* is omitted, or its final statement is `return;`, or its final statement is not a `return` statement, the final statement is treated as being `return;`, `T` is `void`, and no value is wrapped into the `Awaitable` object. Otherwise, the final statement has the form `return` *expression*`;`, `T` is the type of *expression*, and the value of *expression* is wrapped into the `Awaitable` object.

**Examples**

```Hack
$x = await async {
  $y = await task1();
  $z = await task2();
  return $y + $z;
};
```

## Postfix Operators

### General

**Syntax**

<pre>
  <i>postfix-expression:</i>
    <i>primary-expression</i>
    <i>clone-expression</i>
    <i>object-creation-expression</i>
    <i>array-creation-expression</i>
    <i>subscript-expression</i>
    <i>function-call-expression</i>
    <i>member-selection-expression</i>
    <i>null-safe-member-selection-expression</i>
    <i>postfix-increment-expression</i>
    <i>postfix-decrement-expression</i>
    <i>scope-resolution-expression</i>
    <i>exponentiation-expression</i>
</pre>

**Defined elsewhere**

* [*array-creation-expression*](10-expressions.md#array-creation-operator)
* [*clone-expression*](10-expressions.md#the-clone-operator)
* [*exponentiation-expression*](10-expressions.md#exponentiation-operator)
* [*function-call-expression*](10-expressions.md#function-call-operator)
* [*member-selection-expression*](10-expressions.md#member-selection-operator)
* [*null-safe-member-selection-expression*](10-expressions.md#null-safe-member-selection-operator)
* [*object-creation-expression*](10-expressions.md#the-new-operator)
* [*postfix-decrement-expression*](10-expressions.md#postfix-increment-and-decrement-operators)
* [*postfix-increment-expression*](10-expressions.md#postfix-increment-and-decrement-operators)
* [*primary-expression*](10-expressions.md#general-1)
* [*scope-resolution-expression*](10-expressions.md#scope-resolution-operator)
* [*subscript-expression*](10-expressions.md#subscript-operator)

**Semantics**

These operators associate left-to-right.

### The `clone` Operator

**Syntax**

<pre>
  <i>clone-expression:</i>
    clone  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

*expression* must designate an object.

**Semantics**

The `clone` operator creates a new object that is a [shallow copy](04-basic-concepts.md#cloning-objects) of the object designated by *expression*. Then, if the class type of *expression* has a method called [`__clone`](16-classes.md#method-__clone), that is called to perform a [deep copy](04-basic-concepts.md#cloning-objects). The result is a handle that points to the new object.

**Examples**

Consider a class `Employee`, from which is derived a class `Manager`. Let us
assume that both classes contain properties that are objects. clone is
used to make a copy of a Manager object, and behind the scenes, the
`Manager` object uses clone to copy the properties for the base class,
`Employee`.

```Hack
class Employee
{
  ...
  public function __clone(): void  {
    // make a deep copy of Employee object
  }
}
class Manager extends Employee
{
  ...
  public function __clone(): void
  {
    $v = parent::__clone();
    // make a deep copy of Manager object

  }
}
$obj1 = new Manager("Smith", 23);
$obj2 = clone $obj1;  // creates a new Manager that is a deep copy
```

### The `new` Operator

**Syntax**

<pre>
  <i>object-creation-expression:</i>
    new  <i>class-type-designator</i>  (  <i>argument-expression-list<sub>opt</sub></i>  )

  <i>class-type-designator:</i>
    parent
    self
    static
    <i>member-selection-expression<i>
    <i>null-safe-member-selection-expression<i>
    <i>qualified-name</i>
    <i>scope-resolution-expression</i>
    <i>subscript-expression<i>
    <i>variable-name</i>
</pre>

**Defined elsewhere**

* [*argument-expression-list*](10-expressions.md#function-call-operator)
* [*member-selection-expression*](10-expressions.md#member-selection-operator)
* [*null-safe-member-selection-expression*](10-expressions.md#null-safe-member-selection-operator)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*scope-resolution-expression*](10-expressions.md#scope-resolution-operator)
* [*subscript-expression*](10-expressions.md#subscript-operator)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

If the *class-type-designator* is a *scope-resolution-expression* then it must not have `class` as the right hand side of the `::` operator.

Otherwise, if the *class-type-designator* is a *qualified-name* or *scope-resolution-expression* which resolves a qualified name, or `self`, or `parent`, then it must designate a class.

Otherwise,the *class-type-designator* must be an expression evaluating to a value having the [`classname` type](05-types.md#the-classname-type).  Furthermore, it must designate a class that has the attribute  [`__ConsistentConstruct`](21-attributes.md#attribute-__consistentconstruct), or that has an abstract constructor or a final constructor.

The *class-type-designator* must not designate an [abstract class](16-classes.md#general).

The *class-type-designator* must not be a [generic type parameter](14-generic-types-methods-and-functions.md#type-parameters).

The *object-creation-expression* will invoke the constructor of the class designated by the *class-type-designator*. 

*argument-expression-list* must contain an argument for each parameter in the
[constructor's definition](15-functions.md#function-definitions) not having a default value, and each
argument's type must be a subtype of the corresponding parameter's type.

If the constructor is not variadic, the call must not contain more arguments
than there are corresponding parameters.

**Semantics**

The `new` operator allocates memory for an object that is an instance of
the class specified by the *class-type-designator*.

The object is initialized by calling the class's constructor (16.8)
passing it the optional *argument-expression-list*. If the class has no
constructor, the constructor that class inherits (if any) is used.
Otherwise, each instance property having any nullable type takes on the value
`null`.

The result of an *object-creation-expression* is a handle to an object
of the type specified by the *class-type-designator*.

From within a method, the use of `static` corresponds to the class in the
inheritance context in which the method is called. The type of the object created by an expression of the form `new static` is [`this`](05-types.md#the-this-type).

Because a constructor call is a function call, the relevant parts of
10.5.6 also apply.

**Examples**

```Hack
class Point
{
  public function __construct(float $x = 0, float $y = 0)
  {
    ...
  }
  ...
}
$p1 = new Point();     // create Point(0, 0)
$p1 = new Point(12);   // create Point(12, 0)
// -----------------------------------------
class C { ... }
function f(classname<C> $clsname): void {
  $w = new $clsname();
  …
}
```

### Array Creation Operator

An array is created and initialized by one of two equivalent ways: via
the array-creation operator `[]`, as described below, or the [intrinsic
`array`](10-expressions.md#array).

**Syntax**

<pre>
  <i>array-creation-expression:</i>
    array  (  <i>array-initializer<sub>opt</sub></i>  )
    [ <i>array-initializer<sub>opt</sub></i> ]

  <i>array-initializer:</i>
    <i>array-initializer-list</i>  ,<sub>opt</sub>

  <i>array-initializer-list:</i>
    <i>array-element-initializer</i>
    <i>array-element-initializer  ,  array-initializer-list</i>

  <i>array-element-initializer:</i>
    <i>element-value</i>
    element-key  =>  <i>element-value</i>

  <i>element-key:</i>
    <i>expression</i>

  <i>element-value</i>
    <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

If any *array-element-initializer* in an *array-initializer-list* contains
an *element-key*, then all *array-element-initializers* in that
*array-initializer-list* must contain an *element-key*.

**Semantics**

This operator creates an array. If *array-initializer* contains any
*element-keys*, the resulting array is a map-like array; otherwise, it is a
vector-like array. If *array-initializer* is omitted, the array has zero elements, and the resulting array is neither a vector-like nor a map-like
array, although it is a subtype of both types.. For convenience, an
*array-initializer* may have a trailing comma; however, this comma has no
purpose. An *array-initializer-list* consists of a vector-like array. If
*array-initializer* is omitted, the array has zero elements. For convenience,
an *array-initializer* may have a trailing comma; however, this comma has no
purpose. An *array-initializer-list* consists of a comma-separated list of
one or more *array-element-initializer*s, each of which is used to provide an
*element-value* and an optional *element-key*.

If the value of *element-key* is neither `int` nor `string`, keys with `float`
or `bool` values, or strings whose contents match exactly the pattern of
[*decimal-literal*](09-lexical-structure.md#integer-literals), are [converted to `int`](08-conversions.md#converting-to-integer-type), and values
of all other key types are [converted to `string`](08-conversions.md#converting-to-string-type).

If *element-key* is omitted from an *array-element-initializer*, an
element key of type `int` is associated with the corresponding
*element-value*. The key associated is one more than the previously
assigned `int` key for this array. However, if this is the first element
with an `int` key, key zero is associated.

Once the element keys have been converted to `int` or `string`, if two or more
*array-element-initializer*s in an *array-initializer* contain the same
key, the lexically right-most one is the one whose element-value is used
to initialize that element.

The result of this operator is a handle to the set of array elements.

**Examples**

```Hack
$v = [];      // array has 0 elements
$v = array(true);   // vector-like array has 1 element, true
$v = [123, -56];  // vector-like array of two ints, with keys 0 and 1
$v = [0 => 123, 1 => -56]; // map-like array of two ints, with keys 0 and 1
$i = 10;
$v = [$i - 10 => 123, $i - 9 => -56]; // key can be a runtime expression
$i = 6; $j = 12;
$v = [7 => 123, 3 => $i, 6 => ++$j];  // keys are in arbitrary order
$v[4] = 99;   // extends array with a new element
$v = [2 => 23, 1 => 10, 2 => 46, 1.9 => 6];
     // map-like array has 2 elements, with keys 2 and 1, values 46 and 6
$v = ["red" => 10, "4" => 3, 9.2 => 5, "12.8" => 111, null => 1];
     // map-like array has 5 elements, with keys "red", 4, 9, "12.8", and "".
$c = array("red", "white", "blue");
$v = array(10, $c, null, array(false, null, $c));
$v = array(2 => true, 0 => 123, 1 => 34.5, -1 => "red");
foreach($v as $e) { … } // iterates over keys 2, 0, 1, -1
for ($i = -1; $i <= 2; ++$i) { … $v[$i] } // retrieves via keys -1, 0, 1, 2
```

### Subscript Operator

**Syntax**

<pre>
  <i>subscript-expression:</i>
    <i>postfix-expression</i>  [  <i>expression<sub>opt</sub></i>  ]
    <i>postfix-expression</i>  {  <i>expression<sub>opt</sub></i>  }   <b>[Deprecated form]</b>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*postfix-expression*](10-expressions.md#general-3)

**Constraints**

If *postfix-expression* designates a string, *expression* must not
designate a string.

*expression* can be omitted only if *subscript-expression* is used in a
modifiable-lvalue context and *postfix-expression* does not designate a
string.

If *subscript-expression* is used in a non-lvalue context, the element
being designated must exist.

When *postfix-expression* designates a vector-like array, *expression* must
have type `int`.

When *postfix-expression* designates a map-like array, elements cannot be appended using empty `[]`.

When *postfix-expression* designates a tuple, *expression* must be a constant.

When *postfix-expression* designates a shape, *expression* must be a
[*single-quoted-string-literal*](09-lexical-structure.md#single-quoted-string-literals) that specifies a key in that
shape's [*shape-specifier*](05-types.md#shape-types).

When postfix-expression designates an instance of a collection class:

* The deprecated form, `{ … }`, is not supported.
* `Vector` or `ImmVector`, *expression* must have type `int`.
* `Vector`, if *expression* is omitted, *subscript-expression* must be the
left-hand side of a [*simple-assignment-expression*](10-expressions.md#simple-assignment).
* `Map` or `ImmMap`, *expression* must have type `int` or `string`.
* `Map`, if *expression* is omitted, *subscript-expression* must be the
left-hand side of a *simple-assignment-expression* whose right-hand operand
has type `Pair`.
* `Set` or `ImmSet`, subscripting is not permitted.
* `Pair`, *expression* must be either the literal 0 or 1.


**Semantics**

A *subscript-expression* designates a (possibly non-existent) element of
an array, a string, a vector, a map, or a Pair. When *subscript-expression* designates an object of
a type that implements [`ArrayAccess`](17-interfaces.md#interface-arrayaccess), the minimal semantics are
defined below; however, they can be augmented by that object's methods
[`offsetGet`](17-interfaces.md#interface-arrayaccess) and [`offsetSet`](17-interfaces.md#interface-arrayaccess).

The element key is designated by *expression*. If the value of
*element-key* is neither `int` nor `string`, keys with `float` or `bool` values,
or strings whose contents match exactly the pattern of [*decimal-literal*](09-lexical-structure.md#integer-literals), are [converted to `int`](08-conversions.md#converting-to-integer-type), and values of all other key
types are [converted to `string`](08-conversions.md#converting-to-string-type).

If both *postfix-expression* and *expression* designate strings,
*expression* is treated as if it specified the `int` key zero instead.

A *subscript-expression* designates a modifiable lvalue if and only if
*postfix-expression* designates a modifiable lvalue.

*postfix-expression designates an array*

If *expression* is present, if the designated element exists, the type
and value of the result is the type and value of that element;
otherwise, the result is `null`.

If *expression* is omitted, a new element is inserted. Its key has type
`int` and is one more than the highest, previously assigned, non-negative
`int` key for this array. If this is the first element with a non-negative
`int` key, key zero is used. However, if the highest, previously assigned
`int` key for this array is [`PHP_INT_MAX`](06-constants.md#core-predefined-constants), **no new element is
inserted**. The type and value of the result is the type and value of
the new element.

* If the usage context is as the left-hand side of a [*simple-assignment-expression*](10-expressions.md#simple-assignment): The value of the new element is the value of the right-hand side of that *simple-assignment-expression*.
* If the usage context is as the left-hand side of a [*compound-assignment-expression*](10-expressions.md#compound-assignment): The expression `e1 op= e2` is evaluated as `e1 = null op (e2)`.
* If the usage context is as the operand of a [postfix-](10-expressions.md#postfix-increment-and-decrement-operators) or [prefix-increment or decrement operator](10-expressions.md#prefix-increment-and-decrement-operators): The value of the new element is `null`.

*postfix-expression designates a string*

If the designated element exists, the type and value of the result is
the type and value of that element; otherwise, the result is an empty
string.

*postfix-expression designates a vector*

For a `Vector` or `ImmVector`, if *expression* is present, if the designated
element exists, the type and value of the result is the type and value of that
element; otherwise, an exception of type `\OutOfBoundsException` is thrown.

For a `Vector`, if *expression* is omitted, a new element is inserted whose
value is that of the right-hand side of the *simple-assignment-expression*.
Its key has type `int` and is one more than the highest, previously assigned,
`int` key for this `Vector`. If this is the first element, key zero is used.
The type and value of the result is the type and value of the new element.

*postfix-expression designates a map*

For a `Map` or `ImmMap`, if *expression* is present, if the designated element
exists, the type and value of the result is the type and value of that
element; otherwise, an exception of type `\OutOfBoundsException` is thrown.

For a `Map`, if *expression* is omitted, the contents of the `Pair` right-hand
operand of the *simple-assignment-expression* for which this
*subscript-expression* is the left operand, is examined. Element 0 of that
Pair represents the key while element 1 represents the value. If the Map
already contains an element having that key, that element's value is changed
to the value in the Pair; otherwise, a new element is inserted in the Map with
the key and value from the Pair. The type and value of the result is the type
and value of the modified or new element.

*postfix-expression designates a Pair*

If *expression* is the literal 0, the type and value of the result is the type
and value of the first element in that `Pair`.

If *expression* is the literal 1, the type and value of the result is the type
and value of the second element in that `Pair`.

*postfix-expression designates an object of a type that implements*
`ArrayAccess`

If *expression* is present,

* If *subscript-expression* is used in a non-lvalue context, the object's method `offsetGet` is called with an argument of *expression*. The type and value of the result is the type and value returned by `offsetGet`.
* If the usage context is as the left-hand side of a *simple-assignment-expression*: The object's method `offsetSet` is called with a first argument of *expression* and a second argument that is the value of the right-hand side of that *simple-assignment-expression*. The type and value of the result is the type and value of the right-hand side of that *simple-assignment-expression*.
* If the usage context is as the left-hand side of a *compound-assignment-expression*: The expression `e1 op= e2` is evaluated as `e1 = offsetGet(expression) op (e2)`, which is then processed according to the rules for simple assignment immediately above.
* If the usage context is as the operand of a [postfix-](10-expressions.md#postfix-increment-and-decrement-operators) or [prefix-increment or decrement operator](10-expressions.md#prefix-increment-and-decrement-operators): The object's method `offsetGet` is called with an argument of *expression*. However, this method has no way of knowing if an increment or decrement operator was used, or whether it was a prefix or postfix operator. The type and value of the result is the type and value returned by `offsetGet`.

If *expression* is omitted, 

* If the usage context is as the left-hand side of a *simple-assignment-expression*: The object's method [`offsetSet`](15-interfaces.md#interface-arrayaccess) is called with a first argument of `null` and a second argument that is the value of the right-hand side of that *simple-assignment-expression*. The type and value of the result is the type and value of the right-hand side of that *simple-assignment-expression*.
* If the usage context is as the left-hand side of a *compound-assignment-expression*: The expression `e1 op= e2` is evaluated as `e1 = offsetGet(null) op (e2)`, which is then processed according to the rules for simple assignment immediately above.
* If the usage context is as the operand of a [postfix-](10-expressions.md#postfix-increment-and-decrement-operators) or [prefix-increment or decrement operator](10-expressions.md#prefix-increment-and-decrement-operators): The object's method `offsetGet` is called with an argument of `null`. However, this method has no way of knowing if an increment or decrement operator was used, or whether it was a prefix or postfix operator. The type and value of the result is the type and value returned by `offsetGet`.

Note: The brace (`{...}`) form of this operator has been deprecated. 

**Examples**

```Hack
$v = array(10, 20, 30);
$v[1] = 1.234;    // change the value (and type) of element [1]
$v[-10] = 19;     // insert a new element with int key -10
$v["red"] = true; // insert a new element with string key "red"
[[2,4,6,8], [5,10], [100,200,300]][0][2]  // designates element with value 6
["black", "white", "yellow"][1][2]  // designates substring "i" in "white"
function f(): array<int> { return [1000, 2000, 3000]; }
f()[2]           // designates element with value 3000
"red"[1.9]       // designates [1]
"red"[0][0][0]   // designates [0]
// -----------------------------------------
class MyVector implements ArrayAccess { … }
$vect1 = new MyVector(array(10, 'A' => 2.3, "up"));
$vect1[10] = 987; // calls MyVector::offsetSet(10, 987)
$vect1[] = "xxx"; // calls MyVector::offsetSet(null, "xxx")
$x = $vect1[1];   // calls MyVector::offsetGet(1)
// -----------------------------------------
$v1 = Vector {5, 10, 15};
$v1[] = 20;       // add a new element with value 20 to the end
$v1[0] = -5;      // change the value of existing element 0 to -5
// -----------------------------------------
$m1 = Map {'red' => 5, 'green' => 12};
$m1['blue'] = 35;     // add an element with a new key
$m1['red'] = 6;       // change the value of an existing element
$m1[] = Pair {'black', 43};   // append value 43 with key 'black'
$m1[] = Pair {'red', 123};    // replaces existing element's value with 123
// -----------------------------------------
$p1 = Pair {55, 'auto'};
echo "\$p1[0] = " . $p1[0] . "\n";  // outputs '$p1[0] = 55'
```

### Function Call Operator

**Syntax**

<pre>
  <i>function-call-expression:</i>
    <i>postfix-expression</i>  (  <i>argument-expression-list<sub>opt</sub></i>  )

  <i>argument-expression-list:</i>
    <i>argument-expressions</i>  ,<sub>opt</sub>

  <i>argument-expressions:</i>
    <i>expression</i>
    <i>argument-expressions</i>  ,  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*postfix-expression*](10-expressions.md#general-3)


**Constraints**

*postfix-expression* must designate a function, by *name*, be a variable of
[closure type](05-types.md#closure-types).

The function call must contain an argument for each parameter in the called
[function's definition](15-functions.md#function-definitions) not having a default value, and the argument
type must be a subtype of the parameter type.

If the called function is not variadic, the function call must not contain
more arguments than there are corresponding parameters.

**Semantics**

If *postfix-expression* is a [*null-safe-member-selection-expression*](10-expressions.md#null-safe-member-selection-operator), special handling occurs; see later below.

An expression of the form *function-call-expression* is a *function
call*. The postfix expression designates the *called function*, and
*argument-expression-list* specifies the arguments to be passed to that
function. Each argument corresponds to a parameter or the optional ellipsis in
the called function's definition. An argument can have any type. In a function
call, *postfix-expression* is evaluated first, followed by each
*assignment-expression* in the order left-to-right. There is a [sequence
point](#general) right before the function is called. For details of the
type and value of a function call see [§§](11-statements.md#the-return-statement). The value of a function
call, if any, is a non-modifiable lvalue.

If the called function is variadic, the function call can have any number of
arguments, provided the function call has at least an argument for each
parameter not having a default value.

When an argument corresponds to the ellipsis in the called function's
definition, the argument can have any type.

When *postfix-expression* designates an instance method or constructor,
the instance used in that designation is used as the value of `$this` in
the invoked method or constructor. However, if no instance was used in
that designation (for example, in the call `C::instance_method()`) the
invoked instance has no `$this` defined.

When a function is called, the value of each argument passed to it is
assigned to the corresponding parameter in that function's definition,
if such a parameter exists. The assignment of argument values to
parameters is defined in terms of [simple assignment](10-expressions.md#simple-assignment). Any parameters having a
default value but no corresponding argument, takes on that default value.

If an undefined variable is passed using byRef, that variable becomes
defined, with a default value of `null`.

Direct and indirect recursive function calls are permitted.

The following discussion applies when *postfix-expression* is a
*null-safe-member-selection-expression*: If *postfix-expression* is not
`null`, the behavior is the same as if a [*member-selection-expression*](10-expressions.md#member-selection-operator) were used instead of a *null-safe-member-selection-expression*.
Otherwise, no function is called, and the *function-call-expression*
evaluates to `null`. The *expression*s in
*argument-expression-list* are evaluated.

**Examples**

```Hack
function f3(int $p1 = -1, float $p2 = 99.99, string $p3 = '??'): void { … }
f3();                   // $p1 is -1, $p2 is 99.99, $p3 is ??
f3(123);                // $p1 is 123, $p2 is 99.99, $p3 is ??
f3(123, 3.14);          // $p1 is 123, $p2 is 3.14, $p3 is ??
f3(123, 3.14, 'Hello'); // $p1 is 123, $p2 is 3.14, $p3 is Hello
// -----------------------------------------
function fx(int $p1, int $p2, int $p3, int $p4, int $p5): void { … }
function fy(int $p1, int $p2, int $p3, int $p4, int $p5): void { … }
function fz(int $p1, int $p2, int $p3, int $p4, int $p5): void { … }
$funcTable = array(fun('fx'), fun('fy'), fun('fz')); // use lib function fun
$i = 1;
$funcTable[$i++]($i, ++$i, $i, $i = 12, --$i); // calls fy(2,3,3,12,11)
// -----------------------------------------
$anon = function (): void { … };  // store a closure in $anon
$anon();  // call the anonymous function
```

### Member-Selection Operator

**Syntax**

<pre>
  <i>member-selection-expression:</i>
    <i>postfix-expression</i>  ->  <i>name</i>
    <i>postfix-expression</i>  ->  <i>variable-name</i>
</pre>

**Defined elsewhere**

* [*name*](09-lexical-structure.md#names)
* [*postfix-expression*](10-expressions.md#general-3)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

*postfix-expression* must designate an object. 

*name* must designate an instance property, or an instance
method of the class designated by *postfix-expression*.

*variable-name* must name a variable which when evaluated produces a string
containing an instance property or an instance method of the class 
designated by *postfix-expression*.


**Semantics**

A *member-selection-expression* designates an instance property or an
instance method of the object designated by
*postfix-expression*. For a property, the value is that of the property,
and is a modifiable lvalue if *postfix-expression* is a modifiable
lvalue.

**Examples**

```Hack
class Point {
  private float $x;
  private float $y;
  public function move(float $x, float $y): void {
    $this->x = $x;  // sets private property $x
    $this->y = $y;  // sets private property $x
  }
  public function __toString(): string  {
    return '(' . $this->x . ',' . $this->y . ')';
  }     // get private properties $x and $y
}
$p1 = new Point();
$p1->move(3, 9);  // calls public instance method move by name
```

### Null-Safe Member-Selection Operator

**Syntax**
<pre>
<i>null-safe-member-selection-expression:</i>
  <i>postfix-expression</i>  ?->  <i>name</i>
  <i>postfix-expression</i>  ?->  <i>variable-name</i>
</pre>

**Defined elsewhere**

* [*name*](09-lexical-structure.md#names)
* [*postfix-expression*](10-expressions.md#general-3)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

*postfix-expression* must designate a nullable-typed object.

*name* must designate an instance property or an instance method of the class designated by *postﬁx-expression*.

*variable-name* must name a variable which when evaluated produces a string
containing an instance property or an instance method of the class 
designated by *postfix-expression*.

**Semantics**

If *postﬁx-expression* is `null`, no property or method is selected and the 
resulting value is `null`. Otherwise, the behavior is like that of the 
[member-selection operator `->`](10-expressions.md#member-selection-operator), 
except that the resulting value is not an lvalue.

### Postfix Increment and Decrement Operators

**Syntax**

<pre>
  <i>postfix-increment-expression:</i>
    <i>unary-expression</i>  ++

  <i>postfix-decrement-expression:</i>
    <i>unary-expression</i>  --
</pre>

**Defined elsewhere**

* [*unary-expression*](10-expressions.md#general-4)

**Constraints**

The operand of the postfix ++ and -- operators must be a modifiable
lvalue that has arithmetic type.

**Semantics**

These operators behave like their [prefix counterparts](10-expressions.md#prefix-increment-and-decrement-operators) except
that the value of a postfix ++ or -- expression is the value before any
increment or decrement takes place.

**Examples**

```Hack
$i = 10; $j = $i-- + 100;   // old value of $i (10) is added to 100
$a = array(100, 200); $v = $a[1]++; // old value of $ia[1] (200) is assigned
```

### Scope-Resolution Operator

**Syntax**

<pre>
  <i>scope-resolution-expression:</i>
    <i>scope-resolution-qualifier</i>  ::  <i>name</i>
    <i>scope-resolution-qualifier</i>  ::  <i>variable-name</i>
    <i>scope-resolution-qualifier</i>  ::  class

  <i>scope-resolution-qualifier:</i>
    <i>qualified-name</i>
    <i>variable-name</i>
    self
    parent
    static
</pre>

**Defined elsewhere**

* [*name*](09-lexical-structure.md#names)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

Scope resolution expressions of the form <i>qualified-name</i>`::`<i>name</i> must have the name of an enum, class or interface type on the left of the `::`, and an enumeration constant or type member on the right.

Scope resolution expressions of the form <i>qualified-name</i>`::`<i>variable-name</i> must have the name of a class or interface type on the left of the `::`, and a static property of that type on the right.

Scope resolution expressions of the form <i>qualified-name</i>`::`class must have the name of a class or interface type on the left of the `::`.

Scope resolution expressions with a *variable-name* to the left of the `::` must name a variable having the [`classname` type](05-types.md#the-classname-type).

*variable-name* `:: class` is not permitted.

**Semantics**

When *qualified-name* is the name of an enumerated type, *scope-resolution-expression* designates an enumeration constant within that type.

From inside or outside a class or interface, operator `::` allows the
selection of a constant. From inside or outside a class, this operator
allows the selection of a static property, static method, or instance
method. From within a class, it also allows the selection of an
overridden property or method. For a property, the value is that of the
property, and is a modifiable lvalue if *name* is
a modifiable lvalue.

From within a class, `self::m` refers to the member `m` in that class,
whereas `parent::m` refers to the closest member `m` in the base-class
hierarchy, not including the current class. From within a method,
`static::m` refers to the member `m` in the class that corresponds to the
class inheritance context in which the method is called. This allows
*late static binding*. Consider the following scenario:

```Hack
class Base
{
  public function b(): void
  {
    static::f();  // calls the most appropriate f()
  }
  public function f(): void { ... }
}
class Derived extends Base
{
  public function f(): void { ... }
}
$b1 = new Base();
$b1->b(); // as $b1 is an instance of Base, Base::b() calls Base::f()
$d1 = new Derived();
$d1->b(); // as $d1 is an instance of Derived, Base::b() calls Derived::f()
```

The value of the form of *scope-resolution-expression* ending in `::class`
is a string containing the fully qualified name of the current class,
which for a static qualifier, means the current class context.

*variable-name* `::` *name* results in a constant whose value has the [`classname` type](05-types.md#the-classname-type) for the type designated by *variable-name*.

**Examples**

```Hack
final class MathLibrary
enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}
function main(ControlStatus $p1): void {
  switch ($p1)
  {
  case ControlStatus::Stopped:
    …
    break;
  case ControlStatus::Stopping:
    …
    break;
  …
  }
  …
}
// -----------------------------------------
final class MathLibrary {
  public static function sin(): float { … }
  …
}
$v = MathLibrary::sin(2.34);  // call directly by class name
// -----------------------------------------
class MyRangeException extends Exception {
  public function __construct(string $message, …)
  {
    parent::__construct($message);
    …
  }
  …
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

### Exponentiation Operator

**Syntax**

<pre>
  <i>exponentiation-expression:</i>
    <i>expression  **  expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

Both *expression*s must have an arithmetic type.

**Semantics**

The `**` operator produces the result of raising the value of the
left-hand operand to the power of the right-hand one. If both operands have non-negative integer
values and the result can be represented as an `int`, the result has type
`int`; otherwise, the result has type `float`.

**Examples**

```Hack
2**3;   // int with value 8
2**3.0;   // float with value 8.0
"2.0"**"3"; // float with value 8.0
```

## Unary Operators

### General

**Syntax**

<pre>
  <i>unary-expression:</i>
    <i>postfix-expression</i>
    <i>prefix-increment-expression</i>
    <i>prefix-decrement-expression</i>
    <i>unary-op-expression</i>
    <i>error-control-expression</i>
    <i>cast-expression</i>
    <i>await-expression</i>
</pre>

**Defined elsewhere**

* [*await-expression*](10-expressions.md#await-operator)
* [*cast-expression*](10-expressions.md#cast-operator)
* [*error-control-expression*](10-expressions.md#error-control-operator)
* [*postfix-expression*](10-expressions.md#general-3)
* [*prefix-decrement-expression*](10-expressions.md#prefix-increment-and-decrement-operators)
* [*prefix-increment-expression*](10-expressions.md#prefix-increment-and-decrement-operators)
* [*unary-op-expression*](10-expressions.md#unary-arithmetic-operators)

**Semantics**

These operators associate right-to-left.

### Prefix Increment and Decrement Operators

**Syntax**

<pre>
  <i>prefix-increment-expression:</i>
    ++ <i>unary-expression</i>

  <i>prefix-decrement-expression:</i>
    -- <i>unary-expression</i>
</pre>

**Defined elsewhere**

* [*unary-expression*](10-expressions.md#general-4)

**Constraints**

The operand of the prefix `++` or `--` operator must be a modifiable lvalue
that has arithmetic type.

**Semantics**

*Arithmetic Operands*

For a prefix `++` operator, the [side effect](#general) of the operator is to increment by 1, as appropriate, the
value of the operand. The result is the value of the operand after it
has been incremented. If an int operand's value is the largest
representable for that type, the type and value of [the result is implementation-defined](05-types.md#the-integer-type).

For a prefix `--` operator, the side
effect of the operator is to decrement by 1, as appropriate, the value
of the operand. The result is the value of the operand after it has been
decremented. If an int operand's value is the smallest representable for
that type, the type and value of [the result is implementation-defined](05-types.md#the-integer-type).

For a prefix `++` or `--` operator used with an operand having the value
`INF`, `-INF`, or `NAN`, there is no side effect, and the result is the
operand's value.

**Examples**

```Hack
$i = 10; $j = --$i + 100;   // new value of $i (9) is added to 100
$a = array(100, 200); $v = ++$a[1]; // new value of $ia[1] (201) is assigned
```

### Unary Arithmetic Operators

**Syntax**

<pre>
  <i>unary-op-expression:</i>
    <i>unary-operator cast-expression</i>

  <i>unary-operator: one of</i>
    +  -  !  ~
</pre>

**Defined elsewhere**

* [*cast-expression*](10-expressions.md#cast-operator)

**Constraints**

The operand of the unary `+` and unary `-` operators must have
arithmetic type.

The operand of the unary `!` operator must have arithmetic or enumerated type. (**The validity of allowing this operator to have an enumerated type operand is questionable; avoid such usage lest support for it disappears.**)

The operand of the unary `~` operator must have integer type.

**Semantics**

For a unary `+` operator, the type and
value of the result is the type and value of the operand. 

For a unary `-` operator, the value of the
result is the negated value of the operand. However, if an int operand's
original value is the smallest representable for that type, the type and
value of [the result is implementation-defined](05-types.md#the-integer-type).

For a unary `!` operator, the type of the
result is `bool`. The value of the result is `true` if the value of the
operand is non-zero (or for a string-based enumeration, a non-empty string); otherwise, the value of the result is `false`. For
the purposes of this operator, `NAN` is considered a non-zero value. The
expression `!E` is equivalent to `(E == 0)`.

For a unary `~` operator, the type of the result
is `int`. The value of the result is the bitwise complement of the value
of the operand (that is, each bit in the result is set if and only if
the corresponding bit in the operand is clear).

**Examples**

```Hack
$v = +10;
if ($v1 > -5) ...
$t = true;
if (!$t) ...
$v = ~0b1010101;
```

### Error Control Operator

**Syntax**

<pre>
  <i>error-control-expression:</i>
    @  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Semantics**

Operator `@` suppresses any error messages generated by the evaluation of
*expression*.

If a custom error-handler has been established using the library
function [`set_error_handler`](http://docs.hhvm.com/manual/en/function.set-error-handler.php) that handler is
still called.

**Examples**

```Hack
$infile = @fopen("NoSuchFile.txt", 'r');
```

On open failure, the value returned by `fopen` is `false`, which is
sufficient to know to handle the error. There is no need to have any
error message displayed.

### Cast Operator

**Syntax**

<pre>
  <i>cast-expression:</i>
    (  <i>cast-type</i>  ) <i>unary-expression</i>

  <i>cast-type: one of</i>
    bool  int  float  string
</pre>

**Defined elsewhere**

* [*unary-expression*](10-expressions.md#general-4)

**Semantics**

The
value of the operand *cast-expression* is converted to the type
specified by *cast-type*, and that is the type and value of the result.
This construct is referred to a *cast,* and is used as the verb, "to
cast". If no conversion is involved, the type and value of the result
are the same as those of *cast-expression*.

A cast can result in a loss of information.

A *cast-type* of `bool` results in a conversion to type `bool`.
See [§§](08-conversions.md#converting-to-boolean-type) for details.

A *cast-type* of `int` results in a conversion to type `int`. See [§§](08-conversions.md#converting-to-integer-type) for details.

A *cast-type* of `float` results in a conversion to type `float`. See [§§](08-conversions.md#converting-to-floating-point-type) for details.

A *cast-type* of `string` results in a conversion to type `string`. See [§§](08-conversions.md#converting-to-string-type)
for details.

Note that *cast-type* cannot be a [generic type parameter](14-generic-types-methods-and-functions.md#type-parameters).

**Examples**

```Hack
(int)(10/3)          // results in the int 3 rather than the float 3.333...
```

### Await Operator

**Syntax**

<pre>
  <i>await-expression:</i>
    await  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)

**Constraints**

This operator must be used within an[ asynchronous function](15-functions.md#asynchronous-functions).

*expression* must have a type that implements [`Awaitable<T>`(17-interfaces.md#interface-awaitable).

The return type of the function containing a use of this operator must be a type that implements `Awaitable<T>`.

*await-expression* can only be used in the following contexts:
*	As an [*expression-statement*](11-statements.md#expression-statements)
*	As the *assignment-expression* in a [*simple-assignment-expression*](10-expressions.md#simple-assignment)
*	As *expression* in a [*return-statement*](11-statements.md#the-return-statement)

**Semantics**

`await` suspends the execution of an async function until the result of the asynchronous operation represented by *expression* is available. See [§§](15-functions.md#asynchronous-functions) for more information.

The resulting value is the value of type `T` that was wrapped in the object of type `Awaitable<T>`` returned from the async function. Consider the following:

```Hack
async function f(): Awaitable<int> {…}

$x = await f();		// $x is an int
$x = f();			// $x is an Awaitable<int>
```

**Examples**

```Hack
async function f(): Awaitable<int> {
  …
  $r1 = await g();
  …
  return $r1;
}

async function g(): Awaitable<int> {
  …
  return $r2;
}

function main (): void {
  …
  $v = f();
  …
}
```

Function `main` calls async function `f`, which in turn awaits on async function `g`. When `g` terminates normally, the `int` value returned is automatically wrapped in an object of type `Awaitable<int>`. Back in function `f`, that object is unwrapped, and the `int` it contained is extracted and assigned to local variable `$r1`. When `f` terminates normally, the `int` value returned is automatically wrapped in an object of type `Awaitable<int>`. Back in function `main`, that object is assigned to local variable `$v`.

## `instanceof` Operator

**Syntax**

<pre>
<i>instanceof-expression:</i>
  <i>unary-expression</i>
  <i>instanceof-subject</i>  instanceof   <i>instanceof-type-designator</i>

<i>instanceof-subject:</i>
  <i>expression</i>

<i>instanceof-type-designator:</i>
  <i>qualified-name</i>
  <i>variable-name</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*qualified-name*](20-namespaces.md#defining-namespaces)
* [*unary-expression*](10-expressions.md#general-4)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

The *expression* in *instanceof-subject* must designate a variable.

*qualified-name* must be the name of a class or interface type.

*variable-name* must name a value having the [`classname` type](05-types.md#the-classname-type).

**Semantics**

Operator `instanceof` returns `true` if the variable designated by
*expression* in *instanceof-subject* is an object having type
*qualified-name* or *variable-name*, is an object whose type is derived from type
*qualified-name* or *variable-name*, or is an object whose type implements interface
*qualified-name* or *variable-name*. Otherwise, it returns `false`.

If either *expression* is not an instance, `false` is returned.

Note: This operator supersedes the library function [`is_a`](http://www.php.net/is_a), which
has been deprecated.

**Examples**

```Hack
class C1 { … } $c1 = new C1();
class C2 { … } $c2 = new C2();
class D extends C1 { … } $d = new D();
$d instanceof C1      // true
$d instanceof C2      // false
$d instanceof D       // true
// -----------------------------------------
interface I1 { … }
interface I2 { … }
class E1 implements I1, I2 { … }
$e1 = new E1();
$e1 instanceof I1     // true
```

## Multiplicative Operators

**Syntax**

<pre>
  <i>multiplicative-expression:</i>
    <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  *  <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  /  <i>instanceof-expression</i>
    <i>multiplicative-expression</i>  %  <i>instanceof-expression</i>
</pre>

**Defined elsewhere**

* [*instanceof-expression*](10-expressions.md#instanceof-operator)

**Constraints**

The operands of the `*` and `/` operators must have arithmetic type.

The operands of the `%` operator must have integer type.

The right-hand operand of operator `/` and operator `%` must not be zero.

**Semantics**

The binary `*` operator produces the product of its operands. If either operand has type
`float`, the other is converted to that type, and the result has type
`float`. Otherwise, both operands have type `int`, in which case, if the
resulting value can be represented in type `int` that is the result type.
Otherwise, the type and value of [the result is implementation-defined](05-types.md#the-integer-type).

Division by zero results in a diagnostic followed by a `bool` result
having value `false`. (The values +/- infinity and NaN cannot be generated
via this operator; instead, use the predefined constants `INF` and `NAN`.)

The binary `/` operator produces the quotient from dividing the left-hand
operand by the right-hand one. If either operand has type `float`, the other is
converted to that type, and the result has type `float`. Otherwise, both
operands have type `int`, in which case, if the mathematical value of the
computation can be preserved using type `int`, that is the result type;
otherwise, the type of the result is `float`.

The binary `%` operator produces the remainder from dividing the left-hand
operand by the right-hand one. The result has type `int`.

These operators associate left-to-right.

**Examples**

```Hack
-10 * 100 → int with value -1000
100 * -3.4e10 → float with value -3400000000000
"123" * "2e+5" → float with value 24600000
100 / 100 → int with value 1
100 / 123 → float with value 0.8130081300813
123 % 100 → int with value 23
```

## Additive Operators

**Syntax**

<pre>
  <i>additive-expression:</i>
    <i>multiplicative-expression</i>
    <i>additive-expression</i>  +  <i>multiplicative-expression</i>
    <i>additive-expression</i>  -  <i>multiplicative-expression</i>
    <i>additive-expression</i>  .  <i>multiplicative-expression</i>
</pre>

**Defined elsewhere**

* [*multiplicative-expression*](#multiplicative-operators)

**Constraints**

If either operand has array type, the other operand must also have array
type, and the two types must have a subtype relationship.

If the operands of the `*` and `/` operators do not both have array type, they must both have arithmetic type.

**Semantics**

For non-array operands, the binary `+` operator produces the sum of those
operands, while the binary `- `operator produces the difference of its
operands when subtracting the right-hand operand from the left-hand one.
If either operand has type `float`, the other is converted to that type, and
the result has type `float`. Otherwise, both operands have type `int`, in
which case, if the resulting value can be represented in type `int` that
is the result type. Otherwise, the type and value of [the result is implementation-defined](05-types.md#the-integer-type).

If both operands have array type, the binary `+` operator produces a new
array that is the union of the two operands. The result is a copy of the
left-hand array with elements inserted at its end, in order, for each
element in the right-hand array whose key does not already exist in the
left-hand array. Any element in the right-hand array whose key exists in
the left-hand array is ignored. In this context, this operator is not commutative.

The binary `.` operator creates a string that is the concatenation of the
left-hand operand and the right-hand operand, in that order. If either
or both operands have types other than `string`, their values are
converted to type `string`. The result has type `string`.

These operators associate left-to-right.

**Examples**

```Hack
-10 + 100 → int with value 90
100 + -3.4e10 → float with value -33999999900
100 - 123 → int with value 23
-3.4e10 - abc → float with value -34000000000
// -----------------------------------------
array(66) + array(100, 200) → array(66, 200)
array(2 => 'aa') + array(-4 => 'bb', 6 => 'cc') → array(2 => 'aa', -4 => 'bb', 6 => 'cc')
array('red' => 12, 'green' => 7) + array('blue' => 3) → array('red' => 12, 'green' => 7, 'blue' => 3)
// -----------------------------------------
-10 . NAN → string with value "-10NAN"
INF . "2e+5" → string with value "INF2e+5"
true . null → string with value "1"
10 + 5 . 12 . 100 - 50 → int with value 1512050; ((((10 + 5).12).100)-50)
```

## Bitwise Shift Operators

**Syntax**

<pre>
  <i>shift-expression:</i>
    <i>additive-expression</i>
    <i>shift-expression</i>  &lt;&lt;  <i>additive-expression</i>
    <i>shift-expression</i>  &gt;&gt;  <i>additive-expression</i>
</pre>

**Defined elsewhere**

* [*additive-expression*](10-expressions.md#additive-operators)

**Constraints**

Each of the operands must have `int` type.

**Semantics**

Given the expression `e1 << e2`, the bits in the value of `e1` are shifted
left by `e2` positions. Bits shifted off the left end are discarded, and
zero bits are shifted on from the right end. Given the expression
`e1 >> e2`, the bits in the value of `e1` are shifted right by
`e2` positions. Bits shifted off the right end are discarded, and the sign
bit is propagated from the left end. 

The type of the result is `int`, and the value of the result is that after
the shifting is complete. The values of `e1` and `e2` are unchanged.

If the shift count is negative, the actual shift applied is `n -
(-shift count % n)`, where `n` is the number of bits per `int`. If the
shift count is greater than the number of bits in an `int`, the actual
shift applied is shift count `% n`.

These operators associate left-to-right.

**Examples**

```Hack
1000 >> 2   // 3E8 is shifted right 2 places
-1000 << 2      // FFFFFC18 is shifted left 5 places
123 >> 128      // adjusted shift count = 0
123 << 33   // For a 32-bit int, adjusted shift count = 1; otherwise, 33
```

## Relational Operators

**Syntax**

<pre>
  <i>relational-expression:</i>
    <i>shift-expression</i>
    <i>relational-expression</i>  &lt;   <i>shift-expression</i>
    <i>relational-expression</i>  &gt;   <i>shift-expression</i>
    <i>relational-expression</i>  &lt;=  <i>shift-expression</i>
    <i>relational-expression</i>  &gt;=  <i>shift-expression</i>
</pre>

**Defined elsewhere**

* [*shift-expression*](10-expressions.md#bitwise-shift-operators)

**Constraints**

If either operand has an enumerated type, the other operand must have the exact same type.

**Semantics**

Operator `<` represents *less-than*, operator `>` represents
*greater-than*, operator `<=` represents *less-than-or-equal-to*, and
operator `>=` represents *greater-than-or-equal-to*.

The type of the result is `bool`. 

The operands are processed using the following steps, in order:

1.  If the operands both have arithmetic type, the result is the numerical comparison of the two operands after conversion.
2.  If both operands are non-numeric strings, the result is the lexical comparison of the two operands. Specifically, the strings are compared byte-by-byte starting with their first byte. If the two bytes compare equal and there are no more bytes in either string, the strings are equal and the comparison ends; otherwise, if this is the final byte in one string, the shorter string compares less-than the longer string and the comparison ends. If the two bytes compare unequal, the string having the lower-valued byte compares less-than the other string, and the comparison ends. If there are more bytes in the strings, the process is repeated for the next pair of bytes.
3.  If both operands are numeric strings, the result is the numeric comparison of the two operands after conversion.
4.  If both operands have vector-like or map-like array type, if the arrays have different numbers of elements, the one with the fewer is considered less-than the other one—regardless of the keys and values in each—, and the comparison ends. For arrays having the same numbers of elements, if the next key in the left-hand operand exists in the right-hand operand, the corresponding values are compared. If they are unequal, the array containing the lesser value is considered less-than the other one, and the comparison ends; otherwise, the process is repeated with the next element. If the next key in the left-hand operand does not exist in the right-hand operand, the arrays cannot be compared and `false` is returned. For array comparison, the order of insertion of the elements into those arrays is irrelevant.
5.  If the operands have the same object type, the result is decided by comparing the lexically first-declared instance property in each object. If those properties have object type, the comparison is applied recursively.

Regarding operands having the same enumeration type, for an `int`-based enumeration the enumeration is compared using its value directly as an `int`. For a `string`-based enumeration, the enumeration is compared using its value directly as a `string`. (**The validity of allowing these operators to have an enumerated type as an operand is questionable; avoid such usage lest support for it disappears.**)

These operators associate left-to-right.

**Examples**

```Hack
"" < "ab"       → result has value true
"a" > "A"       → result has value true
"a0" < "ab"     → result has value true
"aA <= "abc"    → result has value true
// -----------------------------------------
10 <= 0         → result has value false
'123' <= '4') → false; is doing a numeric comparison
'X123' <= 'X4'  → true; is doing a string comparison
// -----------------------------------------
[100] < [10,20,30] → result has value true (LHS array is shorter)
```

**Notes**

Ideally, one might expect some constraints on the combination of operand types. However, for historical reasons, other behaviors from PHP have been retained, as documented.

## Equality Operators

**Syntax**

<pre>
  <i>equality-expression:</i>
    <i>relational-expression</i>
    <i>equality-expression</i>  ==  <i>relational-expression</i>
    <i>equality-expression</i>  !=  <i>relational-expression</i>
    <i>equality-expression</i>  ===  <i>relational-expression</i>
    <i>equality-expression</i>  !==  <i>relational-expression</i>
</pre>

**Defined elsewhere**

* [*relational-expression*](10-expressions.md#relational-operators)

**Semantics**

Operator `==` represents *value-equality*, operator `!=` represents *value-inequality*, operator `===` represents
*same-type-and-value-equality*, and operator `!==` represents
*not-same-type-and-value-equality*. However, when comparing two objects,
operator `===` represents *identity* and operator `!==` represents
*non-identity*. Specifically, in this context, these operators check to
see if the two operands are the exact same object, not two different
objects of the same type and value.

The type of the result is `bool`. 

The operands are processed using the following steps, in order:

1.  For operators `==`, `!=`, and `<>`, if either operand has the value
    `null`, then if the other operand has type string, the `null` is
    converted to the empty string (""); otherwise, the `null` is converted
    to type bool.
2.  If both operands are non-numeric strings or one is a numeric string
    and the other a leading-numeric string, the result is the lexical
    comparison of the two operands. Specifically, the strings are
    compared byte-by-byte starting with their first byte. If the two
    bytes compare equal and there are no more bytes in either string,
    the strings are equal and the comparison ends; otherwise, if this is
    the final byte in one string, the shorter string compares less-than
    the longer string and the comparison ends. If the two bytes compare
    unequal, the string having the lower-valued byte compares less-than
    the other string, and the comparison ends. If there are more bytes
    in the strings, the process is repeated for the next pair of bytes.
3.  If either operand has type bool, for operators `==` and `!=`, the
    other operand is converted to that type. The result is the logical
    comparison of the two operands after any conversion, where `false` is
    defined to be less than `true`.
4.  If the operands both have arithmetic type, string type, or are
    resources, for operators `==` and `!=`, they are converted to the
    corresponding arithmetic type [§§](08-conversions.md#converting-to-integer-type) and [§§](08-conversions.md#converting-to-floating-point-type)). The result is the
    numerical comparison of the two operands after any conversion.
5.  If both operands have array type, for operators `==` and `!=`,
    the arrays are equal if they have the same set of key/value pairs and the corresponding values have the same type,
    after element type conversion, without regard to the order of
    insertion of their elements. For operators `===` and `!==` the arrays
    are equal if they have the same set of key/value pairs, the
    corresponding values have the same type, and the order of insertion
    of their elements are the same.
6.  If only one operand has object type, the two operands are never
    equal.
7.  If only one operand has array type, the two operands are never
    equal.
8.  If the operands have different object types, the two operands are
    never equal except that a `Vector` and an `ImmVector` having the same member type and value set can be equal, as can a `Map` and an `ImmMap`, and a `Set` and an `ImmSet`.
9.  If the operands have the same object type, the two operands are
    equal if the instance properties in each object have the same
    values. Otherwise, the objects are unequal. The instance properties
    are compared, one at a time, in the lexical order of their
    declaration. For properties that have object type, the comparison is
    applied recursively.

These operators associate left-to-right.

**Examples**

```Hack
"a" <> "aa" // result has value true
// -----------------------------------------
null == 0   // result has value true
null === 0  // result has value false
true != 100  // result has value false
true !== 100  // result has value true
// -----------------------------------------
"10" != 10  // result has value false
"10" !== 10 // result has value true
// -----------------------------------------
[10,20] == [10,20.0]  // result has value true
[10,20] === [10,20.0] // result has value false
["red"=>0,"green"=>0] === ["red"=>0,"green"=>0] // result has value true
["red"=>0,"green"=>0] === ["green"=>0,"red"=>0] // result has value false
```

**Notes**

Ideally, one might expect some constraints on the combination of operand types. However, for historical reasons, other behaviors from PHP have been retained, as documented.

# # Bitwise AND Operator

**Syntax**

<pre>
  <i>bitwise-AND-expression:</i>
    <i>equality-expression</i>
    <i>bit-wise-AND-expression</i>  &amp;  <i>equality-expression</i>
</pre>

**Defined elsewhere**

* [*equality-expression*](10-expressions.md#equality-operators)

**Constraints**

Each of the operands must have `int` type.

**Semantics**

The result of this operator is the bitwise-AND of the two operands, and
the type of that result is `int`.

This operator associates left-to-right.

**Examples**

```Hack
0b101111 & 0b101          // 0b101
$lLetter = 0x73;          // letter 's'
$uLetter = $lLetter & ~0x20;  // clear the 6th bit to make letter 'S'
```

## Bitwise Exclusive OR Operator

**Syntax**

<pre>
  <i>bitwise-exc-OR-expression:</i>
    <i>bitwise-AND-expression</i>
    <i>bitwise-exc-OR-expression</i>  ^   <i>bitwise-AND-expression</i>
</pre>

**Defined elsewhere**

* [*bitwise-AND-expression*](10-expressions.md#bitwise-and-operator)

**Constraints**

Each of the operands must have `int` type.

**Semantics**

The result of this operator is the bitwise exclusive-OR of the two
operands, and the type of that result is `int`.

This operator associates left-to-right.

**Examples**

```Hack
0b101111 | 0b101    // 0b101010
$v1 = 1234; $v2 = -987; // swap two integers having different values
$v1 = $v1 ^ $v2;
$v2 = $v1 ^ $v2;
$v1 = $v1 ^ $v2;    // $v1 is now -987, and $v2 is now 1234
```

## Bitwise Inclusive OR Operator

**Syntax**

<pre>
  <i>bitwise-inc-OR-expression:</i>
    <i>bitwise-exc-OR-expression</i>
    <i>bitwise-inc-OR-expression</i>  |  <i>bitwise-exc-OR-expression</i>
</pre>

**Defined elsewhere**

* [*bitwise-exc-OR-expression*](10-expressions.md#bitwise-exclusive-or-operator)

**Constraints**

Each of the operands must have `int` type.

**Semantics**

The result of this operator is the bitwise inclusive-OR of the two
operands, and the type of that result is `int`.

This operator associates left-to-right.

**Examples**

```Hack
0b101111 | 0b101      // 0b101111
$uLetter = 0x41;      // letter 'A'
$lLetter = $upCaseLetter | 0x20;  // set the 6th bit to make letter 'a'
```

## Logical AND Operator

**Syntax**

<pre>
  <i>logical-AND-expression:</i>
    <i>bitwise-inc-OR-expression</i>
    <i>logical-AND-expression</i>  &amp;&amp;  <i>bitwise-inc-OR-expression</i>
</pre>

**Defined elsewhere**

* [*bitwise-inc-OR-expression*](10-expressions.md#bitwise-inclusive-or-operator)

**Constraints**

Each of the operands must have scalar type.

**Semantics**

If either operand does not have type bool, its value is first converted
to that type. An `int`-based enumeration with value zero is converted to `false`, while a non-zero value is converted to `true`. Likewise, a `string`-based enumeration with value empty string is converted to `false`, while a non-empty value is converted to `true`. (**The validity of allowing this operator to have an enumerated type operand is questionable; avoid such usage lest support for it disappears.**)

Given the expression `e1 && e2, e1` is evaluated first. If `e1` is `false`, `e2` is not evaluated, and the result has type `bool`, value `false`. Otherwise, `e2` is evaluated. If `e2` is `false`, the result has type bool, value `false`; otherwise, it has type `bool`, value `true`. There is a sequence point after the evaluation of `e1`.

This operator associates left-to-right.

**Examples**

```Hack
if ($month > 1 && $month <= 12) ...
```

## Logical Inclusive OR Operator

**Syntax**

<pre>
  <i>logical-inc-OR-expression:</i>
    <i>logical-AND-expression</i>
    <i>logical-inc-OR-expression</i>  ||  <i>logical-AND-expression</i>
</pre>

**Defined elsewhere**

* [*logical-AND-expression*](10-expressions.md#logical-and-operator)

**Constraints**

Each of the operands must have scalar type.

**Semantics**

If either operand does not have type bool, its value is first converted
to that type. An `int`-based enumeration with value zero is converted to `false`, while a non-zero value is converted to `true`. Likewise, a `string`-based enumeration with value empty string is converted to `false`, while a non-empty value is converted to `true`. (**The validity of allowing this operator to have an enumerated type operand is questionable; avoid such usage lest support for it disappears.**)

Given the expression `e1 || e2`, `e1` is evaluated first. If `e1` is true, `e2` is not evaluated, and the result has type `bool`, value `true`. Otherwise, `e2` is evaluated. If `e2` is `true`, the result has type `bool`, value `true`; otherwise, it has type `bool`, value `false`. There is a sequence point after the evaluation of `e1`.

This operator associates left-to-right.

**Examples**

```Hack
if ($month < 1 || $month > 12) ...
```

## Conditional Operator

**Syntax**

<pre>
  <i>conditional-expression:</i>
    <i>logical-inc-OR-expression</i>
    <i>logical-inc-OR-expression</i>  ?  <i>expression<sub>opt</sub></i>  :  <i>conditional-expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*logical-inc-OR-expression*](10-expressions.md#logical-inclusive-or-operator)

**Semantics**

Given the expression `e1 ? e2 : e3`, if `e1` is `true`, then and only then is `e2` evaluated, and the result and its type become the result and type of
the whole expression. Otherwise, then and only then is `e3` evaluated, and
the result and its type become the result and type of the whole
expression. There is a sequence point after the evaluation of `e1`. If `e2`
is omitted, the result and type of the whole expression is the value and
type of `e1` when it was tested.

Regarding a left-most operand that is an enumeration, an `int`-based enumeration with value zero is converted to `false`, while a non-zero value is converted to `true`. Likewise, a string-based enumeration with value empty string is converted to `false`, while a non-empty value is converted to `true`. (**The validity of allowing this operator to have an enumerated type as its first operand is questionable; avoid such usage lest support for it disappears.**)

This operator associates left-to-right.

**Examples**

```Hack
for ($i = -5; $i <= 5; ++$i)
  echo "$i is ".(($i & 1 == true) ? "odd\n" : "even\n");
// -----------------------------------------
$a = 10 ? : "Hello";  // result is int with value 10
$a = 0 ? : "Hello";     // result is string with value "Hello"
$i = PHP_INT_MAX;
$a = $i++ ? : "red";  // result is int with value 2147483647 (on a 32-bit
                // system) even though $i is now the float 2147483648.0
// -----------------------------------------
$i++ ? f($i) : f(++$i); // the sequence point makes this well-defined
// -----------------------------------------
function factorial(int $int): int
{
  return ($int > 1) ? $int * factorial($int - 1) : $int;
}
```

## Coalesce Operator

**Syntax**

<pre>
  <i>coalesce-expression:</i>
    <i>logical-inc-OR-expression</i>  ??  <i>expression</i>
</pre>

**Defined elsewhere**

* [*expression*](10-expressions.md#yield-operator)
* [*logical-inc-OR-expression*](#logical-inclusive-or-operator)

**Semantics**

Given the expression `e1 ?? e2`, if `e1` is set and not `null`, then the result is `e1`. Otherwise, then and only then is `e2`
evaluated, and the result becomes the result of the whole expression. There is a sequence point after the evaluation of `e1`.

This operator associates right-to-left.

**Examples**

```PHP
function foo(): void {
  echo "executed!", PHP_EOL;
}

function main(): void {
  $arr = ["foo" => "bar", "qux" => null];
  $obj = (object)$arr;

  $a = $arr["foo"] ?? "bang"; // "bar" as $arr["foo"] is set and not null
  $a = $arr["qux"] ?? "bang"; // "bang" as $arr["qux"] is null
  $a = $arr["bing"] ?? "bang"; // "bang" as $arr["bing"] is not set

  $a = $obj->foo ?? "bang"; // "bar" as $obj->foo is set and not null
  $a = $obj->qux ?? "bang"; // "bang" as $obj->qux is null
  $a = $obj->bing ?? "bang"; // "bang" as $obj->bing is not set

  $a = null ?? $arr["bing"] ?? 2; // 2 as null is null, and $arr["bing"] is not set
  var_dump(true ?? foo()); // outputs bool(true), "executed!" does not appear as it short-circuits
}
```

## Pipe Operator

**Syntax**
<pre>
<i>piped-expression:</i>
  <i>coalesce-expression</i>
  <i>piped-expression</i>   |>   <i>coalesce-expression</i>
</pre>

*coalesce-expression* is defined in [§§](10-expressions.md#coalesce-operator).

**Constraints**

*piped-expression* cannot be used as the right-hand operand of an assignment operator.

*coalesce-expression* must contain at least one occurrence of the pipe variable `$$`.

**Semantics**

*piped-expression* is evaluated with the result being stored in the pipe variable `$$`. There is a sequence point after the evaluation of *piped-expression*. Then *coalesce-expression* is evaluated, and its type and value become the type and value of the result.

This operator associates left-to-right.

**Examples**

```PHP
class Widget { … }
function pipe_operator_example(array<Widget> $arr): int {
  return $arr
    |> array_map($x ==> $x->getNumber(), $$)
    |> array_filter($$, $x ==> $x % 2 == 0)
    |> count($$);
}
```

## Lambda Expressions

**Syntax**
<pre>
<i>lambda-expression:</i>
  <i>piped-expression</i>
  async<sub>opt</sub>  <i>lambda-function-signature</i>  ==>  <i>lambda-body</i>

<i>lambda-function-signature:</i>
  <i>variable-name</i>
  (  <i>anonymous-function-parameter-list<sub>opt</sub></i>  )  <i>anonymous-function-return<sub>opt</sub></i>

<i>lambda-body:</i>
  <i>expression</i>
  <i>compound-statement</i>
</pre>

**Defined elsewhere**

* [*anonymous-function-parameter-list*](10-expressions.md#anonymous-function-creation)
* [*anonymous-function-return*](10-expressions.md#anonymous-function-creation)
* [*coalesce-expression*](10-expressions.md#coalesce-operator)
* [*compound-statement*](11-statements.md#compound-statements)
* [*conditional-expression*](10-expressions.md#conditional-operator)
* [*expression*](10-expressions.md#yield-operator)
* [*piped-expression*](10-expressions.md#pipe-operator)
* [*variable-name*](09-lexical-structure.md#names)

**Constraints**

Each variable-name in an *anonymous-function-parameter-list* must be distinct.

If any *anonymous-function-parameter-declaration* has a *default-argument-specifier*, then all subsequent *anonymous-function-parameter-declarations* in the same *anonymous-function-parameter-declaration-list* must also have a *default-argument-specifier*.

If the *type-specifier* in *anonymous-function-return* is `void`, the *compound-statement* must not contain any [`return` statements](11-statements.md#the-return-statement) having an *expression*. Otherwise, if that *type-specifier* is not omitted, the *expression* in *lambda-body*, or all `return` statements in *compound-statement* must contain an *expression* whose type is a subtype of the type indicated by the return type's *type-specifier*.

If `async` is present, *return-type* must be a type that implements [`Awaitable<T>`](17-interfaces.md#interface-awaitable).

**Semantics**

A lambda expression is an anonymous function implemented using an operator. In many cases, the lambda-expression version is simpler to writer and easier to read, as is shown in the following:

```Hack
$doublerl = ($p) ==> $p * 2;
$doubler2 = function ($p) { return $p * 2; };
```

Lambda expressions automatically capture any variables appearing in their body that also appear in the enclosing lexical function scopes transitively (i.e., nested lambda expressions can refer to variables from several levels out, with intermediate lambda expressions capturing that variable so it can be forwarded to the inner lambda expression). Variables are only captured when they are statically visible as names in the enclosing scope; i.e., the capture list is computed statically, not based on dynamically defined names in the scope. A lambda expression's captured variables are captured with the same by-value semantics that are used for variables in an *anonymous-function-use-clause* of an [*anonymous-function-creation-expression*](10-expressions.md#anonymous-function-creation).

This operator is right-associative and lambda-expressions can be chained together, allowing expressions of the form `$f = $x ==> $y ==> $x + $y`.

When a lambda expression is executed, it creates an object of some, unspecified closure type.

If the *type-specifier* for a parameter is omitted, that type is inferred.

If *anonymous-function-return* is omitted, the return type is inferred.

The anonymous function in a *lambda-expression* can be [asynchronous](15-functions.md#asynchronous-functions).

**Examples**

```Hack
// returns 73
$fn = $x ==> $x + 1; $fn(12); // returns 13$fn = () ==> 73; $n();
$fn = ($a = -1, ...): int ==> $a * 2;
// -----------------------------------------
$dump_map = ($name, $x) ==> {
  echo "Map $name has:\n";
  foreach ($x as $k => $v) {
    echo "  $k => $v\n";
  }
};
// -----------------------------------------
$fn1 = $x ==> $y ==> $x + $y;
$fn2 = $fn1(10); $res = $fn2(7);  // result is 17
```

## Assignment Operators

### General

**Syntax**

<pre>
  <i>assignment-expression:</i>
    <i>lambda-expression</i>
    <i>simple-assignment-expression</i>
    <i>compound-assignment-expression</i>
</pre>

**Defined elsewhere**

* [*compound-assignment-expression*](10-expressions.md#compound-assignment)
* [*lambda-expression*](10-expressions.md#lambda-expressions)
* [*simple-assignment-expression*](10-expressions.md#simple-assignment)

**Constraints**

The left-hand operand of an assignment operator must be a modifiable
lvalue.

**Semantics**

These operators associate right-to-left.

### Simple Assignment

**Syntax**

<pre>
  <i>simple-assignment-expression:</i>
    <i>unary-expression</i>  =  <i>assignment-expression</i>
</pre>

**Defined elsewhere**

* [*assignment-expression*](10-expressions.md#general-5)
* [*unary-expression*](10-expressions.md#general-4)

**Constraints**

If the location designated by the left-hand operand is a string element,
the key must not be a negative-valued `int`, and the right-hand operand
must have type `string`.

If *unary-expression* is a [*subscript-expression*](10-expressions.md#subscript-operator) whose *postfix-expression* designates a vector-like array and whose *expression* is present, if *expression* designates a non-existent element, the behavior is unspecified.

If *unary-expression* is a *subscript-expression* whose *postfix-expression* designates a `Map` and whose *expression* is omitted, *assignment-expression* must designate a `Pair`.


**Semantics**

If *assignment-expression* designates an expression having value type,
see [§§](04-basic-concepts.md#value-assignment-of-scalar-types-to-a-local-variable). If *assignment-expression*
designates an expression having a handle, see [§§](04-basic-concepts.md#byref-assignment-for-scalar-types-with-local-variables). If
*assignment-expression* designates an expression having array type, see
[§§](04-basic-concepts.md#value-assignment-of-array-types-to-local-variables). If *assignment-expression* designates an expression having a shape type, it is treated as if it had array type.

The type and value of the result is the type and value of the left-hand
operand after the store (if any [see below]) has taken place. The result
is not an lvalue.

If the location designated by the left-hand operand is a non-existent
array element, a new element is inserted with the designated key and
with a value being that of the right-hand operand.

If the location designated by the left-hand operand is a string element,
then if the key is a negative-valued `int`, there is no side effect.
Otherwise, if the key is a non-negative-valued `int`, the left-most single
character from the right-hand operand is stored at the designated
location; all other characters in the right-hand operand string are
ignored.  If the designated location is beyond the end of the
destination string, that string is extended to the new length with
spaces (U+0020) added as padding beyond the old end and before the newly
added character. If the right-hand operand is an empty string, the null
character \\0 (U+0000) is stored.

**Examples**

```Hack
$a = $b = 10    // equivalent to $a = ($b = 10)
$v = array(10, 20, 30);
$v[-10] = 19;   // insert a new element with int key -10
$s = "red";
$s[1] = "X";    // OK; "e" -> "X"
$s[-5] = "Y";   // warning; string unchanged
$s[5] = "Z";    // extends string with "Z", padding with spaces in [3]-[5]
$s = "red";
$s[0] = "DEF";  // "r" -> "D"; only 1 char changed; "EF" ignored
$s[0] = "";     // "D" -> "\0"
$s["zz"] = "Q"; // warning; defaults to [0], and "Q" is stored there
// -----------------------------------------
class C { … }
$a = new C();   // make $a point to the allocated object  
```

### Compound Assignment

**Syntax**

<pre>
  <i>compound-assignment-expression:</i>
    <i>unary-expression   compound-assignment-operator   assignment-expression</i>

  <i>compound-assignment-operator: one of</i>
    **=  *=  /=  %=  +=  -=  .=  &lt;&lt;=  >>=  &amp;=  ^=  |=
</pre>

**Defined elsewhere**

* [*assignment-expression*](10-expressions.md#general-5)
* [*unary-expression*](10-expressions.md#general-4)

**Constraints**

Any constraints that apply to the corresponding postfix or binary
operator apply to the compound-assignment form as well.

**Semantics**

The expression `e1 op= e2` is equivalent to `e1 = e1 op (e2)`, except
that `e1` is evaluated once only.

**Examples**

```Hack
$v = 10;
$v += 20;   // $v = 30
$v -= 5;    // $v = 25
$v .= 123.45  // $v = "25123.45"
$a = [100, 200, 300];
$i = 1;
$a[$i++] += 50; // $a[1] = 250, $i → 2
```

## `yield` Operator

**Syntax**

<pre>
  <i>expression:</i>
    <i>assignment-expression</i>
    yield  <i>array-element-initializer</i>
</pre>

**Defined elsewhere**

* [*array-element-initializer*](#array-creation-operator)
* [*assignment-expression*](10-expressions.md#general-5)

**Constraints**

`yield` must not be used inside an [async function, method, or closure](15-functions.md#asynchronous-functions).

**Semantics**

Any function containing a yield operator is a *generator function*.
A generator function generates a collection of zero or more key/value
pairs where each pair represents the next in some series. For example, a
generator might *yield* random numbers or the series of Fibonacci
numbers. When a generator function is called explicitly, it returns an
object of type `Generator` (see below and [§§](16-classes.md#class-generator)), which implements the interface
[`Iterator`](15-interfaces.md#interface-iterator). As such, this allows that object to be iterated over
using the [`foreach` statement](11-statements.md#the-foreach-statement). During each iteration, the Engine
calls the generator function implicitly to get the next key/value pair.
Then the Engine saves the state of the generator for subsequent
key/value pair requests.

This operator produces the result `null` unless the method
[`Generator->send`](16-classes.md#class-generator) was called to provide a result value. This
operator has the side effect of generating the next value in the
collection.

Before being used, an *element-key* must have, or be converted to, type
`int` or `string`. Keys with `float` or `bool` values, or strings whose contents
match exactly the pattern of [*decimal-literal*](09-lexical-structure.md#integer-literals), are
[converted to `int`](08-conversions.md#converting-to-integer-type). Values of all other key types are [converted to `string`](08-conversions.md#converting-to-string-type).

If *element-key* is omitted from an *array-element-initializer*, an
element key of type `int` is associated with the corresponding
*element-value*. The key associated is one more than the previously
assigned int key for this collection. However, if this is the first
element in this collection with an `int` key, key zero is used. If
*element-key* is provided, it is associated with the corresponding
*element-value*. The resulting key/value pair is made available by
`yield`.

If *array-element-initializer* is omitted, default int-key assignment is
used and each value is `null`.

A generator function's return type is `Generator<Tk, Tv, Ts>`, where `Tk` is the type of "key" (must be `int` if there is no key), `Tv` is the type of "value", and "`?Ts`" is the result of the entire yield expression. `Continuation<T>` is an alias for `Generator<int, T, void>`; the two are interchangeable. For a generator function that always returns a single value of the same type `T`, declare the return type as `Continuation<T>`.

**Examples**

```Hack
function getTextFileLines(string $filename): Continuation<string> {
  $infile = fopen($filename, 'r');
  if ($infile == false) { /* deal with the file-open failure */ }

  try {
    while ($textLine = fgets($infile))  // while not EOF {
      $textLine = rtrim($textLine, "\r\n"); // strip off terminator
      yield $textLine;
    }
  } finally {
    fclose($infile);
  }
}
foreach (getTextFileLines("Testfile.txt") as $line) { /* process each line */ }
// -----------------------------------------
function series(int $start, int $end, string $keyPrefix = ""):
  Generator<string, int, void> {
  for ($i = $start; $i <= $end; ++$i) {
    yield $keyPrefix . $i => $i;  // generate a key/value pair
  }
}
foreach (series(1, 5, "X") as $key => $val) { /* process each key/val pair */ }
```

## Constant Expressions

**Syntax**

<pre>
  <i>constant-expression:</i>
    <i>array-creation-expression</i>
    <i>collection-literal</i>
    <i>tuple-literal</i>
    <i>shape-literal</i>
    <i>const-expression</i>

  <i>const-expression:</i>
    <i>expression</i>
</pre>

**Defined elsewhere**

* [*array-creation-expression*](10-expressions.md#array-creation-operator)
* [*collection-literal*](10-expressions.md#collection-literals)
* [*expression*](10-expressions.md#yield-operator)
* [*shape-literal*](10-expressions.md#shape-literals)
* [*tuple-literal*](10-expressions.md#tuple-literals)

**Constraints:**

All of the *element-key* and *element-value* *expression*s in
[*array-creation-expression*](10-expressions.md#array-creation-operator) must be [*literals*](09-lexical-structure.md#general-2), or *tuple-literals* and/or *shape-literals* containing only *literals*.

All of the *expression*s in a *collection-literal* must be *literals*.

All of the *expression*s in *tuple-literal* must be *literals*.

All of the *expression*s in *shape-literal* must be *literals*.

*expression* must have a scalar type, and be a literal or the name of an
existing [c-constant](06-constants.md#general) that is currently in scope.

**Semantics:**

A *const-expression* is the value of a c-constant. A *const-expression*
is required in several contexts, such as in initializer values in a
[*const-declaration*](16-classes.md#constants) and default initial values in a [function
definition](15-functions.md#function-definitions).
