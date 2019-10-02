<?hh

class X {
  function foo(inout $a) {
    return $this;
  }
}

class Y {
  function foo($a) {
    if ($a) {
      $this->foo($a[0])->foo($a[0]);
    }
    return $this;
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
