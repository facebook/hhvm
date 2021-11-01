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

    // Lvar on dynamic or any shouldn't error.
    $this->tany()->$x;
    $this->tany()->$x = 1;
    $this->tany()->$x();
    $this->tany()?->$x;
    $d->$x;
    $d?->$x;
    $d->$x = 1;
    $d->$x();
  }

  /* HH_FIXME[4030] */
  public function tany() {}
}
