# Context Constants

Classes and interfaces may define context constants:

```hack
function do_write_props()[write_props]: void {}

class WithConstant {
  const ctx C = [write_props];
  public function work()[self::C]: void {
    do_write_props();
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

```hack
abstract class WithBounds {
  // Subclasses must require *at least* [write_props]
  abstract const ctx CAtLeast as [write_props];
  // Subclasses must require *at most* [defaults]
  abstract const ctx CAtMost super [defaults];
  // Subclasses must require *at most* [defaults] and *at least* [write_props, read_globals]
  abstract const ctx CBoth super [defaults] as [write_props, read_globals];
}
```

and may have defaults, though only when abstract

```hack
interface IWithDefault {
  abstract const ctx C = [defaults];
  abstract const ctx CWithBound super [defaults] = [write_props];
}
```

As with [type constants](/hack/classes/type-constants), when inheriting a class containing a context constant with a default, the first non-abstract class that doesn't define an implementation of that constant gets the default synthesized for it.

One may define a member function whose context depends on the `this` type or the exact value of a context constant.

```hack
class HasGlobals {
  const ctx C = [globals];
}

abstract class AbstractDoer {
  abstract const ctx C;
  abstract public function doWork()[this::C, HasGlobals::C]: void;
}
```

One may define a function whose context depends on the dynamic context constant of one or more passed-in arguments.

```hack no-extract
function uses_const_ctx(AbstractDoer $d)[globals, $d::C]: void {
  $d->doWork();
}
```

See [this page](/hack/contexts-and-capabilities/higher-order-functions) for further details.
