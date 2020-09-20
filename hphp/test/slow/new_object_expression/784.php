<?hh

<<__DynamicallyConstructible>>
class X {
}
class Y {
}
function test($x) {
  return new $x($x = 'y');
}

<<__EntryPoint>>
function main_784() {
var_dump(test('x'));
}
