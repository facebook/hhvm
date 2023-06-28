<?hh

class Y {
  function bar() :mixed{
}
}
class X {
  function foo() :mixed{
    $x = $this;
    if ($this is y) {
      $this->bar();
    }
    return $x;
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
