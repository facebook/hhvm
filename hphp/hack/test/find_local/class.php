<?hh
class C {
  public function f($x) {
    $this->g($this);  // Should find both
  }
  public function g($x) {
    $this->f($this); // Should not find
  }
}
