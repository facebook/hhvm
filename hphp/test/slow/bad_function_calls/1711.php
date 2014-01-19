<?php

error_reporting(E_ALL & ~E_NOTICE);
function foo($a) {
 print $a;
}
 function bar($a) {
 return $a;
}
 foo('ok', bar('bad'));
