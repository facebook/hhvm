<?hh

function cond(): bool {
  return false;
}

class Klass {
  public function foo(): void {
    if (cond()) {
      while (cond()) {
        $x = /*range-start*/3 + 2/*range-end*/;
      }
    }
  }
}
