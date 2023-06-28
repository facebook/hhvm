<?hh

<<__EntryPoint>>
function main_ext_version() :mixed{
$v = phpversion('imagick');
var_dump(version_compare($v, '3.1.0'));
var_dump(version_compare($v, '3.2.0'));
}
