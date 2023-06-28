<?hh


<<__EntryPoint>>
function main_mb_convert_case_with_encoding() :mixed{
$str = "\xEC\xE0ry had a Little lamb and she loved it so";
$str = mb_convert_case($str, MB_CASE_UPPER, "CP1251");
var_dump(urlencode($str));
$str = mb_convert_case($str, MB_CASE_TITLE, "CP1251");
var_dump(urlencode($str));
}
