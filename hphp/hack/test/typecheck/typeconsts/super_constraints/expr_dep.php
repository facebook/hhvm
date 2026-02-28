<?hh

class One {}
class Two extends One {}
class Three extends Two {}

abstract class Foo {
  abstract const type Ta as One super Two;

  abstract public function wibble(): this::Ta;
  abstract public function wobble(this::Ta $_): void;
}

function test(Foo $x): void {
  $ta = $x->wibble();
  $x->wobble($ta);
}
