<?php

class :foo {
  public function __toString() {
    return "foo";
  }
}

$a = null;
$b = $a ?? <foo />;
var_dump((string) $b);
