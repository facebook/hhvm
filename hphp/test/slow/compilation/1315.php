<?hh

class X {
  function foo() :mixed{
    return function() {
      return $this->bar();
    }
;
  }
  function bar() :mixed{
}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
