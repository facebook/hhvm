<?hh // strict

class C {
  public function foo():void { }
}

type s = shape('x' => C);

function test(s $s): void {
  $x = Shapes::idx($s, 'x');
  $x?->foo();
}
