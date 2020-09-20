<?hh // partial

class Foo {}
class Bar {}

function getBar() {
    return new Bar();
}

function my_non_rx_function($foo) {
  return getBar();
}

<<__RxLocal>>
function my_local_rx_function() {
  $x = \HH\Rx\mutable(new Foo());
  $y = \HH\Rx\mutable(my_non_rx_function($x));
}
