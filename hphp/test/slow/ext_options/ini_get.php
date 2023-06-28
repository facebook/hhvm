<?hh

function ini_get_wrapper( $setting ) :mixed{
    return ini_get( $setting );
}


<<__EntryPoint>>
function main_ini_get() :mixed{
var_dump(ini_get_wrapper('some_non_existent_setting'));
var_dump(ini_get_wrapper('some_non_existent_setting'));
}
