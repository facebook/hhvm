<?hh

function test($x, $v) :mixed{
 $r = is_object($x) ? $x->$v : null; $x->$v++; var_dump($r);
 }

<<__EntryPoint>>
function main_708() :mixed{
  test(new stdClass(), "\0foo");
}
