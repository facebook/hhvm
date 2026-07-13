<?hh

function test($x, $v) :mixed{
 $r = is_object($x) ? $x->$v : null; $x->$v++; var_dump($r);
 }

<<__EntryPoint>>
function main_707() :mixed{
test(true, "");
test(true, "\0foo");
test(new stdClass(), "");
}
