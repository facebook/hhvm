<?php

function err() { echo "yep\n"; }
class dtor { private $i; function __construct($i) { $this->i = $i; }
                         function __destruct()    { echo "dtor $this->i\n"; } }

class A {
  public $z = array(1,2,3);
}
function x($a) {
  var_dump($a->z[new dtor(1)] = new dtor(5));
}

<<__EntryPoint>>
function main_set_bad_key_dtor() {
set_error_handler('err');
x(new A);
echo "done\n";
}
