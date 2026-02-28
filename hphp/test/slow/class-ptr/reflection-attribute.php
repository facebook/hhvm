<?hh

class Foo implements HH\ClassLikeAttribute {
  public function __construct(public int $i)[]: void {}
}

<<Foo(3)>>
class C {}

<<__EntryPoint>>
function main() {
  $c = new ReflectionClass(C::class);

  $f1 = $c->getAttributeClass(Foo::class);
  var_dump($f1->i);

  $f2 = $c->getAttributeClass(nameof Foo);
  var_dump($f2->i);
}
