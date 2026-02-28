<?hh

//////////////////////////////////////////////////////////////////////

class Dtor {
 }

function id($x) :mixed{
  return $x;
}

function test1() :mixed{
  $k = new Dtor();
  id($k);
}

function test2() :mixed{
  id(new Dtor());
}

function test3() :mixed{
  echo id("haha");
  echo "\n";
}

function printer($x, $y) :mixed{
  echo $x;
  echo $y;
  echo "\n";
}

function test31() :mixed{
  printer("asd ", id("foo"));
}

function test32() :mixed{
  echo id(id("foo"));
  echo "\n";
}


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_1() :mixed{
test1();
test2();
test3();
test31();
test32();
}
