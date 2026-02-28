# Anonymous Functions

Hack supports anonymous functions.

In the example below, the anonymous function, `$f`, evaluates to a function that
returns the value of `$x + 1`.

``` Hack
$f = $x ==> $x + 1;

$two = $f(1); // result of 2
```

To create an anonymous function with more than one parameter, surround the parameter
list with parentheses:

``` Hack
$f = ($x, $y) ==> $x + $y;

$three = $f(1, 2);  // result of 3
```

Anonymous functions pass _by value_, not by reference. This is also true for any
[object property](/hack/expressions-and-operators/member-selection) passed to an
anonymous function.

``` Hack
$x = 5;
$f = $x ==> $x + 1;

$six = $f($x); // pass by value

echo($six); // result of 6
echo("\n");
echo($x); // $x is unchanged; result of 5
```

If you need to mutate the reference, use the HSL `Ref` class.

## Type Inference

Unlike named functions, type annotations are optional on anonymous functions.
You can still add explicit types if you wish.

``` Hack
$f = (int $x): int ==> $x + 1;
```

HHVM will enforce type annotations if they are provided.

If typechecking cannot infer a type for a function, it will show an
error, and you will need to provide a type. Adding explicit type
annotations can also help the typechecker run faster.

## Fat Arrow Syntax

`==>` defines an anonymous function in Hack. An anonymous function can
be a single expression, or a block.

``` Hack
$f1 = $x ==> $x + 1;

$f2 = $x ==> { return $x + 1; };
```

## Legacy PHP-Style Syntax

Hack also supports an anonymous function syntax similar to PHP. These
are less flexible, so we recommend using fat arrow syntax.

``` Hack
$f = function($x) { return $x + 1; };
```

PHP-style lambdas require an explicit `{ ... }` block.

PHP-style lambdas also require `use` to refer to enclosing variables. Fat
arrow lambdas do not require this.

``` Hack
$y = 1;

$f = function($x) use($y) { return $x + $y; };
```

PHP-style lambdas can also specify parameter and return types.

``` Hack
$y = 1;

$f = function(int $x): int use($y) { return $x + $y; };
```

Note that this syntax is not the same as PHP 7 lambdas, which put
`use` before the return type.
