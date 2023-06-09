<?hh

class C {
  public function foo(): void {
    $a = 3;
    if (1 < 2) {
      $a = true;
    }
    $x =/*range-start*/ shape("the_union" => $a) /*range-end*/;
  }
}
