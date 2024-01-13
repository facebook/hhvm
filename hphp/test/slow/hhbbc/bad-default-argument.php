<?hh

function foo(int $a = 123, C $b = C::BAR, int $c = 456) {
  return dict['a' => $a, 'b' => $b, 'c' => $c];
}

<<__EntryPoint>>
function main() {
  var_dump(foo());
}
