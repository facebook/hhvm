<?hh

function test($x, $v) :mixed{
 $r = is_object($x) ? $x->$v : null; $x->$v++; var_dump($r);
 }

<<__EntryPoint>>
function main_709() :mixed{
test(new stdClass, "");
}
