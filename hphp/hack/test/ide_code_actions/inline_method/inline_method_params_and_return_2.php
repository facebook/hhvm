<?hh

class A {
  private function inline_me(): void {
    $x = 3;
    $z = $a . $b;
    echo $z;
  }
  public function foo(): void {
    // We remove the call when it's to a void function.
    $this->/*range-start*/inline_me/*range-end*/();
    echo "done";
  }
}
