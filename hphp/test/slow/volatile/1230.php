<?hh

function f($cls) {
}

<<__EntryPoint>>
function main_1230() {
var_dump(spl_autoload_functions());
spl_autoload_register('f');
var_dump(spl_autoload_functions());
spl_autoload_unregister('f');
var_dump(spl_autoload_functions());
spl_autoload_unregister('spl_autoload_call');
var_dump(spl_autoload_functions());
spl_autoload_register('f');
var_dump(spl_autoload_functions());
spl_autoload_unregister('spl_autoload_call');
var_dump(spl_autoload_functions());
}
