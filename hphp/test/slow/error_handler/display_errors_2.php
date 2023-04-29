<?hh


<<__EntryPoint>>
function main_display_errors_2() {
ini_set('display_errors', '2');
var_dump(ini_get('display_errors'));
fclose(HH\stdout());
trigger_error('Should see', E_USER_NOTICE);

// stdout, which we just closed - shouldn't be rendered
ini_set('display_errors', '1');
trigger_error('Should not see', E_USER_NOTICE);
}
