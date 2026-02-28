<?hh

class Klass {
  public function foo(): int {
    /*range-start*/
    if (1 < 2) {
      return 1;
    } else {
      $x = 4;
    }
    /*range-end*/
    return 4;
}
