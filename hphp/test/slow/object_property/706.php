<?hh

function test($x, $v) :mixed{
 $r = is_object($x) ? 1 : null; $x->$v = 1; var_dump($r);
 }

<<__EntryPoint>>
function main_706() :mixed{
test(new stdClass, "\0foo");
}
