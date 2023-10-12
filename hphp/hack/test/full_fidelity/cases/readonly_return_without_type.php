<?hh
class Foo {
  public int $prop = 44;

  public function foo() : readonly {
    $x = function (): readonly {};
    $f = function (): readonly use ($x) {};
    $f = () : readonly ==>  {};
    $g = () : readonly int ==> {
      return 5;
    };
    return 5;
  }
}
