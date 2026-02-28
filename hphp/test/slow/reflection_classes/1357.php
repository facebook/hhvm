<?hh

class z {
  const foo = 10;
}
class c {
  const bar = z::foo;
}

<<__EntryPoint>>
function main_1357() :mixed{
var_dump(c::bar);
$r = new ReflectionClass('c');
var_dump($r->getConstant("bar"));
}
