<?php
// Copyright 2004-present Facebook. All Rights Reserved.

// MockClass attribute lets us extend final classes
<<__MockClass>>
class YoloPair extends Pair {
  public function __construct() {
    // override Pair's ctor that just throws
  }
  public function __clone() {
    echo "overridden clone\n";
  }
}

<<__MockClass>>
class YoloVector extends Vector {
  // no overridden clone
}

$p = new YoloPair();
$p2 = clone $p;
var_dump($p);
var_dump($p2);
var_dump((array)$p);

$v = new YoloVector();
$v2 = clone $v;
$v->append(123);
var_dump($v);
var_dump($v2);
var_dump((array)$v);
