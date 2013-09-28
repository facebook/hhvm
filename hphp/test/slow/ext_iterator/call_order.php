<?php

class A extends RecursiveDirectoryIterator {
  function __construct($a) {
    echo __FUNCTION__."\n";
    return parent::__construct($a);
  }
  function current() {
    echo __FUNCTION__."\n";
    return parent::current();
  }
  function next() {
    echo __FUNCTION__."\n";
    return parent::next();
  }
  function rewind() {
    echo __FUNCTION__."\n";
    return parent::rewind();
  }
}

$a = new A(__DIR__.'/../../sample_dir/');
echo "done construct\n";
foreach ($a as $filename => $cur) {
}
