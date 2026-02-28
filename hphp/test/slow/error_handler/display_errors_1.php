<?hh


<<__EntryPoint>>
function main_display_errors_1() :mixed{
ini_set('display_errors', '1');
var_dump(ini_get('display_errors'));
fclose(HH\stderr());
trigger_error('Should see', E_USER_NOTICE);

// stderr, which we just closed - shouldn't be rendered
ini_set('display_errors', '2');
trigger_error('Should not see', E_USER_NOTICE);
}
