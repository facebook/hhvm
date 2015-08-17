<?hh

namespace MyNamespace;

type Foo = shape('herp' => ?string);

function do_stuff(Foo $shape): void {
  $herp = Shapes::idx($shape, 'herp');
  var_dump($herp);
}

do_stuff(shape('herp' => 'derp'));
