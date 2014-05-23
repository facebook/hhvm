<?php
trait T2 {
  function F() {}
}
trait T3 {
  use T2 {
    F as G;
  }
}
$rc3 = new ReflectionClass('T3');
var_dump($rc3->getTraitAliases());
