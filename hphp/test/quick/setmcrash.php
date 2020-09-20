<?hh

class X {
  private $foo;
  function foo(inout $b) {
    $this->foo = $b;
  }
}
<<__EntryPoint>> function main(): void {
$x = new X;
$t = null;
$x->foo(inout $t);
}
