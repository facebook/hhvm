<?hh

function test($x, $v) :mixed{
 var_dump($x->$v += 1);
 }

<<__EntryPoint>>
function main_713() :mixed{
test(new stdClass, "");
}
