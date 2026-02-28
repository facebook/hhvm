<?hh


<<__EntryPoint>>
function main_preg_has_jit() :mixed{
var_dump(ini_get('hhvm.pcre.jit'));
}
