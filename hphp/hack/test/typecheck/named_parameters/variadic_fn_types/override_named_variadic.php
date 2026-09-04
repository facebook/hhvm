<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>


class C {
  // Analogous situation for positional args
  public function bar(int $x, string $y, int $n = 0):void { }
  public function foo(int $x, named string $y, named int $n = 0):void { }
}

class D extends C {
  public function bar(int $x, arraykey... $rest):void { }
  public function foo(int $x, named arraykey...):void { }
}

function call_c(C $c):void {
  $c->bar(32, "A");
  $c->bar(32, "A", 12);
  $c->foo(32, y="A");
  $c->foo(32, y="A", n=12);
}

<<__EntryPoint>>
function main():void {
  $d = new D();
  call_c($d);
}
