<?hh

function test($x, $v) :mixed{
 unset($x->$v);
 var_dump($x);
 }

<<__EntryPoint>>
function main_716() :mixed{
test(new stdClass, "\0foo");
}
