<?hh

function test1() {
  $x = __hhvm_intrinsics\launder_value(varray[1, 2, 3]);
  $x[] = 4;
  $v = new Vector($x);
  $x = null;
  $v2 = $v->toVArray();
  $v = new Vector($v2);
  var_dump($v);
}


function test2() {
  $x = __hhvm_intrinsics\launder_value(darray["yes" => "no"]);
  $x["zing"] = 4;
  $v = new Map($x);
  $x = null;
  $v2 = $v->toDArray();
  $v = new Map($v2);
  var_dump($v);
}

<<__EntryPoint>>
function main() {
  test1();
  test2();
}
