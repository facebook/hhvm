<?hh

class A {
  /** This is the doc comment
   * for the function foo
  */
  public function foo(): void {
    /*range-start*/$x = vec[1];/*range-end*/
    var_dump($x);
  }
}
