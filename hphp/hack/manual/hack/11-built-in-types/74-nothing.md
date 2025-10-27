# Nothing

The type `nothing` is the bottom type in the Hack typesystem. This means that there is no way to create a value of the type `nothing`. `nothing` only exists in the typesystem, not in the runtime.

The concept of a bottom type is quite difficult to grasp, so I'll first compare it to the supertype of everything `mixed`. `mixed` is the most general thing you can imagine within the hack typesystem. Everything "extends" `mixed` if you will. `nothing` is the exact opposite of that.

Let's work out the hierarchy of scalar types. Forget about nullable types and `dynamic` for the moment, they would make this example far more complex without adding much value.

 - `mixed` is at the top. Everything is a subtype of `mixed`, either directly (types that have no other supertypes) or indirectly (via their supertypes).
 - `num` is a subtype of `mixed`.
 - `arraykey` is a subtype of `mixed`.
 - `bool` is a subtype of `mixed`.
 - `int` is a subtype of `num` and `arraykey`.
 - `float` is a subtype of `num`.
 - `string` is a subtype of `arraykey`.
 - `nothing` is a subtype of `int`, `float`, `string`, and `bool`.

The important thing to note here is that `nothing` is never between two types. `nothing` only shows up right below a type with no other subtypes.

## Usages

When defining a function that will never return (it either throws, loops forever, or terminates the request) you can use `nothing` for the return type. This gives more information to the caller than `void` and is more flexible than [noreturn](/hack/built-in-types/noreturn). `nothing` can be used in expressions (like `nullable T ?? nothing`) and it will typecheck "as if it wasn't there", since `(T | nothing)` is _just_ `T`.

`nothing` can be used to create a `throw` expression in this way.

```hack
function throw_as_an_expression(\Throwable $t): nothing {
  throw $t;
}

function returns_an_int(?int $nullable_int): int {
  // You can not use a `throw` statement in an expression bodied lambda.
  // You need to add curly braces to allow a `throw` statement.
  $throwing_lambda = () ==> {
    throw new \Exception();
  };

  $throwing_expr_lambda = () ==> throw_as_an_expression(new \Exception());

  // You can't write a statement on the RHS of an operator, because it operates on expressions.
  // The type of the `??` operator is `(nothing | int)`, which simplifies to `int`,
  // so this return statement is valid.
  return $nullable_int ?? throw_as_an_expression(new \Exception());
}

<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  echo returns_an_int(1);
}
```

<hr />

When writing a new bit of functionality, you may need to pass a value to a function you can't produce without a lot of work. `nothing` can be used as a placeholder value in place of any type without causing type errors. The typechecker will continue checking the rest of your program and the runtime will throw if this code path gets executed. I have called this function `undefined`, as an homage to Haskell [undefined](https://wiki.haskell.org/Undefined).

```hack file:undefined.hack
type undefined = nothing;

function undefined(): undefined {
  throw new Exception('NOT IMPLEMENTED: `undefined` cannot be produced.');
}
```

And here is how to use it

```hack file:undefined.hack
interface MyInterface {
  public function isAmazed(): bool;
}

function do_something(MyInterface $my_interface): bool {
  return $my_interface->isAmazed();
}

<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  $my_interface = undefined();
  // We won't ever reach this line, since `undefined()` will halt the program by throwing.
  // We can't produce a MyInterface just yet, since there are no classes which implement it.
  // `undefined` is a placeholder for now.
  // We can continue writing our business logic and come back to this later.
  if (do_something($my_interface)) {
    // Write the body first, worry about the condition later.
  }
}
```

You could make your staging environment remove the file which declared the `undefined()` function. That way you'll get a typechecker error when you accidentally push code that has these placeholders in it. This prevents you from accidentally deploying unfinished code to production.

<hr />

When making a new / empty `Container<T>`, Hack will infer its type to be `Container<nothing>`. It is not that there are actual value of type `nothing` in the `Container<T>`, it is just that this is a very nice way of modeling empty `Container<T>`s.

Should you be able to pass an empty vec where a `vec<string>` is expected? Yes, there is no element inside that is not a `string`, so that should be fine. You can even pass the same vec into a function that takes a `vec<bool>` since there are no elements that are not of type `bool`. What are you allowed to do with the `$nothing` of this foreach? Well, you can do anything to it. Since nothing is a subtype of everything, you can pass it to any method and do all the things you want to.

```hack
function takes_vec_of_strings(vec<string> $_): void {}
function takes_vec_of_bools(vec<bool> $_): void {}

<<__EntryPoint>>
async function main_async(): Awaitable<void> {
  $empty_vec = vec[];
  takes_vec_of_bools($empty_vec);
  takes_vec_of_strings($empty_vec);

  foreach ($empty_vec as $nothing) {
    $nothing->whatever();
    takes_vec_of_strings($nothing);
  }
}
```

<hr />

To make an interface that requires that you implement a method, without saying anything about its types. This does still make a requirement about the amount of parameters that are required parameters.

```hack
interface DontForgetToImplementShipIt {
  public function shipIt(nothing $_): mixed;
}

abstract class Software implements DontForgetToImplementShipIt {
}

class HHVM extends Software {
  public function shipIt(string $version): string {
    return 'Shipping HHVM version '.$version.'!';
  }
}

class HSL extends Software {
  private function __construct(public bool $has_new_functions) {
  }

  public function shipIt(bool $has_new_functions): HSL {
    return new HSL($has_new_functions);
  }
}

class HHAST extends Software {
  public function shipIt(Container<string> $linters): void {
    foreach ($linters as $linter) {
      invariant(
        Str\ends_with($linter, 'Linter'),
        'Linter %s does not have a name that ends in "Linter"!',
        $linter,
      );
    }
  }
}
```

It is important to note that `Software::shipIt()` is not directly callable without knowing what subtype of `Software` you have.

<hr />

Contravariant generic types can use `nothing` to allow all values to be passed. This acts in a similar way that `mixed` acts of covariant generics, such as `vec<mixed>`.

```hack
final class MyClass<-T> {
  public function consume(T $value): void {}
  public function someOtherMethod(): void {}
}

// We don't use the `T` from `->consume(T): void` in the function,
// so we can use `nothing` for the generic and accept any and all MyClass instances.
function some_function(MyClass<nothing> $a, MyClass<nothing> $b): void {
  if ($a !== $b) {
    $b->someOtherMethod();
    echo "different\n";
  }
}

<<__EntryPoint>>
async function main_async(): Awaitable<void> {

  $my_class_int = new MyClass<int>();
  $my_class_string = new MyClass<string>();

  some_function($my_class_int, $my_class_string);
}
```
