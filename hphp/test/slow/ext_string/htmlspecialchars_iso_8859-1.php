<?hh


<<__EntryPoint>>
function main_htmlspecialchars_iso_8859_1() {
$s = chr(0xAE);
echo 'Original: '.bin2hex($s)."\n";

$s1 = htmlspecialchars($s, ENT_QUOTES | ENT_IGNORE, 'ISO-8859-1');
echo 'htmlspecialchars: '.bin2hex($s1)."\n";
}
