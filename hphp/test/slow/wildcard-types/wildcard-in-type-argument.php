<?hh

function identity<T>(T $x):T {
  return $x;
}
class C {
  public function identity<T>(T $x):T {
    return $x;
  }
}
class G<T> {
  public function __construct(private T $item) { }
}

<<__EntryPoint>>
function main():void {
  $x = identity<_>(3);
  var_dump($x);
  $y = identity<vec<_>>(vec[2]);
  var_dump($y);
  $c = new C();
  $a = $c->identity<_>(3);
  var_dump($a);
  $b = $c->identity<vec<_>>(vec[2]);
  var_dump($b);
  $g = new G<_>(3);
  var_dump($g);
  $h = new G<vec<_>>(vec[2]);
  var_dump($h);
}
