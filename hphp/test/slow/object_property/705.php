<?hh

function test($x, $v) :mixed{
 var_dump($x->$v = 1);
 }

<<__EntryPoint>>
function main_705() :mixed{
test(new stdClass, "");
}
