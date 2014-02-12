<?php
class Test extends \SplObjectStorage {
  function __construct() {
    $o1 = new StdClass;
    $o2 = new StdClass;
    $o3 = new StdClass;
    $this->attach($o1);
    $this->attach($o2);
    $this->attach($o3);

    foreach($this as $key => $val) {
      print($key);
    }
  }
}
new Test();
