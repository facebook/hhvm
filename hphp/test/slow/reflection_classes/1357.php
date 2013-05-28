<?php

class z {
  const foo = 10;
}
class c {
  const bar = z::foo;
}
var_dump(c::bar);
$r = new ReflectionClass('c');
var_dump($r->getConstant("bar"));
