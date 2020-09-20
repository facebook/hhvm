<?hh

function foo() {
  yield "abc" => "def";
}


<<__EntryPoint>>
function main_g13() {
$x = foo();
$y = clone $x;
foreach($x as $k => $v) {
  var_dump($k, $v);
}
foreach($y as $k => $v) {
  var_dump($k, $v);
}
}
