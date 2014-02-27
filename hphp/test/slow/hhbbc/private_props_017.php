<?hh

/*
 * Test a case where unset is supposed to define an uninitialized
 * property.
 *
 * In practice, we aren't implementing this correctly in hhvm right
 * now, so it doesn't really matter.  Also, in practice, once you've
 * unset a property in hhbbc's current type system, it will end up at
 * TCell, so we can't really see this.
 *
 * The point of this unit test is to detect regressions if/when we fix
 * either of those issues, though.  The associated code is in miProp()
 * for MIA_unset.
 */
class A {
  private $x = array(1,2,3);
  public function __construct() {
    unset($this->x);
    $y = $this;
    unset($y->x->z->p); // Should define "$this->x" to null, but
                        // doesn't at the time of this writing.
    var_dump($y);
    var_dump($this);
  }
  public function getter() { return $this->x; }
}

function main() {
  $a = new A();
  return $a->getter();
}
main();
