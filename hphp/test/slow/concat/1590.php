<?hh

function test($a, $b) {
  return $a . "\0" . $b . "\0" . $a . $b . $a . $b;
}

<<__EntryPoint>>
function main_1590() {
var_dump(json_encode(test('x', 'y')));
}
