<?php

class Bar{
  use _ZendTestTrait;
}

$bar = new Bar();
var_dump($bar->testMethod());
// destruction causes a double-free and explodes

?>
