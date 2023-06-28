<?hh
interface I { function foo():mixed; }
trait T0 implements I {}
trait T implements I { use T0; }
abstract class X implements I { use T; }
abstract class Y implements I { use T; }
class A {
  function f() :mixed{
    return self::class;
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
