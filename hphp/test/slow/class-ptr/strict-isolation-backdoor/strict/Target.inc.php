<?hh

namespace StrictBackdoor;

<<__DynamicallyReferenced>>
final class Target {
  public static function run(): void {
    echo "called target\n";
  }
}

final class Missing {
  public static function run(): void {
    echo "called missing\n";
  }
}

<<__DynamicallyReferenced(1)>>
final class Soft {
  public static function run(): void {
    echo "called soft\n";
  }
}
