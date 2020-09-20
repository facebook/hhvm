<?hh

abstract class Base {
  abstract function thing();
}

class D1 extends Base { function thing() { return 1; } }
class D2 extends Base { function thing() { return 2; } }

function go(Base $x) {
  var_dump($x->thing());
}
function handler($name, $obj, inout $args, $data, inout $done) {
  $done = true;
  return "string!";
}


<<__EntryPoint>>
function main_intercept_func_families() {
go(new D1);
go(new D2);
fb_intercept('D2::thing', 'handler');
go(new D1);
go(new D2);
}
