<?php

trait T1 {
  function fruit() {
    yield 'apple';
    yield 'banana';
  }
}
trait T2 {
  function fruit() {
    yield 'pear';
    yield 'grape';
  }
}
class C1 {
  use T1, T2 {
    T1::fruit insteadof T2;
    T2::fruit as fruit2;
  }
}
$o = new C1;
foreach ($o->fruit() as $fruit) {
  var_dump($fruit);
}
foreach ($o->fruit2() as $fruit) {
  var_dump($fruit);
}
