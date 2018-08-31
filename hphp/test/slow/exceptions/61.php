<?php

class X {
  static function eh($errno, $errstr) {
    echo "eh: $errno\n";
    die;
  }
}

<<__EntryPoint>>
function main_61() {
;
set_error_handler(array('X', 'eh'));
$g = array();
echo $g['foobar'];
}
