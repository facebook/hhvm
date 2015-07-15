<?hh

type s = shape('x' => ?int);

class C {
  public s $s;
  public function __construct() {
    //UNSAFE
  }
}

function test(C $c): void {
  Shapes::removeKey($c->s, 'x');
}
