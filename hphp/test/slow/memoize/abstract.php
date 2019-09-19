<?hh

abstract class foo {
  <<__Memoize>>
  abstract protected static function bar();
}

<<__EntryPoint>> function main(): void {}
