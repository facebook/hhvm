<?php

function handler($code, $msg) {
   var_dump(strpos($msg, 'system error') !== false);
 return true;
}
 set_error_handler('handler');
function a() {
}
 set_error_handler('a');
restore_error_handler();
trigger_error('system error');

