<?hh

function f1() {
  $x = varray[1,2,3];
  unset($x[0]);
  var_dump($x);
}
function f2() {
  $x = varray[1,2,3];
  unset($x[0][0]);
  var_dump($x);
}
function f3() {
  $x = varray[varray[4,5,6],2,3];
  unset($x[0][0]);
  var_dump($x);
}
function f4() {
  $x = varray[varray[4,5,6],2,3];
  unset($x[0][0][0]);
  var_dump($x);
}

<<__EntryPoint>>
function main_1110() {
f1();
f2();
f3();
f4();
}
