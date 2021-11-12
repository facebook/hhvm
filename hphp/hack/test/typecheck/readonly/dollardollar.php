<?hh
class Bar {}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public static ?Bar $static_prop = null;
  public Bar $not_ro;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
    $this->not_ro = new Bar();
  }
}

function takes_readonly(readonly Foo $x) : readonly Foo {
  return $x;
}
function takes_mutable(Foo $x): Foo {
  return $x;
}

<<__EntryPoint>>
function test() : void {
  $y = readonly new Foo();
  $z = $y |> readonly takes_readonly($$) // ok
          |> takes_mutable($$); // not ok
}
