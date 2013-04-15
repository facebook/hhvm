<?php
abstract class :x:base {
  abstract public function __construct();
}

abstract class :x:composable-element extends :x:base {
  final public function __construct($attributes,
                                    $children,
                                    $file = null,
                                    $line = null) {
  }
}
echo "PASS with hphpCompat\n";
