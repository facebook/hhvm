<?hh

namespace ImplicitMember;

class Target {
  <<__DynamicallyCallable>>
  public static function method(): void {}
}
