# Dependent Contexts Continued

Dependent contexts may be accessed off of nullable parameters. If the dynamic value of the parameter is null, then the capability set required by that parameter is empty.

```hack
abstract class HasCtx {
  abstract const ctx C;
  public function work()[this::C]: void {}
}

function type_const(
  ?HasCtx $t,
)[$t::C]: void {
  $t?->work();
}

function fn_arg(
  ?(function()[_]: void) $f,
)[ctx $f]: void {
  if ($f is nonnull) {
    $f();
  }
}
```

Parameters used for accessing a context constant may not be reassigned.

```hack no-extract
function nope(HasCtx $t)[$t::C]: void {
  $t = get_some_other_value();
}
```

Explicit closure contexts may not be reference dependent contexts.

```hack no-extract
function f(
  (function()[_]: void) $f,
  HasCtx $t,
)[write_props, ctx $f, $t::C]: void {
  (()[ctx $f] ==> 1)(); // disallowed
  (()[$t::C] ==> 1)();  // disallowed
  (()[write_props] ==> 1)();   // allowed: not a dependent context
  (()[] ==> 1)();       // allowed
  (() ==> 1)();         // allowed: inherits [write_props, ctx $f, $t::C]
}
```
