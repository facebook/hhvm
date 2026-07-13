<?hh

<<__DynamicallyConstructible>>
class X {
}
class Y {
}
function test($x) :mixed{
  $cls = $x; $x = 'Y'; return new $cls($x);
}

<<__EntryPoint>>
function main_784() :mixed{
var_dump(test('X'));
}
