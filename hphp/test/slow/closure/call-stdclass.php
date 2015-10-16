<?php
$foo = new StdClass;
$foo->bar = 3;
$foobar = function ($add) {
  var_dump($this->bar + $add);
};
$foobar->call($foo, 4); // prints int(7)
