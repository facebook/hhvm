<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>


class C {
  public function bar(int $x, string... $rest):void { }
  public function foo(int $x, named string...):void { }
}

class D extends C {
  public function bar(int $x, arraykey $y, arraykey $a):void { }
  public function foo(int $x, named arraykey $y, named arraykey $a):void { }
}

function call_c(C $c):void {
  $c->bar(32, "A", "B");
  $c->foo(32, y="A", z="B");
}

<<__EntryPoint>>
function main():void {
  $d = new D();
  call_c($d);
}
