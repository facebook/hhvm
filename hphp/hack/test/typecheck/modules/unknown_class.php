//// mod.php
<?hh

new module foo {}
//// file1.php
<?hh

module foo;
interface IFoo {}
internal class Sens implements IFoo {
  public static function bar(): void {}
  public function foo(): void {}
}
trait TFoo<T> {
  public ?T $x;
}
class Foo {
  use TFoo<Sens>;
}

function test(): void {
  $x = new Foo();
  $y = $x->x;
  nullthrows($y)->foo(); // ok
}

function nullthrows<T>(?T $x): T {
  if ($x is null) {
    throw new  Exception("");
  } else {
    return $x;
  }
}
function takes_ifoo(IFoo $if) : void {
}
//// file2.php
<?hh
function test2(): void {
  $x = new Foo();
  $y = nullthrows($x->x);

  takes_ifoo($y);
  $y->foo();
  $y::foo(); // TODO: improve error message
}
