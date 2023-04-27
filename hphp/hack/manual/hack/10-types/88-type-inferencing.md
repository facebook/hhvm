While certain kinds of variables must have their type declared explicitly, others can have their type inferred by having the implementation
look at the context in which those variables are used.  For example:

```Hack
function foo(int $i): void {
    $v = 100;
}
```

As we can see, `$v` is implicitly typed as `int`, and `$i` is explicitly typed.

In Hack,
* Types **must be declared** for properties and for the parameters and the return type of named functions.
* Types **must be inferred** for local variables, which includes function statics and parameters.
* Types **can be declared or inferred** for constants and for the parameters and return type of unnamed functions.

The process of type inferencing does not cross function boundaries.

Here's an example involving a local variable:

```Hack file:c.hack
function f(): void {
  $v = 'acb';       // $v has type string
  // ...
  $v = true;        // $v has type bool
  // ...
  $v = dict['red' => 10, 'green' => 15]; // $v has type dict<string, int>
  // ...
  $v = new C();     // $v has type C
}
```

For each assignment, the type of `$v` is inferred from the type of the expression on the right-hand side, as shown in the comments. The type
of function statics is inferred in the same manner, as are function parameters. For example:

```Hack
function g(int $p1 = -1): void {
  // on entry to the function, $p1 has the declared type int
  // ...
  $p1 = 23.56;      // $p1 has type float
  // ...
}
```

As a parameter, `$p1` is required to have a declared type, in this case, `int`. However, when used as an expression, `$p1`'s type can change, as shown.

In the case of a class constant, if the type is omitted, it is inferred from the initializer:

```Hack file:c.hack
class C {
  const C1 = 10;            // type int inferred from initializer
  const string C2 = "red";  // type string declared
}
```

Let's consider types in closures:

```Hack
$doubler = $p ==> $p * 2;
$doubler(3);
```

The type of the parameter `$p` and the function's return type have been omitted. These types are inferred each time the anonymous function
is called through the variable `$doubler`. When `3` is passed, as that has type `int`, that is inferred as the type of `$p`. The literal `2`
also has type `int`, so the type of the value returned is the type of `$p * 2`, which is `int`, and that becomes the function's return type.

We can add partial explicit type information; the following all result in the same behavior:

```Hack
$doubler = (int $p) ==> $p * 2;
$doubler = ($p = 0) ==> $p * 2;
$doubler = ($p): int ==> $p * 2;
```

In the first case, as `$p` has the declared type `int`, and `int * int` gives `int`, the return type is inferred as `int`. In the second
case, as the default value `0` has type `int`, `$p` is inferred to also have that type, and `int * int` gives `int`, so the return type
is inferred as `int`. In the third case, as the return type is declared as `int`, and `$p * 2` must have that type, the type of `$p` is
inferred as `int`, so that must also be the type of the parameter.

While all three of these cases allow a call such as `$doubler(3)`, none of them allow a call such as `$doubler(4.2)`. So, the fact that
type information can be provided explicitly in these cases doesn't mean it's necessarily a good idea to do so.
