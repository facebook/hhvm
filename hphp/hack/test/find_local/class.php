<?hh
class C {
  function f($x) {
    $this->g($this);  // Should find both
  }
  function g($x) {
    $this->f($this); // Should not find
  }
}
