<?php

class A {
  public function Gen() {
    var_dump($this);
    yield 1; yield 2; yield 3;
  }

  public static function SGen() {
    var_dump(get_called_class());
    yield 4; yield 5; yield 6;
  }
}


$a = new A();
foreach ($a->Gen() as $num) { var_dump($num); }
foreach (A::SGen() as $num) { var_dump($num); }
