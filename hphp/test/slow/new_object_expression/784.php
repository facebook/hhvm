<?hh

<<__DynamicallyConstructible>>
class X {
}
class Y {
}
function test($x) :mixed{
  return new $x($x = 'Y');
}

<<__EntryPoint>>
function main_784() :mixed{
var_dump(test('X'));
}
