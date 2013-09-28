<?php

function handler($code, $msg) {
   var_dump(strpos($msg, 'system error') !== false);
 return true;
}
 set_error_handler('handler');
user_error('system error');

