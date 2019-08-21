<?hh

function handler($code, $msg) {
   var_dump(strpos($msg, 'system error') !== false);
 return true;
}


 <<__EntryPoint>>
function main_1367() {
set_error_handler(fun('handler'));
user_error('system error');
}
