<?hh

function test($x, $v) :mixed{
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_708() :mixed{
  test(new stdClass(), "\0foo");
}
