<?hh

class Asd {
  static $heh = Map { 'a' => 'b' };
  function foo() {
    var_dump(self::$heh);
  }
}

function x() {
  $y = new Asd();
  $y->foo();
  var_dump($y);
}

x();
