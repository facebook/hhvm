A variable is a named area of data storage that has a type and a value.  Distinct variables may have the same name provided
they are in different [scopes](scope.md).  A [constant](constants.md) is a variable that, once initialized, its value cannot
be changed.   Based on the context in which it is declared, a variable has a scope.

The following kinds of variable may exist in a script:
-   [Local variable](#local-variables)
-   [Array element](#array-elements)
-   [Instance property](#instance-properties)
-   [Static property](#static-properties)
-   [Class and interface constant](#class-and-interface-constants)

## Local Variables

Except for function parameters, a local variable is never defined explicitly; instead, it is created when it is first
assigned a value. A local variable can be assigned to as a parameter in the parameter list of a function definition or
inside any compound statement. It has function scope.

Consider the following example:

```Hack
function do_it(bool $p1): void {
  $count = 10;
  // ...
  if ($p1) {
    $message = "Can't open file.";
    // ...
  }
  // ...
}

function call_it(): void {
  do_it(true);
}
```

Here, the parameter `$p1` (which is a local variable) takes on the value `true` when `do_it` is called. The local
variables `$count` and `$message` take on the type of the respective value being assigned to them.

Consider the following example:

```hack
function f(): void {
  $lv = 1;
  echo "\$lv = $lv\n";
  ++$lv;
}

<<__EntryPoint>>
function main(): void {
  for ($i = 1; $i <= 3; ++$i)
    f();
}
```

In this example, the value of the local variable `$lv` is not preserved between
the function calls, so this function `f` outputs "`$lv = 1`" each time.

## Array Elements

An array is created via a vec-literal, a dict-literal, a
keyset-literal. At the same time, one or more elements may be created
for that array. New elements are inserted into an existing array via
the [simple-assignment](../expressions-and-operators/assignment.md)
operator in conjunction with the [subscript
`[]`](../expressions-and-operators/subscript.md) operator.

The scope of an array element is the same as the scope of that array's name.

```Hack
$colors1 = vec["green", "yellow"];   // create a vec of two elements
$colors1[] = "blue";                 // add element 2 with value "blue"
$colors2 = dict[];                   // create an empty dict
$colors2[4] = "black";               // create element 4 with value "black"
```

## Instance Properties

These are described in the [class instance properties](../classes/properties.md) section. They have class scope.

## Static Properties

These are described in the [class static properties](../classes/properties.md) section. They have class scope.

## Class and Interface Constants

These are described in the [class constants](../classes/constants.md) section. They have class or interface scope.
