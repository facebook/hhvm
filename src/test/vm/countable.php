<?php

class klass implements Countable {
  public function count() { return 123; }
}

$k = new klass;
var_dump(count($k));
