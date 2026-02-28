<?hh


<<__EntryPoint>>
function main_mb_convert_variables() :mixed{
$str  = "Pr\xC3\x9C"."fung";
$str1 = "Pr\xC3\x9C"."fung";
$str2 = "Pr\xC3\x9C"."fung";
$inputenc = mb_convert_variables("ISO-8859-1", "UTF-8", inout $str,
                                 $str1, $str2);
var_dump($str);
var_dump($str1);
var_dump($str2);
}
