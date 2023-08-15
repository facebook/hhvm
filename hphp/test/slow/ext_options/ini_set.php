<?hh


<<__EntryPoint>>
function main_ini_set() :mixed{
// Constant
var_dump(ini_set('hphp.compiler_id', 'foo'));
var_dump(ini_get('hphp.compiler_id'));

// Config
var_dump(ini_set('file_uploads', '0'));
var_dump(ini_get('file_uploads'));

var_dump(ini_set('expose_php', '0'));
var_dump(ini_get('expose_php'));

// Request
var_dump(ini_set('always_populate_raw_post_data', '1'));
var_dump(ini_get('always_populate_raw_post_data'));

// ini_set with values expecting numbers but given an empty string
var_dump(ini_set('error_reporting', ''));
var_dump(ini_get('error_reporting'));
}
