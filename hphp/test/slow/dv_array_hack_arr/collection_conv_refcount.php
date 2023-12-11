<?hh

function test1() :mixed{
  $x = __hhvm_intrinsics\launder_value(vec[1, 2, 3]);
  $x[] = 4;
  $v = new Vector($x);
  $x = null;
  $v2 = $v->toVArray();
  $v = new Vector($v2);
  var_dump($v);
}


function test2() :mixed{
  $x = __hhvm_intrinsics\launder_value(dict["yes" => "no"]);
  $x["zing"] = 4;
  $v = new Map($x);
  $x = null;
  $v2 = $v->toDArray();
  $v = new Map($v2);
  var_dump($v);
}

<<__EntryPoint>>
function main() :mixed{
  test1();
  test2();
}
