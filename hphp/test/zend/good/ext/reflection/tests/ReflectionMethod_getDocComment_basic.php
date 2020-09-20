<?hh
/**
 * My Doc Comment for A
 */
class A {
    /**
     * My Doc Comment for A::f
     */
    function f() {}

    /**
     * My Doc Comment for A::privf
     */
    private function privf() {}

    /** My Doc Comment for A::protStatf */
    protected static function protStatf() {}

    /**

     * My Doc Comment for A::finalStatPubf
     */
	final static public function finalStatPubf() {}
	
}


class B extends A {
    /*** Not a doc comment */
    function f() {}

    /** *
     * My Doc Comment for B::privf
     */




    private function privf() {}


    /** My Doc Comment for B::protStatf 




    */
    protected static function protStatf() {}

}
<<__EntryPoint>>
function main_entry(): void {

  foreach (varray['A', 'B'] as $class) {
      $rc = new ReflectionClass($class);
      $rms = $rc->getMethods();
      foreach ($rms as $rm) {
          echo "\n\n---> Doc comment for $class::" . $rm->getName() . "():\n";
          var_dump($rm->getDocComment());
      }
  }
}
