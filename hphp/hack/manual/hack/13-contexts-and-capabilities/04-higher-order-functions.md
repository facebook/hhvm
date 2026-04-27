# Higher Order Functions

One may define a higher order function whose context depends on the dynamic context of one or more passed in function arguments.

```hack
function has_dependent_fn_arg(
  (function()[_]: void) $f,
)[ctx $f]: void {
  /* some code */
  $f();
  /* more code */
}
```

The semantics of the argument-dependent-context are such that the higher order function's type is specialized per invocation.

```hack error
final class Box {
  public static ?Box $global = null;
  public function __construct(public mixed $value)[] {}
}

function callee((function(Box)[_]: void) $f, Box $b)[globals, ctx $f]: void {
  /* some code */
  $f($b);
  Box::$global = $b;
  /* more code */
}

function caller()[globals]: void {
  callee(
    ($_) ==> {}, // implicitly [globals]
    new Box(null),
  ); // invoke requires context [globals]

  callee(
    ($box)[write_props] ==> {
      $box->value = 0;
    },
    new Box(null),
  ); // ERROR because invoke requires context [globals, write_props]
}
```

Note that this suggests and requires that all other code within `callee` requires only the `globals` context's capabilities, as the actual capabilities provided via the passed argument cannot be depended upon to be any specific capabilities.
