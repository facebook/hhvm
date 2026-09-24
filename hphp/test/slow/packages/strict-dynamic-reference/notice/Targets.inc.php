<?hh

namespace StrictNotice;

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
