<?hh

<<__EntryPoint>>
function main_ignore_ok() :mixed{
$text = "This is the Euro symbol '\xE2\x82\xAC'.";
var_dump(iconv("UTF-8", "ISO-8859-1//IGNORE", $text));
}
