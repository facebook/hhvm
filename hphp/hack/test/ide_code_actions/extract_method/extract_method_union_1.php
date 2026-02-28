<?hh


class Klass {
  public function foo(): void {
    if (rand()) {
      $z = true;
    } else {
      $z = "";
    }
    /*range-start*/
    $x = $z;/*range-end*/
    $x + 2;
  }
}
