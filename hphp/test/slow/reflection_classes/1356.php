<?hh
class BbBb {
}


<<__EntryPoint>>
function main_1356() :mixed{
$z=true;
if ($z) {
  include '1356-1.inc';
}
 else {
  include '1356-2.inc';
}
$r = new ReflectionClass('aaaa');
var_dump($r->getName());
$r = new ReflectionClass('bbbb');
var_dump($r->getName());
$a = new aaaa;
$a->f();
}
