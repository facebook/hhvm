<?hh

<<__DynamicallyConstructible>>
class X {
}
class Y {
}
function test($x) {
  return new $x($x = 'Y');
}

<<__EntryPoint>>
function main_784() {
var_dump(test('X'));
}
