<?hh

<<__EntryPoint>>
function main_subdir2() :mixed{
var_dump(ini_get('hhvm.deployment_id'));
var_dump(ini_get('hhvm.custom_settings'));
var_dump(ini_get('hhvm.relative_configs'));
}
