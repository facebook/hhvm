<?hh

  class X {
 static function foo() {
 var_dump(__METHOD__);
 }
 }
  class Y extends X {
}

$x = 1;
