<?hh

class Asd {
  public static $heh = Map { 'a' => 'b' };
  function foo() :mixed{
    var_dump(self::$heh);
  }
}

function x() :mixed{
  $y = new Asd();
  $y->foo();
  var_dump($y);
}


<<__EntryPoint>>
function main_static_inits() :mixed{
x();
}
