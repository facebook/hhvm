<?hh

class c {
  protected $cm = 'get';
  function x() {
    var_dump($this->cm);
  }
}
class c2 extends c {
}

<<__EntryPoint>>
function main_1498() {
$y = new c;
$y->x();
$z = clone $y;
$z->x();
$y = new c2;
$y->x();
$z = clone $y;
$z->x();
}
