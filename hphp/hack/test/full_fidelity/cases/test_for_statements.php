<?hh
function foo() {
  for($x;$y;$z) {
    return $a;
  }
  for($x;$y;$z) {}
  for(;$y;$z) {}
  for($x;;$z) {}
  for($x;$y;) {}
  for($x;;) {}
  for(;$y;) {}
  for(;;$z) {}
  for($x, $y;;) {}
  for(;$x, $y, $z;) {}
}
