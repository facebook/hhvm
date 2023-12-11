<?hh

function f1() :mixed{
  $x = vec[1,2,3];
  unset($x[2]);
  var_dump($x);
}
function f2() :mixed{
  $x = vec[1,2,3];
  unset($x[0][0]);
  var_dump($x);
}
function f3() :mixed{
  $x = vec[vec[4,5,6],2,3];
  unset($x[0][2]);
  var_dump($x);
}
function f4() :mixed{
  $x = vec[vec[4,5,6],2,3];
  unset($x[0][0][0]);
  var_dump($x);
}

<<__EntryPoint>>
function main_1110() :mixed{
f1();
f2();
f3();
f4();
}
