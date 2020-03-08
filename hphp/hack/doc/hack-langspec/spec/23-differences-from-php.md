# Differences from PHP

## General
This annex identifies the PHP features that are either not supported in Hack or that have different semantics.

## Program Start-Up
Hack's strict mode does not allow access to any of the following: `$argc`, `$argv`, `$_COOKIE`,` $_ENV`, `$_FILES`, `$_GET`, `$GLOBALS`, `$_POST`, `$_REQUEST`, `$_SERVER`, or `$_SESSION`.

## Constants
Constants cannot be defined at the top level.

Regarding `define()`'s names, lookup doesn’t work like in PHP. In Hack, have to use function `constant('name')` to refer to a defined name.

`__CLASS__` cannot be used outside a class or trait.

`__TRAIT__` cannot be used outside a trait.

## Variables
Constants are not supported except in classes and interfaces.

Global variables are not supported.

Superglobals don’t exist.

## Conversions
Unlike PHP, Hack's conversion rules are very strict. As such, they may invalidate existing PHP code in the contexts of assignment, argument passing, and value returning. Specifically:
* No non-`bool` type can be converted implicitly to `bool`. All other conversions must be explicit.
* No non-`int` type can be converted implicitly to `int`. All other conversions must be explicit.
* No non-`float` type can be converted implicitly to `float`. All other conversions must be explicit.
* No non-`string` type can be converted implicitly to `string`. All other conversions must be explicit.
* For arrays of different types, no implicit conversions exist. There are no explicit conversions to any array type.
* An object type can be converted implicitly to any object type from which the first object type is directly or indirectly derived. There are no other implicit or explicit conversions.
* An object type can be converted implicitly to any interface type that object type implements directly or indirectly. An interface type can be converted implicitly to any interface type from which the first interface type is directly or indirectly derived. There are no other implicit or explicit conversions.
* No non-`resource` type can be converted implicitly to `resource`. No explicit conversions exist.

## Lexical Structure

### Comments
Hack treats comments of the forms `// strict` and `// FALLTHROUGH` in a special way.

### Names
In Hack, function and method names are case-sensitive.

### Keywords
The following PHP identifiers are keywords in Hack: `arraykey`, `async`, `enum`, `mixed`, `newtype`, `num`, `parent`, `self`, `shape`, `tuple`, and `type`.

## Expressions

### Primary Expressions

#### General
The name of a function cannot be used as an expression without the function-call operator. Unlike in PHP, that name is not treated as a string containing that function's name.

#### Intrinsics
The following intrinsics are not supported: `empty`, `eval`, `die`, `isset`, `print`, and `unset`.

For the intrinsic `list`, the target variables must be defined; the right-hand operand cannot be a map-like array; there must not be fewer element candidates in the source than there are target variables; and only the right-most variable can be omitted. Lists of lists are not permitted.

#### Anonymous Function-Creation
The `&` byRef notation is not permitted on the function's return type nor in the use clause.

### Postfix Operators

#### The `new` Operator
A call to a constructor without any arguments requires the function-call parentheses.

The *class-type-designator* cannot be a string containing a class name; it must be the class name itself.

#### Array Creation Operator
The `&` byRef notation is not permitted.

With the distinction between vector-like arrays and map-like arrays, either all of the initializers must contain keys or none of them can have keys.

#### Function call operator
The caller must pass an argument for each parameter not having a default value.

Each argument passed is type-checked against the corresponding parameter.

Variable functions are not permitted; however, the same thing can be achieved via the library function `fun`.

#### Member-Selection Operator
The member name must be hard-coded; it cannot be expressed as a string.

This operator cannot be used to access a static method via an instance.

#### Postfix Increment and Decrement Operators
The operand must have arithmetic type.

#### Exponentiation Operator
Both operands must have arithmetic type.

### Unary Operators

#### Prefix Increment and Decrement Operators
The operand must have arithmetic type.

#### Unary Arithmetic Operators
The operand must have arithmetic type.

#### Shell Command Operator
This operator is not supported.

#### Cast Operator
Hack does not allow casts to `array`, `binary`, `boolean`, `double`, `integer`, `object`, `real`, or `unset`.

#### Variable-Name Creation Operator

PHP allows variables to reference each other by means of a "variable variable". There are two syntaxes in PHP; if `$y` is equal to the string `"x"` then `$$y` and `${$y}` are both aliases for `$x`. This operator is not supported.

### `instanceof` Operator
The right-hand operand cannot be a string.

