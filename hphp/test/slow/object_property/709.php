<?hh

function test($x, $v) :mixed{
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_709() :mixed{
test(new stdClass, "");
}
