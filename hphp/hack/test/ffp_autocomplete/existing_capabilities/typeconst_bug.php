<?hh
// AUTOCOMPLETE 9 12
/**
 * Regression test: running autocomplete on a file with erroneous typeconst
 * used to crash the typechecker.
 */
class C {
  public function test(): C::Z {
    $this->
  }
}
