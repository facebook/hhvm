<?hh
namespace ROTDB\SelfMethod2;
class Foo {
  public int $x = 0;
  public static function meth_ret_mut(): Foo {
    return new Foo();
  }
  public static function meth_ret_ro(): readonly Foo {
    return readonly new Foo();
  }
  public static function run(): void {
    () ==> {
      $foo_mut = self::meth_ret_mut();
      $foo_mut->x = 3;

      $foo_ro = self::meth_ret_ro(); // inferred readonly
      $foo_ro->x = 3; // fatal
    }();
  }
}

<<__EntryPoint>>
function main(): void {
  Foo::run();
}
