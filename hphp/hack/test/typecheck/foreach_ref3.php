<?hh
function foo(): int {
  $arr = array(array(1, "hi"), array(2, "hi"), array(3, "hi"));
  $y = 0;
  foreach ($arr as &list($v1, $v2)) {
    $y += $v1++ + strlen($v2);
  }
  var_dump($arr);
  return $y;
}
foo();
