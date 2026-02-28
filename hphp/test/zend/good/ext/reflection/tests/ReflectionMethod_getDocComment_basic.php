<?hh
/**
 * My Doc Comment for A
 */
class A {
    /**
     * My Doc Comment for A::f
     */
    function f() :mixed{}

    /**
     * My Doc Comment for A::privf
     */
    private function privf() :mixed{}

    /** My Doc Comment for A::protStatf */
    protected static function protStatf() :mixed{}

    /**

     * My Doc Comment for A::finalStatPubf
     */
	final static public function finalStatPubf() :mixed{}
	
}


class B extends A {
    /*** Not a doc comment */
    function f() :mixed{}

    /** *
     * My Doc Comment for B::privf
     */




    private function privf() :mixed{}


    /** My Doc Comment for B::protStatf 




    */
    protected static function protStatf() :mixed{}

}
<<__EntryPoint>>
function main_entry(): void {

  foreach (vec['A', 'B'] as $class) {
      $rc = new ReflectionClass($class);
      $rms = $rc->getMethods();
      foreach ($rms as $rm) {
          echo "\n\n---> Doc comment for $class::" . $rm->getName() . "():\n";
          var_dump($rm->getDocComment());
      }
  }
}
