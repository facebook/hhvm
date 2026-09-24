<?hh

namespace StrictEnforced;

class Target {
  const int VALUE = 1;
  const type T = int;
  const type CLASS_TYPE = this;
  public static int $value = 2;
  <<__DynamicallyCallable>>
  public static function method(): void {}
}

<<__DynamicallyReferenced>>
class Allowed {
  const type T = int;
  <<__DynamicallyCallable>>
  public static function method(): void {}
}

<<__DynamicallyReferenced(1)>>
class Soft {
  const type T = int;
  <<__DynamicallyCallable>>
  public static function method(): void {}
}

function same_package_classname_to_class(): void {
  \HH\classname_to_class(
    \__hhvm_intrinsics\launder_value(nameof Allowed)."",
  );
}
