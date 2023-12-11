<?hh

function foo() :mixed{
  $x = new stdClass;
  $x->v = vec[1, 2];
  return $x;
}

<<__EntryPoint>>
function main_695() :mixed{
foo()->v[0] += 5;
$v = foo()->v;
var_dump(shuffle(inout $v));
}
