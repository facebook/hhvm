<?hh

class Klass {
  public function foo(): void {
    $ignore1 = 1;
    /*range-start*/
    $local = 100;
    /*range-end*/
    $ignore2 = 1;
  }
}
