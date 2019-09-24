<?hh

class X {
  function foo() {
    return function() {
      return $this->bar();
    }
;
  }
  function bar() {
}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
