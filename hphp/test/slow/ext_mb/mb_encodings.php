<?hh



<<__EntryPoint>>
function main_mb_encodings() :mixed{
var_dump(array_search("UTF-8", mb_list_encodings()) !== false);
var_dump(mb_list_encodings_alias_names()['Quoted-Printable']);
var_dump(mb_list_encodings_alias_names('Quoted-Printable'));
var_dump(mb_list_mime_names()['UUENCODE']);
var_dump(mb_list_mime_names('UUENCODE'));
var_dump(mb_check_encoding());
var_dump(mb_check_encoding("Pr\xC3\x9C\xC3\x9D"."fung"));
var_dump(mb_check_encoding("Pr\xC3\x9C\xC3\x9D"."fung", "UTF-8"));

$str = "Pr\xC3\x9C"."fung";
var_dump(mb_convert_encoding($str, "ISO-8859-1", "UTF-8"));
var_dump(mb_convert_encoding($str, "ISO-8859-1", "UTF-8, JIS"));
var_dump(mb_convert_encoding($str, "ISO-8859-1", "auto"));

var_dump(mb_convert_kana("foo"));
var_dump(mb_convert_kana("foo", "a"));

$str = "Pr\xC3\x9C\xC3\x9D"."fung";

/* Detect character encoding with current detect_order */
var_dump(mb_detect_encoding($str));

/* "auto" is expanded to "ASCII,JIS,UTF-8,EUC-JP,SJIS" */
var_dump(mb_detect_encoding($str, "auto"));

/* Specify encoding_list character encoding by comma separated list */
var_dump(mb_detect_encoding($str, "JIS, eucjp-win, sjis-win"));

/* Use array to specify encoding_list  */
$ary = vec["ASCII", "JIS", "EUC-JP"];
var_dump(mb_detect_encoding($str, $ary));
}
