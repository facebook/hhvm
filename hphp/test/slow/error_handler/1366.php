<?hh

function handler($code, $msg) {
   var_dump(strpos($msg, 'system error') !== false);
 return true;
}
function a() {
}


 <<__EntryPoint>>
function main_1366() {
set_error_handler(fun('handler'));
 set_error_handler(fun('a'));
restore_error_handler();
trigger_error('system error');
}
