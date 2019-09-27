<?hh

function foo() {
  $x = new stdClass;
  $x->v = array(1, 2);
  return $x;
}

<<__EntryPoint>>
function main_695() {
foo()->v[0] += 5;
$v = foo()->v;
var_dump(shuffle(inout $v));
}
