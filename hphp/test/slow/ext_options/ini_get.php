<?hh

function ini_get_wrapper( $setting ) {
    return ini_get( $setting );
}


<<__EntryPoint>>
function main_ini_get() {
var_dump(ini_get_wrapper('some_non_existent_setting'));
var_dump(ini_get_wrapper('some_non_existent_setting'));
}
