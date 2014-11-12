<?php
class C {
  public function test() {
    list($this) = array(null);
  }
}
function main() {
  $c = new C();
  $c->test();
  echo "Done\n";
}
main();
