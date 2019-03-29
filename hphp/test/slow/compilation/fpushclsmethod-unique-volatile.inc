<?hh

class X {
  static function foo($i) {
    $t = new static;
    $t->bar($i);
  }
  function bar($i) { if (!$i) var_dump('here'); }
}
class Y extends X {}

$x = 1;
