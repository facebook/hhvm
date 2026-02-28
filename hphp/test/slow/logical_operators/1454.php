<?hh

function f($a) :mixed{
 var_dump('f:'.$a);
 return $a;
 }
function foo($a) :mixed{
  var_dump($a && true);
  var_dump(f($a) && true);
  var_dump(true && $a);
  var_dump(true && f($a));
  var_dump($a && false);
  var_dump(f($a) && false);
  var_dump(false && $a);
  var_dump(false && f($a));
  var_dump($a || true);
  var_dump(f($a) || true);
  var_dump(true || $a);
  var_dump(true || f($a));
  var_dump($a || false);
  var_dump(f($a) || false);
  var_dump(false || $a);
  var_dump(false || f($a));
}

<<__EntryPoint>>
function main_1454() :mixed{
foo(34);
foo(0);
}