### Multiplicative Operators
The operands of the `*` and `/` operators must have arithmetic type.

The operands of the `%` operator must have integer type.

### Bitwise Shift Operators
Both operands must have type `int`; there is no implicit conversion.

### Bitwise `AND` Operator
Both operands must have type `int`; there is no implicit conversion.

### Bitwise Exclusive `OR` Operator
Both operands must have type `int`; there is no implicit conversion.

### Bitwise Inclusive `OR` Operator
Both operands must have type `int`; there is no implicit conversion.

### Assignment Operators

#### byRef Assignment
This is not supported.

### Logical `AND`, `OR`, `XOR` Operators (Alternate Forms)
The `and`, `xor`, and `or` alternate forms are not supported.

### String Literals

PHP allows the syntax `"${ expression }"` in a double-quoted string literal, where the expression can be the name of a variable, or any expression which evaluates to a string naming a variable. This is an invalid string interpolation syntax in Hack.

## Statements

### General
Statements cannot exist at the top level of a script.

### Labeled Statements
There are no named labels or `goto` statement.

### The `if` Statement
The alternate `: endif;` syntax is not supported.

### The `switch` Statement
The alternate `: endswitch;` syntax is not supported.

Unlike PHP, in Hack, each label expression's type must be a subtype of the switch expression type. For example, `switch(10)` won’t work with case `$a < $b:`.

### The `while` Statement
The alternate `: endwhile;` syntax is not supported.

### The `for` Statement
The alternate `: endfor;` syntax is not supported.

### The `foreach` Statement
The alternate` : endforeach;` syntax is not supported.

Unlike PHP, given `foreach ($colors as $index => $color)`, in Hack, the names `$index` and `$color` are not in scope outside the `foreach` body.

### The `goto` Statement
This statement is not supported.

### The `continue` Statement
`continue n;` is not supported.

In PHP, `continue;` inside a `switch` statement is equivalent to` break;`. Hack does not emulate this.

### The `break` Statement
`break n;` is not supported.

### The `return` Statement
This can't appear in a finally block.

This can’t be used to terminate a require file.

### The declare Statement
This is not supported.

## Script Inclusion
`include` and `include_once` are not supported.

`require` and `require_once` can be used only at the top level. They are not operators, so they do not produce a value.

## Functions
Nested/conditional functions are not supported; except for anonymous functions, functions can only be defined at the top level.

Passing and/or return by reference is not supported.

The `array` and `callable` type hints are not supported.

Every parameter must have a type.

If any parameter has a default argument value, then all parameters following it must also have one.

An empty parameter list means, "no arguments can be passed".

A return type is required and a return type of `void` won’t let a value be returned. A non-`void` return type requires a return value of some subtype.

## Classes

### Class Members
All members except `const` must have a visibility modifier; there is no defaulting.

### Dynamic Members
Dynamic properties are not supported.

Dynamic methods are supported, but `__call` cannot be called directly.

### Properties
A type specifier is needed.

The `var` modifier is not supported.

All properties of non-nullable type must be initialized explicitly either by a *property-initializer* or by the constructor. Properties of nullable type that are not explicitly initialized take on the value null. (as with PHP).

### Methods
Hack requires parameter and return-type type specifiers.

###  Constructors
The deprecated form of constructor name (using the name of the class) is not supported.

### Methods with Special Semantics

#### General
Hack requires parameter and return-type type specifiers.

#### Method `__call`
It works, but it can't be called directly.

#### Method `__get`
This method does not exist, as dynamic properties are not supported.

#### Method `__invoke`
Using an instance like a function call is not supported.

#### Method __isset
This method does not exist, as dynamic properties are not supported.

#### Method __set
This method does not exist, as dynamic properties are not supported.

#### Method __set_state
This is not useful, as the intrinsic `eval` is not supported.

#### Method __unset
This method does not exist, as dynamic properties are not supported.

### Predefined Classes

#### Class Closure
This type does not exist.

## Interfaces

### Interface Members
All members except `const` must have a visibility modifier; there is no defaulting.

### Methods
Hack requires parameter and return-type type specifiers.

### Predefined Interfaces
The interfaces `ArrayAccess`, `Iterator`, `IteratorAggregate`, and `Traversable` are now generic interfaces.

## Traits

### Trait Declarations
Hack doesn’t support aliases and selection in a trait.

## Namespaces

### Namespace Use Declarations
Can’t have a use clause, with or without an “as” at other than the top level of a script.
