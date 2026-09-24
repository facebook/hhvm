<?hh

namespace StrictEnforced;

class Target {
  const int VALUE = 1;
  public static int $value = 2;
  public static function method(): void {}
}

<<__DynamicallyReferenced>>
class Allowed {
  public static function method(): void {}
}

<<__DynamicallyReferenced(1)>>
class Soft {
  public static function method(): void {}
}

function same_package_classname_to_class(): void {
  \HH\classname_to_class(
    \__hhvm_intrinsics\launder_value(nameof Allowed)."",
  );
}
