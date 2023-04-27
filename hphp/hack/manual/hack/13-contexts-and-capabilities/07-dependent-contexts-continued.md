**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

Dependent contexts may be accessed off of nullable parameters. If the dynamic value of the parameter is null, then the capability set required by that parameter is empty.

```hack no-extract
function type_const(
  ?SomeClassWithConstant $t,
)[$t::C]: void {
  $t?->foo();
}

function fn_arg(
  ?(function()[_]: void) $f,
)[ctx $f]: void {
  if ($f is nonnull) {
    $f();
  }
}
```

Parameters used for accessing a dependent context may not be reassigned.

```hack no-extract
function nope(SomeClassWithConstant $t, (function()[_]: void) $f)[$t::C, ctx $f]: void {
  // both disallowed
  $t = get_some_other_value();
  $f = get_some_other_value();
}
```

Dependent contexts may not be referenced within the body of a function. This restriction may be relaxed in a future version.

```hack no-extract
function f(
  (function()[_]: void $f,
  SomeClassWithConstant $t,
)[rand, ctx $f, $t::C]: void {
  (()[ctx $f] ==> 1)(); // Disallowed
  (()[$t::C] ==> 1)();  // Disallowed
  (()[rand] ==> 1)();   // Allowed, not a dependent context
  (()[] ==> 1)();       // Allowed
  (() ==> 1)();         // Allowed. Note that this is logically equivalent to [rand, ctx $f, $t::C]
}
```
