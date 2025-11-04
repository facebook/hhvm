<?hh

// Package override to foo, but all references are from bar

<<file: __PackageOverride('foo')>>
type foo_override_typedef = int;

function foo_override_fun(): void {}

class Foo_override_class {
  public static function foo_override_static_method(): void {}
}

const int FOO_OVERRIDE_CONST = 1;
