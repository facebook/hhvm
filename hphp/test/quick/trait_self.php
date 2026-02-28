<?hh

trait T {
  static function foo() :mixed{ self::bar(); }
  static function bar() :mixed{ var_dump(__METHOD__); }
}
<<__EntryPoint>> function main(): void {
T::foo();
}
