<?hh // partial

class Foo {}
class Bar {}

<<__Rx>>
function my_rx_function() {
  return new Bar();
}

<<__RxLocal>>
function my_local_rx_function() {
  $y = \HH\Rx\mutable(my_rx_function());
}
