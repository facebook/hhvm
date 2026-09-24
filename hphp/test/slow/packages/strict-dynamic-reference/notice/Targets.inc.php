<?hh

namespace StrictNotice;

class Target {
  const int VALUE = 1;
  const type T = int;
  const type CLASS_TYPE = this;
  public static int $value = 2;
  <<__DynamicallyCallable>>
  public static function method(): void {}
  public static function undynamic(): void {}
}

<<__DynamicallyReferenced>>
class Allowed {
  public static function method(): void {}
}

<<__DynamicallyReferenced(1)>>
class Soft {
  public static function method(): void {}
}
