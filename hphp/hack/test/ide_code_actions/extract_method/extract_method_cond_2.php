<?hh

class Klass {
  public function foo(): void {
    if (1 < 2) {
      // extracting a method here should not change what is printed
      /*range-start*/ $x = 1; /*range-end*/
    } else {
      $x = 4;
    }
    echo $x;
  }
}
