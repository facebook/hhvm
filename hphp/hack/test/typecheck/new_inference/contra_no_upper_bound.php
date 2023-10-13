<?hh

class Ctv<-T> {
  public function __construct(T $x) {}
  public function put(T $x): void {}
}

function test(): Ctv<bool> {
  $x = new Ctv(0);
  // we can forever put whatever we want in $x
  $x->put("");
  $x->put(new Ctv(2));
  return $x;
}
