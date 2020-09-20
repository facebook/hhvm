<?hh // strict

<<Foo('hi')>>
class C {
  <<Foo(varray[1000],6), Bar('lel'), Baz(7)>>
  public function foo(int $n): void {}
}

class D extends C {
  <<__Override>>
  public function foo(int $n): void {}
}
// hhvm seems inconsistent in how this array is sorted, so...
function dump(array<string, mixed> $x): void {
  /* HH_FIXME[2049] */
  /* HH_FIXME[4107] */
  ksort($x);
  var_dump($x);
}

function test(): void {
  /* HH_FIXME[2049] */
  $rc = new ReflectionClass('C');
  dump($rc->getAttributes());
  var_dump($rc->hasAttribute('Foo'));
  var_dump($rc->hasAttribute('Bar'));
  dump($rc->getMethod('foo')->getAttributes());
  var_dump($rc->hasAttribute('Baz'));
  var_dump($rc->hasAttribute('Qux'));
  /* HH_FIXME[2049] */
  $rc = new ReflectionClass('D');
  dump($rc->getMethod('foo')->getAttributes());
  var_dump($rc->hasAttribute('__Override'));
  var_dump($rc->hasAttribute('__Memoize'));
}
