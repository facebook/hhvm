<?hh

class A {
  public function foo(): void {
    /*range-start*/$x = vec[1];/*range-end*/
    var_dump($x);
  }
}
