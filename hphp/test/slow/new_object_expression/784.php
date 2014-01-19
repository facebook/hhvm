<?php

class X {
}
class Y {
}
function test($x) {
  return new $x($x = 'y');
}
var_dump(test('x'));
