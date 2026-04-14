<?hh

class C {
  public int $prop = 0;

  public function set_prop(): void {
    $x = 42;
    $this->prop = $x;
//                ^ enforcement-at-caret
  }
}
