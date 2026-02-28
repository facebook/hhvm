<?hh

class Bar {}

class Foo {
  public function testObjGetLvar(dynamic $d): void {
    $x = 'foo';
    (new Bar())->$x;
    (new Bar())->$x();
    $this->$x;
    $this->$x();
    $this->$x = 1;
    $y = null as ?Bar;
    $y?->$x;
    (null as ?Bar)?->$x;

    // Lvar on dynamic shouldn't error.
    $d->$x;
    $d?->$x;
    $d->$x = 1;
    $d->$x();
  }
}
