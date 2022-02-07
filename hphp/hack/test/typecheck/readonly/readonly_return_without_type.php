<?hh
class Foo {
  public int $prop = 44;
  // Typechecker still requires a return type in methods
  public function foo() : readonly int {
    $x = readonly function (): readonly { return 0; };
    hh_show($x);
    $f = function (): readonly use ($x) { return $x; };
    hh_show($f);
    $f = ($x) : readonly ==>  {
      return $x + 5;
    };
    hh_show($f);
    $g = () : readonly int ==> {
      return 5;
    };
    return 5;
  }
}
