<?hh

<<file: __EnableUnstableFeatures('readonly')>>

class Bar {}
class Foo {
  public function __construct(public int $prop)[] {}
  public readonly function get(): int {
    return $this->prop + 0;
  }
  public function set($x) : void {
    $this->prop = $x;
  }
}

<<__EntryPoint>>
function test() : void {
  $x = new Foo(4);
  $x->set(5); // ok
  $y = readonly new Foo(5);
  $z = $y->get(); // ok
  $y->set($z); /* not ok */
  $id = __hhvm_intrinsics\launder_value('set');
  $y->$id($z); /* not ok */
  echo "Done\n";
}
