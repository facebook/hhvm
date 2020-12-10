<?hh

class Foo {}
class Bar {}

<<__Rx>>
function my_rx_function(): Bar {
  return new Bar();
}

<<__RxLocal>>
function my_local_rx_function(): void {
  $y = \HH\Rx\mutable(my_rx_function());
}
