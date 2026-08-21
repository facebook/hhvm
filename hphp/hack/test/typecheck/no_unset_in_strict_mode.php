<?hh

function f(): void {
  $a = "hello";
  unset($a);
}

class C {
  public int $i = 0;

  public function f(): void {
    unset($this->i);
  }
}
