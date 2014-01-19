<?php

define('FOO', 3);
define('BAR', true);
define('GOO', FOO + 4);
define('HOO', FOO);
var_dump(FOO);
var_dump(BAR);
var_dump(GOO);
var_dump(HOO);
class A {
  const C1 = 1;
  const C2 = '2';
  const C3 = FOO;
  const C4 = BAR;
  const C5 = GOO;
  const C6 = HOO;
}
var_dump(a::C1);
var_dump(a::C2);
var_dump(a::C3);
var_dump(a::C4);
var_dump(a::C5);
var_dump(a::C6);
