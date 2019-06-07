<?hh

class c {
  function x() {
    var_dump($this);
  }
}

<<__EntryPoint>>
function main_732() {
$x = new c;
$x->x();
}
