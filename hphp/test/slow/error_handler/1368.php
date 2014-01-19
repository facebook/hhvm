<?php

function handler($e) {
   var_dump(strpos((string)$e, 'bomb') !== false);
 return true;
}
 set_exception_handler('handler');
function a() {
}
 set_exception_handler('a');
restore_exception_handler();
throw new Exception('bomb');

