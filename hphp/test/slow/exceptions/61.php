<?php

class X {
  static function eh($errno, $errstr) {
    echo "eh: $errno\n";
    die;
  }
}
;
set_error_handler(array('X', 'eh'));
$g = array();
echo $g['foobar'];
