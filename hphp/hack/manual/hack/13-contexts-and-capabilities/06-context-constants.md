**Note:** Context and capabilities are enabled by default since
[HHVM 4.93](https://hhvm.com/blog/2021/01/19/hhvm-4.93.html).

**Note:** Context constant *constraints* are available since HHVM 4.108.

Classes and interfaces may define context constants:

```hack no-extract
class WithConstant {
  const ctx C = [io];
  public function has_io()[self::C]: void {
    echo "I have IO!";
  }
}
```

They may be abstract,

```hack
interface IWithConstant {
  abstract const ctx C;
}
```

may have one or more bounds,

```hack no-extract
abstract class WithConstant {
  // Subclasses must require *at least* [io]
  abstract const ctx CAnotherOne as [io];
  // Subclasses must require *at most* [defaults]
  abstract const ctx COne super [defaults];
  // Subclasses must require *at most* [defaults] and *at least* [io, rand]
  abstract const ctx CMany super [defaults] as [io, rand];
}
```

and may have defaults, though only when abstract

```hack no-extract
interface IWithConstant {
  abstract const ctx C = [defaults];
  abstract const ctx CWithBound super [defaults] = [io];
}
```

When inheriting a class containing a context constant with a default, the first non-abstract class that doesn’t define an implementation of that constant  gets the default synthesized for it.


One may define a member function whose context depends on the `this` type or the exact value of context constant.

```hack no-extract
class ClassWithConstant {
  const ctx C = [io];
}

abstract class AnotherClassWithConstant {
  abstract const ctx C;
  abstract public function usesC()[this::C, ClassWithConstant::C]: void;
}
```

One may define a function whose context depends on the dynamic context constant of one or more passed in arguments.

```hack no-extract
function uses_const_ctx(SomeClassWithConstant $t)[$t::C]: void {
  $t->usesC();
}
```

One may reference the dependent context constant of a argument in later arguments as well as in the return type.

```hack no-extract
function uses_const_ctx_more(
  SomeClassWithConstant $t,
  (function()[$t::C]: void) $f
)[$t::C]: (function()[$t::C]: void) {
  $f();
  $t->usesC();
  return $f;
}
```
