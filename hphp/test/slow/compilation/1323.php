<?hh

class Y {
  function bar() {
}
}
class X {
  function foo() {
    $x = $this;
    if ($this is y) {
      $this->bar();
    }
    return $x;
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
