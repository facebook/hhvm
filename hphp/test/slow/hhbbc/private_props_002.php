<?hh

/*
 * Unserialization can remove static-ness, and we don't try to enforce
 * that it has the same value, which means even if a property is only
 * initialized with a in-class initializer it should not be inferred
 * as a SStr or SArr.
 */
class SerDe {
  private $arr = array(1,2,3);
  private $str = "one two three";

  public function foo() {
    var_dump($this->arr, $this->str);
  }
}

function main() {
  $x = new SerDe;
  $x = unserialize(serialize($x));
  $x->foo();
}

main();
