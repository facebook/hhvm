<?hh

//////////////////////////////////////////////////////////////////////

class Dtor {
 }

function id($x) {
  return $x;
}

function test1() {
  $k = new Dtor();
  id($k);
}

function test2() {
  id(new Dtor());
}

function test3() {
  echo id("haha");
  echo "\n";
}

function printer($x, $y) {
  echo $x;
  echo $y;
  echo "\n";
}

function test31() {
  printer("asd ", id("foo"));
}

function test32() {
  echo id(id("foo"));
  echo "\n";
}


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_1() {
test1();
test2();
test3();
test31();
test32();
}
