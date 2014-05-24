#!/bin/env php
<?php

trait bar {
  public function call() {
    var_dump(self::class);
    var_dump(__CLASS__);
    var_dump(array(__CLASS__ => __CLASS__));
  }
}

var_dump(__CLASS__);
function a() {
  var_dump(__CLASS__);
}
a();

class foo { use bar; }
class baz extends foo {}
(new foo)->call();
(new baz)->call();
