<?hh

class Foo {
  const BAR = 1;
}
function test(int $a = -Foo::BAR) {
return $a;
}

<<__EntryPoint>>
function main_2190() {
var_dump(test());
var_dump(test(2));
}
