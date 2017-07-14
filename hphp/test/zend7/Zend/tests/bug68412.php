<?php
class C {
  public function __call($x, $y) {
    global $z;
    $z->bar();
  }
}
$z = new C;
function main() {
  global $z;
  $z->foo();
}
main();
?>
