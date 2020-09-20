#!/bin/env php
<?hh

trait bar {
  public function call() {
    var_dump(self::class);
    var_dump(__CLASS__);
    var_dump(darray[__CLASS__ => __CLASS__]);
  }
}
function a() {
  var_dump(__CLASS__);
}

class foo { use bar; }
class baz extends foo {}


<<__EntryPoint>>
function main_class() {
var_dump(__CLASS__);
a();
(new foo)->call();
(new baz)->call();
}
