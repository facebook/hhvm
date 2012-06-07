<?php

class klass implements Countable {
  public function count() { return 123; }
}

function main() {
  $k = new klass;
  var_dump(count($k));
}
main();

