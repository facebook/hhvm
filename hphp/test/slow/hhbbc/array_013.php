<?hh

function foo(bool $x) {
  return array('flag' => array('flag2' => $x));
}
function bar() {
  $x = foo(true);
  $y = foo(false);
  $z = $x['flag']['flag2'];
  $l = $y['flag']['flag2'];
  var_dump($z, $l);
}
bar();
