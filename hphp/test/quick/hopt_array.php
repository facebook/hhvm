<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

echo "Starting\n";

class C {
  public function foo() {
    echo "foo\n";
    $x = array();
    if ($x) {
      print_r($x);
    } else {
      print_r($x);
    }
  }
}

$c = new C;
$val = $c->foo();
echo $val;
echo "\n";
echo "Done\n";
