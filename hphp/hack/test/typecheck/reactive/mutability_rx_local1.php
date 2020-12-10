<?hh

class Foo {}
class Bar {}

function getBar(): Bar {
    return new Bar();
}

function my_non_rx_function(Foo $foo): Bar {
  return getBar();
}

<<__RxLocal>>
function my_local_rx_function(): void {
  $x = \HH\Rx\mutable(new Foo());
  $y = \HH\Rx\mutable(my_non_rx_function($x));
}
