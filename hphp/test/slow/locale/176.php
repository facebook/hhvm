<?hh


<<__EntryPoint>>
function main_176() {
$a = varray["\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf"];
asort(inout $a);
var_dump($a);

$a = varray["\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf"];
asort(inout $a, SORT_LOCALE_STRING);
var_dump($a);

$a = varray["\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf"];
var_dump(setlocale(LC_ALL, 'zh_CN.utf8'));
asort(inout $a);
var_dump($a);

$a = varray["\xe8\xaf\xb6", "\xe6\xaf\x94", "\xe8\xa5\xbf"];
var_dump(setlocale(LC_ALL, 'zh_CN.utf8'));
asort(inout $a, SORT_LOCALE_STRING);
var_dump($a);
}
