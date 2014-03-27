<?php

abstract class Base {
  abstract function thing();
}

class D1 extends Base { function thing() { return 1; } }
class D2 extends Base { function thing() { return 2; } }

function go(Base $x) {
  var_dump($x->thing());
}

go(new D1);
go(new D2);
function handler($name, $obj, $args, $data, &$done) {
  $done = true;
  return "string!";
}
fb_intercept('D2::thing', 'handler');
go(new D1);
go(new D2);
