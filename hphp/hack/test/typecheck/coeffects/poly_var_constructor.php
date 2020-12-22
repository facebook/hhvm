<?hh

abstract class A { abstract const type C; }

class X extends A { const type C = int; }
class Y extends A { const type C = string; }

class C {
  public function __construct(A $a)[$a::C]: void {}
}

function x(X $x, Y $y)[int]: void {
  new C($x);
  new C($y);
}

function y(X $x, Y $y)[string]: void {
  new C($x);
  new C($y);
}
