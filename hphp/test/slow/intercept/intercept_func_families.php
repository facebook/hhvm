<?hh

abstract class Base {
  abstract function thing():mixed;
}

class D1 extends Base { function thing() :mixed{ return 1; } }
class D2 extends Base { function thing() :mixed{ return 2; } }

function go(Base $x) :mixed{
  var_dump($x->thing());
}
function handler($name, $obj, inout $args) :mixed{
  return shape('value' => "string!");
}


<<__EntryPoint>>
function main_intercept_func_families() :mixed{
go(new D1);
go(new D2);
fb_intercept2('D2::thing', handler<>);
go(new D1);
go(new D2);
}
