<?php

class A {
  public function gen() {
    var_dump($this);
    yield 1; yield 2; yield 3;
  }

  public static function sgen() {
    var_dump(get_called_class());
    yield 4; yield 5; yield 6;
  }
}


$a = new A();
foreach ($a->gen() as $num) { var_dump($num); }
foreach (A::sgen() as $num) { var_dump($num); }
