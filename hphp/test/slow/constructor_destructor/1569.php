<?php

class parent_c {
  public function __construct() {
    echo "parent__construct";
  }
  public function __destruct() {
    echo "parent__destruct";
  }
}
class child_c extends parent_c {
  public function __construct() {
    echo "child__construct";
    parent::__construct();
  }
  public function __destruct() {
    echo "child__destruct";
    parent::__destruct();
  }
}
$v = new child_c;
unset($v);
