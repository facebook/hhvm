<?hh
class Foo {
  public int $prop = 44;

  public function foo() :readonly mixed{
    $x = function (): readonly {
      echo '$x';
      echo "\n";
    };
    $f = function (): readonly use ($x) {
      echo '$f';
      echo "\n";
    };
    readonly $f();
    $h = () : readonly ==>  {
      return 5;
    };
    $g = () : readonly int ==> {
      return 5;
    };
    readonly $x();
    readonly $g();
    return readonly $h();
  }
}

<<__EntryPoint>>
function main() : void {
  $x = new Foo();
  echo(readonly $x->foo());
  // error, need to write readonly
  $x->foo();
}
