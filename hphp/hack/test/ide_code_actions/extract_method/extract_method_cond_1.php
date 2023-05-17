<?hh

function cond(): bool {
  return false;
}

class Klass {
  public function foo(): void {
    if (cond()) {
      while (cond()) {
        /*range-start*/
        $x = 1;
        $y = $x;
        /*range-end*/;
      }
    }
  }
}
