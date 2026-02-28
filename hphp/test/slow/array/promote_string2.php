<?hh

class X {
  function __toString()[] :mixed{ return __METHOD__; }
}

function test($a) :mixed{
  var_dump($a[-1] = new X);
  var_dump($a);
}

<<__EntryPoint>>
function main_promote_string2() :mixed{
test("x");
echo "2\n";
}
