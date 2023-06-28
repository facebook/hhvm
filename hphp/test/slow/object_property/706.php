<?hh

function test($x, $v) :mixed{
 var_dump($x->$v = 1);
 }

<<__EntryPoint>>
function main_706() :mixed{
test(new stdClass, "\0foo");
}
