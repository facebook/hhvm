<?hh
function yuk($x = "sooo") {
  return $x;
}
function foo(
  $x,
  $y = null,
  $z = array("hi")
) {
  if (!$y) {
    echo ($x.$z[0]);
  }
}
foo("cappuccino");
