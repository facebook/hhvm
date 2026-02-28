<?hh

class Foo {
  const BAR = 1;
}
function test(int $a = -Foo::BAR) :mixed{
return $a;
}

<<__EntryPoint>>
function main_2190() :mixed{
var_dump(test());
var_dump(test(2));
}
