<?php

class Foo {
  const BAR = 1;
}
function test(int $a = -Foo::BAR) {
return $a;
}
var_dump(test());
var_dump(test(2));
