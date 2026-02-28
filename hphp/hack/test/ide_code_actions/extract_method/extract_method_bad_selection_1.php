<?hh

class Klass {
  public function foo(): void {
    /*range-start*/$x = 1;
    $y = /*range-end*/3;
  }
}
