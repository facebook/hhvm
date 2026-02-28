<?hh

function handler($code, $msg) :mixed{
   var_dump(strpos($msg, 'system error') !== false);
 return true;
}
function a() :mixed{
}


 <<__EntryPoint>>
function main_1366() :mixed{
set_error_handler(handler<>);
 set_error_handler(a<>);
restore_error_handler();
trigger_error('system error');
}
