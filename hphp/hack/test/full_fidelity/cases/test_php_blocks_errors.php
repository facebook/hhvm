<?php
function foo() {
  $a = 1;
}
?>

<?hh
class foo {
  public function bar(): int {
    return 5;
  }
}
