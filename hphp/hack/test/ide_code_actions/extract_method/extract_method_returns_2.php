<?hh

class Klass {
  public function foo(): void {
    $ignore1 = 1;
    /*range-start*/
    $return = 100;
    /*range-end*/
    $ignore2 = 1 + $return;
  }
}
